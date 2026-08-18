#include "../include/syscall.h"

long __syscall3(long n, long a, long b, long c)
{
    long ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a), "S"(b), "d"(c)
        : "rcx", "r11", "memory"
    );
    return ret;
}
