#include <cpu.h>
#include <mmu.h>
#include <panic.h>
#include <pmm.h>
#include <memory.h>

/* Segment tables for the current kernel address space */
segment_entry_t *g_segtab = NULL;

int HwTurnOnMmu(struct dat_device *dev)
{
    unsigned int cr1;
    size_t i;

    g_segtab =  MmAllocatePhysical(sizeof(segment_entry_t) * 16, 4096);
    if(g_segtab == NULL) {
        KePanic("Out of memory\r\n");
    }
    KeSetMemory(g_segtab, 0, sizeof(segment_entry_t) * 16);

    for(i = 0; i < 16; i++) {
        void *ptab;

        ptab =  MmAllocatePhysical(sizeof(page_entry_t) * 256, 4096);
        if(ptab == NULL) {
            KePanic("Out of memory\r\n");
        }
        KeSetMemory(ptab, 0, sizeof(page_entry_t) * 256);
        g_segtab[i] = S390_STE_PT_ORIGIN((unsigned int)ptab);

        KeDebugPrint("STE: %p -> %p\n", (i * 4096), ptab);
    }

    /* Set origin real address of the segment table */
    cr1 = S390_CR1_PSGT_ORIGIN((unsigned int)g_segtab);
    /*
    __asm__ __volatile__(
        "LCTL 1, 1, %0"
        :
        : "m"(cr1)
        : "memory"
    );
    */
    return 0;
}

int HwTurnOffMmu(struct dat_device *dev)
{
    return 0;
}

int HwMapPage(struct dat_device *dev, void *phys, void *virt)
{
    return 0;
}

int HwUnmapPage(struct dat_device *dev)
{
    return 0;
}

int HwVirtualToPhysical(struct dat_device *dev)
{
    return 0;
}

int HwSetVirtualSpace(struct dat_device *dev, isovirt_space data)
{
    return 0;
}

isovirt_space HwGetVirtualSpace(struct dat_device *dev)
{
    return dev->vspace;
}
