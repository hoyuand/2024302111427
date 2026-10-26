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
static struct spinlock wait_lock;
static struct spinlock ticks_lock;
static uint64 ticks;

struct cpu_state {
  struct context scheduler;
  struct proc *proc;
  int noff;
  int intena;
};
static struct cpu_state cpus[NCPU];

extern void swtch(struct context *, struct context *);

void push_off(void)
{
  int was = intr_get();
  intr_off();
  if (cpus[0].noff++ == 0)
    cpus[0].intena = was;
}

void pop_off(void)
{
  if (cpus[0].noff <= 0) return;
  cpus[0].noff--;
  if (cpus[0].noff == 0 && cpus[0].intena) intr_on();
}

void acquire(spinlock_t *lk)
{
  push_off();
  while (__sync_lock_test_and_set(&lk->locked, 1)) asm volatile("nop");
  asm volatile("fence rw,rw" ::: "memory");
}

void release(spinlock_t *lk)
{
  asm volatile("fence rw,rw" ::: "memory");
  __sync_lock_release(&lk->locked);
  pop_off();
}

int holding(spinlock_t *lk) { return lk->locked != 0; }

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
static int default_tickets(const char *name)
{
  if (kstrcmp(name, "hi") == 0) return 1;
  if (kstrcmp(name, "spin") == 0) return 2;
  if (kstrcmp(name, "sh") == 0) return 4;
  return 7 + (COURSE_SID % 13);
}

static void align_pass(struct proc *p)
{
  uint64 min = (uint64)-1;
  for (int i = 0; i < NPROC; i++) {
    struct proc *q = &ptable[i];
    if (q == p || q->state == UNUSED || q->state == ZOMBIE) continue;
    if (q->pass < min) min = q->pass;
  }
  if (min != (uint64)-1 && p->pass < min) p->pass = min;
}

static void forkret(void)
{
  struct proc *p = current_proc;
    /* scheduler held p->lock across swtch for this first activation. */
  release(&p->lock);
  usertrapret();
}

static void sched(void)
{
  struct proc *p = current_proc;
  if (!p || !holding(&p->lock) || cpus[0].noff != 1 ||
      p->state == RUNNING || intr_get()) {
    printf("lab4: sched invariant violated noff=%d\n", cpus[0].noff);
    for (;;) asm volatile("wfi");
  }
  int intena = cpus[0].intena;
  swtch(&p->context, &cpus[0].scheduler);
  cpus[0].intena = intena;
}

static void scheduler(void) __attribute__((noreturn));
static void scheduler(void)
{
  struct cpu_state *c = &cpus[0];
  c->proc = 0;
  for (;;) {
    intr_on();
    struct proc *best = 0;
    for (int i = 0; i < NPROC; i++) {
      struct proc *p = &ptable[i];
      acquire(&p->lock);
      if (p->state == RUNNABLE && (!best || p->pass < best->pass)) {
        if (best) release(&best->lock);
        best = p;
      } else {
        release(&p->lock);
      }
    }
    if (!best) { asm volatile("wfi"); continue; }
    best->state = RUNNING;
    best->last_slice_ticks = 0;
    c->proc = best;
    current_proc = best;
    swtch(&c->scheduler, &best->context);
    c->proc = 0;
    current_proc = 0;
    release(&best->lock);
  }
}

