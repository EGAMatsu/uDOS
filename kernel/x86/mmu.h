#ifndef X86_MMU_H
#define X86_MMU_H

#include <stddef.h>
#include <stdint.h>

typedef void *virtual_space_t;

struct mmu_dev {
    virtual_space_t vspace;
};

int mmu_turn_on(struct mmu_dev *dev);
int mmu_turn_off(struct mmu_dev *dev);
int mmu_map(struct mmu_dev *dev, void *phys, void *virt);
int mmu_unmap(struct mmu_dev *dev);
int mmu_virt2phys(struct mmu_dev *dev);
int mmu_set_vspace(struct mmu_dev *dev, virtual_space_t space);
virtual_space_t mmu_get_vspace(struct mmu_dev *dev);

#endif