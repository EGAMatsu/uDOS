/* zdsfs.c
 *
 */

#include <mm/mm.h>
#include <debug/printf.h>
#include <s390/asm.h>
#include <s390/css.h>
#include <fs/fs.h>
#include <fs/zdsfs.h>

/* Driver global for VFS */
static struct FsDriver *driver;

int ModGetZdsfsFile(
    struct FsHandle *hdl,
    struct FsFdscb *out_fdscb,
    const char *name)
{
    struct FsFdscb fdscb = {0};
    struct zdsfs_dscb_fmt1 dscb1;
    char *tmpbuf;
    int r;

    tmpbuf = MmAllocate(20);
    if(tmpbuf == NULL) {
        KePanic("Out of memory");
    }

    /* The VTOC is at 0, 0, 3 */
    fdscb.cyl = 0;
    fdscb.head = 0;
    fdscb.rec = 3;
    
    r = KeReadWithFdscbFsNode(hdl, &fdscb, tmpbuf, 20);
    if(r >= 20) {
        int errcnt = 0;

        KeCopyMemory(&fdscb.cyl, tmpbuf + 15, 2); /* 15-16 */
        KeCopyMemory(&fdscb.head, tmpbuf + 17, 2); /* 17-18 */
        KeCopyMemory(&fdscb.rec, tmpbuf + 19, 1); /* 19-19 */

        while(errcnt < 4) {
            r = KeReadWithFdscbFsNode(hdl, &fdscb, &dscb1, sizeof(dscb1));
            if(r < 0) {
                errcnt++;
                if(errcnt == 1) {
                    fdscb.rec++;
                } else if(errcnt == 2) {
                    fdscb.rec = 1;
                    fdscb.head++;
                } else if(errcnt == 3) {
                    fdscb.rec = 1;
                    fdscb.head = 0;
                    fdscb.cyl++;
                }
                continue;
            }

            errcnt = 0;
            if(r >= (int)sizeof(dscb1)) {
                if(dscb1.ds1fmtid == '1') {
                    dscb1.ds1fmtid = ' ';
                    
                    if(!KeCompareMemory(&dscb1.ds1dsnam, name, KeStringLength(name))) {
                        out_fdscb->cyl = dscb1.start_cc;
                        out_fdscb->head = dscb1.start_hh;
                        out_fdscb->rec = 1;

                        kprintf("File %s @ CYL=%i,HEAD=%i,RECORD=%i\r\n", name,
                            (int)out_fdscb->cyl, (int)out_fdscb->head,
                            (int)out_fdscb->rec);
                        break;
                    }
                } else if(dscb1.ds1dsnam[0] == '\0') {
                    MmFree(tmpbuf);
                    return -1;
                }
            }
            fdscb.rec++;
        }
    }

    MmFree(tmpbuf);
    if(r <= 0) {
        return -1;
    }
    return 0;
}

struct FsNode *ModRequestZdsfsNode(
    const struct FsNode *root,
    const char *path)
{
    return root;
}

int ModInitZdsfs(
    void)
{
    struct FsNode *node;

    driver = KeCreateFsDriver();
    driver->request_node = &ModRequestZdsfsNode;

    kprintf("x3390: Initializing driver\r\n");
    node = KeCreateFsNode("A:\\MODULES", "IBM-3390");
    return 0;
}