/* ibm3390.c
 *
 * A driver implementation for the 3390 DASD hard disk command subsystem device,
 * this driver will write and read rawly to the 3390, in order to actually read
 * useful data see fs/zdfs.c for the implementation of the common filesystem
 * used normally on these 3390 DASD drives.
 * 
 * Note that the driver should encompass all DASD drives from the 3310 to the
 * 3390 drives. However they are called 3390 in documentation of this driver
 * to mantain simplicity. But their correct terminology would be 33X0
 */

#include <alloc.h>
#include <printf.h>
#include <s390/asm.h>
#include <s390/css.h>
#include <s390/ibm3390.h>
#include <vfs.h>

struct ibm3390_seek {
    uint16_t block;
    uint16_t cyl;
    uint16_t head;
    uint8_t record;
} __attribute__((aligned(8)));

struct ibm3390_info {
    struct css_device dev;
    struct ibm3390_seek seek_ptr;
};
static struct ibm3390_info drive_info = {0};

int ibm3390_read_fdscb(
    struct vfs_node *node,
    struct vfs_fdscb *fdscb,
    void *buf,
    size_t n)
{
    struct ibm3390_info *drive = node->driver_data;
    struct css_request *req;
    int r;

    req = css_new_request(&drive->dev, 4);
    if(req == NULL) {
        kpanic("Out of memory");
    }

    req->ccws[0].cmd = IBM3390_CMD_SEEK;
    req->ccws[0].addr = (uint32_t)&drive->seek_ptr.block;
    req->ccws[0].flags = CSS_CCW_CC;
    req->ccws[0].length = 6;

    req->ccws[1].cmd = IBM3390_CMD_SEARCH;
    req->ccws[1].addr = (uint32_t)&drive->seek_ptr.cyl;
    req->ccws[1].flags = CSS_CCW_CC;
    req->ccws[1].length = 5;

    req->ccws[2].cmd = CSS_CMD_TIC;
    req->ccws[2].addr = (uint32_t)&req->ccws[1];
    req->ccws[2].flags = 0x00;
    req->ccws[2].length = 0;

    req->ccws[3].cmd = IBM3390_CMD_LD;
    req->ccws[3].addr = (uint32_t)buf;
    req->ccws[3].flags = CSS_CCW_SLI;
    req->ccws[3].length = (uint16_t)n;

    drive->dev.orb.flags = 0x0080FF00;

    drive->seek_ptr.block = 0;
    drive->seek_ptr.cyl = fdscb->cyl;
    drive->seek_ptr.head = fdscb->head;
    drive->seek_ptr.record = fdscb->rec;

    css_send_request(req);
    css_do_request(req);
    css_destroy_request(req);

    return (int)n - (int)drive->dev.irb.scsw.count;
no_op:
    kprintf("ibm3390: Not operational - drive was unplugged?\n");
    return -1;
}

int ibm3390_read(
    struct vfs_node *node,
    void *buf,
    size_t n)
{
    struct vfs_fdscb fdscb = {0, 0, 3};
    return ibm3390_read_fdscb(node, &fdscb, buf, n);
}

int ibm3390_init(
    void)
{
    struct vfs_node *node;
    kprintf("ibm3390: Initializing\n");
    node = vfs_new_node("\\SYSTEM\\DEVICES", "IBM3390");
    node->driver = vfs_new_driver();
    node->driver->read = &ibm3390_read;
    node->driver->read_fdscb = &ibm3390_read_fdscb;

    struct ibm3390_info *drive;
    drive = kzalloc(sizeof(struct ibm3390_info));
    if(drive == NULL) {
        kpanic("Out of memory");
    }
    drive->dev.schid.id = 1;
    drive->dev.schid.num = 1;
    node->driver_data = drive;

    kprintf("ibm3390: Drive address is %i:%i\n", (int)drive->dev.schid.id,
        (int)drive->dev.schid.num);
    return 0;
}