#ifndef S390_CPU_H
#define S390_CPU_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define S390_CR0_SSM_CTRL(x) \
    ((x) << 33) /* Set system mask supression control */
#define S390_CR0_TOD_CLOCK_SYNC_CTRL(x) \
    ((x) << 34) /* Time-Of-Day clock synchronization control */
#define S390_CR0_LA_PROTECT_CTRL(x) ((x) << 35) /* Low address protection */
#define S390_CR0_EXA_CTRL(x) \
    ((x) << 36) /* Extraction instruction authorization control */
#define S390_CR0_SSPACE_CTRL(x)                                              \
    ((x) << 37) /* Secondary space control instruction authorization control \
                   */
#define S390_CR0_FETCH_PROTECT_CTRL(x) \
    ((x) << 38) /* Fetch protection override control */

#define S390_CR1_PSG_CTRL(x) \
    ((x) << S390_BIT(64, 54)) /* Primary subspace group control */
#define S390_CR1_PPS_CTRL(x) \
    ((x) << S390_BIT(64, 55)) /* Primary private space control */
#define S390_CR1_PSAE_CTRL(x) \
    ((x) << S390_BIT(64, 56)) /* Primary storage alteration event control */
#define S390_CR1_PSSE_CTRL(x) \
    ((x) << S390_BIT(64, 57)) /* Primary space-switch event control */
#define S390_CR1_PRS_CTRL(x) \
    ((x) << S390_BIT(64, 58)) /* Primary real-space control */
#define S390_CR1_PDT_CTRL(x) \
    ((x) << S390_BIT(64, 60)) /* Primary designation-type control */
#define S390_CR1_TABLE_LEN_CTRL(x) \
    ((x) << S390_BIT(64,           \
         62)) /* Table length (in multiples of 4096 bytes or 512 entries) */

unsigned int s390_cpuid(void);
unsigned int s390_store_then_or_system_mask(uint8_t mask);

#define S390_SIGP_SENSE 0x01 /* Sense data */
#define S390_SIGP_EXTCALL 0x02 /* External call */
#define S390_SIGP_EGCYCALL 0x03 /* Emergency ~~meeting~~ call */
#define S390_SIGP_START 0x04 /* Start */
#define S390_SIGP_STOP 0x05 /* Stop */
#define S390_SIGP_RESTART 0x06 /* Restart */
#define S390_SIGP_STOP_AND_STORE 0x09 /* Stop and store status */
#define S390_SIGP_INIT_RESET 0x0b /* Initial CPU reset */
#define S390_SIGP_CPU_RESET 0x0c /* CPU reset */
#define S390_SIGP_SET_PREFIX 0x0d /* Set prefix */
#define S390_SIGP_STORE_STATUS 0x0e /* Store status at address */
#define S390_SIGP_SET_ARCH 0x12 /* Set operational architecture */

int s390_signal_processor(unsigned int cpu_addr, unsigned int param);

#ifdef __cplusplus
}
#endif
#endif