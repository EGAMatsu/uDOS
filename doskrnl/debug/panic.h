#ifndef PANIC_H
#define PANIC_H

#include <stdarg.h>
#include <debug/printf.h>

void KePanic(const char *fmt, ...);

#endif