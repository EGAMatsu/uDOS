#ifndef PMM_H
#define PMM_H

#include <stddef.h>
#include <scheduler.h>

#define PMM_BLOCK_NOT_PRESENT 0x00
#define PMM_BLOCK_FREE 0x01
#define PMM_BLOCK_USED 0x02

struct PmmBlock {
    size_t size;
    struct PmmBlock *next;
    unsigned char flags;

    /* This is only accounted for when the block is set as USED, otherwise it is
     * ignored completely */
    job_t job_id;
};

#define PMM_REGION_NOT_PRESENT 0x00
#define PMM_REGION_PUBLIC 0x01

struct PmmRegion {
    void *base;
    size_t size;
    struct PmmBlock *head;
    unsigned char flags;
};

struct PmmRegion *MmCreateRegion(void *base, size_t size);
void MmDeleteRegion(struct PmmRegion *region);
void *MmAllocatePhysical(size_t size, size_t align);
void  MmFreePhysical(void *ptr);

#endif