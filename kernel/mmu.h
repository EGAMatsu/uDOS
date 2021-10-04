#ifndef MMU_H
#define MMU_H

#include <stddef.h>
#include <stdint.h>

/* Include the arch specific MMU definitions */
#include <arch/mmu.h>

struct mmu_dev {
    size_t numeric_data[2];
    uint8_t binary_data[16];
    void *extended;
};

int mmu_turn_on(struct mmu_dev *dev);
int mmu_turn_off(struct mmu_dev *dev);
int mmu_map(struct mmu_dev *dev, void *phys, void *virt);
int mmu_unmap(struct mmu_dev *dev);
int mmu_virt2phys(struct mmu_dev *dev);
int mmu_set_vspace(struct mmu_dev *dev, void *data);
void *mmu_get_vspace(struct mmu_dev *dev);

#endif