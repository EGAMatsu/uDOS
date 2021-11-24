#ifndef KRNL32_H
#define KRNL32_H

#include <stdint.h>
#include <stddef.h>

#if defined(TARGET_S390)
#   define RtlDoSvc _Zhwdsvc
#endif
unsigned RtlDoSvc(unsigned code, unsigned arg1, unsigned arg2,
    unsigned arg3);

#endif
