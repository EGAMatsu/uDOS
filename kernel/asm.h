#ifndef S390_ASM_H
#define S390_ASM_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif

/* s390 manual describes bits as MSB - so for sake of readability we do this */
#define S390_BIT(n, x) (((n) - 1) - (x))

/* Note that AM24 and AM31 have to be along with the instruction address (in a
 * 128-bit PSW they have to be on the low part of the flags */
#define PSW_AM24 0x00000000
#define PSW_AM31 0x80000000
#define PSW_AM64 0x00000001

/* This varies depending on the given system build */
#if (MACHINE >= 360u)
#   define PSW_DEFAULT_AMBIT (PSW_AM31)
#else
#   define PSW_DEFAULT_AMBIT (PSW_AM24)
#endif

/* See Figure 4.2 of Chapter 4. (page 141) of the z/Arch Principles of Operation
 * for a more detailed overview about the structure of the PSW */
#ifndef __ASSEMBLER__
struct s390_psw {
    uint32_t flags;
    uint32_t address;
} __attribute__((packed, aligned(8)));

struct s390x_psw {
    uint32_t hi_flags;
    uint32_t lo_flags; /* It's all zero except for the MSB (in S/390 order) */
    union {
        struct {
            uint32_t hi_address;
            uint32_t lo_address;
        };
        uint64_t address;
    };
} __attribute__((packed, aligned(8)));
#endif

/* Program event recording - this is for debugging stuff */
#define PSW_PER ((1) << S390_BIT(32, 1))

/* Controls dynamic address translation */
#define PSW_DAT ((1) << S390_BIT(32, 5))

/* I/O interrupt mask */
#define PSW_IO_INT ((1) << S390_BIT(32, 6))

/* External interrupt mask stuff like processor signals and clocks */
#define PSW_EXTERNAL_INT ((1) << S390_BIT(32, 7))

/* Archmode that the PSW will run in */
#define PSW_ENABLE_ARCHMODE ((1) << S390_BIT(32, 12))

/* Helper for assignment of PSW, 1 = ESA, 0 = z/Arch */
#if (MACHINE > 390u)
#   define PSW_DEFAULT_ARCHMODE (0)
#else
#   define PSW_DEFAULT_ARCHMODE (PSW_ENABLE_ARCHMODE)
#endif

/* Enable machine check interrupts */
#define PSW_ENABLE_MCI ((1) << S390_BIT(32, 13))

/* Makes the processor halt until an interrupt comes */
#define PSW_WAIT_STATE ((1) << S390_BIT(32, 14))

/* Problem state - aka. userland switch */
#define PSW_PROBLEM_STATE ((1) << S390_BIT(32, 15))

/* Helper function to create a PSW adjusted to the current machine */
#if (MACHINE > 390u)
#   define PSW_DEFAULT_TYPE struct s390x_psw
#   define PSW_DECL(name, address, flags) \
PSW_DEFAULT_TYPE name = { \
    (flags) | PSW_AM64, PSW_AM31, 0, (uint32_t)(address) \
}
#else
#   define PSW_DEFAULT_TYPE struct s390_psw
#   define PSW_DECL(name, address, flags) \
PSW_DEFAULT_TYPE name = { \
    (flags), (uint32_t)(address) \
}
#endif

#endif
