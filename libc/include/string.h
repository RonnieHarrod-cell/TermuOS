#pragma once

typedef unsigned long size_t;

size_t strlen(const char *s);
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int strcmp(const char *a, const char *b);
char *strcpy(char *dst, const char *src);
