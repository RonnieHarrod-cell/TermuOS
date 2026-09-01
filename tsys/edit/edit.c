#include <unistd.h>
#include <syscall.h>
#include <string.h>

#define CAP (64 * 1024)
#define ROWS 24
#define COLS 80

static char buf[CAP];
static int len;
static int cur;
static char path[256];
static int dirty;

static void clear_screen(void)
{
    /* ANSI — harmless if your console ignores it */
    write(1, "\033[2J\033[H", 7);
}

static void draw(void)
{
    int i, row, col;

    clear_screen();
    write(1, "TermuOS edit  ", 14);
    write(1, path, strlen(path));
    if (dirty)
        write(1, " *", 2);
    write(1, "\n^S save  ^Q quit\n\n", 19);

    row = 0;
    col = 0;
    for (i = 0; i < len && row < ROWS - 4; i++)
    {
        char c = buf[i];
        if (c == '\n')
        {
            write(1, "\n", 1);
            row++;
            col = 0;
        }
        else
        {
            write(1, &c, 1);
            col++;
            if (col >= COLS)
            {
                write(1, "\n", 1);
                row++;
                col = 0;
            }
        }
    }
    write(1, "\n", 1);
}

static int load(const char *p)
{
    int fd;
    int n;

    len = 0;
    cur = 0;
    dirty = 0;

    strncpy(path, p, sizeof path - 1);
    path[sizeof path - 1] = 0;

    fd = open(p, O_RDONLY);
    if (fd < 0)
    {
        /* new empty file */
        len = 0;
        return 0;
    }
    while (len < CAP)
    {
        n = (int)read(fd, buf + len, (size_t)(CAP - len));
        if (n <= 0)
            break;
        len += n;
    }
    close(fd);
    if (cur > len)
        cur = len;
    return 0;
}

static int save(void)
{
    int fd;
    int off;
    int n;

    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        write(1, "\nsave: open failed\n", 19);
        return -1;
    }
    off = 0;
    while (off < len)
    {
        n = (int)write(fd, buf + off, (size_t)(len - off));
        if (n <= 0)
        {
            close(fd);
            write(1, "\nsave: write failed\n", 20);
            return -1;
        }
        off += n;
    }
    close(fd);
    dirty = 0;
    return 0;
}

static int getkey(void)
{
    char c;
    if (read(0, &c, 1) != 1)
        return -1;
    return (unsigned char)c;
}

static void insert(char c)
{
    int i;
    if (len >= CAP - 1)
        return;
    for (i = len; i > cur; i--)
        buf[i] = buf[i - 1];
    buf[cur] = c;
    len++;
    cur++;
    dirty = 1;
}

static void backspace(void)
{
    int i;
    if (cur <= 0)
        return;
    for (i = cur - 1; i < len - 1; i++)
        buf[i] = buf[i + 1];
    len--;
    cur--;
    dirty = 1;
}

int main(int argc, char **argv)
{
    int k;

    if (argc < 2)
    {
        write(2, "usage: edit <file>\n", 19);
        return 1;
    }
    load(argv[1]);

    for (;;)
    {
        draw();
        k = getkey();
        if (k < 0)
            break;

        if (k == 17)
        { /* Ctrl-Q */
            break;
        }
        if (k == 19)
        { /* Ctrl-S */
            save();
            continue;
        }
        if (k == 127 || k == 8)
        {
            backspace();
            continue;
        }
        if (k == '\r')
            k = '\n';
        if (k >= 32 || k == '\n')
            insert((char)k);
    }

    clear_screen();
    return 0;
}