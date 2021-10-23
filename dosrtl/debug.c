#include <debug.h>
#include <krnl32.h>

void RtlDebugPrint(
    const char *str)
{
    S390_DO_SVC(100, (uintptr_t)str, 0, 0, NULL);
    return;
}