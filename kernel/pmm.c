#include <pmm.h>
#include <panic.h>
#include <stdint.h>
#include <string.h>

#define MAX_PMM_REGIONS 8

struct pmm_table {
    struct pmm_region regions[MAX_PMM_REGIONS];
}g_phys_mem_table = {0};

struct pmm_region *pmm_create_region(
    void *base,
    size_t size)
{
    size_t i;

    for(i = 0; i < MAX_PMM_REGIONS; i++) {
        struct pmm_region *region = &g_phys_mem_table.regions[i];
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

        size_t free = 0, used = 0;
        for(i = 0; i < n_blocks; i++) {
            const struct pmm_block *b_block = &heap[i];
            //kprintf("Found suitable allocation block at index %i (size of %zu)\n", i, size);
            if(b_block->flags == PMM_BLOCK_FREE) {
                free += b_block->size;
            } else if(b_block->flags == PMM_BLOCK_USED) {
                used += b_block->size;
            }
        }
        kprintf("Memory Stats: %zu free, %zu used\n", free, used);
        kprintf("Found suitable allocation block at index %i (size of %zu)\n", i, size);

        goto set_block;
    }

    if(heap[1].flags != PMM_BLOCK_FREE
    || heap[1].size < sizeof(struct pmm_block)) {
        kpanic("Out of memory for heap");
    }

    heap[0].size += sizeof(struct pmm_block);
    heap[1].size -= sizeof(struct pmm_block);
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
        const struct pmm_region *region = &g_phys_mem_table.regions[i];
        const struct pmm_block *block = region->head;
        size_t size = 0;

        while(block != NULL) {
            size += block->size;
            block = block->next;
        }

        if(size != region->size) {
            kpanic("(ALLOC) Size recollected %zu... but it should be %zu!", size,
                region->size);
        }
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

    for(i = 0; i < MAX_PMM_REGIONS; i++) {
        struct pmm_region *region = &g_phys_mem_table.regions[i];
        struct pmm_block *block = region->head;
        uintptr_t current_ptr = (uintptr_t)region->base;

        if(region->flags != PMM_REGION_PUBLIC) {
            continue;
        }

        while(block != NULL) {
            size_t left_size, right_size;
            struct pmm_block *new_block;

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
            block->flags = PMM_BLOCK_USED;

            current_ptr += (uintptr_t)left_size;
            if(align && current_ptr % align != 0) {
                kpanic("Unalign alloc of %zu bytes in %p", left_size,
                    current_ptr);
            }

            /* Create a block on the right (next) to this block if there are any
             * remaining bytes */
            right_size = block->size - size;
            if(right_size) {
                block->next = pmm_create_block(region, right_size, PMM_BLOCK_FREE,
                    block->next);

                kprintf("RightSize Alloc %p -> %p\n", block, block->next);
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

void pmm_free(void *ptr)
{
    size_t i;

    for(i = 0; i < MAX_PMM_REGIONS; i++) {
        struct pmm_region *region = &g_phys_mem_table.regions[i];
        struct pmm_block *block = region->head;
        uintptr_t current_ptr = (uintptr_t)region->base;
        while(block != NULL) {
            if(block->flags != PMM_BLOCK_FREE) {
                goto next_block;
            }

            /* Coalescence */
            if(block->next != NULL && block->next->flags == PMM_BLOCK_FREE) {
                block->next->flags = PMM_BLOCK_NOT_PRESENT;
                block->size += block->next->size;
                block->next = block->next->next;
                kprintf("Coalsence %p -> %p\n", block, block->next);
            }

            /* Free the requested block */
            if((uintptr_t)ptr >= current_ptr &&
                (uintptr_t)ptr <= current_ptr + block->size) {
                block->flags = PMM_BLOCK_FREE;
            }

        next_block:
            if(block == block->next) {
                kpanic("Self reference!");
            }

            current_ptr += block->size;
            block = block->next;
            continue;
        }
    }

    pmm_sanity_check();
    return;
}