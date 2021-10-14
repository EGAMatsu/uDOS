/* pmm.c
 *
 * Implements a linked-list algorithm for managing the physical memory of a
 * system - note that the actual memory probing depends on arch's kinit function
 * which should call the pmm_create_region function adequately
 */

#include <pmm.h>
#include <panic.h>
#include <stdint.h>
#include <string.h>

#define MAX_PMM_REGIONS 8

struct pmm_table {
    struct pmm_region regions[MAX_PMM_REGIONS];
}g_pmm_regions = {0};

struct pmm_region *pmm_create_region(
    void *base,
    size_t size)
{
    size_t i;

    for(i = 0; i < MAX_PMM_REGIONS; i++) {
        struct pmm_region *region = &g_pmm_regions.regions[i];
        if(region->flags != PMM_REGION_NOT_PRESENT) {
            continue;
        }

        region->base = base;
        region->size = size;
        region->head = region->base;
        region->flags = PMM_REGION_PUBLIC;

        region->head[0].size = ((region->size / 1024) + 1) * sizeof(struct pmm_block);
        region->head[0].flags = PMM_BLOCK_USED;
        region->head[0].next = &region->head[1];

        /* Block is created next to the head */
        region->head[1].size = region->size - region->head[0].size;
        region->head[1].flags = PMM_BLOCK_FREE;
        region->head[1].next = NULL;
        return region;
    }

    kpanic("TODO: Do a method for getting more regions");
    return NULL;
}

void pmm_delete_region(
    struct pmm_region *region)
{
    memset(region, 0, sizeof(struct pmm_region));
    return;
}

struct pmm_block *pmm_create_block(
    struct pmm_region *region,
    size_t size,
    unsigned char flags,
    struct pmm_block *next)
{
    struct pmm_block *heap = (struct pmm_block *)region->base;
    struct pmm_block *block;
    size_t n_blocks = region->head->size / sizeof(struct pmm_block);
    size_t i;

    for(i = 0; i < n_blocks; i++) {
        block = &heap[i];
        if(block->flags != PMM_BLOCK_NOT_PRESENT) {
            continue;
        }
        goto set_block;
    }

    if(heap[1].flags != PMM_BLOCK_FREE
    || heap[1].size < sizeof(struct pmm_block) * 32) {
        kpanic("Out of memory for heap");
    }

    heap[0].size += sizeof(struct pmm_block) * 32;
    heap[1].size -= sizeof(struct pmm_block) * 32;
    block = &heap[n_blocks];

set_block:
    block->flags = flags;
    block->size = size;
    block->next = next;
    return block;
}

void pmm_sanity_check(
    void)
{
    size_t i;

    for(i = 0; i < MAX_PMM_REGIONS; i++) {
        const struct pmm_region *region = &g_pmm_regions.regions[i];
        const struct pmm_block *block = region->head;
        size_t n_blocks = region->head->size / sizeof(struct pmm_block);
        size_t size = 0, free = 0, used = 0;

        if(region->flags != PMM_REGION_PUBLIC) {
            continue;
        }

        /*kprintf("Check for region %zu (with %zu blocks)\n", i, n_blocks);*/
        while(block != NULL) {
            size += block->size;
            if(block->flags == PMM_BLOCK_FREE) {
                free += block->size;
            } else if(block->flags == PMM_BLOCK_USED) {
                used += block->size;
            }

            /*kprintf("%p -> %p\n", block, block->next);*/
            block = block->next;
            if(block == block->next) {
                kpanic("Self reference to self block");
            }
        }

        if(size != region->size) {
            kpanic("Size recollected %zu... but it should be %zu!", size,
                region->size);
        }
        /*kprintf("Memory Stats: %zu free, %zu used\n", free, used);*/
    }
    return;
}

/* TODO: Aligned allocations (where align != 0) breaks everything! - watch out
 * for that! */
