/* pmm.c
 *
 * Implements a linked-list algorithm for managing the physical memory of a
 * system - note that the actual memory probing depends on arch's kinit function
 * which should call the MmCreateRegion function adequately
 */

#include <stdint.h>
#include <stddef.h>
#include <pmm.h>
#include <panic.h>
#include <memory.h>
#include <assert.h>

#define MAX_PMM_REGIONS 8

static struct pmm_table {
    struct PmmRegion regions[MAX_PMM_REGIONS];
}g_pmm_regions = {0};

struct PmmRegion *MmCreateRegion(void *base, size_t size)
{
    size_t i;

    for(i = 0; i < MAX_PMM_REGIONS; i++) {
        struct PmmRegion *region = &g_pmm_regions.regions[i];
        if(region->flags != PMM_REGION_NOT_PRESENT) {
            continue;
        }

        region->base = base;
        region->size = size;
        region->head = region->base;
        region->flags = PMM_REGION_PUBLIC;

        region->head[0].size = ((region->size / 2048) + 1) * sizeof(struct PmmBlock);
        region->head[0].flags = PMM_BLOCK_USED;
        region->head[0].next = &region->head[1];
        region->head[0].job_id = (job_t)-1;

        /* Block is created next to the head */
        region->head[1].size = region->size - region->head[0].size;
        region->head[1].flags = PMM_BLOCK_FREE;
        region->head[1].next = NULL;
        region->head[1].job_id = (job_t)-1;

        DEBUG_ASSERT(region->head[0].size + region->head[1].size == region->size);
        return region;
    }

    KePanic("TODO: Do a method for getting more regions\r\n");
    return NULL;
}

void MmDeleteRegion(struct PmmRegion *region)
{
    KeSetMemory(region, 0, sizeof(struct PmmRegion));
    return;
}

static struct PmmBlock *MmCreateBlock(struct PmmRegion *region, size_t size, unsigned char flags, struct PmmBlock *next)
{
    struct PmmBlock *heap = (struct PmmBlock *)region->base;
    struct PmmBlock *block;
    size_t n_blocks = region->head->size / sizeof(struct PmmBlock);
    size_t i;

    for(i = 0; i < n_blocks; i++) {
        block = &heap[i];
        if(block->flags != PMM_BLOCK_NOT_PRESENT) {
            continue;
        }
        goto set_block;
    }

    if(heap[1].flags != PMM_BLOCK_FREE || heap[1].size < sizeof(struct PmmBlock) * 32) {
        KePanic("Out of memory for heap\r\n");
    }

    heap[0].size += sizeof(struct PmmBlock) * 32;
    heap[1].size -= sizeof(struct PmmBlock) * 32;
    block = &heap[n_blocks];
    
    /* Set all to zero when expanding to prevent spurious blocks */
    /* n_blocks = region->head->size / sizeof(struct PmmBlock); */
    KeSetMemory(&heap[n_blocks], 0, sizeof(struct PmmBlock) * 32);
set_block:
    block->flags = flags;
    block->size = size;
    block->next = next;
    return block;
}

#if defined(DEBUG)
static void MmCheckHeap(void)
{
    size_t i;

    for(i = 0; i < MAX_PMM_REGIONS; i++) {
        const struct PmmRegion *region = &g_pmm_regions.regions[i];
        const struct PmmBlock *block = region->head;
        const size_t n_blocks = region->head->size / sizeof(struct PmmBlock);
        size_t size = 0, free = 0, used = 0;
        size_t map_size = 0;

        if(region->flags != PMM_REGION_PUBLIC) {
            continue;
        }

        KeDebugPrint("Check for region %zu (with %zu blocks)\r\n", i, n_blocks);
        while(block != NULL) {
            size += block->size;
            if(block->flags == PMM_BLOCK_FREE) {
                free += block->size;
            } else if(block->flags == PMM_BLOCK_USED) {
                used += block->size;
            }

            /*KeDebugPrint("%p -> %p\r\n", block, block->next);*/
            block = block->next;
            if(block == block->next) {
                KePanic("Self reference to self block\r\n");
            }
        }

        if(size != region->size) {
            KePanic("Size recollected %zu... but it should be %zu!\r\n", size, region->size);
        }
        
        KeDebugPrint("Memory Stats: %zu free, %zu used\r\n", free, used);
    }
    return;
}
#endif

