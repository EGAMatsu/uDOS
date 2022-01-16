#include <mm.h>
#include <memory.h>
#include <panic.h>

void *MmAllocate(size_t size)
{
    void *ptr;
    ptr =  MmAllocatePhysical(size, 16);
    return ptr;
}

void *MmAllocateArray(size_t n, size_t size)
{
    return MmAllocate(n * size);
}

void *MmAllocateZero(size_t size)
{
    void *ptr;
    ptr = MmAllocate(size);
    if(ptr == NULL) {
        KePanic("Out of memory\r\n");
    }
    KeSetMemory(ptr, 0, size);
    return ptr;
}

void *MmAllocateZeroArray(size_t n, size_t size)
{
    return MmAllocateZero(n * size);
}

void *MmReallocate(void *ptr, size_t size)
{
    void *old_ptr = ptr;
    if(old_ptr == NULL) {
        return MmAllocate(size);
    }
    
    ptr = MmAllocate(size);
    if(ptr == NULL) {
        return NULL;
    }
    KeMoveMemory(ptr, old_ptr, size);
    MmFree(old_ptr);
    return ptr;
}

void *MmReallocateArray(void *ptr, size_t n, size_t size)
{
    return MmReallocate(ptr, n * size);
}

void MmFree(void *ptr)
{
    if(ptr == NULL) {
        return;
    }
    
    MmFreePhysical(ptr);
    return;
}
