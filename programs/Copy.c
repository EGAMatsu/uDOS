/* copy.c
 *
 * Sample COPY program (works only with datasets)
 */

#include <rtl.h>

int PgMain(ExecParams *exec)
{
    Dataset *out_ds;
    size_t i;

    RtlDebugPrint("SAMPLE.003 - Dataset copy\r\n");

    if(exec->n_dsnames < 2) {
        RtlDebugPrint("Please specify atleast 2 datasets (in...) (out)\r\n");
    }

    out_ds = RtlOpenDataset(exec->dsnames[exec->n_dsnames - 1], "W");
    for(i = 0; i < exec->n_dsnames - 1; i++) {
        Dataset *in_ds;
        int ch;

        in_ds = RtlOpenDataset(exec->dsnames[i], "R");
        while((ch = RtlReadDatasetChar(out_ds)) != EOF) {
            RtlWriteDataset(out_ds, &ch, sizeof(ch));
        }
        RtlCloseDataset(in_ds);
    }
    RtlCloseDataset(out_ds);
    return 0;
}
