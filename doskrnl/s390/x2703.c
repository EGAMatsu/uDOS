/* x2703.c
 *
 * Driver for 2703 BSC communication line devices
 */

#include <mm/mm.h>
#include <debug/printf.h>
#include <s390/asm.h>
#include <s390/css.h>
#include <s390/x2703.h>
#include <fs/fs.h>

struct x2703_info {
    struct css_device dev;
};

static int x2703_enable(
    struct x2703_info *info)
{
    struct css_request *req;
    req = css_new_request(&info->dev, 1);
    req->ccws[0].cmd = 0x27;
    req->ccws[0].addr = 0;
    req->ccws[0].flags = 0;
    req->ccws[0].length = 0;

    info->dev.orb.flags = 0x0080FF00;
    req->flags = CSS_REQUEST_MODIFY | CSS_REQUEST_IGNORE_CC;

    css_send_request(req);
    css_do_request(req);
    css_destroy_request(req);
    return 0;
}

static int x2703_write(
    struct FsHandle *hdl,
    const void *buf,
    size_t n)
{
    struct x2703_info *drive = hdl->node->driver_data;
    struct css_request *req;
    int r;
    
    req = css_new_request(&drive->dev, 1);

    req->ccws[0].cmd = CSS_CMD_WRITE;
    req->ccws[0].addr = (uint32_t)buf;
    req->ccws[0].flags = 0;
    req->ccws[0].length = (uint16_t)n;

    drive->dev.orb.flags = 0x0080FF00;
    req->flags = CSS_REQUEST_MODIFY;

    css_send_request(req);
    r = css_do_request(req);
    css_destroy_request(req);
    return r;
no_op:
    kprintf("x2703: Not operational - terminal was disconnected?\r\n");
    return -1;
}

static int x2703_read(
    struct FsHandle *hdl,
    void *buf,
    size_t n)
{
    struct x2703_info *drive = hdl->node->driver_data;
    struct css_request *req;
    int r;
    
    req = css_new_request(&drive->dev, 1);

    req->ccws[0].cmd = CSS_CMD_READ;
    req->ccws[0].addr = (uint32_t)buf;
    req->ccws[0].flags = 0;
    req->ccws[0].length = (uint16_t)n;

    drive->dev.orb.flags = 0x0080FF00;
    req->flags = CSS_REQUEST_MODIFY;

    css_send_request(req);
    r = css_do_request(req);
    css_destroy_request(req);
    return r;
no_op:
    kprintf("x2703: Not operational - terminal was disconnected?\r\n");
    return -1;
}

int x2703_init(
    void)
{
    /* Driver that manages nodes */
    struct FsDriver *driver;
    driver = KeCreateFsDriver();
    driver->write = &x2703_write;
    driver->read = &x2703_read;

    struct x2703_info *drive;
    struct FsNode *node;

    /* Nodes under our control, nodes has a driver_data parameter we can use
     * to store virtually anything we want there */
    node = KeCreateFsNode("A:\\MODULES", "IBM-2703");
    KeAddFsNodeToDriver(driver, node);

    /* Store information about the x2703 inside the node */
    drive = MmAllocateZero(sizeof(struct x2703_info));
    if(drive == NULL) {
        KePanic("Out of memory");
    }
    drive->dev.schid.id = 1;
    drive->dev.schid.num = 0;
    x2703_enable(drive);
    node->driver_data = drive;

    kprintf("x2703: Device address is %i:%i\r\n",
        (int)drive->dev.schid.id, (int)drive->dev.schid.num);
    return 0;
}