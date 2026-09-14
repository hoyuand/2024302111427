/* Lab1：个性化 banner、自检序列与 printf 边界回归。 */
#include "types.h"
#include "course_sid.h"

typedef long int64;

extern void printf(const char *fmt, ...);
extern void console_checksum_begin(void);
extern uint32 console_checksum_end(void);

void
main(void)
{
  /* 协议 2：校验和是整段启动输出的 ASCII 字节和（含换行）mod 10000，
   * 最终以 [chk=十进制] 输出；关闭累加后不再产生正文。 */
  console_checksum_begin();

  /* 个性化 banner：学号十进制、sid%97 的小写无前导零十六进制。 */
  printf("OSLAB1 sid=%ld mod97=0x%x\n", (int64)COURSE_SID,
         (uint)(COURSE_SID % 97));

  /* 自检序列：覆盖 0、负数、最大 int、空字符串和连续长字符串。 */
  printf("printf-boundary zero=%d neg=%d max=%d empty=\"%s\"\n",
         0, -42, 2147483647, "");
  printf("printf-long=");
  printf("0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz"
         "0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz\n");

  uint32 checksum = console_checksum_end();
  printf("[chk=%u]\n", checksum);
}
