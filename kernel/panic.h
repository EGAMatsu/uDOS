#ifndef PANIC_H
#define PANIC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <printf.h>
#include <stdarg.h>

static void kpanic(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    kvprintf(fmt, args);
    kflush();
    va_end(args);

    while (1)
        ;
    return;
}

#ifdef __cplusplus
}
#endif
#endif