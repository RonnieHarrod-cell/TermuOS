static inline long sys3(long n, long a, long b, long c)
{
    long ret;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(n), "D"(a), "S"(b), "d"(c)
                     : "rcx", "r11", "memory");
    return ret;
}

#define SYS_WRITE 1

static unsigned long xstrlen(const char *s)
{
    unsigned long n = 0;
    while (s[n])
        n++;
    return n;
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (argv[i])
            sys3(SYS_WRITE, 1, (long)argv[i], (long)xstrlen(argv[i]));
        if (i + 1 < argc)
            sys3(SYS_WRITE, 1, (long)" ", 1);
    }
    sys3(SYS_WRITE, 1, (long)"\n", 1);
    return 0;
}
