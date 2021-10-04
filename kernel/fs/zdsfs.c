#include <fs/zdsfs.h>
#include <string.h>
#include <alloc.h>

int zdsfs_get_file(
    struct vfs_node *node,
    struct vfs_fdscb *out_fdscb,
    const char *name)
{
    struct vfs_fdscb fdscb = {0};
    struct zdsfs_dscb_fmt1 dscb1;
    char *tmpbuf;
    int r;

    tmpbuf = kmalloc(20);
    
    /* The VTOC is at 0, 0, 3 */
    fdscb.cyl = 0;
    fdscb.head = 0;
    fdscb.rec = 3;

    r = vfs_read_fdscb(node, &fdscb, tmpbuf, 20);
    if(r >= 20) {
        int errcnt = 0;

        memcpy(&fdscb.cyl, tmpbuf + 15, 2); /* 15-16 */
        memcpy(&fdscb.head, tmpbuf + 17, 2); /* 17-18 */
        memcpy(&fdscb.rec, tmpbuf + 19, 1); /* 19-19 */

        while(errcnt < 4) {
            r = vfs_read_fdscb(node, &fdscb, &dscb1, sizeof(dscb1));
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
                    
                    if(!memcmp(&dscb1.ds1dsnam, name, strlen(name))) {
                        out_fdscb->cyl = dscb1.start_cc;
                        out_fdscb->head = dscb1.start_hh;
                        out_fdscb->rec = 1;

                        kprintf("File %s @ CYL=%i,HEAD=%i,RECORD=%i\n", name,
                            (int)out_fdscb->cyl, (int)out_fdscb->head,
                            (int)out_fdscb->rec);
                        break;
                    }
                } else if(dscb1.ds1dsnam[0] == '\0') {
                    kfree(tmpbuf);
                    return -1;
                }
            }
            fdscb.rec++;
        }
    }

    kfree(tmpbuf);
    if(r <= 0) {
        return -1;
    }
    return 0;
}