#include <ctype.h>
#include <string.h>

void *memcpy(
    void *restrict dest,
    const void *restrict src,
    size_t n)
{
    const char *c_src = (const char *)src;
    char *c_dest = (char *)dest;
    while(n) {
        *(c_dest++) = *(c_src++);
        --n;
    }
    return dest;
}

void *memmove(
    void *dest,
    const void *src,
    size_t n)
{
    const char *c_src = (const char *)src;
    char *c_dest = (char *)dest;

    if((ptrdiff_t)c_dest < (ptrdiff_t)c_src) {
        while(n) {
            *(c_dest++) = *(c_src++);
            --n;
        }
    } else {
        c_dest += n;
        c_src += n;
        while(n) {
            *(c_dest--) = *(c_src--);
            --n;
        }
    }
    return c_dest;
}

void *memset(
    void *s,
    char c,
    size_t n)
{
    char *c_s = (char *)s;
    while(n) {
        *(c_s++) = c;
        --n;
    }
    return s;
}

int memcmp(
    const void *s1,
    const void *s2,
    size_t n)
{
    int diff = 0;
    const char *_s1 = (const char *)s1;
    const char *_s2 = (const char *)s2;

    while(n) {
        diff += *(_s1++) - *(_s2++);
        --n;
    }
    return diff;
}

size_t strlen(
    const char *s)
{
    size_t i = 0;
    while(*s != '\0') {
        ++i;
        ++s;
    }
    return i;
}

int strcmp(
    const char *s1,
    const char *s2)
{
    size_t n = strlen(s1);
    int diff = 0;

    while(n && *s1 != '\0' && *s2 != '\0') {
        diff += *(s1++) - *(s2++);
        --n;
    }

    if(*s1 != *s2) {
        return -1;
    }
    return diff;
}

int strncmp(
    const char *s1,
    const char *s2,
    size_t n)
{
    int diff = 0;

    while(n && *s1 != '\0' && *s2 != '\0') {
        diff += *(s1++) - *(s2++);
        --n;
    }

    if(*s1 != *s2 && n > 0) {
        return -1;
    }
    return diff;
}

char *strchr(
    const char *s,
    char c)
{
    while(*s != '\0' && *s != c) {
        ++s;
    }

    if(*s == '\0') {
        return NULL;
    }
    return (char *)s;
}

size_t strspn(
    const char *s,
    const char *accept)
{
    size_t spn = 0;

    while(*s != '\0') {
        char is_match = 0;
        size_t i;

        for(i = 0; i < strlen(accept); i++) {
            if(*s != accept[i]) {
                continue;
            }

            is_match = 1;
            ++spn;
            break;
        }

        if(!is_match) {
            return spn;
        }
        ++s;
    }
    return spn;
}

char *strpbrk(
    char *s,
    const char *accept)
{
    while(*s != '\0') {
        size_t i;
        for(i = 0; i < strlen(accept); i++) {
            if(*s == accept[i]) {
                return s;
            }
        }
        ++s;
    }
    return s;
}

char *strcpy(
    char *restrict s1,
    const char *restrict s2)
{
    while(*s2 != '\0') {
        *(s1++) = *(s2++);
    }
    *(s1++) = '\0';
    return s1;
}

char *strncpy(
    char *restrict s1,
    const char *restrict s2,
    size_t n)
{
    while(n && *s2 != '\0') {
        *(s1++) = *(s2++);
        --n;
    }
    *(s1++) = '\0';
    return s1;
}

char *strcat(
    char *restrict s1,
    const char *restrict s2)
{
    while(*s1 != '\0') {
        ++s1;
    }

    while(*s2 != '\0') {
        *(s1++) = *(s2++);
    }
    *(s1++) = '\0';
    return s1;
}

char *strncat(
    char *restrict s1,
    const char *restrict s2,
    size_t n)
{
    while(*s1 != '\0') {
        ++s1;
    }

    while(*s2 != '\0' && n) {
        *(s1++) = *(s2++);
        --n;
    }
    *(s1++) = '\0';
    return s1;
}

const char *strstr(
    const char *restrict haystack,
    const char *restrict needle)
{
    while(*haystack != '\0') {
        if(*haystack == *needle) {
            if(!strncmp(haystack, needle, strlen(needle))) {
                return (char *)haystack;
            }
        }
        ++haystack;
    }
    return NULL;
}

int atoi(
    const char *s)
{
    char is_neg = 0;
    int num = 0;

    s += strspn(s, " \t");
    if(*s == '-') {
        is_neg = 1;
        ++s;
    }

    while(isdigit(*s)) {
        num *= 10;
        num += *s - '0';
        ++s;
    }

    if(is_neg) {
        num = -num;
    }
    return num;
}