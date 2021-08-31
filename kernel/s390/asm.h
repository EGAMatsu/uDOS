#ifndef S390_ASM_H
#define S390_ASM_H
#ifdef __cplusplus
extern "C" {
#endif

#define S390_BIT(n, x)                                                  \
    ((n)-1 - (x)) /* s390 manual describes bits as MSB - so for sake of \
                     readability we do this */

#ifdef __cplusplus
}
#endif
#endif