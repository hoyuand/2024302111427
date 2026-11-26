// 课程预置校验工具 · cat.c：读取并输出文件内容（供 crashmap 断电恢复校验与日常文件查看使用）
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static char buf[512];

static void
cat_fd(int fd)
{
  int n;
  while ((n = read(fd, buf, sizeof(buf))) > 0) {
    if (write(1, buf, n) != n) {
      printf("cat: write error\n");
      exit(1);
    }
  }
  if (n < 0) {
    printf("cat: read error\n");
    exit(1);
  }
}

int
main(int argc, char *argv[])
{
  if (argc <= 1) {
    cat_fd(0);
    exit(0);
  }

  for (int i = 1; i < argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      printf("cat: cannot open %s\n", argv[i]);
      exit(1);
    }
    cat_fd(fd);
    close(fd);
  }
  exit(0);
}
