#include <printf.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vfs.h>

struct vfs_handle *g_stdout_fd = NULL, *g_stdin_fd = NULL;

int kgetc(
    void)
{
    return (int)'A';
}

int kputc(
    int c)
{
    char ch = (char)c;

    if(g_stdout_fd == NULL) {
        return 0;
    }
    vfs_write(g_stdout_fd, &ch, sizeof(ch));
    return 0;
}

void kflush(
    void)
{
    if(g_stdout_fd == NULL) {
        return;
    }
    vfs_flush(g_stdout_fd);
    return;
}

static char numbuf[32] = {0};
#define NUMBER_TO_STRING(name, type, is_signed)\
void name(\
    type val,\
    char *str,\
    int base)\
{\
    size_t i, j = 0;\
    if(val == 0) {\
        strcpy(&str[0], "0");\
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
NUMBER_TO_STRING(uptrtoa, uintptr_t, 0)
NUMBER_TO_STRING(usizetoa, size_t, 0)

int kvsnprintf(
    char *s,
    size_t n,
    const char *fmt,
    va_list args)
{
    size_t i = 0;

    memset(s, 0, n);
    while(*fmt != '\0' && i < n - 1) {
        i = strlen(s);
        if(*fmt == '%') {
            ++fmt;
            if(!strncmp(fmt, "s", 1)) {
                const char *str = va_arg(args, const char *);
                if(str == NULL) {
                    str = "NULL";
                }
                memcpy(&s[strlen(s)], str, strlen(str));
            } else if(!strncmp(fmt, "zu", 2)) {
                size_t val = va_arg(args, size_t);
                usizetoa(val, &s[i], 10);
                ++fmt;
            } else if(!strncmp(fmt, "u", 1)) {
                unsigned int val = va_arg(args, unsigned int);
                ltoa(val, &s[i], 10);
            } else if(!strncmp(fmt, "p", 1)) {
                uintptr_t val = va_arg(args, uintptr_t);
                uptrtoa(val, &s[i], 16);
            } else if(!strncmp(fmt, "x", 1)) {
                unsigned val = va_arg(args, unsigned);
                ltoa(val, &s[i], 16);
            } else if(!strncmp(fmt, "i", 1)) {
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
    char tmpbuf[80];
    
    kvsnprintf(&tmpbuf[0], 80, fmt, args);
    if(g_stdout_fd == NULL) {
        hdebug_write(NULL, &tmpbuf[0], strlen(&tmpbuf[0]));
        return 0;
    }

    vfs_write(g_stdout_fd, &tmpbuf[0], strlen(&tmpbuf[0]));
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

int kprintf(
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