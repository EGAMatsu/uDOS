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
#if (MACHINE >= M_S360)
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
#if (MACHINE >= M_ZARCH)
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

/*
 * In S/390 there is an area starting at 0, with a length of 8192 (z/Arch) or
 * 1024 (S/390) in that area there are various elements which can be tinkered
 * with. It has PSWs which helps the machine to jump to an address when
 * something happens, normally fields with FLC* are S/390 and the ones with
 * FLCE* are z/Arch exclusively.
 *
 * The PSWs can be tought of a traditional interrupt vector table, however the
 * table is not linear and it's spread out everywhere. And there is only about 8
 * interrupts you can program.
 *
 * The IRQ routing system helps to mitigate this hardware problem but it gives
 * more work to the I/O interrupt due to the work of routing every single device
 * wanting to do I/O.
 */

/* Restart new PSW, unknown when it's invoked */
#define PSA_FLCERNPSW 0x1A0

/* External new PSW, invoked with a timer or an external device call */
#define PSA_FLCENPSW 0x58
#define PSA_FLCEOPSW 0x18

#define PSA_FLCEENPSW 0x1B0
#define PSA_FLCEEOPSW 0x130

/* SVC new psw, in short, this psw serves as a syscall as it is executed
 * when a SVC instruction is executed */
#define PSA_FLCSNPSW 0x60
#define PSA_FLCSOPSW 0x20

#define PSA_FLCESNPSW 0x1C0
#define PSA_FLCESOPSW 0x140

/* Interrupt code for service call */
#define PSA_FLCESICODE 0x8A
#define PSA_FLCSVCN 0x8A
#define PSA_FLCSVILC 0x89

/* Machine check new PSW, invoked primarly on hardware related */
#define PSA_FLCMNPSW 0x70
#define PSA_FLCEMNPSW 0x1E0

/* Program check new PSW, invoked primarly on software related errors */
#define PSA_FLCPNPSW 0x68
#define PSA_FLCPOPSW 0x28

#define PSA_FLCEPNPSW 0x1D0
#define PSA_FLCEPOPSW 0x150

/* Interruption code for program exceptions */
#define PSA_FLCPICOD 0x8E

/* Input/Output new PSW, unknown when it's invoked */
#define PSA_FLCINPSW 0x78
#define PSA_FLCEINPSW 0x1F0

#define PSA_FLCIOPSW 0x38
#define PSA_FLCEIOPSW 0x170

/* Machine check old PSW - also called MCOPSW/FLCMOPSW */
#define PSA_FLCCSW 0x40
#define PSA_FLCCAW 0x48

/* 8 bytes of I/O information code, the first 4 bytes are the subsystem ID */
#define PSA_FLCIOA 0xB8

/* 32-bit general register save area */
#define PSA_FLCGRSAV 0x180

/* ... and the control register save area */
#define PSA_FLCCRSAV 0x1C0

/* Installed facilities information (8 bytes) */
#define PSA_FLCFACL(x) (0xC8 + (x))

#define PSA_FLCFACL0_N3 0x80
#define PSA_FLCFACL0_ZARCH_INSTALL 0x40
#define PSA_FLCFACL0_ZARCH_ACTIVE 0x20

/* (Only on z/Arch) IDTE facility installed */
#define PSA_FLCFACL0_IDTE 0x10

/* (Only on z/Arch) clear segment upon invalidation */
#define PSA_FLCFACL0_IDTE_CLEAR_SEGMENT 0x08

/* (Only on z/Arch) clear region upon invalidation */
#define PSA_FLCFACL0_IDTE_CLEAR_REGION 0x04

/* ASN and LX reuse facility is installed */
#define PSA_FLCFACL0_ASN_LX_REUSE 0x02

/* STFLE instruction is available */
#define PSA_FLCFACL0_STFLE 0x01

/* Dynamic Address Translation facility is installed */
#define PSA_FLCFACL1_DAT 0x80

/* Sense running status */
#define PSA_FLCFACL1_SRS 0x40

/* SSKE instruction is installed */
#define PSA_FLCFACL1_SSKE 0x20

/* STSI enhancement */
#define PSA_FLCFACL1_CTOP 0x10

/* 110524 */
#define PSA_FLCFACL1_QCIF 0x08

/* IPTE Range facility is installed */
#define PSA_FLCFACL1_IPTE 0x04

/* NQ Key setting */
#define PSA_FLCFACL1_NQKEY 0x02

/* APFT Facility is installed */
#define PSA_FLCFACL1_APFT 0x01

#define PSA_FLCFACL2_ETF2 0x80
#define PSA_FLCFACL2_CRYA 0x40
#define PSA_FLCFACL2_LONGDISP 0x20
#define PSA_FLCFACL2_LONGDISPHP 0x10
#define PSA_FLCFACL2_HFP_MULSUB 0x08
#define PSA_FLCFACL2_EIMM 0x04
#define PSA_FLCFACL2_ETF3 0x02
#define PSA_FLCFACL2_HFP_UN 0x01

#if (MACHINE >= M_ZARCH)
#   define PSA_SIZE 8192
#else
#   define PSA_SIZE 1024
#endif

/* Helper function to create a PSW adjusted to the current machine */
#if (MACHINE >= M_ZARCH)
#   define PSW_DEFAULT_TYPE struct s390x_psw
#   define PSW_DECL(name, address, flags)\
 PSW_DEFAULT_TYPE name = {\
    (flags) | PSW_AM64, PSW_AM31, 0, (uint32_t)(address)\
}
#else
#   define PSW_DEFAULT_TYPE struct s390_psw
#   define PSW_DECL(name, address, flags)\
 PSW_DEFAULT_TYPE name = {\
    (flags), (uint32_t)(address) + PSW_DEFAULT_AMBIT\
}
#endif

#endif