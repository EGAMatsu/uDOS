#ifndef KRNL32_H
#define KRNL32_H

#include <stdint.h>
#include <stddef.h>

#if defined(TARGET_S390)
static inline uintptr_t RtlDoSvc(
    int code,
    uintptr_t arg1,
    uintptr_t arg2,
    uintptr_t arg3)
{
    /* Arguments */
    register uintptr_t a1 __asm__("1") = arg1;
    register uintptr_t a2 __asm__("2") = arg2;
    register uintptr_t a3 __asm__("3") = arg3;

    /* Return value (Also used as a call code) */
    register uintptr_t a4 __asm__("4") = (uintptr_t)code;

    __asm__ __volatile__(
        "svc 26\r\n"
        : "=r"(a4)
        : "r"(a1), "r"(a2), "r"(a3), "r"(a4)
        :);
    return a4;
}
#endif

#if defined(TARGET_X86)
static inline uintptr_t RtlDoSvc(
    int code,
    uintptr_t arg1,
    uintptr_t arg2,
    uintptr_t arg3)
{
    /* Arguments */
    register uintptr_t a1 __asm__("1") = arg1;
    register uintptr_t a2 __asm__("2") = arg2;
    register uintptr_t a3 __asm__("3") = arg3;

    /* Return value (Also used as a call code) */
    register uintptr_t a4 __asm__("4") = (uintptr_t)code;

    __asm__ __volatile__(
        "int $20\r\n"
        : "=r"(a4)
        : "r"(a1), "r"(a2), "r"(a3), "r"(a4)
        :);
    return a4;
}
#endif

#endif