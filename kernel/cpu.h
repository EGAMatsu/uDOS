#ifndef S390_CPU_H
#define S390_CPU_H

#include <stddef.h>
#include <stdint.h>
#include <asm.h>
#include <mmu.h>

#if (MACHINE > 390u)
typedef uint64_t register_t;
#else
/* TODO: This is wrong */
typedef uint32_t register_t;
#endif

typedef struct _cpu_context {
    union {
        register_t gp_regs[16];
        struct {
            register_t r0;
            register_t r1;
            register_t r2;
            register_t r3;
            register_t r4;
            register_t r5;
            register_t r6;
            register_t r7;
            register_t r8;
            register_t r9;
            register_t r10;
            register_t r11;
            register_t r12;
            register_t r13;
            register_t r14;
            register_t r15;
        };
    };

    PSW_DEFAULT_TYPE psw;
}cpu_context;

/* The scratch frame is an abstract memory area representing where the
 * registers at the time of an interruption are stored at, this is so
 * the scheduler can retrieve them */
#define HwGetScratchContextFrame _Zhwsctx
extern cpu_context *HwGetScratchContextFrame(void);

#include <scheduler.h>
void HwSwitchThreadContext(struct scheduler_thread *old_thread, struct scheduler_thread *new_thread);

/* Permanent storage assign is a memory area, something like the 8086 IVT table
 * but with more fun stuff */
struct ProcStorageArea {
    uint32_t ipl_psw;
    uint32_t ipl_ccw[2];
    uint32_t external_old_psw;
    uint32_t svc_old_psw;
    uint32_t program_old_psw;
    uint32_t mcheck_old_psw;
    uint32_t io_old_psw;
    uint32_t channel_status;
    uint16_t channel_address;
    uint16_t unused1;
    uint16_t timer;
    uint16_t unused2;
    uint32_t ext_new_psw;
    uint32_t svc_new_psw;
    uint32_t program_new_psw;
    uint32_t mcheck_new_psw;
    uint32_t io_new_psw;
} __attribute((packed));

#define HwCPUID _Zhwpuid
extern unsigned int HwCPUID(void);
#define HwSignalCPU _Zhwsigp
extern int HwSignalCPU(unsigned int cpu_addr, unsigned int param);
#define HwDoSVC _Zhwdsvc
extern unsigned HwDoSVC(unsigned code, unsigned arg1, unsigned arg2, unsigned arg3);

#define HwCheckAddress _Zhwchka
int HwCheckAddress(volatile const void *probe);
size_t HwGetMemorySize(void);
void HwWaitIO(void);

#define HwSetCPUTimerDelta _Zhwctid
int HwSetCPUTimerDelta(int ms);

struct CPU_Info {
    struct dat_device *dev;
    cpu_context context;
};

enum sigp_codes {
    /* Sense data */
    S390_SIGP_SENSE = 0x01,
    /* External call */
    S390_SIGP_EXTCALL = 0x02,
    /* Emergency call */
    S390_SIGP_EGCY_CALL = 0x03,
    /* Start */
    S390_SIGP_START = 0x04,
    /* Stop */
    S390_SIGP_STOP = 0x05,
    /* Restart */
    S390_SIGP_RESTART = 0x06,
    /* Stop and store status */
    S390_SIGP_STOP_AND_STORE = 0x09,
    /* Initial CPU reset */
    S390_SIGP_INIT_RESET = 0x0B,
    /* CPU reset */
    S390_SIGP_CPU_RESET = 0x0C,
    /* Set prefix */
    S390_SIGP_SET_PREFIX = 0x0D,
    /* Store status at address */
    S390_SIGP_STORE_STATUS = 0x0E,
    /* Set operational architecture */
    S390_SIGP_SET_ARCH = 0x12,

    /* TODO: Are these z/Arch exclusive? */
    /* Conditional emergency */
    S390_SIGP_EGCY_COND = 0x13,
    /* Sense running status */
    S390_SIGP_SENSE_RUN = 0x15
};

#if (MACHINE > 390u)
/* Tracing Time-Of-Day control */
#define S390_CR0_TRACE_TOD ((1) << S390_BIT(64, 32))

/* Set system mask supression control */
#define S390_CR0_SSM ((1) << S390_BIT(64, 33))

/* Time-Of-Day clock synchronization control */
#define S390_CR0_TOD_CLOCK_SYNC ((1) << S390_BIT(64, 34))

/* Low address protection */
#define S390_CR0_LA_PROTECT ((1) << S390_BIT(64, 35))

/* Extraction instruction authorization control */
#define S390_CR0_EXA ((1) << S390_BIT(64, 36))

/* Secondary space control instruction authorization control */
#define S390_CR0_SSPACE ((1) << S390_BIT(64, 37))

/* Fetch protection override control */
#define S390_CR0_FETCH_PROTECT(x) ((x) << S390_BIT(64, 38))

/* CPU-Timer subclass mask */
#define S390_CR0_TIMER_MASK ((1) << S390_BIT(64, 53))
#endif

/* CPU-Timer subclass mask */
#define S390_CR0_TIMER_MASK ((1) << S390_BIT(64, 53))

#if (MACHINE > 390u)
/* Primary subspace group control */
#define S390_CR1_PSG ((1) << S390_BIT(64, 54))

/* Primary private space control */
#define S390_CR1_PPS ((1) << S390_BIT(64, 55))

/* Primary storage alteration event control */
#define S390_CR1_PSAE ((1) << S390_BIT(64, 56))

/* Primary space-switch event control */
#define S390_CR1_PSSE ((1) << S390_BIT(64, 57))

/* Primary real-space control */
#define S390_CR1_PRS(x) ((x) << S390_BIT(64, 58))

/* Primary designation-type control */
#define S390_CR1_PDT(x) ((x) << S390_BIT(64, 60))

/* Table length (in multiples of 4096 bytes or 512 entries) */
#define S390_CR1_TABLE_LEN(x) ((x) << S390_BIT(64, 62))
#else
/* Primary segment table origin */
#define S390_CR1_PSGT_ORIGIN(x) ((x) << S390_BIT(32, 1))
#endif

#define MAX_CPUS 248

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

/* 32-bit general register save area and the control register save area
 * respectively */
#define PSA_FLCGRSAV 0x180
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

#if (MACHINE > 390u)
#   define PSA_SIZE 8192
#else
#   define PSA_SIZE 1024
#endif

#endif
