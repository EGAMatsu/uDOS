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
    struct css_schid schid;
    struct css_irb irb;
    struct css_orb orb;

    struct ibm3390_seek seek_ptr;
    struct css_ccw1 cmds[4];
};
static struct ibm3390_info drive_info = {0};

int ibm3390_read_fdscb(
    struct vfs_node *node,
    struct vfs_fdscb *fdscb,
    void *buf,
    size_t n)
{
    int r;

    drive_info.cmds[0].cmd = IBM3390_CMD_SEEK;
    drive_info.cmds[0].addr = (uint32_t)&drive_info.seek_ptr.block;
    drive_info.cmds[0].flags = CSS_CCW_CC(1);
    drive_info.cmds[0].length = 6;

    drive_info.cmds[1].cmd = IBM3390_CMD_SEARCH;
    drive_info.cmds[1].addr = (uint32_t)&drive_info.seek_ptr.cyl;
    drive_info.cmds[1].flags = CSS_CCW_CC(1);
    drive_info.cmds[1].length = 5;

    drive_info.cmds[2].cmd = IBM3390_CMD_TIC;
    drive_info.cmds[2].addr = (uint32_t)&drive_info.cmds[1];
    drive_info.cmds[2].flags = 0x00;
    drive_info.cmds[2].length = 0;

    drive_info.cmds[3].cmd = IBM3390_CMD_LD;
    drive_info.cmds[3].addr = (uint32_t)buf;
    drive_info.cmds[3].flags = CSS_CCW_SLI(1);
    drive_info.cmds[3].length = (uint16_t)n;

    drive_info.seek_ptr.block = 0;
    drive_info.seek_ptr.cyl = fdscb->cyl;
    drive_info.seek_ptr.head = fdscb->head;
    drive_info.seek_ptr.record = fdscb->rec;

    *((volatile uint32_t *)S390_FLCCAW) = (uint32_t)&drive_info.cmds[0];

    r = css_test_channel(drive_info.schid, &drive_info.irb);
    if(r == 3) {
        goto no_op;
    }

    r = css_start_channel(drive_info.schid, &drive_info.orb);
    if(r == 3) {
        goto no_op;
    }

    s390_wait_io();
    
    r = css_test_channel(drive_info.schid, &drive_info.irb);
    if(r == 3) {
        goto no_op;
    }

    if(drive_info.irb.scsw.cpa_addr != (uint32_t)&drive_info.cmds[4]) {
        kprintf("ibm3390: Command chain not completed\n");
        return -1;
    }
    return (int)n - (int)drive_info.irb.scsw.count;
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
    struct vfs_driver *driver;
    struct vfs_node *node;

    driver = vfs_new_driver();
    driver->read = &ibm3390_read;
    driver->read_fdscb = &ibm3390_read_fdscb;

    node = vfs_new_node("\\SYSTEM\\DEVICES", "IBM3390");
    node->driver = driver;

    drive_info.orb.flags = 0x0080FF00;
    drive_info.orb.prog_addr = (uint32_t)&drive_info.cmds[0];
    drive_info.schid.id = 1;
    drive_info.schid.num = 1;

    kprintf("ibm3390: Drive address is %i:%i\n", (int)drive_info.schid.id,
        (int)drive_info.schid.num);
    return 0;
}