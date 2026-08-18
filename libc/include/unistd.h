#pragma once

typedef long ssize_t;
typedef unsigned long size_t;

ssize_t write(int fd, const void *buf, size_t count);
void _exit(int status) __attribute__((noreturn));
