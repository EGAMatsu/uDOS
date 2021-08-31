#ifndef S390_DASD_H
#define S390_DASD_H
#ifdef __cplusplus
extern "C" {
#endif

#include <s390/asm.h>
#include <stddef.h>
#include <stdint.h>

#define DASD_CMD_SEEK 0x07
#define DASD_CMD_SEARCH 0X31
#define DASD_CMD_TIC 0x08
#define DASD_CMD_LD 0x0E

#ifdef __cplusplus
}
#endif
#endif