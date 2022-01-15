#include <panic.h>
#include <memory.h>

void KePanic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    g_stdout_fd = NULL;
    g_stdin_fd = NULL;

    kvprintf(fmt, args);
    kflush();

    va_end(args);
    while(1);
}
