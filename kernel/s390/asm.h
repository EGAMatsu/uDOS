#ifndef S390_ASM_H
#define S390_ASM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* s390 manual describes bits as MSB - so for sake of readability we do this */
#define S390_BIT(n, x) (((n) - 1) - (x))

/* Note that AM24 and AM31 have to be along with the instruction address (in a 128-bit PSW they
 * have to be on the low part of the flags */
#define PSW_AM24 0x00000000
#define PSW_AM31 0x80000000
#define PSW_AM64 0x00000001

/* This varies depending on the given system build */
#define PSW_DEFAULT_AMBIT (PSW_AM31)

/* See Figure 4.2 of Chapter 4. (page 141) of the z/Arch Principles of Operation for a more
 * detailed overview about the structure of the PSW
 */

/* Program event recording - this is for debugging stuff */
#define PSW_PER (1 << S390_BIT(32, 1))

/* Controls dynamic address translation */
#define PSW_DAT (1 << S390_BIT(32, 5))

/* I/O interrupt mask */
#define PSW_IO_INT (1 << S390_BIT(32, 6))

/* External interrupt mask stuff like processor signals and clocks */
#define PSW_EXTERNAL_INT (1 << S390_BIT(32, 7))

/* Enable machine check interrupts */ 
#define PSW_ENABLE_MCI (1 << S390_BIT(32, 13))

/* Makes the processor halt until an interrupt comes */
#define PSW_WAIT_STATE (1 << S390_BIT(32, 14))

/* Problem state - aka. userland switch */
#define PSW_PROBLEM_STATE (1 << S390_BIT(32, 15))

struct s390_psw {
    uint32_t flags;
    uint32_t address;
} __attribute__((packed, aligned(8)));

struct s390x_psw {
    uint32_t flags_hi;
    uint32_t flags_lo; /* It's all zero except for the first bit (in S/390 order of course) */
    uint32_t address_hi;
    uint32_t address_lo;
} __attribute__((packed, aligned(8)));

/*
 * In S/390 there is an area starting at 0, with a length of 8192 (z/Arch) or 1024 (S/390)
 * in that area there are various elements which can be tinkered with. It has PSWs which helps
 * the machine to jump to an address when something happens, normally fields with FLC* are S/390
 * and the ones with FLCE* are z/Arch exclusively.
 * 
 * The PSWs can be tought of a traditional interrupt vector table, however the table is not linear
 * and it's spread out everywhere. And there is only about 8 interrupts you can program.
 * 
 * The IRQ routing system helps to mitigate this hardware problem but it gives more work to the I/O
 * interrupt due to the work of routing every single device wanting to do I/O.
 */

/* SVC new psw, in short, this psw serves as a syscall as it is executed
 * when a SVC instruction is executed */
#define FLCSNPSW 0x60
#define FLCESNPSW 0x1C0

/* Machine check new PSW, unknown purpouse */
#define FLCMNPSW 0x70
#define FLCEMNPSW 0x1E0

/* Program check new PSW, unknown purpouse */
#define FLCPNPSW 0x68
#define FLCEPNPSW 0x1D0

/* Input/Output new PSW, unknown when it's invoked */
#define FLCINPSW 0x78
#define FLCEINPSW 0x1F0

/* Machine check old PSW - also called MCOPSW/FLCMOPSW */
#define FLCCAW 0x48

/* 8 bytes of I/O information code, the first 4 bytes are the subsystem ID */
#define FLCIOA 0xB8

#ifdef __cplusplus
}
#endif
#endif