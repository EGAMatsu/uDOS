#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#define RtlCopyMemory _Zrm000
void *RtlCopyMemory(void *restrict s1, const void *restrict s2, size_t n);
#define RtlMoveMemory _Zrm001
void *RtlMoveMemory(void *s1, const void *s2, size_t n);
#define RtlSetMemory _Zrm002
void *RtlSetMemory(void *s, char c, size_t n);
#define RtlCompareMemory _Zrm003
int RtlCompareMemory(const void *s1, const void *s2, size_t n);

#define RtlStringLength _Zrs000
size_t RtlStringLength(const char *s);
#define RtlCompareString _Zrs001
int RtlCompareString(const char *s1, const char *s2);
#define RtlCompareStringEx _Zrs002
int RtlCompareStringEx(const char *s1, const char *s2, size_t n);
#define RtlGetCharPtrString _Zrs003
char *RtlGetCharPtrString(const char *s, char c);
#define RtlSpanString _Zrs004
size_t RtlSpanString(const char *s, const char *accept);
#define RtlBreakCharPtrString _Zrs005
char *RtlBreakCharPtrString(char *s, const char *accept);
#define RtlCopyString _Zrs006
char *RtlCopyString(char *restrict s1, const char *restrict s2);
#define RtlCopyStringEx _Zrs007
char *RtlCopyStringEx(char *restrict s1, const char *restrict s2, size_t n);
#define RtlConcatString _Zrs008
char *RtlConcatString(char *restrict s1, const char *restrict s2);
#define RtlConcatStringEx _Zrs009
char *RtlConcatStringEx(char *restrict s1, const char *restrict s2, size_t n);
#define RtlFindStringString _Zrs010
const char *RtlFindStringString(const char *restrict haystack,
    const char *restrict needle);
#define RtlConvertStringToInt _Zrs011
int RtlConvertStringToInt(const char *s);

#endif
