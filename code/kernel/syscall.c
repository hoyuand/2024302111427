#include "types.h"
extern uint64 sys_open(void);
extern uint64 sys_close(void);
extern uint64 sys_fstat(void);
extern uint64 sys_dup(void);
extern uint64 sys_link(void);
extern uint64 sys_unlink(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_chdir(void);
extern uint64 sys_mknod(void);
extern uint64 sys_crash_at(void);
extern uint64 sys_read(void);
extern uint64 sys_write(void);
#include "defs.h"
#include "types.h"
#include "riscv.h"
#include "proc.h"
#include "memlayout.h"
#include "console.h"
#include "trap.h"
#include "syscall.h"
#include "lab5.h"

static int
tf_argint(struct trapframe *tf, int n, int *value)
{
  if (n < 0 || n > 2)
    return -1;
  *value = (int)(n == 0 ? tf->a0 : n == 1 ? tf->a1 : tf->a2);
  return 0;
}

static int
tf_argaddr(struct trapframe *tf, int n, uint64 *value)
{
  if (n < 0 || n > 2)
    return -1;
  *value = n == 0 ? tf->a0 : n == 1 ? tf->a1 : tf->a2;
  return 0;
}

static int
sys_exec(struct trapframe *tf)
{
  char name[MAXPATH];
  uint64 uargv;
  struct proc *p = current_proc;
  if (copyinstr(p->pagetable, name, tf->a0, sizeof(name)) < 0 ||
      tf_argaddr(tf, 1, &uargv) < 0) return -1;
  char *buf = kalloc();
  if (!buf) return -1;
  const char *argv[MAXARG];
  int argc = 0;
  uint used = 0;
  for (;;) {
    uint64 arg;
    if (uargv + (uint64)argc * sizeof(arg) < uargv ||
        copyin(p->pagetable, &arg, uargv + argc * sizeof(arg), sizeof(arg)) < 0) goto bad;
    if (!arg) break;
    if (argc == MAXARG - 1 || used == PGSIZE) goto bad;
    int len = copyinstr(p->pagetable, buf + used, arg, PGSIZE - used);
    if (len < 0) goto bad;
    argv[argc++] = buf + used;
    used += len;
  }
  int result = proc_exec(name, argc, argv);
  kfree(buf);
  return result;
bad:
  kfree(buf);
  return -1;
}

static uint64
sys_sbrk(struct trapframe *tf)
{
  int delta, policy;
  if (tf_argint(tf, 0, &delta) < 0 || tf_argint(tf, 1, &policy) < 0)
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
    if (tf_argint(tf, 0, &status) < 0)
      status = -1;
    proc_exit(status);
  }
  case SYS_wait: {
    uint64 addr;
    if (tf_argaddr(tf, 0, &addr) < 0)
      return (uint64)-1;
    return proc_wait(addr);
  }
  case SYS_read: return sys_read();
  case SYS_write: return sys_write();
  case SYS_exec: return sys_exec(tf);
  case SYS_open: return sys_open();
  case SYS_close: return sys_close();
  case SYS_fstat: return sys_fstat();
  case SYS_dup: return sys_dup();
  case SYS_link: return sys_link();
  case SYS_unlink: return sys_unlink();
  case SYS_mkdir: return sys_mkdir();
  case SYS_chdir: return sys_chdir();
  case SYS_mknod: return sys_mknod();
  case SYS_crash_at: return sys_crash_at();
  case SYS_getpid: return proc_getpid();
  case SYS_sbrk: return sys_sbrk(tf);
  case SYS_pmc:
    if ((int)tf->a0 == 0) return kcow_copy_count();
    if ((int)tf->a0 == 1) return kshared_count();
    return (uint64)-1;
  case SYS_pause: return proc_pause((int)tf->a0);
  case SYS_time_rdtime: return proc_time_rdtime();
  case SYS_setickets: return proc_settickets((int)tf->a0);
  case SYS_kill: return proc_kill((int)tf->a0);
  case SYS_schedstat: {
    int cpu, slice;
    if (proc_schedstat((int)tf->a0, &cpu, &slice) < 0) return (uint64)-1;
    if (copyout(current_proc->pagetable, tf->a1, &cpu, sizeof(cpu)) < 0 ||
        copyout(current_proc->pagetable, tf->a2, &slice, sizeof(slice)) < 0) return (uint64)-1;
    return 0;
  }
  default: return (uint64)-1;
  }
}
