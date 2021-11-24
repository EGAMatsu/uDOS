#ifndef X2703_H
#define X2703_H

#include <s390/css.h>

#define ModAddX2703Device _Ma2703d
int ModAddX2703Device(struct css_schid schid, struct css_senseid *sensebuf);
#define ModInitX2703 _Mi2703
int ModInitX2703(void);

#endif
