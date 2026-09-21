#ifndef LAB2_PROC_H
#define LAB2_PROC_H

#include "types.h"
#include "param.h"

#define PROC_MEM_SIZE (256 * 1024)
#define KSTACK_SIZE   4096

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

enum procstate { UNUSED, RUNNABLE, RUNNING, WAITING, ZOMBIE };

struct proc {
  int pid;
  enum procstate state;
  struct proc *parent;
  int exit_status;
  uint64 userbase;
  uint memsz;
  uint brk;
  char name[16];
  uchar *mem;
  uchar kstack[KSTACK_SIZE] __attribute__((aligned(16)));
  struct trapframe tf __attribute__((aligned(16)));
};

extern struct proc *current_proc;

void procinit(void);
int proc_exec(const char *name);
int proc_fork(void);
int proc_wait(int *status);
void proc_exit(int status) __attribute__((noreturn));
int proc_getpid(void);
void proc_start(void) __attribute__((noreturn));
void usertrap(void);
void kerneltrap(void *frame);
void usertrap_return(void) __attribute__((noreturn));
int user_range(const void *addr, uint n);

#endif
