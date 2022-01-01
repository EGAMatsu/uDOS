#include <memory.h>
#include <mm/pmm.h>

int KeInit(
    void)
{
	__asm__ __volatile__(
		"movb $0xE9, %%al\r\n"
		"outb %%al, $65\r\n"
		:
		:
		: "al"
	);
	
    /* ********************************************************************** */
    /* PHYSICAL MEMORY MANAGER                                                */
    /* ********************************************************************** */
    /*MmCreateRegion(&heap_start, 0xFFFF * 16);*/
	MmCreateRegion((void *)0xF00000, 0xFFFF * 16);

    KeMain();
    return 0;
}