#ifndef X2703_H
#define X2703_H

#include <s390/css.h>

int ModAddX2703Device(struct css_schid schid, struct css_senseid *sensebuf);
int ModInitX2703(void);

#endif