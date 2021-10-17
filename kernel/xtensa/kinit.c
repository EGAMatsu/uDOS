#include <string.h>
#include <pmm.h>

extern void *heap_start;

int kinit(
    void)
{
    /* ********************************************************************** */
    /* PHYSICAL MEMORY MANAGER                                                */
    /* ********************************************************************** */
    pmm_create_region(&heap_start, 0xFFFF * 16);

    kmain();
    return 0;
}