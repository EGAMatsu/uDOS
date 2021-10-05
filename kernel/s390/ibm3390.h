#ifndef IBM3390_H
#define IBM3390_H

#include <s390/asm.h>
#include <stddef.h>
#include <stdint.h>

#define IBM3390_CMD_SEEK 0x07
#define IBM3390_CMD_LD 0x0E
#define IBM3390_CMD_SEARCH 0x31

int ibm3390_init(void);

#endif