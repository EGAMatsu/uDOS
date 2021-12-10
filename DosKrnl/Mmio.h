#ifndef MMIO_H
#define MMIO_H

struct MmioArea {
    void *base;
    size_t size;

    uint8_t (*read8)(struct MmioArea *area, uintptr_t offset);
    uint16_t (*read16)(struct MmioArea *area, uintptr_t offset);
    uint32_t (*read32)(struct MmioArea *area, uintptr_t offset);
    uint64_t (*read64)(struct MmioArea *area, uintptr_t offset);
    void *(*read)(struct MmioArea *area, uintptr_t offset, size_t len);

    void (*write8)(struct MmioArea *area, uintptr_t offset, uint8_t data);
    void (*write16)(struct MmioArea *area, uintptr_t offset, uint16_t data);
    void (*write32)(struct MmioArea *area, uintptr_t offset, uint32_t data);
    void (*write64)(struct MmioArea *area, uintptr_t offset, uint64_t data);
    void (*write)(struct MmioArea *area, uintptr_t offset, const void *data,
        size_t len);
};

struct MmioArea *MmioReserveArea(void *base, size_t size);
void MmioFreeArea(void *base);

#endif