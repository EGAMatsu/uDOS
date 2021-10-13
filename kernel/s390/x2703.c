/* x2703.c
 *
 * Driver for 2703 BSC communication line devices
 */

#include <alloc.h>
#include <printf.h>
#include <s390/asm.h>
#include <s390/css.h>
#include <s390/x2703.h>
#include <vfs.h>

struct x2703_info {
    struct css_device dev;
};
static struct x2703_info drive_info = {0};

int x2703_open(
    struct vfs_node *node)
{
    return 0;
}

int x2703_enable(
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

int x2703_write(
    struct vfs_node *node,
    const void *buf,
    size_t n)
{
    struct x2703_info *drive = node->driver_data;
    struct css_request *req;
    int r;

    req = css_new_request(&drive->dev, 1);

    req->ccws[0].cmd = CSS_CMD_WRITE;
    req->ccws[0].addr = (uint32_t)buf;
    req->ccws[0].flags = CSS_CCW_SLI;
    req->ccws[0].length = (uint16_t)n;

    drive->dev.orb.flags = 0x0080FF00;
    req->flags = CSS_REQUEST_MODIFY;

    css_send_request(req);
    r = css_do_request(req);
    css_destroy_request(req);
    return r;
no_op:
    kprintf("x2703: Not operational - terminal was disconnected?\n");
    return -1;
}

int x2703_read(
    struct vfs_node *node,
    void *buf,
    size_t n)
{
    struct x2703_info *drive = node->driver_data;
    struct css_request *req;
    int r;

    req = css_new_request(&drive->dev, 2);

    req->ccws[0].cmd = 0x06;
    req->ccws[0].addr = (uint32_t)buf;
    req->ccws[0].flags = CSS_CCW_CC;
    req->ccws[0].length = (uint16_t)n;

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
    kprintf("x2703: Not operational - terminal was disconnected?\n");
    return -1;
}

int x2703_init(
    void)
{
    struct x2703_info *drive;
    struct vfs_node *node;

    kprintf("x2703: Initializing\n");
    node = vfs_new_node("\\SYSTEM\\DEVICES", "IBM-2703");
    node->driver = vfs_new_driver();
    node->driver->open = &x2703_open;
    node->driver->write = &x2703_write;
    node->driver->read = &x2703_read;

    drive = kzalloc(sizeof(struct x2703_info));
    if(drive == NULL) {
        kpanic("Out of memory");
    }
    drive->dev.schid.id = 1;
    drive->dev.schid.num = 0;
    node->driver_data = drive;

    x2703_enable(drive);

    kprintf("x2703: Device address is %i:%i\n",
        (int)drive_info.dev.schid.id, (int)drive_info.dev.schid.num);
    return 0;
}