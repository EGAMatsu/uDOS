#ifndef MMU_H
#define MMU_H

#include <stddef.h>
#include <stdint.h>

struct mmu_data_space {
    size_t numeric_data[2];
    uint8_t binary_data[16];
    void *extended;
};

struct mmu_dev {
    int (*turn_on)(struct mmu_dev *dev);
    int (*turn_off)(struct mmu_dev *dev);

    int (*map)(struct mmu_dev *dev, void *phys, void *virt);
    int (*unmap)(struct mmu_dev *dev);
    int (*virt2phys)(struct mmu_dev *dev);

    int (*set_vspace)(struct mmu_dev *dev, void *data);
    void *(*get_vspace)(struct mmu_dev *dev);

    struct mmu_data_space data;
};

#endif