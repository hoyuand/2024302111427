#ifndef LAB2_CONSOLE_H
#define LAB2_CONSOLE_H

#include "types.h"

void consoleinit(void);
void uart_enable_interrupts(void);
void uartputc_sync(int c);
void console_raw_putc(int c);
void console_raw_puts(const char *s);
void console_checksum_begin(void);
uint32 console_checksum_end(void);
void consoleintr(void);
int console_input_available(void);
int console_read(char *dst, int n);

#endif
