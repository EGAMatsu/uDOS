#ifndef PANIC_H
#define PANIC_H

#include <stdarg.h>
#include <Debug/Printf.h>

void KePanic(const char *fmt, ...);

#endif
