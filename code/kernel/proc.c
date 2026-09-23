#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "vm.h"
#include "proc.h"

extern void printf(const char *fmt, ...);
extern uchar _uprog_table[];
extern char kernel_end[];
extern void trampoline(void);

struct proc *current_proc;
static struct proc ptable[NPROC];
/* satp=0 means every process uses one physical activity slot.  fork keeps
 * the parent's image in this rollback buffer until the child exits. */
static uchar usermem[PROC_MEM_SIZE]
    __attribute__((aligned(PGSIZE), section(".userimage")));
static uchar fork_backup[PROC_MEM_SIZE]
    __attribute__((aligned(PGSIZE), section(".userimage")));
static int next_pid = 2;

/* A compact SV39 address space is enough for this pre-filesystem stage. */
static uint64 user_root[512] __attribute__((aligned(PGSIZE)));
static uint64 low_level1[512] __attribute__((aligned(PGSIZE)));
static uint64 user_level0[512] __attribute__((aligned(PGSIZE)));
static uint64 high_level1[512] __attribute__((aligned(PGSIZE)));
static uint64 high_level0[512] __attribute__((aligned(PGSIZE)));
static uint64 active_satp;

static void
page_zero(uint64 *page)
{
  for (uint i = 0; i < 512; i++)
    page[i] = 0;
}

static void
build_pagetable(void)
{
  page_zero(user_root);
  page_zero(low_level1);
  page_zero(user_level0);
  page_zero(high_level1);
  page_zero(high_level0);

  /* Identity-map the kernel image and its BSS after kernel_end. */
  user_root[PX(2, KERNBASE)] = PA2PTE(KERNBASE) | PTE_V | PTE_R | PTE_W | PTE_X;

  /* Map user virtual addresses [0, PROC_MEM_SIZE) to the active image. */
  user_root[0] = PA2PTE(low_level1) | PTE_V;
  low_level1[0] = PA2PTE(user_level0) | PTE_V;
  for (uint i = 0; i < PROC_MEM_SIZE / PGSIZE; i++)
    user_level0[i] = PA2PTE(usermem + i * PGSIZE) | PTE_V | PTE_R | PTE_W | PTE_X | PTE_U;

  /* Keep each MMIO window reachable while the kernel runs on this table.
   * These are distinct level-1 indices, so 2 MiB leaves are sufficient. */
  low_level1[PX(1, CLINT_BASE)] = PA2PTE(CLINT_BASE) | PTE_V | PTE_R | PTE_W;
  low_level1[PX(1, PLIC)] = PA2PTE(PLIC) | PTE_V | PTE_R | PTE_W;
  /* PLIC supervisor enable/claim registers live in the next 2 MiB window. */
  low_level1[PX(1, PLIC + 0x200000)] =
      PA2PTE(PLIC + 0x200000) | PTE_V | PTE_R | PTE_W;
  low_level1[PX(1, UART0)] = PA2PTE(UART0) | PTE_V | PTE_R | PTE_W;

  /* The gift trampoline ABI requires fixed high virtual addresses. */
  user_root[PX(2, TRAMPOLINE)] = PA2PTE(high_level1) | PTE_V;
  high_level1[PX(1, TRAMPOLINE)] = PA2PTE(high_level0) | PTE_V;
  high_level0[PX(0, TRAMPOLINE)] =
      PA2PTE(PGROUNDDOWN((uint64)trampoline)) | PTE_V | PTE_R | PTE_X;

  active_satp = MAKE_SATP(user_root);
}

uint64
proc_satp(void)
{
  return active_satp;
}

void
proc_map_trapframe(void)
{
  if (current_proc == 0)
    return;
  /* Every struct proc trapframe is page-aligned by proc.h. */
  high_level0[PX(0, TRAPFRAME)] =
      PA2PTE(&current_proc->tf) | PTE_V | PTE_R | PTE_W;
  sfence_vma();
}

static void
kmemset(void *dst, int value, uint n)
{
  uchar *p = (uchar *)dst;
  for (uint i = 0; i < n; i++)
    p[i] = (uchar)value;
}

static void
kmemcpy(void *dst, const void *src, uint n)
{
  uchar *d = (uchar *)dst;
  const uchar *s = (const uchar *)src;
  for (uint i = 0; i < n; i++)
    d[i] = s[i];
}

static uint
kstrlen(const char *s)
{
  uint n = 0;
  while (s[n])
    n++;
  return n;
}

static int
kstrcmp(const char *a, const char *b)
{
  while (*a && *a == *b) {
    a++;
    b++;
  }
  return (uchar)*a - (uchar)*b;
}

static void
copy_name(char *dst, const char *src)
{
  uint i = 0;
  while (i + 1 < 16 && src[i] && src[i] != '\n' && src[i] != '\r') {
    dst[i] = src[i];
    i++;
  }
  dst[i] = 0;
}

