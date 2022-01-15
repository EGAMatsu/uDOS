#ifndef S390_CPU_H
#define S390_CPU_H

#ifndef __ASSEMBLER__
#include <stddef.h>
#include <stdint.h>
#include <asm.h>
#include <context.h>
#include <mmu.h>

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
extern unsigned HwDoSVC(unsigned code, unsigned arg1, unsigned arg2,
    unsigned arg3);
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
#endif

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

#endif
