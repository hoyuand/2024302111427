/* ABI boundaries and repeated sleep/wakeup with wait status write-back. */
#include "common.h"

int
main(void)
{
  int cpu = -1, slice = -1;
  ASSERT(setickets(0) == -1, "zero tickets accepted");
  ASSERT(setickets(-1) == -1, "negative tickets accepted");
  ASSERT(setickets(18) == 0, "valid tickets rejected");
  ASSERT(schedstat(-1, &cpu, &slice) == 0 && cpu >= 0 && slice >= 0,
         "self schedstat failed");
  ASSERT(schedstat(2147483647, &cpu, &slice) == -1,
         "missing pid accepted");
  ASSERT(schedstat(-2, &cpu, &slice) == -1, "invalid pid accepted");
  /* 0x3fffffe000 is the user stack; TRAPFRAME at 0x3ffffff000 is mapped
     without PTE_U and therefore exercises the copyout permission boundary. */
  ASSERT(schedstat(-1, (int *)0x3ffffff000UL, &slice) == -1,
         "supervisor-only copyout accepted");
  ASSERT(schedstat(-1, &cpu, (int *)0xffffffffffffffffUL) == -1,
         "overflow copyout accepted");
  ASSERT(pause(0) == 0, "pause zero failed");
  ASSERT(wait(0) == -1, "wait without children accepted");
  printf("RESULT abi_boundaries = 10\n");

  unsigned long per_tick = tick_cycles();
  unsigned long before = time_rdtime();
  ASSERT(pause(3) == 0, "pause three failed");
  unsigned long elapsed = time_rdtime() - before;
  /* Entry can be just before a tick boundary: n ticks span at least n-1 periods. */
  ASSERT(elapsed > per_tick, "pause returned before timer progression");

  int pids[6];
  for (int i = 0; i < 6; i++) {
    pids[i] = fork();
    ASSERT(pids[i] >= 0, "boundary fork failed");
    if (pids[i] == 0) {
      for (int j = 0; j < 6; j++)
        ASSERT(pause(1) == 0, "child pause failed");
      exit(31 + i);
    }
  }
  int seen = 0;
  for (int i = 0; i < 6; i++) {
    int st = -1;
    int pid = wait(&st);
    ASSERT(pid > 0 && st >= 31 && st <= 36, "wait status writeback failed");
    ASSERT(!(seen & (1 << (st - 31))), "child reaped twice");
    seen |= 1 << (st - 31);
    ASSERT(schedstat(pid, &cpu, &slice) == -1, "reaped pid still alive");
  }
  ASSERT(seen == 63 && wait(0) == -1, "child reclamation incomplete");
  printf("RESULT sleep_wait = 6 children x 6 pauses; elapsed=%lu\n", elapsed);
  PASS("lab4check");
}
