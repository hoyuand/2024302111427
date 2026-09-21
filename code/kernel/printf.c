/* Lab1：不依赖 libc 的最小 printf。 */
#include "types.h"
#include <stdarg.h>

typedef long int64;

extern void console_raw_putc(int c);
extern void console_raw_puts(const char *s);

static void
print_unsigned(uint64 value, uint32 base)
{
  static const char digits[] = "0123456789abcdef";
  char buf[32];
  uint32 n = 0;

  if (value == 0) {
    console_raw_putc('0');
    return;
  }
  while (value != 0) {
    buf[n++] = digits[value % base];
    value /= base;
  }
  while (n > 0)
    console_raw_putc(buf[--n]);
}

static void
print_signed(int64 value)
{
  uint64 magnitude;
  if (value < 0) {
    console_raw_putc('-');
    /* 这样处理 INT64_MIN 也不会在取反时溢出。 */
    magnitude = (uint64)(-(value + 1)) + 1;
  } else {
    magnitude = (uint64)value;
  }
  print_unsigned(magnitude, 10);
}

void
printf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  for (; *fmt; fmt++) {
    if (*fmt != '%') {
      console_raw_putc((uchar)*fmt);
      continue;
    }
    fmt++;
    uint32 long_arg = 0;
    if (*fmt == 'l') {
      long_arg = 1;
      fmt++;
    }
    switch (*fmt) {
    case 'c':
      console_raw_putc(va_arg(ap, int));
      break;
    case 's': {
      const char *s = va_arg(ap, const char *);
      console_raw_puts(s ? s : "(null)");
      break;
    }
    case 'd':
      if (long_arg)
        print_signed(va_arg(ap, int64));
      else
        print_signed((int64)va_arg(ap, int));
      break;
    case 'u':
      if (long_arg)
        print_unsigned(va_arg(ap, uint64), 10);
      else
        print_unsigned((uint64)va_arg(ap, uint), 10);
      break;
    case 'x':
      if (long_arg)
        print_unsigned(va_arg(ap, uint64), 16);
      else
        print_unsigned((uint64)va_arg(ap, uint), 16);
      break;
    case 'p':
      console_raw_puts("0x");
      print_unsigned((uint64)va_arg(ap, uint64), 16);
      break;
    case '%':
      console_raw_putc('%');
      break;
    default:
      /* 未知格式原样输出，便于发现测试/实现中的拼写错误。 */
      console_raw_putc('%');
      if (long_arg)
        console_raw_putc('l');
      console_raw_putc((uchar)*fmt);
      break;
    }
  }
  va_end(ap);
}
