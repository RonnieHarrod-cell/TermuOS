#pragma once

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_EXIT 60

#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR 0x03
#define O_CREAT 0x40

long __syscall0(long n);
long __syscall1(long n, long a);
long __syscall2(long n, long a, long b);
long __syscall3(long n, long a, long b, long c);
