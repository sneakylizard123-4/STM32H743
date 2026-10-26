#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>

void console_init(void);
int console_ready(void);
void console_puts(const char *s);
int console_printf(const char *fmt, ...);

#endif
