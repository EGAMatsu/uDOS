#ifndef S390_ASM_H
#define S390_ASM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define S390_BIT(n, x)                                                  \
    ((n)-1 - (x)) /* s390 manual describes bits as MSB - so for sake of \
                     readability we do this */

#define AMBIT 0x80000000
#define AM64 0x1

struct s390_psw {
    uint32_t lo;
    uint32_t hi;
} __attribute__((packed, aligned(8)));

struct s390x_psw {
    uint32_t flags;
    uint32_t flags_ext;
    uint32_t unknown;
    uint32_t address;
} __attribute__((packed, aligned(8)));

#define FLCCAW 0x48
#define FLCSNPSW 0x60
#define FLCPNPSW 0x68
#define FLCINPSW 0x78
#define FLCMNPSW 0x70
#define FLCIOA 0xB8

/* Extended stuff */
#define FLCESNPSW 0x1C0
#define FLCEPNPSW 0x1D0
#define FLCEMNPSW 0x1E0
#define FLCEINPSW 0x1F0

#ifdef __cplusplus
}
#endif
#endif