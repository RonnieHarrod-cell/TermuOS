#include "../include/unistd.h"
#include "../include/syscall.h"

void _exit(int status)
{
    __syscall3(SYS_EXIT, status, 0, 0);
    for (;;)
        ;
}