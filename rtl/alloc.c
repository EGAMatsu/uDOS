#include <alloc.h>
#include <krnl32.h>

void *RtlAllocateMemory(size_t size)
{
    void *p;
    p = (void *)RtlDoSvc(190, (unsigned int)size, 0, 0);
    return p;
}

void *RtlReallocateMemory(void *p, size_t size)
{
    RtlDoSvc(192, (unsigned int)size, (unsigned int)&p, 0);
    return p;
}

void RtlFreeMemory(void *p)
{
    RtlDoSvc(191, (unsigned int)p, 0, 0);
    return;
}
