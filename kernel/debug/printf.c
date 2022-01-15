#include <stddef.h>
#include <stdint.h>
#include <Debug/Printf.h>
#include <Memory.h>
#include <Fs/Fs.h>
#include <Hdebug.h>

struct FsHandle *g_stdout_fd = NULL, *g_stdin_fd = NULL;

int kgetc(
    void)
{
    char ch;
    if(g_stdin_fd == NULL) {
        return '\0';
    }

    KeReadFsNode(g_stdin_fd, &ch, sizeof(char));
    return (int)ch;
}

int kputc(
    int c)
{
    char ch = (char)c;

    if(g_stdout_fd == NULL) {
        return 0;
    }
    KeWriteFsNode(g_stdout_fd, &ch, sizeof(ch));
    return 0;
}

void kflush(
    void)
{
    if(g_stdout_fd == NULL) {
        return;
    }
    KeFlushFsNode(g_stdout_fd);
    return;
}

#define NUMBER_TO_STRING(name, type, is_signed)\
void name(\
    type val,\
    char *str,\
    int base)\
{\
    char numbuf[24] = {0};\
    size_t i, j = 0;\
    if(val == 0) {\
        KeCopyString(&str[0], "0");\
        return;\
    }\
    if(is_signed && val < 0) {\
        *(str++) = '-';\
        val = -val;\
    }\
    while(val) {\
        int rem = (int)(val % (type)base);\
        numbuf[j] = (rem >= 10) ? rem - 10 + 'A' : rem + '0';\
        val /= (type)base;\
        j++;\
    }\
    for(i = 0; i != j; i++) {\
        str[i] = numbuf[(j - 1) - i];\
    }\
    str[i] = '\0';\
    return;\
}

NUMBER_TO_STRING(itoa, signed int, 1)
NUMBER_TO_STRING(litoa, signed long int, 1)
NUMBER_TO_STRING(ltoa, unsigned long, 0)
NUMBER_TO_STRING(uptrtoa, unsigned int, 0)
NUMBER_TO_STRING(usizetoa, size_t, 0)

int kvsnprintf(
    char *s,
    size_t n,
    const char *fmt,
    va_list args)
{
    size_t i = 0;

    KeSetMemory(s, 0, n);
    while(*fmt != '\0' && i < n - 1) {
        i = KeStringLength(s);
        if(*fmt == '%') {
            ++fmt;
            if(!KeCompareStringEx(fmt, "s", 1)) {
                const char *str = va_arg(args, const char *);
                size_t len = 0;

                if(str == NULL) {
                    str = "NULL";
                }

                len = KeStringLength(str);
                if(len >= n - i) {
                    len = n - i;
                }
                KeCopyMemory(&s[KeStringLength(s)], str, len);
            } else if(!KeCompareStringEx(fmt, "zu", 2)) {
                size_t val = va_arg(args, size_t);
                usizetoa(val, &s[i], 10);
                ++fmt;
            } else if(!KeCompareStringEx(fmt, "u", 1)) {
                unsigned int val = va_arg(args, unsigned int);
                ltoa(val, &s[i], 10);
            } else if(!KeCompareStringEx(fmt, "p", 1)) {
                unsigned int val = va_arg(args, unsigned int);
                uptrtoa(val, &s[i], 16);
            } else if(!KeCompareStringEx(fmt, "x", 1)) {
                unsigned val = va_arg(args, unsigned);
                ltoa(val, &s[i], 16);
            } else if(!KeCompareStringEx(fmt, "i", 1)) {
                signed int val = va_arg(args, signed int);
                itoa(val, &s[i], 10);
            }
            ++fmt;
        } else {
            s[i++] = *(fmt++);
            s[i] = '\0';
        }
    }
    return 0;
}

int kvprintf(
    const char *fmt,
    va_list args)
{
    char tmpbuf[320];
    kvsnprintf(&tmpbuf[0], 320, fmt, args);
    if(g_stdout_fd == NULL) {
#if defined(TARGET_S390)
        ModWriteHercDebug(NULL, &tmpbuf[0], KeStringLength(&tmpbuf[0]));
        return 0;
#endif
    }

    KeWriteFsNode(g_stdout_fd, &tmpbuf[0], KeStringLength(&tmpbuf[0]));
    return 0;
}

int ksnprintf(
    char *s,
    size_t n,
    const char *fmt,
    ...)
{
    va_list args;
    int r;

    va_start(args, fmt);
    r = kvsnprintf(s, n, fmt, args);
    va_end(args);
    return r;
}

int KeDebugPrint(
    const char *fmt,
    ...)
{
    va_list args;
    int r;

    va_start(args, fmt);
    r = kvprintf(fmt, args);
    va_end(args);
    return r;
}
