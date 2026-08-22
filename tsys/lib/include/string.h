#pragma once

#include <stddef.h>

size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strcpy(char *d, const char *s);
char *strncpy(char *d, const char *s, size_t n);
void *memcpy(void *d, const void *s, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *d, const void *s, size_t n);