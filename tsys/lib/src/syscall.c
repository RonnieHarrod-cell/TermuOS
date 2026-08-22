#include <syscall.h>

long __syscall0(long n)
{
    long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
    return ret;
}

long __syscall1(long n, long a)
{
    long ret;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(n), "D"(a)
                     : "rcx", "r11", "memory");
    return ret;
}

long __syscall2(long n, long a, long b)
{
    long ret;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(n), "D"(a), "S"(b)
                     : "rcx", "r11", "memory");
    return ret;
}

long __syscall3(long n, long a, long b, long c)
{
    long ret;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(n), "D"(a), "S"(b), "d"(c)
                     : "rcx", "r11", "memory");
    return ret;
}
