#ifndef S390_CPU_H
#define S390_CPU_H

#include <stddef.h>
#include <stdint.h>
#include <s390/asm.h>

/* Permanent storage assign is a memory area, something like the 8086 IVT table
 * but with more fun stuff */
struct s390x_psa {
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

/* Set system mask supression control */
#define S390_CR0_SSM_CTRL(x) ((x) << S390_BIT(64, 33))

/* Time-Of-Day clock synchronization control */
#define S390_CR0_TOD_CLOCK_SYNC_CTRL(x) ((x) << S390_BIT(64, 34))

/* Low address protection */
#define S390_CR0_LA_PROTECT_CTRL(x) ((x) << S390_BIT(64, 35))

/* Extraction instruction authorization control */
#define S390_CR0_EXA_CTRL(x) ((x) << S390_BIT(64, 36))

/* Secondary space control instruction authorization control */
#define S390_CR0_SSPACE_CTRL(x) ((x) << S390_BIT(64, 37))

/* Fetch protection override control */
#define S390_CR0_FETCH_PROTECT_CTRL(x) ((x) << S390_BIT(64, 38))

/* Primary subspace group control */
#define S390_CR1_PSG_CTRL(x) ((x) << S390_BIT(64, 54))

/* Primary private space control */
#define S390_CR1_PPS_CTRL(x) ((x) << S390_BIT(64, 55))

/* Primary storage alteration event control */
#define S390_CR1_PSAE_CTRL(x) ((x) << S390_BIT(64, 56))

/* Primary space-switch event control */
#define S390_CR1_PSSE_CTRL(x) ((x) << S390_BIT(64, 57))

/* Primary real-space control */
#define S390_CR1_PRS_CTRL(x) ((x) << S390_BIT(64, 58))

/* Primary designation-type control */
#define S390_CR1_PDT_CTRL(x) ((x) << S390_BIT(64, 60))

/* Table length (in multiples of 4096 bytes or 512 entries) */
#define S390_CR1_TABLE_LEN_CTRL(x) ((x) << S390_BIT(64, 62))

/* Sense data */
#define S390_SIGP_SENSE 0x01

/* External call */
#define S390_SIGP_EXTCALL 0x02

/* Emergency call */
#define S390_SIGP_EGCYCALL 0x03

/* Start */
#define S390_SIGP_START 0x04

/* Stop */
#define S390_SIGP_STOP 0x05

/* Restart */
#define S390_SIGP_RESTART 0x06

/* Stop and store status */
#define S390_SIGP_STOP_AND_STORE 0x09

/* Initial CPU reset */
#define S390_SIGP_INIT_RESET 0x0b

/* CPU reset */
#define S390_SIGP_CPU_RESET 0x0c

/* Set prefix */
#define S390_SIGP_SET_PREFIX 0x0d

/* Store status at address */
#define S390_SIGP_STORE_STATUS 0x0e

/* Set operational architecture */
#define S390_SIGP_SET_ARCH 0x12

#define MAX_CPUS 248

unsigned int s390_cpuid(void);
S390_PSW_DEFAULT_TYPE s390_store_then_or_system_mask(unsigned int mask);

int s390_signal_processor(unsigned int cpu_addr, unsigned int param);
int s390_address_is_valid(volatile const void *probe);
size_t s390_get_memsize(void);
void s390_wait_io(void);

int cpu_set_timer_delta_ms(int ms);

#include <s390/context.h>
#include <mmu.h>

typedef struct cpu_info {
    struct mmu_dev *dev;
    arch_context_t context;
}arch_cpu_info_t;

#endif