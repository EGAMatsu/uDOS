#include <debug.h>
#include <krnl32.h>

void RtlDebugPrint(
    const char *str)
{
    RtlDoSvc(100, (unsigned int)str, 0, 0);
    return;
}
