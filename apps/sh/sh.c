#include "../lib/sys.h"

static void print(const char *s)
{
    long n = 0;
    while (s[n])
        n++;
    sys_write(1, s, n);
}

static int readline(char *buf, int max)
{
    int i = 0;
    while (i < max - 1)
    {
        char c;
        long n = sys_read(0, &c, 1);
        if (n <= 0)
            break;
        if (c == '\r')
            continue;
        if (c == '\n')
        {
            buf[i] = 0;
            sys_write(1, "\n", 1);
            return i;
        }
        if (c == '\b' || c == 127)
        {
            if (i > 0)
            {
                i--;
                sys_write(1, "\b \b", 3);
            }
            continue;
        }
        buf[i++] = c;
        sys_write(1, &c, 1);
    }
    buf[i] = 0;
    return i;
}

static int streq(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }
    return *a == *b;
}

static void run_prog(const char *path)
{
    long pid = sys_spawn(path);
    if (pid < 0)
    {
        print("spawn failed\n");
        return;
    }
    sys_wait(pid);
}

void _start(void)
{
    char line[128];

    print("TermuOS userspace sh\n");

    for (;;)
    {
        print("TermuOS# ");
        readline(line, sizeof line);

        if (line[0] == 0)
            continue;

        if (streq(line, "exit"))
            sys_exit(0);

        // echo
        if (line[0] == 'e' && line[1] == 'c' && line[2] == 'h' &&
            line[3] == 'o' && (line[4] == 0 || line[4] == ' '))
        {
            if (line[4] == ' ')
                print(line + 5);
            print("\n");
            continue;
        }

        if (line[0] == '/')
        {
            char path[128];
            const char *prefix = "/mnt/bin/";
            int i = 0, j = 0;
            while (prefix[i] && j < 120)
                path[j++] = prefix[i++];
            i = 0;
            while (line[i] && line[i] != ' ' && j < 126)
                path[j++] = line[i++];
            path[j] = 0;
            run_prog(path);
        }

        print("unknown command\n");
    }
}
