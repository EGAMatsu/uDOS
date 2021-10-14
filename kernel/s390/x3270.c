/* x3270.c
 *
 * Driver for x3270 family of terminals
 */

/* TODO: This driver is broken */

#include <alloc.h>
#include <printf.h>
#include <s390/asm.h>
#include <s390/css.h>
#include <s390/x3270.h>
#include <vfs.h>

struct x3270_info {
    struct css_device dev;
};
static struct x3270_info drive_info = {0};

int x3270_open(
    struct vfs_handle *hdl)
{
    return 0;
}

int x3270_write(
    struct vfs_handle *hdl,
    const void *buf,
    size_t n)
{
    return 0;
no_op:
    kprintf("x3270: Not operational - terminal was disconnected?\n");
    return -1;
}

int x3270_read(
    struct vfs_handle *hdl,
    void *buf,
    size_t n)
{
    return 0;
no_op:
    kprintf("x3270: Not operational - terminal was disconnected?\n");
    return -1;
}

int x3270_init(
    void)
{
    struct x3270_info *drive;
    struct vfs_node *node;
    kprintf("ibm3270: Initializing\n");
    node = vfs_new_node("\\SYSTEM\\DEVICES", "IBM-3270");
    node->driver = vfs_new_driver();
    node->driver->open = &x3270_open;
    node->driver->write = &x3270_write;
    node->driver->read = &x3270_read;

    drive = kzalloc(sizeof(struct x3270_info));
    if(drive == NULL) {
        kpanic("Out of memory");
    }
    drive->dev.schid.id = 1;
    drive->dev.schid.num = 0;
    node->driver_data = drive;

    kprintf("x3270: Device address is %i:%i\n",
        (int)drive_info.dev.schid.id, (int)drive_info.dev.schid.num);
    return 0;
}