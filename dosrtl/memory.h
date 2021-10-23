#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void *RtlCopyMemory(void *restrict s1, const void *restrict s2, size_t n);
void *RtlMoveMemory(void *s1, const void *s2, size_t n);
void *RtlSetMemory(void *s, char c, size_t n);
int RtlCompareMemory(const void *s1, const void *s2, size_t n);

size_t RtlStringLength(const char *s);
int RtlCompareString(const char *s1, const char *s2);
int RtlCompareStringEx(const char *s1, const char *s2, size_t n);
char *RtlGetCharPtrString(const char *s, char c);
size_t RtlSpanString(const char *s, const char *accept);
char *RtlBreakCharPtrString(char *s, const char *accept);
char *RtlCopyString(char *restrict s1, const char *restrict s2);
char *RtlCopyStringEx(char *restrict s1, const char *restrict s2, size_t n);
char *RtlConcatString(char *restrict s1, const char *restrict s2);
char *RtlConcatStringEx(char *restrict s1, const char *restrict s2, size_t n);
const char *RtlFindStringString(const char *restrict haystack, const char *restrict needle);
int RtlConvertStringToInt(const char *s);

#endif