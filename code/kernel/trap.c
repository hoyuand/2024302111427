#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "course_sid.h"
#include "proc.h"
#include "console.h"
#include "trap.h"

#define SCAUSE_INTERRUPT (1ULL << 63)
#define SCAUSE_STIMER    5
#define SCAUSE_SSOFT     1
#define SCAUSE_SEXT      9
#define TIMER_QUANTUM    1000000ULL
#define CLINT_MTIME      (CLINT_BASE + 0xbff8)
#define CLINT_MTIMECMP0  (CLINT_BASE + 0x4000)

extern void s_trap_vector(void);
extern void kernel_trap_vector(void);
extern uint64 syscall_dispatch(struct trapframe *tf);

static volatile uint64 ticks;

static void
plicinit(void)
{
  volatile uint32 *priority = (volatile uint32 *)PLIC_PRIORITY;
  volatile uint32 *enable = (volatile uint32 *)PLIC_SENABLE(0);
  volatile uint32 *threshold = (volatile uint32 *)PLIC_SPRIORITY(0);
  *priority = 1;
  *enable |= 1U << UART0_IRQ;
  *threshold = 0;
  io_fence();
}

static void
timerinit(void)
{
  ticks = 0;
  *(volatile uint64 *)CLINT_MTIMECMP0 =
      *(volatile uint64 *)CLINT_MTIME + LAB2_TICK * TIMER_QUANTUM;
  io_fence();
}

static int
devintr(uint64 cause)
{
  if ((cause & SCAUSE_INTERRUPT) == 0)
    return 0;
  uint64 code = cause & 0xfff;
  if (code == SCAUSE_SEXT) {
    volatile uint32 *claim = (volatile uint32 *)PLIC_SCLAIM(0);
    uint32 irq = *claim;
    if (irq == UART0_IRQ)
      consoleintr();
    if (irq != 0)
      *claim = irq;
    return 1;
  }
  if (code == SCAUSE_STIMER) {
    ticks++;
    *(volatile uint64 *)CLINT_MTIMECMP0 =
        *(volatile uint64 *)CLINT_MTIME + LAB2_TICK * TIMER_QUANTUM;
    io_fence();
    return 2;
  }
  if (code == SCAUSE_SSOFT) {
    *(volatile uint32 *)CLINT_BASE = 0;
    io_fence();
    ticks++;
    return 2;
  }
  return 0;
}

void
trapinit(void)
{
  w_stvec((uint64)s_trap_vector);
  plicinit();
  uart_enable_interrupts();
  timerinit();
  w_sie(SIE_SEIE | (1L << 1));
}

void
trap_enable_user(void)
{
  struct trapframe *tf = &current_proc->tf;
  w_stvec((uint64)s_trap_vector);
  asm volatile("csrw sscratch, %0" : : "r"((uint64)tf));
  w_sepc(tf->epc);
  uint64 status = r_sstatus();
  status &= ~SSTATUS_SPP;
  status |= SSTATUS_SPIE;
  status &= ~SSTATUS_SIE;
  w_sstatus(status);
}

void
trap_wait_begin(void)
{
  w_stvec((uint64)kernel_trap_vector);
  asm volatile("csrw sscratch, zero");
}

void
trap_wait_end(void)
{
  w_stvec((uint64)s_trap_vector);
  asm volatile("csrw sscratch, %0" : : "r"((uint64)&current_proc->tf));
}

void
usertrap(void)
{
  struct proc *before = current_proc;
  struct trapframe *tf = &before->tf;
  uint64 cause = r_scause();
  if (cause & SCAUSE_INTERRUPT) {
    if (!devintr(cause))
      proc_exit(-1);
  } else if (cause == 8) {
    tf->epc = r_sepc() + 4;
    uint64 result = syscall_dispatch(tf);
    if (current_proc == before)
      tf->a0 = result;
  } else {
    extern void printf(const char *fmt, ...);
    printf("lab2: unexpected user trap cause=%lx epc=%lx\n", cause, r_sepc());
    proc_exit(-1);
  }
  usertrap_return();
}

void
kerneltrap(void *frame)
{
  (void)frame;
  (void)devintr(r_scause());
}

void
usertrap_return(void)
{
  struct trapframe *tf = &current_proc->tf;
  w_stvec((uint64)s_trap_vector);
  w_sepc(tf->epc);
  uint64 status = r_sstatus();
  status &= ~SSTATUS_SPP;
  status |= SSTATUS_SPIE;
  status &= ~SSTATUS_SIE;
  w_sstatus(status);
  asm volatile("csrw sscratch, %0" : : "r"((uint64)tf));
  extern void userret_direct(void) __attribute__((noreturn));
  userret_direct();
  for (;;)
    asm volatile("wfi");
}
