#ifndef X3390_H
#define X3390_H

#include <stddef.h>
#include <stdint.h>
#include <asm.h>

#define X3390_CMD_SEEK 0x07
#define X3390_CMD_LD 0x0E
#define X3390_CMD_SEARCH 0x31

#define ModAdd3390Device _Ma3390d
int ModAdd3390Device(struct css_schid schid, struct css_senseid *sensebuf);
#define ModInit3390 _Mi3390
int ModInit3390(void);

#endif
