/* forth.c
 *
 * A mini Forth implementation using dosrtl
 */

#include <rtl.h>

/* Path list for includes */
LIST_SINGLE_INSTANCE(g_inc_plist, char *);

char tmpbuf[80 + 1];

int main(
    ExecParams *exec)
{
    RtlDebugPrint("SAMPLE.002 - Forth interpreter\r\n");
    while(1) {
        RtlDebugPrint("> PROMPT?\r\n");
        RtlPromptInputString(&tmpbuf[0], 80);
        RtlDebugPrint(&tmpbuf[0]);
    }
    return 0;
}
