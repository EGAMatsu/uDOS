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

struct x3270_drive_info {
    struct css_device dev;

    size_t rows, cols;
    size_t x, y;

    unsigned char *buffer;
    size_t bufsize;
};

const unsigned char ebcdic_map[] = {
    0x40, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0x4A, 0x4B,
    0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0xE2, 0xE3,
    0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x7B,
    0x7C, 0x7D, 0x7E, 0x7F
};

static unsigned int x3270_get_address(
    int addr)
{
    unsigned int tmp;

    /* 12-bit conversion does not need to be done since it's higher than the
     * imposed 4K limit */
    if(addr >= 0xfff) {
        return (unsigned int)addr;
    }

    /* Take only in account low 6-bits */
    return ((unsigned int)ebcdic_map[(addr >> 6) & 0x3F] << 8) | ebcdic_map[addr & 0x3F];
}

int x3270_enable(
    struct x3270_drive_info *info)
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

int x3270_write(
    struct vfs_handle *hdl,
    const void *buf,
    size_t n)
{
    struct x3270_drive_info *drive = hdl->node->driver_data;
    struct css_request *req;
    size_t i;
    int r;

    const unsigned char *c_buf = (const unsigned char *)buf;
    unsigned int addr;
    
    req = css_new_request(&drive->dev, 1);

    drive->buffer = krealloc(drive->buffer, 6 + n + 7);
    if(drive->buffer == NULL) {
        kpanic("Out of memory\r\n");
    }

    drive->buffer[0] = '\xC3';
    
    drive->buffer[1] = X3270_ORDER_START_FIELD;
    drive->buffer[2] = '\xF6';

    drive->buffer[3] = X3270_ORDER_SET_BUFFER_ADDR;
    addr = x3270_get_address(drive->x + (drive->y * drive->cols));
    drive->buffer[4] = (unsigned char)(addr >> 8);
    drive->buffer[5] = (unsigned char)addr;

    for(i = 0; i < n; i++) {
        char ch = c_buf[i];
        switch(ch) {
        case '\r':
            drive->x = 0;
            break;
        case '\n':
            drive->y++;

            drive->buffer[3] = X3270_ORDER_SET_BUFFER_ADDR;
            addr = x3270_get_address(drive->x + (drive->y * drive->cols));
            drive->buffer[4] = (unsigned char)(addr >> 8);
            drive->buffer[5] = (unsigned char)addr;
            break;
        default:
            drive->buffer[6 + i] = ch;
            break;
        }
    }

    drive->buffer[6 + n + 0] = X3270_ORDER_START_FIELD;
    drive->buffer[6 + n + 1] = '\x00';

    drive->buffer[6 + n + 2] = X3270_ORDER_INSERT_CURSOR;

    drive->buffer[6 + n + 3] = '\x3C';
    addr = x3270_get_address(drive->x + (drive->y * drive->cols));
    drive->buffer[6 + n + 4] = (unsigned char)(addr >> 8);
    drive->buffer[6 + n + 5] = (unsigned char)addr;
    drive->buffer[6 + n + 6] = '\x00';

    drive->x = 0;
    drive->y = 0;

    req->ccws[0].cmd = CSS_CMD_WRITE;
    req->ccws[0].addr = (uint32_t)drive->buffer;
    req->ccws[0].flags = 0;
    req->ccws[0].length = (uint16_t)6 + n + 7;

    drive->dev.orb.flags = 0x0080FF00;
    req->flags = CSS_REQUEST_MODIFY;

    css_send_request(req);
    r = css_do_request(req);
    css_destroy_request(req);
    return 0;
no_op:
    kprintf("x3270: Not operational - terminal was disconnected?\r\n");
    return -1;
}

int x3270_read(
    struct vfs_handle *hdl,
    void *buf,
    size_t n)
{
    struct x3270_drive_info *drive = hdl->node->driver_data;
    struct css_request *req;
    int r;
    
    req = css_new_request(&drive->dev, 1);

    req->ccws[0].cmd = X3270_CMD_READ_MOD;
    req->ccws[0].addr = (uint32_t)buf;
    req->ccws[0].flags = 0;
    req->ccws[0].length = (uint16_t)n;

    drive->dev.orb.flags = 0x0080FF00;
    req->flags = CSS_REQUEST_MODIFY;

    css_send_request(req);
    r = css_do_request(req);
    css_destroy_request(req);
    return 0;
no_op:
    kprintf("x3270: Not operational - terminal was disconnected?\r\n");
    return -1;
}

int x3270_init(
    void)
{
    struct vfs_driver *driver;
    driver = vfs_new_driver();
    driver->write = &x3270_write;
    driver->read = &x3270_read;

    struct x3270_drive_info *drive;
    struct vfs_node *node;
    size_t i;

    kprintf("x3270: Initializing\r\n");

    node = vfs_new_node("\\SYSTEM\\DEVICES", "IBM-3270");
    vfs_driver_add_node(driver, node);

    drive = kzalloc(sizeof(struct x3270_drive_info));
    if(drive == NULL) {
        kpanic("Out of memory\r\n");
    }
    drive->dev.schid.id = 1;
    drive->dev.schid.num = 0;
    drive->cols = 80;
    drive->rows = 24;

    x3270_enable(drive);
    node->driver_data = drive;

    kprintf("x3270: Device address is %i:%i\r\n",
        (int)drive->dev.schid.id, (int)drive->dev.schid.num);
    return 0;
}