void procinit(void)
{
  wait_lock.locked = 0;
  ticks_lock.locked = 0;
  ticks = 0;
  cpus[0].noff = 0;
  for (int i = 0; i < NPROC; i++) {
    ptable[i].lock.locked = 0; ptable[i].state = UNUSED;
    ptable[i].pagetable = 0; ptable[i].parent = 0;
  }
  struct proc *p = &ptable[0];
  p->pid = 1; p->state = USED; p->parent = 0;
  p->tickets = default_tickets("sh"); p->stride = 1000 / p->tickets; if (!p->stride) p->stride = 1;
  current_proc = p;
  if (proc_exec("sh") < 0) { printf("lab4: embedded sh missing or memory exhausted\n"); for (;;) asm volatile("wfi"); }
  p->state = RUNNABLE;
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
  reset_context(p);
  if (p->context.ra == 0) {
    p->context.ra = (uint64)forkret;
    p->context.sp = (uint64)(p->kstack + KSTACK_SIZE);
  }
  if (!p->tickets_set) {
    p->tickets = default_tickets(name);
    p->stride = 1000 / p->tickets; if (!p->stride) p->stride = 1;
  }
  if (oldpt) uvmfree(oldpt, 0);
  return 0;
}
int proc_fork(void)
{
  struct proc *parent = current_proc, *child = 0;
    for (int i = 1; i < NPROC; i++) if (ptable[i].state == UNUSED) { child = &ptable[i]; child->state = USED; break; }
  if (!child) return -1;
  child->pagetable = uvmcreate(child); if (!child->pagetable) { child->state = UNUSED; return -1; }
  /* brk may be above the fixed stack after sbrk; copy every mapped user
   * page through the current heap boundary.  uvmcopy owns cleanup of its
   * partially-built destination on failure. */
  if (uvmcopy(parent->pagetable, child->pagetable, parent->brk) < 0) {
    child->pagetable = 0; child->state = UNUSED; return -1;
  }
  child->pid = next_pid++; child->state = RUNNABLE; child->parent = parent; child->exit_status = 0;
  child->memsz = parent->memsz; child->brk = parent->brk; child->sz = parent->sz;
  child->stack_top = parent->stack_top; child->tf = parent->tf; child->tf.kernel_satp = kvmsatp();
  child->tf.kernel_sp = (uint64)(child->kstack + KSTACK_SIZE); child->tf.kernel_trap = (uint64)usertrap;
  child->tf.a0 = 0; copy_name(child->name, parent->name); map_trapframe(child->pagetable, child);
  for (uint i = 0; i < sizeof(child->context); i++) ((uchar *)&child->context)[i] = 0;
  child->context.ra = (uint64)forkret;
  child->context.sp = (uint64)(child->kstack + KSTACK_SIZE);
  child->tickets = parent->tickets; child->tickets_set = parent->tickets_set;
  child->stride = parent->stride; child->pass = parent->pass;
  child->cpu_ticks = 0; child->last_slice_ticks = 0; child->killed = 0; child->chan = 0;
  parent->tf.a0 = child->pid; return child->pid;
}
int proc_wait(uint64 status_addr)
{
  struct proc *parent = current_proc;
  acquire(&wait_lock);
  for (;;) {
    int have_child = 0;
    for (int i = 1; i < NPROC; i++) {
      struct proc *p = &ptable[i];
      if (p->parent != parent) continue;
      have_child = 1;
      acquire(&p->lock);
      if (p->state != ZOMBIE) { release(&p->lock); continue; }
      if (status_addr && copyout(parent->pagetable, status_addr, &p->exit_status, sizeof(p->exit_status)) < 0) {
        release(&p->lock); release(&wait_lock); return -1;
      }
      int pid = p->pid;
      uvmfree(p->pagetable, p->stack_top); p->pagetable = 0;
      p->state = UNUSED; p->parent = 0; p->pid = 0; p->chan = 0;
      release(&p->lock); release(&wait_lock); return pid;
    }
    if (!have_child) { release(&wait_lock); return -1; }
    proc_sleep(parent, &wait_lock);
  }
}
void proc_exit(int status)
{
  struct proc *p = current_proc;
  if (!p || !p->parent) for (;;) asm volatile("wfi");
  acquire(&wait_lock);
  /* Reparent orphans to sh; its next wait reaps them. */
  for (int i = 1; i < NPROC; i++)
    if (ptable[i].parent == p) { ptable[i].parent = &ptable[0]; proc_wakeup(&ptable[0]); }
  acquire(&p->lock);
  p->exit_status = status; p->state = ZOMBIE; p->chan = 0;
  /* Publish ZOMBIE before waking the parent.  Keeping wait_lock held makes
   * the state check in wait and this transition one atomic condition. */
  proc_wakeup(p->parent);
  release(&wait_lock);
  sched();
  for (;;) asm volatile("wfi");
}
int proc_getpid(void) { return current_proc ? current_proc->pid : -1; }
void proc_start(void) { dump_pagetable(current_proc->pagetable); scheduler(); }

