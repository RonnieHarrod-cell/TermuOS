#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    const char *s = "TermuOS\n";
    write(1, s, strlen(s));
    return 0;
}
