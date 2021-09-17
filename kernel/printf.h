#ifndef PRINTF_H
#define PRINTF_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stddef.h>

int kgetc(void);
int kputc(int c);

void kflush(void);

int kvsnprintf(char *s, size_t n, const char *fmt, va_list args);
int kvprintf(const char *restrict fmt, va_list args);

/* gcc really dislikes formatted printing with EBCDIC character map set */
/*
int kprintf(const char *restrict fmt, ...)
    __attribute__((format(printf, 1, 2)));
*/
int ksnprintf(char *s, size_t n, const char *fmt, ...);
int kprintf(const char *restrict fmt, ...);

#ifdef __cplusplus
}
#endif
#endif