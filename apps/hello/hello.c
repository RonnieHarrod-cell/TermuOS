/*
 * hello.c — minimal test program for TermuOS's exec/run shell command.
 *
 * No libc, no dynamic linking — just raw Linux-ABI syscalls (which is what
 * TermuOS's syscall_dispatch implements), so it links as a plain static
 * ET_EXEC ELF64 binary that exec_load() can map directly.
 */

static inline long syscall3(long n, long a1, long a2, long a3)
{
    long ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory");
    return ret;
}

#define SYS_WRITE 1
#define SYS_EXIT 60

void _start(void)
{
    const char msg[] = "Hello from TermuOS!\n";
    syscall3(SYS_WRITE, 1 /* stdout */, (long)msg, sizeof(msg) - 1);
    syscall3(SYS_EXIT, 0, 0, 0);

    /* should never get here */
    for (;;)
        ;
}