/* ibm3270.c
 *
 * Driver for x3270 family of terminals
 */

/* TODO: This driver is broken */

#include <alloc.h>
#include <printf.h>
#include <s390/asm.h>
#include <s390/css.h>
#include <s390/ibm3270.h>
#include <vfs.h>

struct ibm3270_info {
    struct css_device dev;
    size_t rows;
    size_t cols;
};
static struct ibm3270_info drive_info = {0};

int ibm3270_open(
    struct vfs_node *node)
{
    return 0;
}

int ibm3270_write(
    struct vfs_node *node,
    const void *buf,
    size_t n)
{
    struct ibm3270_info *drive = node->driver_data;
    struct css_request *req;
    int r;

    /* The console does not like writing empty buffers */
    if(n == 0) {
        return -1;
    }

    req = css_new_request(&drive->dev, 2);

    /* If the buffer has a carriage return then we write in no-auto-CR mode */
    if(((const unsigned char *)buf)[n - 1] == '\n') {
        req->ccws[0].cmd = CSS_CMD_WRITE;
    } else {
        req->ccws[0].cmd = 0x08 | CSS_CMD_WRITE;
    }
    req->ccws[0].addr = (uint32_t)buf;
    req->ccws[0].flags = CSS_CCW_SLI | CSS_CCW_CC;
    req->ccws[0].length = (uint16_t)n;

    req->ccws[1].cmd = CSS_CMD_TIC;
    req->ccws[1].addr = (uint32_t)&req->ccws[0];
    req->ccws[1].flags = 0x00;
    req->ccws[1].length = 0;

    drive->dev.orb.flags = 0x0080FF00;
    req->flags = CSS_REQUEST_MODIFY;

    css_send_request(req);
    css_do_request(req);
    css_destroy_request(req);

    kprintf("ibm3270: Write done... checking status\n");
    return 0;
no_op:
    kprintf("ibm3270: Not operational - terminal was disconnected?\n");
    return -1;
}

int ibm3270_init(
    void)
{
    struct vfs_node *node;
    kprintf("ibm3270: Initializing\n");
    node = vfs_new_node("\\SYSTEM\\DEVICES", "IBM3270");
    node->driver = vfs_new_driver();
    node->driver->open = &ibm3270_open;
    node->driver->write = &ibm3270_write;

    struct ibm3270_info *drive;
    drive = kzalloc(sizeof(struct ibm3270_info));
    if(drive == NULL) {
        kpanic("Out of memory");
    }
    node->driver_data = drive;
    drive->dev.schid.id = 1;
    drive->dev.schid.num = 0;
    drive->rows = 43;
    drive->cols = 80;

    kprintf("ibm3270: Device address is %i:%i\n",
        (int)drive_info.dev.schid.id, (int)drive_info.dev.schid.num);
    return 0;
}