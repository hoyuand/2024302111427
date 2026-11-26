// 课程测试 · lab6-test2: 受控六阶段断电注入驱动（Stage 0..5）
//
// 用法：在 sh> 下运行 `crashtest <0..5>`
//   1. 先以完整提交的基线事务创建 crashfile，写入 "OLD-CONTENT\n"（12 字节）；
//   2. 若 stage == 0，在发起新写事务（begin_op）前立即触发断电复位；
//   3. 若 stage == 1..5，先调用 crash_at(stage) 武装下一笔事务的断电点，
//      紧接着向 crashfile 写入 "STAGE-X-COMMITTED\n"（18 字节），在 log.c 对应阶段触发复位；
//   4. 系统自动重启回到 sh> 后，运行 `cat crashfile` 核对恢复状态：
//      - Stage 0 / 1 / 2（提交头块落盘前）：输出 "OLD-CONTENT"；
//      - Stage 3 / 4 / 5（提交头块落盘及之后）：recover_from_log 重放日志，输出 "STAGE-X-COMMITTED"。
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("usage: crashtest <0..5>\n");
    exit(1);
  }

  int stage = atoi(argv[1]);
  if (stage < 0 || stage > 5) {
    printf("crashtest: invalid stage %d\n", stage);
    exit(1);
  }

  unlink("crashfile");
  int fd = open("crashfile", O_CREATE | O_RDWR | O_TRUNC);
  if (fd < 0) {
    printf("crashtest: open baseline failed\n");
    exit(1);
  }
  const char *base = "OLD-CONTENT\n";
  if (write(fd, base, 12) != 12) {
    printf("crashtest: write baseline failed\n");
    exit(1);
  }
  close(fd);

  fd = open("crashfile", O_RDWR);
  if (fd < 0) {
    printf("crashtest: reopen failed\n");
    exit(1);
  }

  char msg[24];
  memcpy(msg, "STAGE-0-COMMITTED\n", 19);
  msg[6] = (char)('0' + stage);

  printf("[crashtest] baseline committed (OLD-CONTENT), triggering stage %d...\n", stage);

  crash_at(stage);
  write(fd, msg, 18);
  close(fd);
  exit(0);
}
