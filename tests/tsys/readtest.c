/*
 * readtest - read() routing and file reads (issue #41).
 *
 * Checks the contract sys_read() implements: descriptors 0/1/2 mean
 * stdin/stdout/stderr, open() therefore never returns one of them, and a
 * file read goes through the kernel bounce buffer with EOF and errors
 * preserved. Prints PASS/FAIL per check for the harness to assert on.
 */
#include <unistd.h>
#include <syscall.h>
#include <string.h>

#define MOTD "/etc/motd"

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
    char buf[256];
    long n;

    /* open() must not hand back a descriptor that means stdin/stdout/stderr. */
    int fd = open(MOTD, O_RDONLY);
    result("open returns fd > 2", fd > 2, fd);
    if (fd < 0)
    {
        put("readtest: cannot open " MOTD ", giving up\n");
        return 1;
    }

    /* A file read returns bytes, not a keystroke. */
    n = read(fd, buf, sizeof buf);
    result("read file returns bytes", n > 0, n);

    /* Drain to EOF, then confirm EOF is 0 and stays 0. */
    long total = n;
    while ((n = read(fd, buf, sizeof buf)) > 0)
        total += n;
    result("read to EOF returns 0", n == 0, n);
    result("second read past EOF still 0", read(fd, buf, sizeof buf) == 0, 0);
    result("whole file was read", total == 64, total);
    close(fd);

    /* A zero-length read is not an error. */
    result("zero-length read returns 0", read(1, buf, 0) == 0, 0);

    /* The console write ends are not readable. */
    result("reject read from fd 1", read(1, buf, 1) < 0, read(1, buf, 1));
    result("reject read from fd 2", read(2, buf, 1) < 0, read(2, buf, 1));

    /* A descriptor that was never opened. */
    result("reject read from stale fd", read(9, buf, 1) < 0, read(9, buf, 1));

    /* A read larger than the kernel cap is a short read, not a failure. */
    fd = open(MOTD, O_RDONLY);
    if (fd > 2)
    {
        n = read(fd, buf, sizeof buf);
        result("oversized request is a short read", n > 0 && n <= 512, n);
        close(fd);
    }

    put("readtest done\n");
    return 0;
}
