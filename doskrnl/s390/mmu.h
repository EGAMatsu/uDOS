#ifndef S390_MMU_H
#define S390_MMU_H

#include <stddef.h>
#include <stdint.h>
#include <s390/space.h>
#include <s390/asm.h>

#if (MACHINE >= M_ZARCH)
#error "Unimplemented paging"
#else
typedef uint32_t segment_entry_t;
typedef uint32_t page_entry_t;

#define S390_STE_PT_ORIGIN(x) ((x) << S390_BIT(32, 1))
#define S390_STE_INVALID ((1) << S390_BIT(32, 26))
#define S390_STE_COMMON ((1) << S390_BIT(32, 27))

/* Length of the page entry table (in multiples of 16 entries/64 bytes) */
#define S390_STE_PT_LENGTH(x) ((x) << S390_BIT(32, 28))
#endif

struct MmuDevice {
    virtual_space_t vspace;
};

int HwTurnOnMmu(struct MmuDevice *dev);
int HwTurnOffMmu(struct MmuDevice *dev);
int HwMapPage(struct MmuDevice *dev, void *phys, void *virt);
int HwUnmapPage(struct MmuDevice *dev);
int HwVirtualToPhysical(struct MmuDevice *dev);
int HwSetVirtualSpace(struct MmuDevice *dev, virtual_space_t space);
virtual_space_t HwGetVirtualSpace(struct MmuDevice *dev);

#endif