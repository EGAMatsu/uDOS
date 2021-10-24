#include <s390/cpu.h>
#include <s390/mmu.h>
#include <debug/panic.h>
#include <mm/pmm.h>

/* Segment tables for the current kernel address space */
segment_entry_t *g_segtab = NULL;

int HwTurnOnMmu(
    struct MmuDevice *dev)
{
    uintptr_t cr1;
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
        g_segtab[i] = S390_STE_PT_ORIGIN((uintptr_t)ptab);

        KeDebugPrint("STE: %p -> %p\n", (i * 4096), ptab);
    }

    /* Set origin real address of the segment table */
    cr1 = S390_CR1_PSGT_ORIGIN((uintptr_t)g_segtab);
    __asm__ __volatile__(
        "lctl 1, 1, %0"
        :
        : "m"(cr1)
        : "memory"
    );
    return 0;
}

int HwTurnOffMmu(
    struct MmuDevice *dev)
{
    return 0;
}

int HwMapPage(
    struct MmuDevice *dev,
    void *phys,
    void *virt)
{
    return 0;
}

int HwUnmapPage(
    struct MmuDevice *dev)
{
    return 0;
}

int HwVirtualToPhysical(
    struct MmuDevice *dev)
{
    return 0;
}

int HwSetVirtualSpace(
    struct MmuDevice *dev,
    virtual_space_t data)
{
    return 0;
}

virtual_space_t HwGetVirtualSpace(
    struct MmuDevice *dev)
{
    return dev->vspace;
}