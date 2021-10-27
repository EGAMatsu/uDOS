#include <dataset.h>
#include <krnl32.h>
#include <memory.h>

Dataset *RtlOpenDataset(
    const char *name,
    const char *recfm)
{
    Dataset *ds;
    size_t i;
    int mode;

    for(i = 0; i < RtlStringLength(recfm); i++) {
        switch(recfm[i]) {
        /* Read */
        case 'R':
            mode |= 0x01;
            break;
        /* Write */
        case 'W':
            mode |= 0x02;
            break;
        /* Buffered */
        case 'B':
            mode |= 0x04;
            break;
        /* All flags */
        case 'U':
            mode |= 0xFF;
            break;
        /* Invert (all flags except the ones specified) */
        case '~':
            mode = ~(mode);
            break;
        /* Default */
        case '?':
            /* Read-only, buffered */
            mode |= 0x01 | 0x04;
            break;
        default:
            break;
        }
    }

    ds = RtlAllocateMemory(sizeof(Dataset));
    if(ds == NULL) {
        return NULL;
    }
    ds->handle = (void *)RtlDoSvc(200, (uintptr_t)name, (uintptr_t)recfm, 0);
    return ds;
}

#include <error.h>
void RtlCloseDataset(
    Dataset *ds)
{
    RtlPrintError("RtlCloseDataset not implemented\r\n");
    return;
}

    // /* Close VFS handle */
    // case 201: {
    //     struct FsHandle *hdl = (struct FsHandle *)frame->r1;
    //     KeCloseFsNode(hdl);
    // } break;
    // /* Read FDSCB-mode in handle */
    // case 202: {
    //     struct FsHandle *hdl = (struct FsHandle *)frame->r1;
    //     struct FsFdscb fdscb;
    //     KeCopyMemory(&fdscb, (struct FsFdscb *)frame->r3, sizeof(struct FsFdscb));
    //     /*KeReadWithFdscbFsNode(hdl, &fdscb, (size_t)frame->r2);*/

int RtlReadDatasetLine(
    Dataset *ds,
    char *buffer,
    size_t n)
{
    RtlPrintError("RtlReadDatasetLine not implemented\r\n");
    return 0;
}

int RtlReadDatasetChar(
    Dataset *ds)
{
    RtlPrintError("RtlReadDatasetChar not implemented\r\n");
    return 0;
}

int RtlReadDataset(
    Dataset *ds,
    char *buffer,
    size_t n)
{
    RtlPrintError("RtlReadDataset not implemented\r\n");
    return 0;
}

int RtlWriteDataset(
    Dataset *ds,
    const char *buffer,
    size_t n)
{
    RtlPrintError("RtlWriteDataset not implemented\r\n");
    return 0;
}

int RtlSeekDataset(
    Dataset *ds,
    int offset)
{
    RtlPrintError("RtlSeekDataset not implemented\r\n");
    return 0;
}

int RtlGetDatasetPos(
    Dataset *ds)
{
    RtlPrintError("RtlGetDatasetPos not implemented\r\n");
    return 0;
}