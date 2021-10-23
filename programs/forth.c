#include <rtl.h>

/* Path list for includes */
LIST_SINGLE_INSTANCE(g_inc_plist, char *);

int main(
    void)
{
    RtlDebugPrint("Licensed under the Unlicense license\r\n");
    RtlDebugPrint("FORTH interpreter\r\n");
    RtlDebugPrint("Hello app world!\r\n");
    return 0;
}
