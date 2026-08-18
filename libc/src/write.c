#include "../include/unistd.h"
#include "../include/syscall.h"

ssize_t write(int fd, const void *buf, size_t count)
{
    return (ssize_t)__syscall3(SYS_WRITE, fd, (long)buf, (long)count);
}
