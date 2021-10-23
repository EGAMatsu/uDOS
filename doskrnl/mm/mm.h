#ifndef _MALLOC_H
#define _MALLOC_H

#include <stddef.h>
#include <stdint.h>
#include <mm/pmm.h>
#include <memory.h>

void *MmAllocate(size_t size);
void *MmAllocateArray(size_t n, size_t size);
void *MmAllocateZero(size_t size);
void *MmAllocateZeroArray(size_t n, size_t size);
void *MmReallocate(void *ptr, size_t size);
void *MmReallocateArray(void *ptr, size_t n, size_t size);
void MmFree(void *ptr);

#endif