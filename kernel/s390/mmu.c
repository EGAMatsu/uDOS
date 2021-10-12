#include <s390/cpu.h>
#include <s390/mmu.h>
#include <panic.h>
#include <pmm.h>

typedef uint64_t s390_segment_entry_t;
typedef uint64_t s390_page_entry_t;

int mmu_turn_on(
    struct mmu_dev *dev)
{
/*
    uint64_t *cr_list = (uint64_t *)cpu_get_current();
    s390_segment_entry_t *kern_seg_table;
    size_t i;

    / Turn DAT on /
    s390_store_then_or_system_mask(0x04);

#ifdef __s390x__
    __asm__ __volatile__("stctg 0, 15, %0" : "=m"(*cr_list) : :);
#endif

    kern_seg_table = pmm_alloc(4096 * 4, 4096);
    if(kern_seg_table == NULL) {
        kpanic("Out of memory");
    }
    cr_list[1] = (uint64_t)kern_seg_table;
    cr_list[1] |= S390_CR1_TABLE_LEN_CTRL(0x03);
    cr_list[1] |= S390_CR1_PDT_CTRL(S390_TABLE_TYPE_SEGMENT);
    for(i = 0; i < 4096 * 4; i++) {
        kern_seg_table[i] = S390_STE_SEGMENT_INVALID;
    }

#ifdef __s390x__
    __asm__ __volatile__("lctlg 0, 15, %0" : : "m"(cr_list) :);
#endif
*/
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
    void *data)
{
    return 0;
}

void *mmu_get_vspace(
    struct mmu_dev *dev)
{
    return NULL;
}