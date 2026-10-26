/* Two independent windows prove setickets changes the live stride policy. */
#include "common.h"

#define WINDOW 80
#define SWITCH 90
#define END 210

static void
child(unsigned long start, unsigned long per_tick, int first, int second)
{
  ASSERT(setickets(first) == 0, "initial tickets rejected");
  burn_until(start + SWITCH * per_tick);
  ASSERT(setickets(second) == 0, "updated tickets rejected");
  burn_until(start + END * per_tick);
  exit(0);
}

int
main(void)
{
  ASSERT(setickets(500) == 0, "parent tickets rejected");
  unsigned long per_tick = tick_cycles();
  unsigned long start = time_rdtime();
  int p1 = fork();
  if (p1 == 0) child(start, per_tick, 10, 30);
  int p2 = fork();
  if (p2 == 0) child(start, per_tick, 30, 10);
  ASSERT(p1 > 0 && p2 > 0, "policy fork failed");
  ASSERT(delta_cpu(p1) == 0 && delta_cpu(p2) == 0, "initial sample failed");
  pause(WINDOW);
  int first1 = delta_cpu(p1), first2 = delta_cpu(p2);
  /* Exclude the boundary where each child performs its own ticket update. */
  pause(20);
  ASSERT(delta_cpu(p1) >= 0 && delta_cpu(p2) >= 0, "transition sample failed");
  pause(WINDOW);
  int second1 = delta_cpu(p1), second2 = delta_cpu(p2);
  printf("RESULT policy_before = %d:%d\n", first1, first2);
  printf("RESULT policy_after = %d:%d\n", second1, second2);
  ASSERT(first1 > 0 && first2 * 10 > first1 * 24 && first2 * 10 < first1 * 36,
         "initial 1:3 share outside tolerance");
  ASSERT(second2 > 0 && second1 * 10 > second2 * 24 && second1 * 10 < second2 * 36,
         "changed 3:1 share outside tolerance");
  kill(p1); kill(p2);
  ASSERT(wait(0) > 0 && wait(0) > 0, "policy children not reaped");
  PASS("policychange");
}
