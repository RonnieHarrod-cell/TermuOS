#include "doomgeneric-src/doomgeneric/doomgeneric.h"

#include <stdint.h>

#define SYS_WRITE 1
#define SYS_EXIT 60

static long sys3(long n, long a, long b, long c)
{
    long ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a), "S"(b), "d"(c)
        : "rcx", "r11", "memory");
    return ret;
}

static void puts_raw(const char *s)
{
    const char *p = s;
    while (*p)
        p++;
    sys3(SYS_WRITE, 1, (long)s, (long)(p - s));
}

void DG_Init(void)
{
    puts_raw("doom: DG_Init\n");
}

void DG_DrawFrame(void)
{
    /* Phase 2 */
}

void DG_SleepMs(uint32_t ms)
{
    (void)ms;
    for (volatile unsigned i = 0; i < 50000; i++)
        ;
}

uint32_t DG_GetTicksMs(void)
{
    static uint32_t t;
    return t += 16;
}

int DG_GetKey(int *pressed, unsigned char *doomKey)
{
    (void)pressed;
    (void)doomKey;
    return 0;
}

void DG_SetWindowTitle(const char *title)
{
    puts_raw("doom: title: ");
    if (title)
        puts_raw(title);
    puts_raw("\n");
}
