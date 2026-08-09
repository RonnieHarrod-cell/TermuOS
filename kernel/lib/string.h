#ifndef STRING_H
#define STRING_H

#include <stddef.h>

int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, size_t n);
void strcpy(char* dst, const char* src);
void* memcpy(void* dst, const void* src, size_t n);
void *memset(void *s, int c, unsigned long n);

#endif