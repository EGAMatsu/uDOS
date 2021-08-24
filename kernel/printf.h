#ifndef PRINTF_H
#define PRINTF_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

int kgetc(void);
int kputc(int c);

void kflush(void);
int kvprintf(const char *restrict fmt, va_list args);
int kprintf(const char *restrict fmt, ...)
    __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif
#endif