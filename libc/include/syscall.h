#pragma once

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_EXIT 60

long __syscall3(long n, long a, long b, long c);
