#ifndef PANIC_H
#define PANIC_H

#include <printf.h>
#include <stdarg.h>

void kpanic(const char *fmt, ...);

#endif