#include "types.h"
#include "riscv.h"
#include "proc.h"
#include "console.h"
#include "trap.h"
#include "syscall.h"

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
argstr(struct trapframe *tf, int n, char **value)
{
  uint64 addr;
  if (argaddr(tf, n, &addr) < 0 || !user_range((void *)addr, 1))
    return -1;
  *value = (char *)addr;
  for (uint i = 0; i < 256; i++) {
    if (!user_range((void *)(addr + i), 1))
      return -1;
    if (((char *)addr)[i] == 0)
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
      !user_range((void *)addr, (uint)n))
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
  return console_read((char *)addr, n);
}

static int
sys_write(struct trapframe *tf)
{
  int fd, n;
  uint64 addr;
  if (argint(tf, 0, &fd) < 0 || argaddr(tf, 1, &addr) < 0 ||
      argint(tf, 2, &n) < 0 || (fd != 1 && fd != 2) || n < 0 ||
      !user_range((void *)addr, (uint)n))
    return -1;
  for (int i = 0; i < n; i++) {
    consoleintr();
    console_raw_putc(((char *)addr)[i]);
  }
  return n;
}

static int
sys_exec(struct trapframe *tf)
{
  char *name;
  if (argstr(tf, 0, &name) < 0)
    return -1;
  return proc_exec(name);
}

static uint64
sys_sbrk(struct trapframe *tf)
{
  int delta, policy;
  if (argint(tf, 0, &delta) < 0 || argint(tf, 1, &policy) < 0)
    return (uint64)-1;
  (void)policy;
  struct proc *p = current_proc;
  uint old = p->brk;
  long long next = (long long)p->brk + delta;
  if (next < 0 || next > PROC_MEM_SIZE)
    return (uint64)-1;
  p->brk = (uint)next;
  return p->userbase + old;
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
    if (addr != 0 && !user_range((void *)addr, sizeof(int)))
      return (uint64)-1;
    return proc_wait((int *)addr);
  }
  case SYS_read: return sys_read(tf);
  case SYS_write: return sys_write(tf);
  case SYS_exec: return sys_exec(tf);
  case SYS_getpid: return proc_getpid();
  case SYS_sbrk: return sys_sbrk(tf);
  default: return (uint64)-1;
  }
}
