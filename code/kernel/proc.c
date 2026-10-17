#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "course_sid.h"
#include "vm.h"
#include "proc.h"

extern void printf(const char *fmt, ...);
extern uchar _uprog_table[];
extern uint64 kvmsatp(void);
extern void trampoline(void);

struct proc *current_proc;
static struct proc ptable[NPROC] __attribute__((aligned(4096)));
static int next_pid = 2;

static uint kstrlen(const char *s) { uint n = 0; while (s[n]) n++; return n; }
static int kstrcmp(const char *a, const char *b)
{
  while (*a && *a == *b) { a++; b++; }
  return (uchar)*a - (uchar)*b;
}
static void copy_name(char *dst, const char *src)
{
  uint i = 0;
  while (i + 1 < 16 && src[i] && src[i] != '\n' && src[i] != '\r')
    dst[i] = src[i], i++;
  dst[i] = 0;
}
static int lookup_program(const char *name, const uchar **start,
                          const uchar **file_end, const uchar **mem_end,
                          const uchar **code_end)
{
  uchar *cursor = _uprog_table;
  while (1) {
    uint64 begin = *(uint64 *)(cursor + 0);
    uint64 finish = *(uint64 *)(cursor + 8);
    uint64 mem_end_va = *(uint64 *)(cursor + 16);
    uint64 code_end_abs = *(uint64 *)(cursor + 24);
    if (begin == 0 && finish == 0) return -1;
    const char *entry_name = (const char *)(cursor + 32);
    if (kstrcmp(entry_name, name) == 0) {
      *start = (const uchar *)begin;
      *file_end = (const uchar *)finish;
      *mem_end = (const uchar *)mem_end_va;
      *code_end = (const uchar *)code_end_abs;
      return 0;
    }
    cursor += (32 + kstrlen(entry_name) + 1 + 7) & ~7U;
  }
}
static void reset_context(struct proc *p)
{
  for (uint i = 0; i < sizeof(p->tf); i++) ((uchar *)&p->tf)[i] = 0;
  p->tf.kernel_satp = kvmsatp();
  p->tf.kernel_sp = (uint64)(p->kstack + KSTACK_SIZE);
  p->tf.kernel_trap = (uint64)usertrap;
  p->tf.kernel_hartid = 0;
  p->tf.epc = 0;
  p->tf.sp = p->stack_top - 16;
}
static int map_trapframe(pagetable_t pt, struct proc *p)
{
  extern pte_t *vm_walk(pagetable_t, uint64, int);
  pte_t *pte = vm_walk(pt, TRAPFRAME, 1);
  if (!pte) return -1;
  *pte = PA2PTE(&p->tf) | PTE_V | PTE_R | PTE_W;
  return 0;
}
static int load_program(struct proc *p, const char *name, pagetable_t *outpt,
                        uint *outmemsz, uint *outbrk, uint64 *outstack_top)
{
  const uchar *start, *file_end, *mem_end_ptr, *code_end_ptr;
  if (lookup_program(name, &start, &file_end, &mem_end_ptr, &code_end_ptr) < 0) return -1;
  uint64 file_length = (uint64)(file_end - start);
  uint64 mem_length = (uint64)(mem_end_ptr - start);
  uint64 code_end = (uint64)(code_end_ptr - start);
  if (file_length == 0 || mem_length < file_length || mem_length > 64 * 1024 || code_end > file_length) return -1;
  pagetable_t pt = uvmcreate(p);
  if (!pt) { printf("lab3: uvmcreate fail free=%d\n", kfreecount()); return -1; }
  uint64 data_end = PGROUNDUP(mem_length);
  uint64 stack_start = data_end + (uint64)LAB3_GUARD_PAGES * PGSIZE;
  uint64 stack_top = stack_start + (uint64)USERSTACK * PGSIZE;
  if (stack_top >= TRAPFRAME) { uvmfree(pt, 0); return -1; }
  for (uint64 va = 0; va < data_end; va += PGSIZE) {
    void *mem = kalloc();
    if (!mem) goto bad;
    uint64 remain = file_length > va ? file_length - va : 0;
    uint n = remain > PGSIZE ? PGSIZE : (uint)remain;
    for (uint i = 0; i < PGSIZE; i++) ((uchar *)mem)[i] = i < n ? start[va + i] : 0;
    int perm = va < code_end ? PTE_R | PTE_X : PTE_R | PTE_W;
    if (uvmmap(pt, va, (uint64)mem, PGSIZE, perm) < 0) { printf("lab3: uvmmap fail va=%lx\n", va); kfree(mem); goto bad; }
  }
  void *stack = kalloc();
  if (!stack || uvmmap(pt, stack_start, (uint64)stack, PGSIZE, PTE_R | PTE_W) < 0) {
    printf("lab3: stack map fail stack=%lx\n", stack_start);
    if (stack) kfree(stack);
    goto bad;
  }
  if (map_trapframe(pt, p) < 0) { printf("lab3: trapframe map fail\n"); goto bad; }
  *outpt = pt; *outmemsz = (uint)data_end; *outbrk = (uint)stack_top;
  *outstack_top = stack_top;
  return 0;
bad:
  uvmfree(pt, 0);
  return -1;
}
void procinit(void)
{
  for (int i = 0; i < NPROC; i++) { ptable[i].state = UNUSED; ptable[i].pagetable = 0; }
  struct proc *p = &ptable[0]; p->pid = 1; p->state = RUNNING; p->parent = 0;
  current_proc = p;
  if (proc_exec("sh") < 0) { printf("lab3: embedded sh missing or memory exhausted\n"); for (;;) asm volatile("wfi"); }
}
void proc_map_trapframe(void)
{
  if (!current_proc || !current_proc->pagetable) return;
  map_trapframe(current_proc->pagetable, current_proc); sfence_vma();
}
uint64 proc_satp(void) { return current_proc ? MAKE_SATP(current_proc->pagetable) : 0; }
int user_range(const void *addr, uint n)
{
  uint64 a = (uint64)addr;
  return current_proc && current_proc->pagetable &&
         (n == 0 ? a < TRAPFRAME : a < TRAPFRAME && (uint64)n <= TRAPFRAME - a);
}
int proc_exec(const char *name)
{
  struct proc *p = current_proc; pagetable_t newpt; uint newmemsz, newbrk; uint64 newstack_top;
  if (load_program(p, name, &newpt, &newmemsz, &newbrk, &newstack_top) < 0) return -1;
  pagetable_t oldpt = p->pagetable; p->pagetable = newpt; p->memsz = newmemsz;
  p->brk = newbrk; p->sz = newbrk; p->stack_top = newstack_top; copy_name(p->name, name);
  reset_context(p); if (oldpt) uvmfree(oldpt, 0); return 0;
}
int proc_fork(void)
{
  struct proc *parent = current_proc, *child = 0;
  for (int i = 1; i < NPROC; i++) if (ptable[i].state == UNUSED) { child = &ptable[i]; break; }
  if (!child) return -1;
  child->pagetable = uvmcreate(child); if (!child->pagetable) return -1;
  if (uvmcopy(parent->pagetable, child->pagetable, parent->stack_top) < 0) {
    uvmfree(child->pagetable, 0); child->pagetable = 0; return -1;
  }
  child->pid = next_pid++; child->state = RUNNING; child->parent = parent; child->exit_status = 0;
  child->memsz = parent->memsz; child->brk = parent->brk; child->sz = parent->sz;
  child->stack_top = parent->stack_top; child->tf = parent->tf; child->tf.kernel_satp = kvmsatp();
  child->tf.kernel_sp = (uint64)(child->kstack + KSTACK_SIZE); child->tf.kernel_trap = (uint64)usertrap;
  child->tf.a0 = 0; copy_name(child->name, parent->name); map_trapframe(child->pagetable, child);
  parent->tf.a0 = child->pid; parent->state = WAITING; current_proc = child; return child->pid;
}
int proc_wait(uint64 status_addr)
{
  struct proc *parent = current_proc;
  for (int i = 1; i < NPROC; i++) {
    struct proc *p = &ptable[i]; if (p->parent != parent || p->state != ZOMBIE) continue;
    if (status_addr && copyout(parent->pagetable, status_addr, &p->exit_status, sizeof(p->exit_status)) < 0) return -1;
    int pid = p->pid; uvmfree(p->pagetable, p->stack_top); p->pagetable = 0;
    p->state = UNUSED; p->parent = 0; p->pid = 0; return pid;
  }
  return -1;
}
void proc_exit(int status)
{
  struct proc *p = current_proc; if (!p || !p->parent) for (;;) asm volatile("wfi");
  p->exit_status = status; p->state = ZOMBIE; p->parent->state = RUNNABLE; current_proc = p->parent; usertrapret();
}
int proc_getpid(void) { return current_proc ? current_proc->pid : -1; }
void proc_start(void) { dump_pagetable(current_proc->pagetable); usertrapret(); }
