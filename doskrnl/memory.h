#ifndef STRING_H
#define STRING_H

#include <stddef.h>

#define KeCopyMemory _Zcrcpym
void *KeCopyMemory(void *restrict s1, const void *restrict s2, size_t n);
#define KeMoveMemory _Zcrmovm
void *KeMoveMemory(void *s1, const void *s2, size_t n);
#define KeSetMemory _Zcrsetm
void *KeSetMemory(void *s, char c, size_t n);
#define KeCompareMemory _Zcrcmpm
int KeCompareMemory(const void *s1, const void *s2, size_t n);

#define KeStringLength _Zcrslen
size_t KeStringLength(const char *s);
#define KeCompareString _Zcrcpstr
int KeCompareString(const char *s1, const char *s2);
#define KeCompareStringEx _Zcrcmpse
int KeCompareStringEx(const char *s1, const char *s2, size_t n);
#define KeGetCharPtrString _Zcrgcs
char *KeGetCharPtrString(const char *s, char c);
#define KeSpanString _Zcrspns
size_t KeSpanString(const char *s, const char *accept);
#define KeBreakCharPtrString _Zcrbrkc
char *KeBreakCharPtrString(char *s, const char *accept);
#define KeCopyString _Zcrcpys
char *KeCopyString(char *restrict s1, const char *restrict s2);
#define KeCopyStringEx _Zcrcpse
char *KeCopyStringEx(char *restrict s1, const char *restrict s2, size_t n);
#define KeConcatString _Zcrcats
char *KeConcatString(char *restrict s1, const char *restrict s2);
#define KeConcatStringEx _Zcrctse
char *KeConcatStringEx(char *restrict s1, const char *restrict s2, size_t n);
#define KeFindStringString _Zcrfss
const char *KeFindStringString(const char *restrict haystack,
    const char *restrict needle);
#define KeConvertStringToInt _Zcrcvti
int KeConvertStringToInt(const char *s);

#endif
