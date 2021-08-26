#include <panic.h>
#include <pmm.h>
#include <stdint.h>

#define MAX_PMM_REGIONS 32

struct pmm_table {
  struct pmm_region regions[MAX_PMM_REGIONS];
} g_phys_mem_table = {0};

struct pmm_region *pmm_create_region(void *base, size_t size) {
  size_t i;

  for (i = 0; i < MAX_PMM_REGIONS; i++) {
    struct pmm_region *region = &g_phys_mem_table.regions[i];
    struct pmm_block *heap_block, *free_block;

    if (region->flags != PMM_REGION_NOT_PRESENT) {
      continue;
    }

    region->base = base;
    region->size = size;
    region->head = region->base;
    region->flags = PMM_REGION_PUBLIC;

    heap_block = region->head;
    heap_block->size = ((region->size / 1024) + 1) * sizeof(struct pmm_block);
    heap_block->flags = PMM_BLOCK_USED;
    heap_block->next = region->head + 1;

    /* Block is created next to the head */
    free_block = region->head + 1;
    free_block->size = size - heap_block->size;
    free_block->flags = PMM_BLOCK_FREE;
    free_block->next = NULL;
    return region;
  }

  kpanic("TODO: Do a method for getting more regions");
  return NULL;
}

void pmm_delete_region(struct pmm_region *region) {
  region->base = NULL;
  region->head = NULL;
  region->size = 0;
  return;
}

static struct pmm_block *pmm_create_block(struct pmm_region *region,
                                          size_t size, unsigned char flags,
                                          struct pmm_block *next) {
  struct pmm_block *heap;
  size_t i;

  heap = (struct pmm_block *)region->base;
  for (i = 0; i < region->head->size / sizeof(struct pmm_block); i++) {
    struct pmm_block *block = &heap[i];
    if (block->flags != PMM_BLOCK_NOT_PRESENT) {
      continue;
    }

    block->flags = flags;
    block->size = size;
    block->next = next;
    return block;
  }

  kpanic("TODO: Get a method to create more blocks");
  return NULL;
}

/* TODO: Aligned allocations (where align != 0) breaks everything! - watch out
 * for that! */
void *pmm_alloc(size_t size, size_t align) {
  size_t i;

  for (i = 0; i < MAX_PMM_REGIONS; i++) {
    struct pmm_region *region = &g_phys_mem_table.regions[i];
    struct pmm_block *block = region->head;
    uintptr_t current_ptr = (uintptr_t)region->base;

    if (region->flags != PMM_REGION_PUBLIC) {
      continue;
    }

    while (block != NULL) {
      size_t left_size, right_size;
      struct pmm_block *new_block;

      /* Check that the block is not used */
      if (block->flags == PMM_BLOCK_USED) {
        goto next_block;
      }

      /* Also check that it is big enough to hold the object (with alignment)*/
      if ((align && (uintptr_t)block->size < size + (current_ptr % align)) ||
          (block->size < size)) {
        goto next_block;
      }

      /* Create remaining "free" block */
      if (align) {
        left_size = (current_ptr + block->size - size) -
                    ((current_ptr + block->size - size) % align) - current_ptr;
        right_size = block->size - left_size - size;
      }
      /* No alignment */
      else {
        left_size = block->size - size;
        right_size = 0;
      }

      if (left_size) {
        block->next = pmm_create_block(region, block->size - left_size,
                                       PMM_BLOCK_FREE, block->next);
        block->size = left_size;
        block->flags = PMM_BLOCK_FREE;
        block = block->next;
      }
      current_ptr += (uintptr_t)left_size;
      if (align && current_ptr % align != 0) {
        kpanic("Unaligned alloc of size %zu with address %p", left_size,
               current_ptr);
      }

      if (right_size) {
        new_block =
            pmm_create_block(region, right_size, PMM_BLOCK_FREE, block->next);
        block->next = new_block;
      }

      block->size = size;
      block->flags = PMM_BLOCK_USED;
      return (void *)current_ptr;
    next_block:
      current_ptr += block->size;
      block = block->next;
      continue;
    }
  }
  return NULL;
}

void pmm_free(void *ptr) {
  size_t i;

  for (i = 0; i < MAX_PMM_REGIONS; i++) {
    struct pmm_region *region = &g_phys_mem_table.regions[i];
    struct pmm_block *block = region->head;
    uintptr_t current_ptr = (uintptr_t)region->base;
    while (block != NULL) {
      if (block->flags != PMM_BLOCK_FREE) {
        goto next_block;
      }

      /* Coalescence */
      if (block->next != NULL && block->next->flags == PMM_BLOCK_FREE) {
        block->next->flags = PMM_BLOCK_NOT_PRESENT;
        block->size += block->next->size;
        block->next = block->next->next;
      }

      /* Free the requested block */
      if ((uintptr_t)ptr >= current_ptr &&
          (uintptr_t)ptr <= current_ptr + block->size) {
        block->flags = PMM_BLOCK_FREE;
      }

    next_block:
      current_ptr += block->size;
      block = block->next;
      continue;
    }
  }

  /* Check sizes match */
  for (i = 0; i < MAX_PMM_REGIONS; i++) {
    struct pmm_region *region = &g_phys_mem_table.regions[i];
    struct pmm_block *block = region->head;
    size_t size = 0;

    while (block != NULL) {
      size += block->size;
      block = block->next;
    }

    if (size != region->size) {
      kpanic("Size recollected %zu... but it should be %zu!", (size_t)size,
             (size_t)region->size);
    }
  }
  return;
}