/* Lab1：S 态的最小 C 入口。 */
#include "types.h"
#include "riscv.h"

extern void s_trap_vector(void);
extern void uartinit(void);
extern void main(void);

void
start(void)
{
  /* entry.S 已在 M 态把 mhartid 保存到 tp；S 态不能再读取 mhartid CSR。 */

  /* 本轮是轮询输出，不启用 S 态设备中断。 */
  intr_off();
  w_stvec((uint64)s_trap_vector);
  uartinit();

  main();

  /* main 返回后没有合法调用者；低功耗停机而不是跑飞。 */
  for (;;)
    asm volatile("wfi");
}
