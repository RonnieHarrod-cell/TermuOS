#include <unistd.h>
#include <syscall.h>
#include <string.h>

int main(int argc, char **argv)
{
    char buf[256];
    int i;

    if (argc < 2)
    {
        // stdin not wired - require a path
        const char *msg = "usage: cat <file>\n";
        write(2, msg, strlen(msg));
        return 1;
    }

    for (i = 1; i < argc; i++)
    {
        int fd = open(argv[i], 0x01);
        if (fd < 0)
        {
            write(2, "cat: open failed\n", 17);
            return 1;
        }
        for (;;)
        {
            ssize_t n = read(fd, buf, sizeof buf);
            if (n <= 0)
                break;
            write(1, buf, (size_t)n);
        }
        close(fd);
    }
    return 0;
}
