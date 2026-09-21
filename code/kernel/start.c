/* Lab1：M 态初始化与向 S 态的切换入口。 */
#include "types.h"
#include "riscv.h"
#include "course_sid.h"
#include "memlayout.h"

extern void s_trap_vector(void);
extern void main(void);
extern void m_trap_vector(void);

/* 页对齐同时满足说明书要求的 16 字节栈对齐。 */
__attribute__((aligned(4096))) uchar bootstack[LAB1_STACK_KB * 1024];
__attribute__((aligned(16))) uint64 machine_scratch[4];

void
start(void)
{
  /* entry.S 已保存 hartid 并建立 C 栈，此处仍运行在 M 态。 */
  asm volatile("csrw mtvec, %0" : : "r"((uint64)m_trap_vector));

  /* 允许 S/U 态访问全部物理地址，包含内核 RAM 与 UART MMIO。 */
  w_pmpaddr0(0x3fffffffffffffL);
  w_pmpcfg0(0xf);

  /* 本阶段不启用分页；异常与中断委托给 S 态处理。 */
  w_satp(0);
  sfence_vma();
  w_medeleg(0xffff);
  w_mideleg(0xffff);
  /* Delegate the CLINT machine timer interrupt to S mode. */
  w_mie(1L << 7);
  w_sie(0);
  *(volatile uint64 *)(CLINT_BASE + 0x4000) =
      *(volatile uint64 *)(CLINT_BASE + 0xbff8) + LAB2_TICK * 1000000ULL;
  asm volatile("csrw mscratch, %0" : : "r"((uint64)machine_scratch));
  uint64 mstatus = r_mstatus();
  mstatus |= (1L << 3); /* MSTATUS.MIE */
  w_mstatus(mstatus);
  w_stvec((uint64)s_trap_vector);

  /* mret 返回到 main，并把目标特权级设置为 S 态。 */
  w_mepc((uint64)main);
  uint64 status = r_mstatus();
  status &= ~MSTATUS_MPP_MASK;
  status |= MSTATUS_MPP_S;
  w_mstatus(status);
  asm volatile("mret");

  /* mret 不应返回；保留兜底等待避免异常路径跑飞。 */
  for (;;)
    asm volatile("wfi");
}
