#include <panic.h>

void kpanic(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    kvprintf(fmt, args);
    kflush();
    va_end(args);

    while (1)
        ;
}