void *pmm_alloc(
    size_t size,
    size_t align)
{
    size_t i;

    if(size == 0) {
        kpanic("Invalid size");
    }

    for(i = 0; i < MAX_PMM_REGIONS; i++) {
        struct pmm_region *region = &g_pmm_regions.regions[i];
        struct pmm_block *block = region->head;
        uintptr_t current_ptr = (uintptr_t)region->base;

        if(region->flags != PMM_REGION_PUBLIC) {
            continue;
        }

        while(block != NULL) {
            size_t left_size, right_size;

            /* Check that the block is not used */
            if(block->flags == PMM_BLOCK_USED) {
                goto next_block;
            }

            /* Check that the block is big enough to hold our aligned object
             * (if there is any align of course) */
            if((align && (uintptr_t)block->size < size + (current_ptr % align))
            || (block->size < size)) {
                goto next_block;
            }

            /* Create a remaining "free" block */
            if(align) {
                left_size = (current_ptr + block->size - size) -
                    ((current_ptr + block->size - size) % align) - current_ptr;
            }
            /* No alignment - so only size is took in account */
            else {
                left_size = block->size - size;
            }
            
            /* Create a block on the left (previous) to this block */
            if(left_size) {
                size_t next_size = block->size - left_size;
                block->size = left_size;
                block->flags = PMM_BLOCK_FREE;
                block->next = pmm_create_block(region, next_size,
                    PMM_BLOCK_USED, block->next);
                block = block->next;
            }

            /* Check for alignment (if any) after this left block */
            current_ptr += (uintptr_t)left_size;
            if(align && current_ptr % align != 0) {
                kpanic("Unalign alloc of %zu bytes in %p", left_size,
                    current_ptr);
            }

            block->flags = PMM_BLOCK_USED;

            /* Create a block on the right (next) to this block if there are any
             * remaining bytes */
            right_size = block->size - size;
            if(right_size) {
                block->next = pmm_create_block(region, right_size,
                    PMM_BLOCK_FREE, block->next);
            }

            /* After we finally sliced up the block we can finally use it */
            block->size = size;

            pmm_sanity_check();
            return (void *)current_ptr;
        next_block:
            current_ptr += block->size;
            block = block->next;
            continue;
        }
    }

    pmm_sanity_check();
    return NULL;
}

void pmm_free(
    void *ptr)
{
    size_t i;

    for(i = 0; i < MAX_PMM_REGIONS; i++) {
        struct pmm_region *region = &g_pmm_regions.regions[i];
        struct pmm_block *block = region->head;
        struct pmm_block *prev = NULL;
        uintptr_t current_ptr = (uintptr_t)region->base;

        if(region->flags != PMM_REGION_PUBLIC) {
            continue;
        }

        /* Pointer must be also inside region */
        if(!((uintptr_t)region->base >= ptr
        && ptr <= (uintptr_t)region->base + region->size)) {
            continue;
        }

        while(block != NULL) {
            if(block->flags != PMM_BLOCK_FREE) {
                goto next_block;
            }

            /* Coalescence after */
            if(block->next != NULL && block->next->flags == PMM_BLOCK_FREE) {
                block->next->flags = PMM_BLOCK_NOT_PRESENT;
                block->size += block->next->size;
                block->next = block->next->next;
            }

            /* Coalescence behind */
            if(prev->flags == PMM_BLOCK_FREE) {
                prev->next = block->next;
                prev->size += block->size;
                block->flags = PMM_BLOCK_NOT_PRESENT;

                block = prev;
            }

            /* Free the requested block */
            if((uintptr_t)ptr >= current_ptr &&
                (uintptr_t)ptr <= current_ptr + block->size) {
                block->flags = PMM_BLOCK_FREE;
                break;
            }

        next_block:
            current_ptr += block->size;
            prev = block;
            block = block->next;
            continue;
        }
    }

    pmm_sanity_check();
    return;
}