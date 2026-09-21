#include "types.h"
#include "riscv.h"
#include "proc.h"

extern void printf(const char *fmt, ...);
extern uchar _uprog_table[];

struct proc *current_proc;
static struct proc ptable[NPROC];
/* satp=0 means every process uses one physical activity slot.  fork keeps
 * the parent's image in this rollback buffer until the child exits. */
static uchar usermem[PROC_MEM_SIZE] __attribute__((aligned(16)));
static uchar fork_backup[PROC_MEM_SIZE] __attribute__((aligned(16)));
static int next_pid = 2;

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
  p->tf.kernel_satp = 0;
  p->tf.kernel_sp = (uint64)(p->kstack + KSTACK_SIZE);
  p->tf.kernel_trap = (uint64)usertrap;
  p->tf.kernel_hartid = 0;
  p->tf.epc = p->userbase;
  p->tf.sp = p->userbase + PROC_MEM_SIZE - 16;
}

void
procinit(void)
{
  for (int i = 0; i < NPROC; i++) {
    ptable[i].state = UNUSED;
    ptable[i].mem = usermem;
    ptable[i].userbase = (uint64)usermem;
  }
  struct proc *p = &ptable[0];
  p->pid = 1;
  p->state = RUNNING;
  p->mem = usermem;
  p->userbase = (uint64)usermem;
  current_proc = p;
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
  child->tf.sp = child->userbase + (parent->tf.sp - parent->userbase);
  child->tf.epc = child->userbase + (parent->tf.epc - parent->userbase);
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
  usertrap_return();
}

int
proc_getpid(void)
{
  return current_proc ? current_proc->pid : -1;
}

void
proc_start(void)
{
  usertrap_return();
}
