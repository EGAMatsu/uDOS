/* newsgrp.c
 *
 * Newsgroup reader using the BSC module
 * 
 * Original written by Paul Edwards
 * See: https://sourceforge.net/p/pdos/gitcode/ci/master/tree/src/pdpnntp.c
 */

#include <Rtl.h>

char buf[512];

int PgMain(
    ExecParams *exec)
{
    Dataset *ds;

    RtlDebugPrint("SAMPLE.003 - Newsgroup reader program\r\n");
    ds = RtlOpenDataset("A:\\MODULES\\BSC", "R");
    if(ds == NULL) {
        RtlPrintError("Cannot open BSC module\r\n");
        return -1;
    }

    RtlReadDataset(ds, buf, sizeof(buf));
    RtlWriteDataset(ds, "LIST\r\n", 6);
    while(1) {
        RtlSetMemory(buf, 0, sizeof(buf));
        RtlReadDataset(ds, buf, sizeof(buf));
        if(buf[0] == '\0') {
            continue;
        }
        
        if(buf[0] == '.') {
            break;
        }

        RtlDebugPrint(buf);
    }
    RtlCloseDataset(ds);
    return 0;
}
