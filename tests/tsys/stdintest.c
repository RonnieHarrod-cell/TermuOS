/* Minimal: block in read(0) for one keystroke, echo it. No open(), no files. */
#include <unistd.h>
#include <string.h>
int main(void)
{
    char c = 0;
    write(1, "stdintest: waiting for a key\n", 29);
    long n = read(0, &c, 1);
    write(1, "stdintest: got ", 15);
    if (n == 1) write(1, &c, 1);
    write(1, "\nstdintest: done\n", 17);
    return 0;
}
