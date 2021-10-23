#ifndef KRNL32_H
#define KRNL32_H

#include <stdint.h>
#include <stddef.h>

#define S390_DO_SVC(id, arg1, arg2, arg3, ret)\
    register uintptr_t a1 __asm__("1") = arg1;\
    register uintptr_t a2 __asm__("2") = arg2;\
    register uintptr_t a3 __asm__("3") = arg3;\
    register uintptr_t a4 __asm__("4");\
    __asm__ __volatile__(\
        "svc " #id\
        : "=r"(a4)\
        : "r"(a1), "r"(a2), "r"(a3)\
        :);\
    if(ret == NULL) { *((uintptr_t *)ret) = a4; }

static inline uintptr_t RtlDoSvc(
    int code,
    uintptr_t arg1,
    uintptr_t arg2,
    uintptr_t arg3)
{
    /* Call code */
    register uintptr_t a0 __asm__("0") = (uintptr_t)code;

    /* Arguments */
    register uintptr_t a1 __asm__("1") = arg1;
    register uintptr_t a2 __asm__("2") = arg2;
    register uintptr_t a3 __asm__("3") = arg3;

    /* Return value */
    register uintptr_t a4 __asm__("4") = 0;

    __asm__ __volatile__(
        "svc 26"
        : "=r"(a4)
        : "r"(code), "r"(a1), "r"(a2), "r"(a3)
        :);
    
    return a4;
}

#endif