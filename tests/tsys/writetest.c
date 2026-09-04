/*
 * writetest - write() routing and file writes (issue #42).
 *
 * Checks that write() reaches the console for fd 1 and 2, reaches the file
 * for anything above them, refuses what it should, and that bytes written
 * to a file survive a close and reopen. Prints PASS/FAIL per check.
 */
#include <unistd.h>
#include <syscall.h>
#include <string.h>

#define SCRATCH "/etc/scratch.txt"
#define NEWFILE "/etc/created-by-writetest.txt"
#define BIG 1500

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

/* write_all loops over short writes, which the kernel is allowed to return. */
static long write_all(int fd, const char *p, long n)
{
    long done = 0;
    while (done < n)
    {
        long w = write(fd, p + done, (size_t)(n - done));
        if (w <= 0)
            return w ? w : done;
        done += w;
    }
    return done;
}

int main(void)
{
    char buf[BIG + 64];
    const char *msg = "written-by-writetest";
    long msglen = (long)strlen(msg);
    long n;
    int fd, i, bad;

    /* fd 1 and 2 go to the console. */
    result("write(1, \"hi\\n\", 3)", write(1, "hi\n", 3) == 3, 3);
    result("write(2) reaches console", write(2, "", 0) == 0, 0);
    result("zero-length write returns 0", write(1, "x", 0) == 0, 0);

    /* An existing file: write, close, reopen, read the bytes back. */
    fd = open(SCRATCH, O_WRONLY);
    result("open existing file O_WRONLY", fd > 2, fd);
    if (fd < 0)
    {
        put("writetest: cannot open " SCRATCH ", giving up\n");
        return 1;
    }
    n = write_all(fd, msg, msglen);
    result("write to file returns len", n == msglen, n);
    close(fd);

    fd = open(SCRATCH, O_RDONLY);
    n = read(fd, buf, sizeof buf);
    bad = (n < msglen);
    for (i = 0; !bad && i < msglen; i++)
        if (buf[i] != msg[i])
            bad = 1;
    result("bytes survive close and reopen", !bad, n);
    close(fd);

    /* More than one capped write, to check the offset advances. */
    for (i = 0; i < BIG; i++)
        buf[i] = (char)('A' + (i % 26));
    fd = open(SCRATCH, O_WRONLY);
    n = write_all(fd, buf, BIG);
    result("large write completes via short writes", n == BIG, n);
    close(fd);

    fd = open(SCRATCH, O_RDONLY);
    long total = 0;
    bad = 0;
    while ((n = read(fd, buf, 256)) > 0)
    {
        for (i = 0; i < n; i++)
            if (total + i < BIG && buf[i] != (char)('A' + ((total + i) % 26)))
                bad = 1;
        total += n;
    }
    result("large write read back intact", !bad && total >= BIG, total);
    close(fd);

    /* A file that does not exist yet: O_CREAT must actually create it. */
    fd = open(NEWFILE, O_WRONLY | O_CREAT);
    result("open O_CREAT makes a new file", fd > 2, fd);
    if (fd > 2)
    {
        n = write_all(fd, msg, msglen);
        result("write to created file", n == msglen, n);
        close(fd);

        fd = open(NEWFILE, O_RDONLY);
        n = read(fd, buf, sizeof buf);
        bad = (n != msglen);
        for (i = 0; !bad && i < msglen; i++)
            if (buf[i] != msg[i])
                bad = 1;
        result("created file reads back", !bad, n);
        close(fd);
    }

    /* Descriptors write() must refuse. */
    result("reject write to fd 0", write(0, "x", 1) < 0, write(0, "x", 1));
    result("reject write to stale fd", write(9, "x", 1) < 0, write(9, "x", 1));

    put("writetest done\n");
    return 0;
}
