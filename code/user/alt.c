/* Two CPU-bound children demonstrate user-mode timer preemption. */
#include "kernel/types.h"
#include "user/user.h"

static void
runA(void)
{
  volatile unsigned long work = 0;
  for (int i = 0; i < 16; i++) {
    for (int d = 0; d < 4000000; d++)
      work += (unsigned long)d;
    printf("A");
    pause(1);
  }
  printf("\n");
  exit(work == 0 ? 1 : 0);
}

static void
runB(void)
{
  volatile unsigned long work = 0;
  for (int i = 0; i < 16; i++) {
    for (int d = 0; d < 4000000; d++)
      work += (unsigned long)d;
    printf("B");
    pause(1);
  }
  printf("\n");
  exit(work == 0 ? 1 : 0);
}

int
main(void)
{
  int a = fork();
  if (a == 0) runA();
  int b = fork();
  if (b == 0) runB();
  if (a < 0 || b < 0) {
    printf("FAIL: alt fork\n");
    exit(1);
  }
  int status = -1;
  for (int i = 0; i < 2; i++) {
    if (wait(&status) < 0 || status != 0) {
      printf("\nFAIL: alt wait status\n");
      exit(1);
    }
  }
  printf("PASS: alt\n");
  exit(0);
}
