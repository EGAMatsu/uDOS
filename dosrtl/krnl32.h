#ifndef KRNL32_H
#define KRNL32_H

#include <stdint.h>
#include <stddef.h>

uintptr_t RtlDoSvc(int code, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);

#endif