/* TODO: Aligned allocations (where align != 0) breaks everything! - watch out for that! */
void *MmAllocatePhysical(size_t size, size_t align)
{
    size_t i;

    if(size == 0) {
        KePanic("Invalid size\r\n");
    }

    for(i = 0; i < MAX_PMM_REGIONS; i++) {
        const struct PmmRegion *region = &g_pmm_regions.regions[i];
        struct PmmBlock *block = region->head;
        unsigned int current_ptr = (unsigned int)region->base;

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
            if((align && (unsigned int)block->size < size + (current_ptr % align)) || (block->size < size)) {
                goto next_block;
            }

            /* Create a remaining "free" block */
            if(align) {
                left_size = (current_ptr + block->size - size) - ((current_ptr + block->size - size) % align) - current_ptr;
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
                block->next = MmCreateBlock(region, next_size, PMM_BLOCK_USED, block->next);
                block = block->next;
            }

            /* Check for alignment (if any) after this left block */
            current_ptr += (unsigned int)left_size;

            /* It must be aligned by now, otherwise the algorithm is faulty */
#if defined(DEBUG)
            if(align) {
                DEBUG_ASSERT(current_ptr % align == 0);
            }
#endif

            block->flags = PMM_BLOCK_USED;
            block->job_id = KeGetCurrentJobId();

            /* Create a block on the right (next) to this block if there are any
             * remaining bytes */
            right_size = block->size - size;
            if(right_size) {
                block->next = MmCreateBlock(region, right_size, PMM_BLOCK_FREE, block->next);
            }

            /* After we finally sliced up the block we can finally use it */
            block->size = size;
#if defined(DEBUG)
            KeDebugPrint("Allocated block of %zu size (and %zu alignment)\r\n", size, align);
            MmCheckHeap();
#endif
            return (void *)current_ptr;
        next_block:
            current_ptr += block->size;
            block = block->next;
            continue;
        }
    }

#if defined(DEBUG)
    KeDebugPrint("Can't allocate block of %zu size (and %zu alignment)\r\n", size, align);
    MmCheckHeap();
#endif
    return NULL;
}

/* Free a block of physical memory, Note that it is the caller's responsability to assert that ptr != NULL */
void  MmFreePhysical(void *ptr)
{
    size_t i;

    DEBUG_ASSERT(ptr != NULL);

    for(i = 0; i < MAX_PMM_REGIONS; i++) {
        const struct PmmRegion *region = &g_pmm_regions.regions[i];
        struct PmmBlock *block = region->head;
        struct PmmBlock *prev = NULL;
        unsigned int current_ptr = (unsigned int)region->base;

        if(region->flags != PMM_REGION_PUBLIC) {
            continue;
        }

        /* Pointer must be also inside region */
        if((unsigned int)ptr < (unsigned int)region->base || (unsigned int)ptr > (unsigned int)region->base + region->size) {
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
            if(prev != NULL && prev->flags == PMM_BLOCK_FREE) {
                current_ptr -= block->size;

                block->flags = PMM_BLOCK_NOT_PRESENT;
                prev->next = block->next;
                prev->size += block->size;

                block = prev;
                prev = NULL;
            }

            /* Free the requested block */
            if((unsigned int)ptr >= current_ptr && (unsigned int)ptr <= current_ptr + block->size - 1) {
                block->flags = PMM_BLOCK_FREE;
                return;
            }
            
            if(current_ptr > ptr) {
#if defined(DEBUG)
                KeDebugPrint("Block %p not found\r\n", ptr);
#endif
                return;
            }
        next_block:
            current_ptr += block->size;
            prev = block;
            block = block->next;
            continue;
        }
    }

#if defined(DEBUG)
    MmCheckHeap();
#endif
    return;
}
