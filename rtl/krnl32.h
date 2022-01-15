#ifndef KRNL32_H
#define KRNL32_H

#include <stdint.h>
#include <stddef.h>

#define RtlDoSvc _Zhwdsvc
unsigned RtlDoSvc(unsigned code, unsigned arg1, unsigned arg2,
    unsigned arg3);

#endif
