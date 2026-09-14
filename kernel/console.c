/* Lab1：QEMU virt 上的 16550 UART 轮询驱动与输出协议辅助。 */
#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "course_sid.h"

#define UART_RHR 0
#define UART_THR 0
#define UART_IER 1
#define UART_FCR 2
#define UART_LCR 3
#define UART_LSR 5
#define UART_LSR_THRE 0x20

#define UART_THROTTLE_PERIOD (16 + (COURSE_SID % 16))
#define UART_THROTTLE_NOPS   (1 + (COURSE_SID % 8))

static volatile uint32 tx_count;
static volatile uint32 checksum_enabled;
static volatile uint32 checksum_value;

static inline volatile uchar *
uart_reg(uint32 offset)
{
  return (volatile uchar *)(UART0 + offset);
}

void
uartinit(void)
{
  /* 关闭中断，设置 8 数据位、无校验、1 停止位，清空 FIFO。 */
  *uart_reg(UART_IER) = 0;
  *uart_reg(UART_LCR) = 0x80;       /* DLAB=1 */
  *uart_reg(0) = 0x03;              /* 115200 baud divisor low */
  *uart_reg(1) = 0x00;              /* divisor high */
  *uart_reg(UART_LCR) = 0x03;       /* DLAB=0, 8N1 */
  *uart_reg(UART_FCR) = 0x07;       /* enable/reset FIFO */
  *uart_reg(UART_IER) = 0;          /* polling only */
  io_fence();
  tx_count = 0;
}

static void
uart_throttle(void)
{
  /* 节流周期与次数均由 COURSE_SID 派生，不能写成无关魔数。 */
  for (volatile uint32 i = 0; i < UART_THROTTLE_NOPS; i++)
    asm volatile("nop");
}

void
uartputc_sync(int c)
{
  volatile uchar *lsr = uart_reg(UART_LSR);
  volatile uchar *thr = uart_reg(UART_THR);

  while ((*lsr & UART_LSR_THRE) == 0)
    asm volatile("nop");
  *thr = (uchar)c;
  io_fence();

  tx_count++;
  if (tx_count >= UART_THROTTLE_PERIOD) {
    tx_count = 0;
    uart_throttle();
  }
}

void
console_raw_putc(int c)
{
  uartputc_sync(c);
  if (checksum_enabled)
    checksum_value = (checksum_value + (uchar)c) % 10000;
}

void
console_raw_puts(const char *s)
{
  while (*s)
    console_raw_putc((uchar)*s++);
}

void
console_checksum_begin(void)
{
  checksum_value = 0;
  checksum_enabled = 1;
}

uint32
console_checksum_end(void)
{
  checksum_enabled = 0;
  return checksum_value;
}
