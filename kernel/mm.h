#ifndef _MALLOC_H
#define _MALLOC_H

#include <stddef.h>
#include <stdint.h>
#include <pmm.h>
#include <memory.h>

#define MmAllocate _Zma
void *MmAllocate(size_t size);
#define MmAllocateArray _Zmaa
void *MmAllocateArray(size_t n, size_t size);
#define MmAllocateZero _Zmaz
void *MmAllocateZero(size_t size);
#define MmAllocateZeroArray _Zmalza
void *MmAllocateZeroArray(size_t n, size_t size);
#define MmReallocate _Zmreal
void *MmReallocate(void *ptr, size_t size);
#define MmReallocateArray _Zmra
void *MmReallocateArray(void *ptr, size_t n, size_t size);
#define MmFree _Zmfee
void MmFree(void *ptr);

#endif
