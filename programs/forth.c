#include <rtl.h>

/* Path list for includes */
LIST_SINGLE_INSTANCE(g_inc_plist, char *);

int main(
    ExecParams *exec)
{
    RtlDebugPrint("SAMPLE.002 - Forth interpreter\r\n");
    return 0;
}
