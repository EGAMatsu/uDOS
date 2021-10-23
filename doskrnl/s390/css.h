#ifndef S390_CSS_H
#define S390_CSS_H

#include <stddef.h>
#include <stdint.h>
#include <s390/asm.h>

/* Subchannel id */
struct css_schid {
    uint16_t id;
    uint16_t num;
} __attribute__((packed));

/* Flags and stuff */

/* Channel control word format 0 */
struct css_ccw0 {
    uint8_t cmd;
    uint16_t lo_addr;
    uint8_t hi_addr;
    uint8_t flags;
    uint8_t reserved;
    uint16_t count;
} __attribute__((packed, aligned(4)));

/* Channel control word format 1 */
struct css_ccw1 {
    uint8_t cmd;
    uint8_t flags;
    uint16_t length;
    uint32_t addr;
} __attribute__((packed, aligned(4)));

enum css_cmd {
/* General purpouse channel subsystem command codes
 * See z/Architecture Principles of Operation, Page 29, Figure 15-5 */
    CSS_CMD_WRITE = 0x01,
    CSS_CMD_READ = 0x02,
    CSS_CMD_READ_BACKWARDS = 0x0C,
    CSS_CMD_CONTROL = 0x03,

/* Obtain basic sense information from device (for identifying the type of
 * device of course) */
    CSS_CMD_SENSE = 0x04,
    CSS_CMD_SENSE_ID = 0xE4,

/* Transfer in Chnannel - Usuaully to retry last failed operation */
    CSS_CMD_TIC = 0x08

/* "Enable this device" */
};

/* Command chain word flags */
#define CSS_CCW_CD ((1) << S390_BIT(8, 0))
#define CSS_CCW_CC ((1) << S390_BIT(8, 1))
#define CSS_CCW_SLI ((1) << S390_BIT(8, 2))
#define CSS_CCW_SPK ((1) << S390_BIT(8, 3))
#define CSS_CCW_PCI ((1) << S390_BIT(8, 4))
#define CSS_CCW_IDA ((1) << S390_BIT(8, 5))
#define CSS_CCW_S ((1) << S390_BIT(8, 6))
#define CSS_CCW_MIDA ((1) << S390_BIT(8, 7))

/* Path management control word */
struct css_pmcw {
    uint32_t int_param;
    uint16_t flags;
    uint16_t dev_num;
    uint8_t lpm; /* Logical path mask */
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

/* I/O Interrupt subclass code */
#define CSS_PMCW_ISC(x) ((x) << S390_BIT(16, 2))

/* Enabled for all I/O functions */
#define CSS_PMCW_ENABLED ((1) << S390_BIT(16, 8))

/* Limit mode */
#define CSS_PMCW_LIMIT ((1) << S390_BIT(16, 9))

/* Measurement mode enable */
#define CSS_PMCW_MM_ENABLE ((1) << S390_BIT(16, 11))

/* Multipath mode */
#define CSS_PMCW_MULTIPATH_MODE ((1) << S390_BIT(16, 13))

/* Timing facility */
#define CSS_PMCW_TIMING ((1) << S390_BIT(16, 14))

/* Device number valid */
#define CSS_PMCW_DNV ((1) << S390_BIT(16, 15))

/* See z/Architecture Principles of Operation, Page 33 */
/* Extended status word (format 0) */
struct css_esw0 {
    uint32_t sc_logout; /* Subchannel logout */
    uint32_t report; /* Extended report word */
    uint64_t fail_addr; /* Failing storage address */
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

/* Operation request block */
struct css_orb {
    uint32_t int_param;
    uint32_t flags;
    uint32_t cpa_addr;

    /* Only used when the ORB extension control is set */
    uint8_t css_priority;
    uint8_t reserved1;
    uint8_t cu_priority;
    uint8_t reserved2;
    uint32_t reserved3[4];
} __attribute__((packed, aligned(4)));

/* Suspend control */
#define CSS_ORB_SUSPEND_CTRL ((1) << S390_BIT(32, 4))

/* Streaming mode for subchannel mode */
#define CSS_ORB_STREAMING_MODE ((1) << S390_BIT(32, 5))

/* Synchronization control */
#define CSS_ORB_SYNC_CTRL(x) ((x) << S390_BIT(32, 6))

/* Format control */
#define CSS_ORB_FORMAT_CTRL ((1) << S390_BIT(32, 8))

/* Prefetch control */
#define CSS_ORB_PREFETCH_CTRL ((1) << S390_BIT(32, 9))

/* Initial status interrupt control */
#define CSS_ORB_ISI_CTRL ((1) << S390_BIT(32, 10))

/* Address limit control */
#define CSS_ORB_ADDRESS_LIMIT_CTRL ((1) << S390_BIT(32, 11))

/* Supress suspend interrupt control */
#define CSS_ORB_SUPRESS_SUSPEND_INT_CTRL ((1) << S390_BIT(32, 12))

/* Format IDAW for CCW */
#define CSS_ORB_FORMAT_2_IDAW_CTRL ((1) << S390_BIT(32, 14))

/* 2K Indirect data address word control */
#define CSS_ORB_2K_IDAW_CTRL ((1) << S390_BIT(32, 15))

/* Logical path mask control */
#define CSS_ORB_LPM_CTRL(x) ((x) << S390_BIT(32, 16))

/* Modified CCW indirect data addressing control */
#define CSS_ORB_MODIFIED_IDA_CTRL(x) ((x) << S390_BIT(32, 25))

/* ORB Extension Control */
#define CSS_ORB_EXTENSION_CTRL(x) ((x) << S390_BIT(32, 31))

/* Interrupt request block */
struct css_irb {
    struct css_scsw scsw;
    uint32_t esw[5];
    uint32_t ecw[8];
    uint32_t emw[8];
} __attribute__((packed, aligned(4)));

int css_start_channel(struct css_schid schid, struct css_orb *schib);
int css_store_channel(struct css_schid schid, void *schib);
int css_modify_channel(struct css_schid schid, struct css_schib *schib);
int css_test_channel(struct css_schid schid, struct css_irb *schib);

#include <mutex.h>
struct css_device {
    struct css_orb orb;
    struct css_irb irb;
    struct css_schid schid;
    struct css_schib schib;
    mutex_t lock;
};

enum css_status {
    CSS_STATUS_OK = 0,
    CSS_STATUS_PENDING = 1,
    CSS_STATUS_NOT_PRESENT = 3,
};

/* Command information word */
typedef uint32_t css_ciw_t;

/* SenseId data */
struct css_senseid {
    uint8_t reserved;
    uint16_t cu_type;
    uint8_t cu_model;
    uint16_t dev_type;
    uint8_t dev_model;
    uint8_t unused;

    /* Extended SENSEID data */
    css_ciw_t ciw[8];
} __attribute__((packed, aligned(4)));

struct css_request {
#if (MACHINE >= M_S390)
    struct css_ccw1 *ccws;
#else
    struct css_ccw0 *ccws;
#endif
    struct css_device *dev;
    size_t n_ccws;
    int flags;
};

enum css_request_flags {
    CSS_REQUEST_MODIFY = 0x01,
    CSS_REQUEST_IGNORE_CC = 0x02,
};

struct css_request_queue {
    struct css_request *requests;
    size_t n_requests;
};

struct css_request *css_new_request(struct css_device *dev, size_t n_ccws);
void css_destroy_request(struct css_request *req);
void css_send_request(struct css_request *req);
int css_do_request(struct css_request *req);

int ModProbeCss(void);

#endif