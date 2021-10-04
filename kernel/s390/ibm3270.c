#include <alloc.h>
#include <printf.h>
#include <s390/asm.h>
#include <s390/css.h>
#include <s390/ibm3270.h>
#include <vfs.h>

struct ibm3270_info {
    struct css_schid schid;
    struct css_irb irb;
    struct css_orb orb;
    struct css_ccw1 cmds[1];
};
static struct ibm3270_info drive_info = {0};

int ibm3270_write(
    struct vfs_node *node,
    const void *buf,
    size_t n)
{
    int r;

    /* The console does not like writing empty buffers */
    if(n == 0) {
        return -1;
    }

    /* If the buffer has a carriage return then we write in no-auto-CR mode */
    if(((const unsigned char *)buf)[n - 1] == '\n') {
        drive_info.cmds[0].cmd = IBM3270_CMD_WRITE_NOCR;
    } else {
        drive_info.cmds[0].cmd = IBM3270_CMD_WRITE_CR;
    }
    drive_info.cmds[0].addr = (uint32_t)buf;
    drive_info.cmds[0].flags = CSS_CCW_SLI(1);
    drive_info.cmds[0].length = (uint16_t)n;
    *((volatile uint32_t *)S390_FLCCAW) = (uint32_t)&drive_info.cmds[0];

    r = css_test_channel(drive_info.schid, &drive_info.irb);

    r = css_modify_channel(drive_info.schid, &drive_info.orb);
    if(r == 3) {
        goto no_op;
    }

    r = css_test_channel(drive_info.schid, &drive_info.irb);
    if(r == 3) {
        goto no_op;
    }

    r = css_start_channel(drive_info.schid, &drive_info.orb);
    if(r == 3) {
        goto no_op;
    }

    /* TODO: What the fuck is happening here? */
    kprintf("IO\n");
    s390_wait_io();

    kprintf("ibm3270: Write done... checking status\n");
    r = css_test_channel(drive_info.schid, &drive_info.irb);
    if(r == 3) {
        goto no_op;
    }
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
    node->driver->write = &ibm3270_write;

    drive_info.orb.flags = 0x0080FF00;
    drive_info.orb.prog_addr = (uint32_t)&drive_info.cmds[0];
    drive_info.schid.id = 1;
    drive_info.schid.num = 0;

    kprintf("ibm3270: Device address is %i:%i\n",
        (int)drive_info.schid.id, (int)drive_info.schid.num);
    return 0;
}