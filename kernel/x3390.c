/* x3390.c
 *
 * A driver implementation for the 3390 DASD hard disk command subsystem device,
 * this driver will write and read rawly to the 3390, in order to actually read
 * useful data see zdfs.c for the implementation of the common filesystem
 * used normally on these 3390 DASD drives.
 * 
 * Note that the driver should encompass all DASD drives from the 3310 to the
 * 3390 drives. However they are called 3390 in documentation of this driver
 * to mantain simplicity. But their correct terminology would be 33X0
 */

#include <mm.h>
#include <printf.h>
#include <panic.h>
#include <asm.h>
#include <css.h>
#include <x3390.h>
#include <fs.h>

/* Driver global for VFS */
static struct fs_driver *driver;
/* Device number allocation for VFS */
static size_t u_devnum = 0;

struct x3390_seek {
    uint16_t block;
    uint16_t cyl;
    uint16_t head;
    uint8_t record;
} __attribute__((aligned(8)));

struct x3390_drive_info {
    struct css_device dev;
    struct x3390_seek seek_ptr;
};

static int x3390_read_fdscb(struct fs_handle *hdl, struct fs_fdscb *fdscb, void *buf, size_t n)
{
    struct x3390_drive_info *drive = hdl->node->driver_data;
    struct css_request *req;

    req = CssNewRequest(&drive->dev, 4);
    if(req == NULL) {
        KePanic("Out of memory");
    }

    req->ccws[0].cmd = X3390_CMD_SEEK;
    CSS_SET_ADDR(&req->ccws[0], &drive->seek_ptr.block);
    req->ccws[0].flags = CSS_CCW_CC;
    req->ccws[0].length = 6;

    req->ccws[1].cmd = X3390_CMD_SEARCH;
    CSS_SET_ADDR(&req->ccws[1], &drive->seek_ptr.cyl);
    req->ccws[1].flags = CSS_CCW_CC;
    req->ccws[1].length = 5;

    req->ccws[2].cmd = CSS_CMD_TIC;
    CSS_SET_ADDR(&req->ccws[2], &req->ccws[1]);
    req->ccws[2].flags = 0x00;
    req->ccws[2].length = 0;

    req->ccws[3].cmd = X3390_CMD_LD;
    CSS_SET_ADDR(&req->ccws[3], buf);
    req->ccws[3].flags = CSS_CCW_SLI;
    req->ccws[3].length = (uint16_t)n;

    drive->dev.orb.flags = 0x0080FF00;

    drive->seek_ptr.block = 0;
    drive->seek_ptr.cyl = fdscb->cyl;
    drive->seek_ptr.head = fdscb->head;
    drive->seek_ptr.record = fdscb->rec;

    CssSendRequest(req);
    if(CssPerformRequest(req) != 0) {
        return -1;
    }
    CssDestroyRequest(req);
    return (int)n - (int)drive->dev.irb.scsw.count;
no_op:
    KeDebugPrint("x3390: Not operational - drive was unplugged?\r\n");
    return -1;
}

static int x3390_read(struct fs_handle *hdl, void *buf, size_t n)
{
    struct fs_fdscb fdscb = {0, 0, 3};
    return x3390_read_fdscb(hdl, &fdscb, buf, n);
}

int ModAddX3390Device(struct css_schid schid, struct css_senseid *sensebuf)
{
    struct x3390_drive_info *drive;
    struct fs_node *node;
    char tmpbuf[2] = {0};

    tmpbuf[0] = u_devnum % 10 + '0';

    drive = MmAllocatePhysical(sizeof(struct x3390_drive_info), 8);
    if(drive == NULL) {
        KePanic("Out of memory");
    }
    KeSetMemory(&drive, 0, sizeof(struct x3390_drive_info));
    KeCopyMemory(&drive->dev.schid, &schid, sizeof(schid));

    /* Create a new node with the format IBM-3390.XXX, number assigned by
     * the variable u_devnum */
    node = KeCreateFsNode("A:\\MODULES\\IBM-3390", &tmpbuf[0]);
    KeAddFsNodeToDriver(driver, node);
    node->driver_data = drive;

    u_devnum++;

    KeDebugPrint("x3390: Drive address is %i:%i\r\n", (int)drive->dev.schid.id, (int)drive->dev.schid.num);
    return 0;
}

int ModInitX3390(void)
{
    struct fs_node *node;

    driver = KeCreateFsDriver();
    driver->read = &x3390_read;
    driver->read_fdscb = &x3390_read_fdscb;

    KeDebugPrint("x3390: Initializing driver\r\n");
    node = KeCreateFsNode("A:\\MODULES", "IBM-3390");
    /*KeAddFsNodeToDriver(driver, node);*/
    return 0;
}
