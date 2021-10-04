#include <panic.h>
#include <stdint.h>
#include <string.h>

void kpanic(
    const char *fmt,
    ...)
{
    va_list args;
    va_start(args, fmt);

    kvprintf(fmt, args);
    kflush();

    s390_program_check_handler_stub();

    va_end(args);
    while(1);
}