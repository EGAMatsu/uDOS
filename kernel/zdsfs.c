/* zdsfs.c
 *
 */

#include <mm.h>
#include <printf.h>
#include <asm.h>
#include <css.h>
#include <fs.h>
#include <zdsfs.h>
#include <panic.h>
#include <dev.h>

/* Driver global for VFS */
static struct fs_driver *driver;

int ModGetZdsfsFile(struct css_device *dev, struct fs_fdscb *out_fdscb, const char *name)
{
    struct fs_fdscb fdscb = {0};
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
    
    r = DevFDSCB_ReadDisk(dev, &fdscb, tmpbuf, 20);
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
                KeDebugPrint("Dataset %s\r\n", &dscb1.ds_name);
                if(dscb1.format_id == '1') {
                    dscb1.format_id = ' ';
                    
                    if(!KeCompareMemory(&dscb1.ds_name, name, KeStringLength(name))) {
                        out_fdscb->cyl = dscb1.start_cc;
                        out_fdscb->head = dscb1.start_hh;
                        out_fdscb->rec = 1;

                        KeDebugPrint("Dataset %s @ CYL=%i,HEAD=%i,REC=%i\r\n", name, (int)out_fdscb->cyl, (int)out_fdscb->head, (int)out_fdscb->rec);
                        break;
                    }
                } else if(dscb1.ds_name[0] == '\0') {
                    MmFree(tmpbuf);
                    return -1;
                }
            }
            fdscb.rec++;
        }
    }

    MmFree(tmpbuf);
    if(r <= 0) {
        KeDebugPrint("Unable to read disk");
        return -1;
    }
    return 0;
}

struct fs_node *ModRequestZdsfsNode(const struct fs_node *root, const char *path)
{
    return root;
}

int ModInitZdsfs(void)
{
    struct fs_node *node;

    driver = KeCreateFsDriver();
    driver->request_node = &ModRequestZdsfsNode;

    KeDebugPrint("x3390: Initializing driver\r\n");
    node = KeCreateFsNode("A:\\MODULES", "IBM-3390");
    return 0;
}
