/* Compatibility for the unmodified Lab5 shell. The older course gets()
 * retains the line ending, while the new parser splits only spaces/tabs.
 * Only sh is compiled with gets=lab5_gets; UART read semantics stay intact. */
#include "kernel/types.h"
#include "user/user.h"
char *lab5_gets(char *buf, int max)
{
  gets(buf, max);
  uint n = strlen(buf);
  while (n && (buf[n-1] == '\n' || buf[n-1] == '\r')) buf[--n] = 0;
  return buf;
}
