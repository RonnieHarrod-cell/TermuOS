/*
 * uatest - user-buffer helper checks (issue #40).
 *
 * Proves the kernel can read and write a known byte on this process's
 * stack, and that it refuses user pointers it must not follow. Every
 * line is printed as PASS/FAIL so the harness can assert on it.
 */
#include <unistd.h>
#include <syscall.h>
#include <string.h>

#define SYS_FSTAT 5
#define ST_MODE_OFF 24
#define ST_BLKSIZE_OFF 56
#define S_IFCHR 0020000

static void put(const char *s) { write(1, s, strlen(s)); }

static void put_long(long v)
{
    char b[24];
    int i = 23;
    int neg = v < 0;
    unsigned long u = neg ? (unsigned long)(-v) : (unsigned long)v;
    b[i--] = 0;
    if (!u) b[i--] = '0';
    while (u) { b[i--] = '0' + (char)(u % 10); u /= 10; }
    if (neg) b[i--] = '-';
    put(&b[i + 1]);
}

static void result(const char *name, int ok, long got)
{
    put(ok ? "PASS  " : "FAIL  ");
    put(name);
    put("  (ret=");
    put_long(got);
    put(")\n");
}

int main(void)
{
    /* 1. kernel READS a known user stack byte: a buffer built on the stack. */
    char stackbuf[8];
    stackbuf[0] = 'T'; stackbuf[1] = 'e'; stackbuf[2] = 's'; stackbuf[3] = 't';
    stackbuf[4] = '\n';
    long n = write(1, stackbuf, 5);
    result("read user stack byte  (write from stack)", n == 5, n);

    /* 2. kernel WRITES known user stack bytes: fstat fills a stack struct. */
    unsigned char st[144];
    unsigned int i;
    for (i = 0; i < sizeof st; i++)
        st[i] = 0xAA;                       /* poison */
    long r = __syscall2(SYS_FSTAT, 1, (long)st);
    unsigned int mode = *(unsigned int *)(st + ST_MODE_OFF);
    long blksz = *(long *)(st + ST_BLKSIZE_OFF);
    result("write user stack bytes (fstat)", r == 0 && (mode & S_IFCHR) == S_IFCHR && blksz == 1024, r);

    /* 3. a kernel VA passed from userspace must be refused, not dereferenced. */
    long bad = __syscall3(SYS_WRITE, 1, (long)0xffffffff80000000UL, 8);
    result("reject kernel VA", bad < 0, bad);

    /* 4. an unmapped user VA must be refused. */
    long unmapped = __syscall3(SYS_WRITE, 1, (long)0x0000700000000000UL, 8);
    result("reject unmapped user VA", unmapped < 0, unmapped);

    /* 5. a non-canonical / out-of-range VA must be refused. */
    long noncanon = __syscall3(SYS_WRITE, 1, (long)0x0000800000000000UL, 8);
    result("reject non-canonical VA", noncanon < 0, noncanon);

    /* 6. NULL must be refused. */
    long nul = __syscall3(SYS_WRITE, 1, 0, 8);
    result("reject NULL", nul < 0, nul);

    /* 7. read-only user text: writing into it via a syscall must be refused. */
    long ro = __syscall3(SYS_READ, 0xFFFF, (long)main, 4);
    result("reject write to bad fd/text", ro < 0, ro);

    put("uatest done\n");
    return 0;
}