void proc_yield(void)
{
  struct proc *p = current_proc;
  if (!p) return;
  acquire(&p->lock);
  p->state = RUNNABLE;
  sched();
  release(&p->lock);
}

void proc_sleep(void *chan, spinlock_t *lk)
{
  struct proc *p = current_proc;
  if (!p) return;
  if (lk != &p->lock) acquire(&p->lock);
  if (lk != &p->lock) release(lk);
  p->chan = chan; p->state = SLEEPING;
  sched();
  p->chan = 0;
  release(&p->lock);
  if (lk != &p->lock) acquire(lk);
}

void proc_wakeup(void *chan)
{
  for (int i = 0; i < NPROC; i++) {
    struct proc *p = &ptable[i];
    /* The caller may be waking a process that is sleeping on the same
     * channel, so the state/channel check and transition must be protected
     * by that process's lock.  The current process is already locked by its
     * sleep/exit path and must not be acquired recursively. */
    if (p == current_proc) continue;
    acquire(&p->lock);
    if (p->state == SLEEPING && p->chan == chan) {
      p->state = RUNNABLE; p->chan = 0; align_pass(p);
    }
    release(&p->lock);
  }
}

int proc_pause(int n)
{
  if (n < 0) return -1;
    acquire(&ticks_lock);
    uint64 target = ticks + (uint)n;
  while (ticks < target) {
    if (current_proc->killed) { release(&ticks_lock); return -1; }
    proc_sleep((void *)&ticks, &ticks_lock);
  }
  release(&ticks_lock);
    return 0;
}

int proc_schedstat(int pid, int *cpu, int *slice)
{
  struct proc *p = 0;
  if (pid == -1) p = current_proc;
  else for (int i = 0; i < NPROC; i++) if (ptable[i].pid == pid) { p = &ptable[i]; break; }
  if (!p) return -1;
  acquire(&p->lock);
  if (p->state == UNUSED || p->state == ZOMBIE) { release(&p->lock); return -1; }
  *cpu = p->cpu_ticks; *slice = p->last_slice_ticks;
  release(&p->lock); return 0;
}

int proc_settickets(int n)
{
  if (n < 1 || !current_proc) return -1;
  acquire(&current_proc->lock);
  current_proc->tickets = n; current_proc->stride = 1000 / n; if (!current_proc->stride) current_proc->stride = 1;
  current_proc->tickets_set = 1;
  release(&current_proc->lock); return 0;
}

int proc_kill(int pid)
{
  for (int i = 0; i < NPROC; i++) {
    struct proc *p = &ptable[i];
    if (p->pid != pid) continue;
    acquire(&p->lock); p->killed = 1;
    if (p->state == SLEEPING) { p->state = RUNNABLE; p->chan = 0; }
    release(&p->lock); return 0;
  }
  return -1;
}

uint64 proc_time_rdtime(void) { return r_time(); }

void proc_clocktick(void)
{
  acquire(&ticks_lock);
  ticks++;
    proc_wakeup((void *)&ticks);
  release(&ticks_lock);
}

void proc_account_tick(void)
{
  struct proc *p = current_proc;
  if (!p) return;
  acquire(&p->lock);
  p->cpu_ticks++; p->last_slice_ticks++;
  p->pass += (uint)p->stride;
  release(&p->lock);
}

void proc_dump(void)
{
  static const char *names[] = { "UNUSED", "USED", "RUNNABLE", "RUNNING", "SLEEPING", "ZOMBIE" };
  printf("\nPID STATE NAME TICK SLICE TICKETS PASS\n");
  for (int i = 0; i < NPROC; i++) {
    struct proc *p = &ptable[i];
    if (p->state == UNUSED) continue;
    printf("%d %s %s %d %d %d %ld\n", p->pid, names[p->state], p->name,
           p->cpu_ticks, p->last_slice_ticks, p->tickets, p->pass);
  }
}
