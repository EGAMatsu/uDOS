#include <s390/dasd.h>
#include <s390/css.h>
#include <s390/asm.h>
#include <alloc.h>
#include <vfs.h>
#include <printf.h>

struct dasd_info {
    uint16_t block;
    uint16_t cylinder;
    uint16_t head;
    uint8_t record;
    struct css_ccw1 cmds[4];
    struct css_schid schid;
};
static struct dasd_info drive_info = {0};

static int dasd_read(int fd, void *buf, size_t n) {
    struct s390_psw rb_wtnoer = {
        0x060E0000,
        PSW_DEFAULT_AMBIT
    };
    struct s390x_psw rb_newio = {
        0x000C0000 | PSW_AM64,
        PSW_DEFAULT_AMBIT,
        0,
        (uint32_t)&&rb_count
    };
    struct css_irb rb_irb = {0};
    struct css_orb rb_orb = {
        .int_param = 0,
        .flags = 0x0080FF00,
        .prog_addr = (uint32_t)&drive_info.cmds[0],
        .css_priority = 0,
        .cu_priority = 0,
        .reserved1 = 0,
        .reserved2 = 0,
        .reserved3 = {0}
    };
    int r;

    kprintf("Super lol\n");

    drive_info.cmds[0].cmd = DASD_CMD_SEEK;
    drive_info.cmds[0].addr = (uint32_t)&drive_info.block;
    drive_info.cmds[0].flags = 0x40;
    drive_info.cmds[0].length = 6;

    drive_info.cmds[1].cmd = DASD_CMD_SEARCH;
    drive_info.cmds[1].addr = (uint32_t)&drive_info.cylinder;
    drive_info.cmds[1].flags = 0x40;
    drive_info.cmds[1].length = 5;

    drive_info.cmds[2].cmd = DASD_CMD_TIC;
    drive_info.cmds[2].addr = (uint32_t)&drive_info.cmds[1];
    drive_info.cmds[2].flags = 0x00;
    drive_info.cmds[2].length = 0;

    drive_info.cmds[3].cmd = DASD_CMD_LD;
    drive_info.cmds[3].addr = (uint32_t)buf;
    drive_info.cmds[3].flags = 0x20;
    drive_info.cmds[3].length = (uint16_t)n;

    drive_info.block = 0;
    drive_info.cylinder = 0;
    drive_info.head = 0;
    drive_info.record = 3;

    memcpy((void *)FLCEINPSW, &rb_newio, sizeof(rb_newio));
    __asm__ __volatile__("stosm %0, %0" : : "d"(FLCEINPSW), "d"(0) : );
    *((volatile uint32_t *)FLCCAW) = (uint32_t)&drive_info.cmds[0];

    kprintf("Drive address is %i:%i\n", (int)drive_info.schid.id, (int)drive_info.schid.num);
    kprintf("Testing channel\n");
    r = css_test_channel(drive_info.schid, &rb_irb);
    if (r != 0) {
        kprintf("Test channel failure %i\n", r);
        kprintf("NOTE: This may be a spurious error\n");
    }

    kprintf("Starting channel\n");
    r = css_start_channel(drive_info.schid, &rb_orb);
    if (r != 0) {
        kprintf("Start channel failure %i\n", r);
        return -1;
    }
    __asm__ goto("lpsw %0\n" : : "m"(rb_wtnoer) : : rb_count);
    
rb_count:
    r = css_test_channel(drive_info.schid, &rb_irb);
    if (r != 0) {
        kprintf("Test channel failure %i\n", r);
        kprintf("NOTE: This may be a spurious error\n");
    }
    return 0;
}

int dasd_init(void) {
    struct vfs_node* node;
    
    node = vfs_new_node("\\SYSTEM", "DASD");
    node->hooks.read = &dasd_read;
    
    drive_info.schid.id = 1;
    drive_info.schid.num = 1;
    return 0;
}