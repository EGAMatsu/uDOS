#include <s390/cpu.h>
#include <s390/mmu.h>
#include <panic.h>
#include <pmm.h>

/* Segment tables for the current kernel address space */
segment_entry_t *g_segtab = NULL;

int mmu_turn_on(
    struct mmu_dev *dev)
{
    uintptr_t cr1;
    size_t i;

    g_segtab = pmm_alloc(sizeof(segment_entry_t) * 16, 4096);
    if(g_segtab == NULL) {
        kpanic("Out of memory\r\n");
    }
    memset(g_segtab, 0, sizeof(segment_entry_t) * 16);

    for(i = 0; i < 16; i++) {
        void *ptab;

        ptab = pmm_alloc(sizeof(page_entry_t) * 256, 4096);
        if(ptab == NULL) {
            kpanic("Out of memory\r\n");
        }
        memset(ptab, 0, sizeof(page_entry_t) * 256);
        g_segtab[i] = S390_STE_PT_ORIGIN((uintptr_t)ptab);

        kprintf("STE: %p -> %p\n", (i * 4096), ptab);
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

int mmu_turn_off(
    struct mmu_dev *dev)
{
    return 0;
}

int mmu_map(
    struct mmu_dev *dev,
    void *phys,
    void *virt)
{
    return 0;
}

int mmu_unmap(
    struct mmu_dev *dev)
{
    return 0;
}

int mmu_virt2phys(
    struct mmu_dev *dev)
{
    return 0;
}

int mmu_set_vspace(
    struct mmu_dev *dev,
    virtual_space_t data)
{
    return 0;
}

virtual_space_t mmu_get_vspace(
    struct mmu_dev *dev)
{
    return dev->vspace;
}