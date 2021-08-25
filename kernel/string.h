#ifndef STRING_H
#define STRING_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, char c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, char c);
size_t strspn(const char *s, const char *accept);
char *strpbrk(char *s, const char *accept);
char *strcpy(char *s1, const char *s2);
char *strncpy(char *s1, const char *s2, size_t n);
char *strcat(char *dest, const char *src);
const char *strstr(const char *haystack, const char *needle);
int atoi(const char *s);

#ifdef __cplusplus
}
#endif
#endif