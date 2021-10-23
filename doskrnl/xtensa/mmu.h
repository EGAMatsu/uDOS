#ifndef XTENSA_MMU_H
#define XTENSA_MMU_H

#include <stddef.h>
#include <stdint.h>

typedef void *virtual_space_t;

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