static int
lookup_program(const char *name, const uchar **start, const uchar **end)
{
  uchar *cursor = _uprog_table;
  while (1) {
    uint64 begin = *(uint64 *)(cursor + 0);
    uint64 finish = *(uint64 *)(cursor + 8);
    if (begin == 0 && finish == 0)
      return -1;
    const char *entry_name = (const char *)(cursor + 16);
    if (kstrcmp(entry_name, name) == 0) {
      *start = (const uchar *)begin;
      *end = (const uchar *)finish;
      return 0;
    }
    uint step = kstrlen(entry_name) + 1;
    cursor += (step + 16 + 7) & ~7U;
  }
}

static void
reset_context(struct proc *p)
{
  kmemset(&p->tf, 0, sizeof(p->tf));
  p->tf.kernel_satp = active_satp;
  p->tf.kernel_sp = (uint64)(p->kstack + KSTACK_SIZE);
  p->tf.kernel_trap = (uint64)usertrap;
  p->tf.kernel_hartid = 0;
  p->tf.epc = 0;
  p->tf.sp = PROC_MEM_SIZE - 16;
}

void
procinit(void)
{
  for (int i = 0; i < NPROC; i++) {
    ptable[i].state = UNUSED;
    ptable[i].mem = usermem;
    ptable[i].userbase = 0;
  }
  struct proc *p = &ptable[0];
  p->pid = 1;
  p->state = RUNNING;
  p->mem = usermem;
  p->userbase = 0;
  current_proc = p;
  build_pagetable();
  proc_map_trapframe();
  if ((uint64)usermem < (uint64)kernel_end) {
    printf("lab2: user image overlaps kernel\n");
    for (;;)
      asm volatile("wfi");
  }
  if (proc_exec("sh") < 0) {
    printf("lab2: embedded sh missing\n");
    for (;;)
      asm volatile("wfi");
  }
}

int
user_range(const void *addr, uint n)
{
  if (current_proc == 0)
    return 0;
  uint64 a = (uint64)addr;
  uint64 lo = current_proc->userbase;
  uint64 hi = lo + PROC_MEM_SIZE;
  return a >= lo && a <= hi && (uint64)n <= hi - a;
}

int
proc_exec(const char *name)
{
  char requested[16];
  copy_name(requested, name);
  const uchar *start, *end;
  if (lookup_program(requested, &start, &end) < 0)
    return -1;
  uint64 length64 = (uint64)(end - start);
  if (length64 == 0 || length64 > PROC_MEM_SIZE - 64)
    return -1;

  struct proc *p = current_proc;
  uint length = (uint)length64;
  kmemset(p->mem, 0, PROC_MEM_SIZE);
  kmemcpy(p->mem, start, length);
  p->memsz = length;
  p->brk = (length + 15) & ~15U;
  copy_name(p->name, requested);
  reset_context(p);
  return 0;
}

int
proc_fork(void)
{
  struct proc *parent = current_proc;
  struct proc *child = 0;
  for (int i = 1; i < NPROC; i++) {
    if (ptable[i].state == UNUSED) {
      child = &ptable[i];
      break;
    }
  }
  if (child == 0)
    return -1;

  child->pid = next_pid++;
  child->state = RUNNING;
  child->parent = parent;
  child->exit_status = 0;
  child->mem = parent->mem;
  child->userbase = parent->userbase;
  child->memsz = parent->memsz;
  child->brk = parent->brk;
  kmemcpy(fork_backup, parent->mem, PROC_MEM_SIZE);
  kmemcpy(&child->tf, &parent->tf, sizeof(child->tf));
  child->tf.kernel_sp = (uint64)(child->kstack + KSTACK_SIZE);
  child->tf.kernel_trap = (uint64)usertrap;
  child->tf.kernel_hartid = 0;
  child->tf.sp = parent->tf.sp;
  child->tf.epc = parent->tf.epc;
  child->tf.a0 = 0;
  copy_name(child->name, parent->name);

  parent->tf.a0 = child->pid;
  parent->state = WAITING;
  current_proc = child;
  return child->pid;
}

int
proc_wait(int *status)
{
  struct proc *parent = current_proc;
  for (int i = 1; i < NPROC; i++) {
    struct proc *p = &ptable[i];
    if (p->parent != parent || p->state != ZOMBIE)
      continue;
    if (status != 0) {
      if (!user_range(status, sizeof(int)))
        return -1;
      *(int *)status = p->exit_status;
    }
    int pid = p->pid;
    p->state = UNUSED;
    p->parent = 0;
    p->pid = 0;
    return pid;
  }
  return -1;
}

void
proc_exit(int status)
{
  struct proc *p = current_proc;
  if (p == 0 || p->parent == 0)
    for (;;)
      asm volatile("wfi");
  p->exit_status = status;
  kmemcpy(p->parent->mem, fork_backup, PROC_MEM_SIZE);
  p->state = ZOMBIE;
  p->parent->state = RUNNABLE;
  current_proc = p->parent;
  usertrapret();
}

int
proc_getpid(void)
{
  return current_proc ? current_proc->pid : -1;
}

void
proc_start(void)
{
  usertrapret();
}
