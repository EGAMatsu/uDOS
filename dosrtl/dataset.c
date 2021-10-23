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

    S390_DO_SVC(200, (uintptr_t)name, (uintptr_t)recfm, 0, &ds);
    return ds;
}

int RtlReadDatasetLine(
    Dataset *ds,
    char *buffer,
    size_t n)
{
    return 0;
}

int RtlReadDataset(
    Dataset *ds,
    char *buffer,
    size_t n)
{
    return 0;
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