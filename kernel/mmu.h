#ifndef S390_MMU_H
#define S390_MMU_H

#include <stddef.h>
#include <stdint.h>
#include <asm.h>

#if (MACHINE > 390u)
#error "Unimplemented paging"
#else

#if (MACHINE > 370u)
typedef uint32_t segment_entry_t;
typedef uint32_t page_entry_t;
#else
typedef uint16_t segment_entry_t;
typedef uint16_t page_entry_t;
#endif

#   define S390_STE_PT_ORIGIN(x) ((x) << S390_BIT(32, 1))
#   define S390_STE_INVALID ((1) << S390_BIT(32, 26))
#   define S390_STE_COMMON ((1) << S390_BIT(32, 27))

/* Length of the page entry table (in multiples of 16 entries/64 bytes) */
#   define S390_STE_PT_LENGTH(x) ((x) << S390_BIT(32, 28))
#endif

typedef struct _isovirt_space {
    /* TODO: Put fields here */
    
    /*
    segment_entry_t segtable[4096];
    page_entry_t pgtable[4096][4096];
    */
}isovirt_space;

struct dat_device {
    isovirt_space vspace;
};

int HwTurnOnMmu(struct dat_device *dev);
int HwTurnOffMmu(struct dat_device *dev);
int HwMapPage(struct dat_device *dev, void *phys, void *virt);
int HwUnmapPage(struct dat_device *dev);
int HwVirtualToPhysical(struct dat_device *dev);
int HwSetVirtualSpace(struct dat_device *dev, isovirt_space space);
isovirt_space HwGetVirtualSpace(struct dat_device *dev);

#endif
