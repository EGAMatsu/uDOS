#ifndef CSS_H
#define CSS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <s390/asm.h>
#include <stddef.h>
#include <stdint.h>

/* Subchannel id */
struct css_schid {
    uint16_t id;
    uint16_t num;
} __attribute__((packed));

/* Channel control word format 0 */
struct css_ccw0 {
    uint8_t cmd;
    uint16_t lo_addr;
    uint8_t hi_addr;
    uint8_t flags;
    uint8_t reserved;
    uint16_t count;
} __attribute__((packed));

/* Channel control word format 1 */
struct css_ccw1 {
    uint8_t cmd;
    uint8_t flags;
    uint16_t length;
    uint32_t addr;
} __attribute__((packed));

#define CSS_PMCW_ISC(x) \
    ((x) << S390_BIT(16, 2)) /* I/O Interrupt subclass code */
#define CSS_PMCW_ENABLED(x) \
    ((x) << S390_BIT(16, 8)) /* Enabled for all I/O functions */
#define CSS_PMCW_LIMIT(x) ((x) << S390_BIT(16, 9)) /* Limit mode */
#define CSS_PMCW_MM_ENABLE(x) \
    ((x) << S390_BIT(16, 11)) /* Measurement mode enable */
#define CSS_PMCW_MULTIPATH_MODE(x) \
    ((x) << S390_BIT(16, 13)) /* Multipath mode */
#define CSS_PMCW_TIMING(x) ((x) << S390_BIT(16, 14)) /* Timing facility */
#define CSS_PMCW_DNV(x) ((x) << S390_BIT(16, 15)) /* Device number valid */

/* Path management control world */
struct css_pmcw {
    uint32_t int_param;
    uint16_t flags;
    uint16_t dev_num;
    uint8_t lpm;
    uint8_t pnom;
    uint8_t lpum;
    uint8_t pim;
    uint16_t mbi;
    uint8_t pom;
    uint8_t pam;
    uint8_t chpid[8];
    uint8_t zero[3];
    uint8_t last_flags; /* Last 3 bits contains flags */
} __attribute__((packed));

/* Subchannel status word */
struct css_scsw {
    uint32_t flags;
    uint32_t cpa_addr;
    uint8_t device_status;
    uint8_t subchannel_status;
    uint16_t count;
} __attribute__((packed));

/* Subchannel information block */
struct css_schib {
    struct css_pmcw pmcw;
    struct css_scsw scsw;
    union {
        uint64_t mb_addr;
        uint32_t md_data[3];
    };
} __attribute__((packed, aligned(4)));

#define CSS_ORB_SUSPEND_CTRL(x)                 \
    ((x) << S390_BIT(32, 4)) /* Suspend control \
                                 */
#define CSS_ORB_STREAMING_MODE(x) \
    ((x) << S390_BIT(32, 5)) /* Streaming mode for subchannel mode */
#define CSS_ORB_SYNC_CTRL(x) \
    ((x) << S390_BIT(32, 6)) /* Synchronization control */
#define CSS_ORB_FORMAT_CTRL(x) ((x) << S390_BIT(32, 8)) /* Format control */
#define CSS_ORB_PREFETCH_CTRL(x)                 \
    ((x) << S390_BIT(32, 9)) /* Prefetch control \
                                    */
#define CSS_ORB_ISI_CTRL(x) \
    ((x) << S390_BIT(32, 10)) /* Initial status interrupt control */
#define CSS_ORB_ADDRESS_LIMIT_CTRL(x) \
    ((x) << S390_BIT(32, 11)) /* Address limit control */
#define CSS_ORB_SUPRESS_SUSPEND_INT_CTRL(x) \
    ((x) << S390_BIT(32, 12)) /* Supress suspend interrupt control */
#define CSS_ORB_FORMAT_2_IDAW_CTRL(x) \
    ((x) << S390_BIT(32, 14)) /* Format IDAW for CCW */
#define CSS_ORB_2K_IDAW_CTRL(x) \
    ((x) << S390_BIT(32, 15)) /* 2K Indirect data address word control */
#define CSS_ORB_LPM_CTRL(x) \
    ((x) << S390_BIT(32, 16)) /* Logical path mask control */
#define CSS_ORB_MODIFIED_IDA_CTRL(x)                                           \
    ((x) << S390_BIT(32, 25)) /* Modified CCW indirect data addressing control \
                                     */
#define CSS_ORB_EXTENSION_CTRL(x) \
    ((x) << S390_BIT(32, 31)) /* ORB Extension Control */

/* Operation request block */
struct css_orb {
    uint32_t int_param;
    uint32_t flags;
    uint32_t prog_addr;

    /* Only used when the ORB extension control is set */
    uint8_t css_priority;
    uint8_t reserved1;
    uint8_t cu_priority;
    uint8_t reserved2;
    uint32_t reserved3[4];
} __attribute__((packed, aligned(4)));

/* Interrupt request block */
struct css_irb {
    struct css_scsw scsw;
    uint32_t esw[5];
    uint32_t ecw[8];
    uint32_t emw[8];
} __attribute__((packed, aligned(4)));

int css_start_channel(struct css_schid schid, void *schib);
int css_store_channel(struct css_schid schid, void *schib);
int css_modify_channel(struct css_schid schid, void *schib);
int css_test_channel(struct css_schid schid, void *schib);

#ifdef __cplusplus
}
#endif
#endif