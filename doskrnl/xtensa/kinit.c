#include <memory.h>
#include <mm/pmm.h>

extern void *heap_start;

int kinit(
    void)
{
    /* ********************************************************************** */
    /* PHYSICAL MEMORY MANAGER                                                */
    /* ********************************************************************** */
    MmCreateRegion(&heap_start, 0xFFFF * 16);

    kmain();
    return 0;
}