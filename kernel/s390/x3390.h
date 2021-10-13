#ifndef X3390_H
#define X3390_H

#include <s390/asm.h>
#include <stddef.h>
#include <stdint.h>

#define X3390_CMD_SEEK 0x07
#define X3390_CMD_LD 0x0E
#define X3390_CMD_SEARCH 0x31

int x3390_init(void);

#endif