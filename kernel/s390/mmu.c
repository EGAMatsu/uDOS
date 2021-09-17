#include <stddef.h>
#include <stdint.h>
#include <s390/cpu.h>
#include <s390/mmu.h>
#include <panic.h>
#include <cpu.h>
#include <pmm.h>

typedef uint64_t s390_segment_entry_t;
typedef uint64_t s390_page_entry_t;

static int s390_turn_on(struct mmu_dev *dev) {
    uint64_t *cr_list = (uint64_t *)cpu_get_current();
    s390_segment_entry_t *kern_seg_table;
    size_t i;

    s390_store_then_or_system_mask(0x04);

#ifdef __s390x__
    __asm__ __volatile__("stctg 0, 15, %0" : "=m"(*cr_list) : :);
#endif

    kern_seg_table = pmm_alloc(4096 * 4, 4096);
    if (kern_seg_table == NULL) {
        kpanic("Out of memory");
    }
    cr_list[1] = (uint64_t)kern_seg_table;
    cr_list[1] |= S390_CR1_TABLE_LEN_CTRL(0x03);
    cr_list[1] |= S390_CR1_PDT_CTRL(S390_TABLE_TYPE_SEGMENT);
    for (i = 0; i < 4096 * 4; i++) {
        kern_seg_table[i] = S390_STE_SEGMENT_INVALID(1);
    }

#ifdef __s390x__
    __asm__ __volatile__("lctlg 0, 15, %0" : : "m"(cr_list) :);
#endif
    return 0;
}

static int s390_turn_off(struct mmu_dev *dev) { return 0; }

static int s390_map(struct mmu_dev *dev, void *phys, void *virt) { return 0; }

static int s390_unmap(struct mmu_dev *dev) { return 0; }

static int s390_virt2phys(struct mmu_dev *dev) { return 0; }

static int s390_set_vspace(struct mmu_dev *dev, void *data) { return 0; }

static void *s390_get_vspace(struct mmu_dev *dev) { return NULL; }

struct mmu_dev s390_mmu = {.turn_on = &s390_turn_on,
    .turn_off                       = &s390_turn_off,
    .map                            = &s390_map,
    .unmap                          = &s390_unmap,
    .virt2phys                      = &s390_virt2phys,
    .set_vspace                     = &s390_set_vspace,
    .get_vspace                     = &s390_get_vspace};