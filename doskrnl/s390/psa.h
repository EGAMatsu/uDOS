#ifndef S390_PSA_H
#define S390_PSA_H

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

#endif