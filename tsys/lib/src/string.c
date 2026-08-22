#include <string.h>

size_t strlen(const char *s)
{
    size_t n = 0;
    while (s[n])
        n++;
    return n;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
    while (n && *a && *a == *b)
    {
        a++;
        b++;
        n--;
    }
    if (n == 0)
        return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

char *strcpy(char *d, const char *s)
{
    char *o = d;
    while ((*d++ = *s++))
        ;
    return o;
}

char *strncpy(char *d, const char *s, size_t n)
{
    size_t i;
    for (i = 0; i < n && s[i]; i++)
        d[i] = s[i];
    for (; i < n; i++)
        d[i] = 0;
    return d;
}

void *memcpy(void *d, const void *s, size_t n)
{
    unsigned char *dd = d;
    const unsigned char *ss = s;
    while (n--)
        *dd++ = *ss++;
    return d;
}

void *memset(void *s, int c, size_t n)
{
    unsigned char *p = s;
    while (n--)
        *p++ = (unsigned char)c;
    return s;
}

void *memmove(void *d, const void *s, size_t n)
{
    unsigned char *dd = d;
    const unsigned char *ss = s;
    if (dd < ss)
    {
        while (n--)
            *dd++ = *ss++;
    }
    else if (dd > ss)
    {
        dd += n;
        ss += n;
        while (n--)
            *--dd = *--ss;
    }
    return d;
}
