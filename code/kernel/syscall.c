#include "types.h"
#include "riscv.h"
#include "proc.h"
#include "memlayout.h"
#include "console.h"
#include "trap.h"
#include "syscall.h"
#include "vm.h"

static int
argint(struct trapframe *tf, int n, int *value)
{
  if (n < 0 || n > 2)
    return -1;
  *value = (int)(n == 0 ? tf->a0 : n == 1 ? tf->a1 : tf->a2);
  return 0;
}

static int
argaddr(struct trapframe *tf, int n, uint64 *value)
{
  if (n < 0 || n > 2)
    return -1;
  *value = n == 0 ? tf->a0 : n == 1 ? tf->a1 : tf->a2;
  return 0;
}

static int
argstr(struct trapframe *tf, int n, uint64 *value)
{
  uint64 addr;
  if (argaddr(tf, n, &addr) < 0)
    return -1;
  *value = addr;
  for (uint i = 0; i < 256; i++) {
    char c;
    if (copyin(current_proc->pagetable, &c, addr + i, 1) < 0)
      return -1;
    if (c == 0)
      return (int)i;
  }
  return -1;
}

static int
sys_read(struct trapframe *tf)
{
  int fd, n;
  uint64 addr;
  if (argint(tf, 0, &fd) < 0 || argaddr(tf, 1, &addr) < 0 ||
      argint(tf, 2, &n) < 0 || fd != 0 || n < 0 ||
      n < 0)
    return -1;
  if (n == 0)
    return 0;

  consoleintr();
  while (!console_input_available()) {
    /* There is no sleep/wakeup yet, so park in WFI with the kernel trap
     * vector installed. Lab4 can replace this bounded wait with a queue. */
    trap_wait_begin();
    intr_on();
    while (!console_input_available())
      asm volatile("wfi");
    intr_off();
    trap_wait_end();
  }
  char tmp[128];
  int got = console_read(tmp, n > (int)sizeof(tmp) ? (int)sizeof(tmp) : n);
  if (got > 0 && copyout(current_proc->pagetable, addr, tmp, got) < 0)
    return -1;
  return got;
}

static int
sys_write(struct trapframe *tf)
{
  int fd, n;
  uint64 addr;
  if (argint(tf, 0, &fd) < 0 || argaddr(tf, 1, &addr) < 0 ||
      argint(tf, 2, &n) < 0 || (fd != 1 && fd != 2) || n < 0 ||
      n < 0)
    return -1;
  for (int i = 0; i < n; i++) {
    consoleintr();
    char c;
    if (copyin(current_proc->pagetable, &c, addr + (uint)i, 1) < 0)
      return -1;
    console_raw_putc(c);
  }
  return n;
}

static int
sys_exec(struct trapframe *tf)
{
  uint64 name_addr;
  if (argstr(tf, 0, &name_addr) < 0)
    return -1;
  char local[16];
  int i;
  for (i = 0; i < (int)sizeof(local) - 1; i++) {
    if (copyin(current_proc->pagetable, &local[i], name_addr + i, 1) < 0)
      return -1;
    if (local[i] == 0)
      break;
  }
  for (int j = 0; j < i; j++)
    if (local[j] == '\n' || local[j] == '\r') { local[j] = 0; break; }
  local[sizeof(local) - 1] = 0;
  return proc_exec(local);
}

static uint64
sys_sbrk(struct trapframe *tf)
{
  int delta, policy;
  if (argint(tf, 0, &delta) < 0 || argint(tf, 1, &policy) < 0)
    return (uint64)-1;
  struct proc *p = current_proc;
  uint64 old = p->brk;
  long long next = (long long)p->brk + delta;
  if (next < (long long)p->stack_top || next > (long long)(TRAPFRAME - LAB3_GUARD_PAGES * PGSIZE))
    return (uint64)-1;
  if (delta > 0) {
    if (uvmalloc(p->pagetable, old, (uint64)next, PTE_R | PTE_W) < 0)
      return (uint64)-1;
  } else if (delta < 0) {
    p->brk = (uint64)uvmdealloc(p->pagetable, old, (uint64)next);
  }
  p->brk = (uint64)next;
  p->sz = p->brk;
  (void)policy; /* SBRK_LAZY follows EAGER semantics in Lab3. */
  return old;
}

uint64
syscall_dispatch(struct trapframe *tf)
{
  switch (tf->a7) {
  case SYS_fork: return proc_fork();
  case SYS_exit: {
    int status;
    if (argint(tf, 0, &status) < 0)
      status = -1;
    proc_exit(status);
  }
  case SYS_wait: {
    uint64 addr;
    if (argaddr(tf, 0, &addr) < 0)
      return (uint64)-1;
    return proc_wait(addr);
  }
  case SYS_read: return sys_read(tf);
  case SYS_write: return sys_write(tf);
  case SYS_exec: return sys_exec(tf);
  case SYS_getpid: return proc_getpid();
  case SYS_sbrk: return sys_sbrk(tf);
  default: return (uint64)-1;
  }
}
