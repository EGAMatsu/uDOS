#include <memory.h>
#include <assert.h>

#define abs(x) (((x) < 0) ? (-x) : (x))

void *KeCopyMemory(void *dest, const void *src, size_t n)
{
    const char *c_src = (const char *)src;
    char *c_dest = (char *)dest;

    /* Check for overlapping argument pointers - copymemory wasn't made for overlapped memory */
    DEBUG_ASSERT_MSG(!(abs((ptrdiff_t)dest - (ptrdiff_t)src) < n), "Overlapping memory");

    while(n) {
        *(c_dest++) = *(c_src++);
        --n;
    }
    return dest;
}

void *KeMoveMemory(void *dest, const void *src, size_t n)
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

void *KeSetMemory(void *s, char c, size_t n)
{
    char *c_s = (char *)s;
    while(n) {
        *(c_s++) = c;
        --n;
    }
    return s;
}

int KeCompareMemory(const void *s1, const void *s2, size_t n)
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

size_t KeStringLength(const char *s)
{
    size_t i = 0;
    while(*s != '\0') {
        ++i;
        ++s;
    }
    return i;
}

int KeCompareString(const char *s1, const char *s2)
{
    size_t n = KeStringLength(s1);
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

int KeCompareStringEx(const char *s1, const char *s2, size_t n)
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

char *KeGetCharPtrString(const char *s, char c)
{
    while(*s != '\0' && *s != c) {
        ++s;
    }

    if(*s == '\0') {
        return NULL;
    }
    return (char *)s;
}

size_t KeSpanString(const char *s, const char *accept)
{
    size_t spn = 0;

    while(*s != '\0') {
        char is_match = 0;
        size_t i;

        for(i = 0; i < KeStringLength(accept); i++) {
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

char *KeBreakCharPtrString(char *s, const char *accept)
{
    while(*s != '\0') {
        size_t i;
        for(i = 0; i < KeStringLength(accept); i++) {
            if(*s == accept[i]) {
                return s;
            }
        }
        ++s;
    }
    return s;
}

char *KeCopyString(char *s1, const char *s2)
{
    while(*s2 != '\0') {
        *(s1++) = *(s2++);
    }
    *(s1++) = '\0';
    return s1;
}

char *KeCopyStringEx(char *s1, const char *s2, size_t n)
{
    while(n && *s2 != '\0') {
        *(s1++) = *(s2++);
        --n;
    }
    *(s1++) = '\0';
    return s1;
}

char *KeConcatString(char *s1, const char *s2)
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

char *KeConcatStringEx(char *s1, const char *s2, size_t n)
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

const char *KeFindStringString(const char *haystack, const char *needle)
{
    size_t needle_len = KeStringLength(needle);
    while(*haystack != '\0') {
        if(*haystack == *needle) {
            if(!KeCompareStringEx(haystack, needle, needle_len)) {
                return (char *)haystack;
            }
        }
        ++haystack;
    }
    return NULL;
}

int KeConvertStringToInt(const char *s)
{
    char is_neg = 0;
    int num = 0;

    s += KeSpanString(s, " \t");
    if(*s == '-') {
        is_neg = 1;
        ++s;
    }

    while(*s >= '0' && *s <= '9') {
        num *= 10;
        num += *s - '0';
        ++s;
    }

    if(is_neg) {
        num = -num;
    }
    return num;
}
