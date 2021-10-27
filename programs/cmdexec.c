/* cmdexec.c
 *
 * Command execution management console
 */

#include <rtl.h>

/* Path list for includes */
LIST_SINGLE_INSTANCE(g_inc_plist, char *);

/* List for macros */
LIST_SINGLE_INSTANCE(g_mac_plist, char *);

/* Binder list for binding multiple objects */
LIST_SINGLE_INSTANCE(g_bind_plist, char *);

const char *p = "         ";

int PgMain(
    ExecParams *exec)
{
    Dataset *ds;
    char tmpbuf[80];

    ds = RtlOpenDataset("C:\\", "R");
    while(RtlReadDatasetLine(ds, &tmpbuf, 80) != 0) {
        /* Ignore comments */
        if(tmpbuf[0] == '*') {
            continue;
        }
    }

    RtlDebugPrint("Hello app world!\r\n");
    return 0;
}
