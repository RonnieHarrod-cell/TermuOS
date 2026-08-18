#include "../include/unistd.h"
#include "../include/syscall.h"

ssize_t read(int fd, void *buf, size_t count)
{
    return (ssize_t)__syscall3(SYS_READ, fd, (long)buf, (long)count);
}
