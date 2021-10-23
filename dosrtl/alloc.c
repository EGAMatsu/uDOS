#include <alloc.h>
#include <krnl32.h>

void *RtlAllocateMemory(
    size_t size)
{
    void *p;
    S390_DO_SVC(190, (uintptr_t)size, 0, 0, &p);
    return p;
}

void *RtlReallocateMemory(
    void *p,
    size_t size)
{
    S390_DO_SVC(192, (uintptr_t)size, (uintptr_t)&p, 0, NULL);
    return p;
}

void RtlFreeMemory(
    void *p)
{
    S390_DO_SVC(191, (uintptr_t)p, 0, 0, NULL);
    return;
}