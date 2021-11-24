#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

void *RtlAllocateMemory(size_t size);
void *RtlReallocateMemory(void *p, size_t size);
void RtlFreeMemory(void *p);

#endif
