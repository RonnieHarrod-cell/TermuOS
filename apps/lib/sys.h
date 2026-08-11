#pragma once

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_SPAWN 57
#define SYS_EXIT 60
#define SYS_WAIT 61

static inline long syscall3(long n, long a, long b, long c)
{
    long ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a), "S"(b), "d"(c)
        : "rcx", "r11", "memory");
    return ret;
}

static inline void sys_exit(int code)
{
    syscall3(SYS_EXIT, code, 0, 0);
    for (;;)
        ;
}

static inline long sys_write(int fd, const void *buf, long len)
{
    return syscall3(SYS_WRITE, fd, (long)buf, len);
}

static inline long sys_read(int fd, void *buf, long len)
{
    return syscall3(SYS_READ, fd, (long)buf, len);
}

static inline long sys_spawn(const char *path)
{
    return syscall3(SYS_SPAWN, (long)path, 0, 0);
}

static inline long sys_wait(long pid)
{
    return syscall3(SYS_WAIT, pid, 0, 0);
}
