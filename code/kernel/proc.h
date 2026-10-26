#ifndef LAB2_PROC_H
#define LAB2_PROC_H

#include "types.h"
#include "param.h"
#include "riscv.h"

#define PROC_MEM_SIZE (256 * 1024)
#define KSTACK_SIZE   4096

/* The scheduler uses a deliberately small xv6-style spin lock.  Lab4 is
 * delivered and graded with one hart, but interrupt exclusion is still
 * required: a timer/UART interrupt must never observe a half transition. */
struct spinlock { volatile uint locked; };
typedef struct spinlock spinlock_t;

struct context {
  uint64 ra;
  uint64 sp;
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

struct trapframe {
  uint64 kernel_satp;
  uint64 kernel_sp;
  uint64 kernel_trap;
  uint64 epc;
  uint64 kernel_hartid;
  uint64 ra;
  uint64 sp;
  uint64 gp;
  uint64 tp;
  uint64 t0;
  uint64 t1;
  uint64 t2;
  uint64 s0;
  uint64 s1;
  uint64 a0;
  uint64 a1;
  uint64 a2;
  uint64 a3;
  uint64 a4;
  uint64 a5;
  uint64 a6;
  uint64 a7;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
  uint64 t3;
  uint64 t4;
  uint64 t5;
  uint64 t6;
};

enum procstate { UNUSED, USED, RUNNABLE, RUNNING, SLEEPING, ZOMBIE };

struct proc {
  spinlock_t lock;
  int pid;
  enum procstate state;
  struct proc *parent;
  int exit_status;
  uint64 userbase;
  uint memsz;
  uint brk;
  uint64 sz;
  uint64 stack_top;
  char name[16];
  pagetable_t pagetable;
  struct context context;
  void *chan;
  int killed;
  int cpu_ticks;
  int last_slice_ticks;
  int tickets;
  int tickets_set;
  int stride;
  uint64 pass;
  uchar kstack[KSTACK_SIZE] __attribute__((aligned(16)));
  /* trampoline.S addresses TRAPFRAME at page offset zero. */
  struct trapframe tf __attribute__((aligned(4096)));
};

extern struct proc *current_proc;

void push_off(void);
void pop_off(void);
void acquire(spinlock_t *lk);
void release(spinlock_t *lk);
int holding(spinlock_t *lk);

void procinit(void);
int proc_exec(const char *name);
int proc_fork(void);
int proc_wait(uint64 status_addr);
void proc_exit(int status) __attribute__((noreturn));
int proc_getpid(void);
void proc_start(void) __attribute__((noreturn));
void proc_yield(void);
void proc_sleep(void *chan, spinlock_t *lk);
void proc_wakeup(void *chan);
int proc_pause(int ticks);
int proc_schedstat(int pid, int *cpu_ticks, int *last_slice_ticks);
int proc_settickets(int tickets);
int proc_kill(int pid);
uint64 proc_time_rdtime(void);
void proc_clocktick(void);
void proc_account_tick(void);
void proc_dump(void);
uint64 proc_satp(void);
void proc_map_trapframe(void);
void usertrap(void);
void kerneltrap(void *frame);
void usertrapret(void) __attribute__((noreturn));
void usertrap_return(void) __attribute__((noreturn));
int user_range(const void *addr, uint n);

#endif
