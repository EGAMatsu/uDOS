#ifndef INTERACT_H
#define INTERACT_H

#include <stddef.h>

#define RtlPromptInputChar _Zrpic
int RtlPromptInputChar(void);
#define RtlPromptInputString _Zrpis
void RtlPromptInputString(char *str, size_t n);

#endif
