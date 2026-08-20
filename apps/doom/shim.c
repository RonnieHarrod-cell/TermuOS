#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_EXIT 60

/* match your kernel open flags if different */
#define O_RDONLY 0

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

/* ---- heap ---- */
#define HEAP_SIZE (8 * 1024 * 1024)
static unsigned char heap[HEAP_SIZE];
static unsigned long heap_pos;

void *malloc(unsigned long n)
{
    n = (n + 15UL) & ~15UL;
    if (heap_pos + n > HEAP_SIZE)
        return 0;
    void *p = &heap[heap_pos];
    heap_pos += n;
    return p;
}

void free(void *p) { (void)p; }

void *calloc(unsigned long n, unsigned long sz)
{
    unsigned long t = n * sz;
    unsigned char *p = malloc(t);
    if (!p)
        return 0;
    for (unsigned long i = 0; i < t; i++)
        p[i] = 0;
    return p;
}

void *realloc(void *p, unsigned long n)
{
    if (!p)
        return malloc(n);
    void *q = malloc(n);
    if (!q)
        return 0;
    /* no old size — good enough only if rarely used; improve later */
    (void)p;
    return q;
}

/* ---- strings / mem ---- */
void *memset(void *s, int c, unsigned long n)
{
    unsigned char *p = s;
    while (n--)
        *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *d, const void *s, unsigned long n)
{
    unsigned char *dd = d;
    const unsigned char *ss = s;
    while (n--)
        *dd++ = *ss++;
    return d;
}

void *memmove(void *d, const void *s, unsigned long n)
{
    unsigned char *dd = d;
    const unsigned char *ss = s;
    if (dd < ss)
        while (n--)
            *dd++ = *ss++;
    else
    {
        dd += n;
        ss += n;
        while (n--)
            *--dd = *--ss;
    }
    return d;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, unsigned long n)
{
    while (n && *a && *a == *b)
    {
        a++;
        b++;
        n--;
    }
    if (n == 0)
        return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

unsigned long strlen(const char *s)
{
    unsigned long n = 0;
    while (s[n])
        n++;
    return n;
}

char *strcpy(char *d, const char *s)
{
    char *o = d;
    while ((*d++ = *s++))
        ;
    return o;
}

char *strncpy(char *d, const char *s, unsigned long n)
{
    unsigned long i;
    for (i = 0; i < n && s[i]; i++)
        d[i] = s[i];
    for (; i < n; i++)
        d[i] = 0;
    return d;
}

char *strcat(char *d, const char *s)
{
    char *o = d;
    while (*d)
        d++;
    while ((*d++ = *s++))
        ;
    return o;
}

int atoi(const char *s)
{
    int n = 0, neg = 0;
    if (*s == '-')
    {
        neg = 1;
        s++;
    }
    while (*s >= '0' && *s <= '9')
        n = n * 10 + (*s++ - '0');
    return neg ? -n : n;
}

int abs(int x) { return x < 0 ? -x : x; }

/* ---- stdio-ish ---- */
typedef struct
{
    int fd;
    int used;
} FILE;

FILE *stdin;
FILE *stdout;
FILE *stderr;

static FILE files[16];

static void stdio_init(void)
{
    files[0].fd = 0;
    files[0].used = 1;
    files[1].fd = 1;
    files[1].used = 1;
    files[2].fd = 2;
    files[2].used = 1;
    stdin = &files[0];
    stdout = &files[1];
    stderr = &files[2];
}

FILE *fopen(const char *path, const char *mode)
{
    (void)mode;
    long fd = sys3(SYS_OPEN, (long)path, O_RDONLY, 0);
    if (fd < 0)
        return 0;
    for (int i = 3; i < 16; i++)
    {
        if (!files[i].used)
        {
            files[i].used = 1;
            files[i].fd = (int)fd;
            return &files[i];
        }
    }
    return 0;
}

int fclose(FILE *f)
{
    if (!f || !f->used)
        return -1;
    sys3(SYS_CLOSE, f->fd, 0, 0);
    f->used = 0;
    return 0;
}

unsigned long fread(void *ptr, unsigned long size, unsigned long nmemb, FILE *f)
{
    if (!f)
        return 0;
    long n = sys3(SYS_READ, f->fd, (long)ptr, (long)(size * nmemb));
    if (n < 0)
        return 0;
    return (unsigned long)n / (size ? size : 1);
}

unsigned long fwrite(const void *ptr, unsigned long size, unsigned long nmemb, FILE *f)
{
    if (!f)
        return 0;
    long n = sys3(SYS_WRITE, f->fd, (long)ptr, (long)(size * nmemb));
    if (n < 0)
        return 0;
    return (unsigned long)n / (size ? size : 1);
}

int fseek(FILE *f, long off, int whence)
{
    (void)f;
    (void)off;
    (void)whence;
    return -1; /* Phase 1: may break some paths; fix with lseek later */
}

long ftell(FILE *f)
{
    (void)f;
    return -1;
}

int feof(FILE *f)
{
    (void)f;
    return 0;
}

int fprintf(FILE *f, const char *fmt, ...)
{
    (void)f;
    (void)fmt;
    return 0; /* silence or implement later */
}

int printf(const char *fmt, ...)
{
    /* very dumb: write fmt literally if no % for bring-up */
    const char *p = fmt;
    while (*p && *p != '%')
        p++;
    if (*p == '\0')
        sys3(SYS_WRITE, 1, (long)fmt, (long)strlen(fmt));
    return 0;
}

int puts(const char *s)
{
    sys3(SYS_WRITE, 1, (long)s, (long)strlen(s));
    sys3(SYS_WRITE, 1, (long)"\n", 1);
    return 0;
}

int putchar(int c)
{
    char ch = (char)c;
    sys3(SYS_WRITE, 1, (long)&ch, 1);
    return c;
}

void exit(int code)
{
    sys3(SYS_EXIT, code, 0, 0);
    for (;;)
        ;
}

void abort(void) { exit(1); }

/* call once from main before doom */
void termuos_shim_init(void) { stdio_init(); }
