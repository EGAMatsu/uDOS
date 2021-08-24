#ifndef S390_MMU_H
#define S390_MMU_H
#ifdef __cplusplus
extern "C" {
#endif

#include <mmu.h>
#include <s390/asm.h>

#define S390_TABLE_TYPE_SEGMENT 0x00
#define S390_TABLE_TYPE_REGION3 0x01
#define S390_TABLE_TYPE_REGION2 0x02
#define S390_TABLE_TYPE_REGION1 0x03

#define S390_STE_PAGE_PROT(x) ((x) << S390_BIT(64, 54)) /* Page protection */
#define S390_STE_SEGMENT_INVALID(x)                                            \
  ((x) << S390_BIT(64, 58)) /* Segment is invalid */
#define S390_STE_COMMON_SEGMENT(x)                                             \
  ((x) << S390_BIT(64, 59)) /* Common segment */
#define S390_STE_TABLE_TYPE(x)                                                 \
  ((x) << S390_BIT(64, 60)) /* Table type for segment */
extern struct mmu_dev s390_mmu;

#ifdef __cplusplus
}
#endif
#endif