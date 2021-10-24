#ifndef INTERACT_H
#define INTERACT_H

#include <stddef.h>

int RtlPromptInputChar(void);
void RtlPromptInputString(char *str, size_t n);

#endif