#include "console.h"

#include <stdarg.h>
#include <stdio.h>

#include "stm32h7xx_hal.h"

#define SEMIHOST_SYS_WRITE0 0x04U
#define SEMIHOST_SYS_WRITE  0x05U

static int semihost_call(uint32_t op, void *arg)
{
    register uint32_t r0 __asm__("r0") = op;
    register void *r1 __asm__("r1") = arg;

    __asm__ volatile("bkpt 0xAB" : "+r"(r0) : "r"(r1) : "memory");

    return (int)r0;
}

int console_ready(void)
{
    return (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0U;
}

void console_init(void)
{
}

void console_puts(const char *s)
{
    if (!console_ready() || (s == NULL))
    {
        return;
    }

    (void)semihost_call(SEMIHOST_SYS_WRITE0, (void *)s);
}

int console_printf(const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    int n;

    if (!console_ready())
    {
        return -1;
    }

    va_start(ap, fmt);
    n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n < 0)
    {
        return -1;
    }

    console_puts(buf);
    return n;
}

int _write(int file, char *ptr, int len)
{
    struct
    {
        int handle;
        const void *buf;
        unsigned int len;
    } args;
    int not_written;

    (void)file;

    if (!console_ready() || (ptr == NULL) || (len <= 0))
    {
        return len;
    }

    args.handle = 1;
    args.buf = ptr;
    args.len = (unsigned int)len;

    not_written = semihost_call(SEMIHOST_SYS_WRITE, &args);
    if (not_written < 0)
    {
        return -1;
    }

    if ((unsigned int)not_written > (unsigned int)len)
    {
        return 0;
    }

    return len - not_written;
}
