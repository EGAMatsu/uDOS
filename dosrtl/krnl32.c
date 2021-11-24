#include <krnl32.h>

#if defined(TARGET_X86)
unsigned RtlDoSvc(
    unsigned code,
    unsigned arg1,
    unsigned arg2,
    unsigned arg3)
{
    /* Arguments */
    register unsigned a1 __asm__("1") = arg1;
    register unsigned a2 __asm__("2") = arg2;
    register unsigned a3 __asm__("3") = arg3;

    /* Return value (Also used as a call code) */
    register unsigned a4 __asm__("4") = (unsigned int)code;

    __asm__ __volatile__(
        "int $20\r\n"
        : "=r"(a4)
        : "r"(a1), "r"(a2), "r"(a3), "r"(a4)
        :);
    return a4;
}
#endif
