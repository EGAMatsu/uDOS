#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>
#include <stddef.h>

int kgetc(void);
int kputc(int c);

void kflush(void);

int kvsnprintf(char *s, size_t n, const char *fmt, va_list args);
int kvprintf(const char *restrict fmt, va_list args);

/* gcc really dislikes formatted printing with EBCDIC character map set */
int KeDebugPrint(const char *restrict fmt, ...)
    __attribute__((format(printf, 1, 2)));

//int KeDebugPrint(const char *restrict fmt, ...);

/*int ksnprintf(char *s, size_t n, const char *fmt, ...);*/

extern struct FsHandle *g_stdout_fd, *g_stdin_fd;

#endif