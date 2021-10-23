#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void *KeCopyMemory(void *restrict s1, const void *restrict s2, size_t n);
void *KeMoveMemory(void *s1, const void *s2, size_t n);
void *KeSetMemory(void *s, char c, size_t n);
int KeCompareMemory(const void *s1, const void *s2, size_t n);

size_t KeStringLength(const char *s);
int KeCompareString(const char *s1, const char *s2);
int KeCompareStringEx(const char *s1, const char *s2, size_t n);
char *KeGetCharPtrString(const char *s, char c);
size_t KeSpanString(const char *s, const char *accept);
char *KeBreakCharPtrString(char *s, const char *accept);
char *KeCopyString(char *restrict s1, const char *restrict s2);
char *KeCopyStringEx(char *restrict s1, const char *restrict s2, size_t n);
char *KeConcatString(char *restrict s1, const char *restrict s2);
char *KeConcatStringEx(char *restrict s1, const char *restrict s2, size_t n);
const char *KeFindStringString(const char *restrict haystack, const char *restrict needle);
int KeConvertStringToInt(const char *s);

#endif