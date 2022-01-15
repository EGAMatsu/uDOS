#ifndef PANIC_H
#define PANIC_H

#include <stdarg.h>
#include <printf.h>

void KePanic(const char *fmt, ...);

#endif
