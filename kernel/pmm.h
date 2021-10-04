#ifndef PMM_H
#define PMM_H

#include <stddef.h>

#define PMM_BLOCK_NOT_PRESENT 0x00
#define PMM_BLOCK_FREE 0x01
#define PMM_BLOCK_USED 0x02

struct pmm_block {
    size_t size;
    struct pmm_block *next;
    unsigned char flags;
};

#define PMM_REGION_NOT_PRESENT 0x00
#define PMM_REGION_PUBLIC 0x01

struct pmm_region {
    void *base;
    size_t size;
    struct pmm_block *head;
    unsigned char flags;
};

struct pmm_region *pmm_create_region(void *base, size_t size);
void pmm_delete_region(struct pmm_region *region);
void *pmm_alloc(size_t size, size_t align);
void pmm_free(void *ptr);

#endif