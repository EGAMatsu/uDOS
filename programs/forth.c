/* forth.c
 *
 * A mini Forth implementation using dosrtl
 */

#include <rtl.h>

/* Path list for includes */
LIST_SINGLE_INSTANCE(g_inc_plist, char *);

/* List for the tokens on the stack */
LIST_SINGLE_INSTANCE(g_stack, char *);

char tmpbuf[80 + 1];

int PgMain(
    ExecParams *exec)
{
    size_t i;

    RtlDebugPrint("SAMPLE.002 - Forth interpreter\r\n");

    while(1) {
        char *str = &tmpbuf[0];

        RtlSetMemory(str, 0, 80);

        RtlDebugPrint(":> \r\n");
        RtlPromptInputString(str, 80);
        RtlDebugPrint(str);
    }
    return 0;
}
