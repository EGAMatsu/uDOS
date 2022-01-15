/* x3270.c
 *
 * Driver for x3270 family of terminals
 */

/* TODO: This driver is broken */

#include <x3270.h>
#include <mm/mm.h>
#include <debug/printf.h>
#include <debug/panic.h>
#include <asm.h>
#include <fs/fs.h>

/* Driver global for VFS */
static struct FsDriver *driver;
/* Device number allocation for VFS */
static size_t u_devnum = 0;

struct DeviceX3270Info {
    struct css_device dev;

    size_t rows, cols;
    size_t x, y;

    unsigned char *buffer;
    size_t bufsize;
};

static const unsigned char ebcdic_map[] = {
    0x40, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0x4A, 0x4B,
    0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0xE2, 0xE3,
    0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x7B,
    0x7C, 0x7D, 0x7E, 0x7F
};

static unsigned int ModGetX3270Address(
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

static int ModEnableX3270(
    struct DeviceX3270Info *info)
{
    struct css_request *req;
    req = CssNewRequest(&info->dev, 1);
    req->ccws[0].cmd = 0x27;
    CSS_SET_ADDR(&req->ccws[0], 0);
    req->ccws[0].flags = 0;
    req->ccws[0].length = 0;

    info->dev.orb.flags = 0x0080FF00;
    req->flags = CSS_REQUEST_MODIFY | CSS_REQUEST_IGNORE_CC;

    CssSendRequest(req);
    CssPerformRequest(req);
    CssDestroyRequest(req);
    return 0;
}

static int ModWriteX3270(
    struct FsHandle *hdl,
    const void *buf,
    size_t n)
{
    struct DeviceX3270Info *drive = hdl->node->driver_data;
    struct css_request *req;
    size_t i;
    int r;

    const unsigned char *c_buf = (const unsigned char *)buf;
    unsigned int addr;
    
    req = CssNewRequest(&drive->dev, 1);

    drive->buffer = MmReallocate(drive->buffer, 4 + n + 5);
    if(drive->buffer == NULL) {
        KePanic("Out of memory\r\n");
    }

    /*
    drive->buffer[0] = '\xC3';
    drive->buffer[0] = X3270_ORDER_START_FIELD;
    drive->buffer[1] = '\xF6';
    */

    drive->buffer[0] = X3270_WCC_SOUND_ALARM;
    drive->buffer[1] = X3270_ORDER_SET_BUFFER_ADDR;
    addr = ModGetX3270Address(drive->x + (drive->y * drive->cols));
    drive->buffer[2] = (unsigned char)(addr >> 8);
    drive->buffer[3] = (unsigned char)addr;
    drive->bufsize += 4;

    for(i = 0; i < n; i++) {
        char ch = c_buf[i];
        switch(ch) {
        case '\r':
            drive->x = 0;
            break;
        case '\n':
            drive->y++;
            break;
        default:
            drive->buffer[4 + i] = ch;
            break;
        }
    }
    drive->bufsize += n;

    drive->buffer[drive->bufsize + 0] = X3270_ORDER_SET_BUFFER_ADDR;
    addr = ModGetX3270Address(drive->x + (drive->y * drive->cols));
    drive->buffer[drive->bufsize + 1] = (unsigned char)(addr >> 8);
    drive->buffer[drive->bufsize + 2] = (unsigned char)addr;
    drive->buffer[drive->bufsize + 3] = X3270_ORDER_INSERT_CURSOR;
    drive->buffer[drive->bufsize + 4] = '\x3C';
    drive->bufsize += 5;

    req->ccws[0].cmd = CSS_CMD_WRITE;
    CSS_SET_ADDR(&req->ccws[0], drive->buffer);
    req->ccws[0].flags = 0;
    req->ccws[0].length = (uint16_t)drive->bufsize;

    drive->dev.orb.flags = 0x0080FF00;
    req->flags = CSS_REQUEST_MODIFY;

    CssSendRequest(req);
    r = CssPerformRequest(req);
    CssDestroyRequest(req);
    return 0;
no_op:
    KeDebugPrint("x3270: Not operational - terminal was disconnected?\r\n");
    return -1;
}

static int ModReadX3270(
    struct FsHandle *hdl,
    void *buf,
    size_t n)
{
    struct DeviceX3270Info *drive = hdl->node->driver_data;
    struct css_request *req;
    int r;
    
    req = CssNewRequest(&drive->dev, 1);

    req->ccws[0].cmd = X3270_CMD_READ_MOD;
    CSS_SET_ADDR(&req->ccws[0], buf);
    req->ccws[0].flags = 0;
    req->ccws[0].length = (uint16_t)n;

    drive->dev.orb.flags = 0x0080FF00;
    req->flags = CSS_REQUEST_MODIFY;

    CssSendRequest(req);
    r = CssPerformRequest(req);
    CssDestroyRequest(req);
    return 0;
no_op:
    KeDebugPrint("x3270: Not operational - terminal was disconnected?\r\n");
    return -1;
}

int ModAddX3270Device(
    struct css_schid schid,
    struct css_senseid *sensebuf)
{
    struct DeviceX3270Info *drive;
    struct FsNode *node;
    char tmpbuf[2] = {0};

    tmpbuf[0] = u_devnum % 10 + '0';

    drive = MmAllocateZero(sizeof(struct DeviceX3270Info));
    if(drive == NULL) {
        KePanic("Out of memory\r\n");
    }
    KeCopyMemory(&drive->dev.schid, &schid, sizeof(schid));
    ModEnableX3270(drive);
    
    /*
    switch(sensebuf->cu_type) {
    case 0x3274:
        drive->cols = 80;
        drive->rows = 24;
        break;
    default:
        KePanic("Unknown model %x\r\n", (unsigned int)sensebuf->cu_type);
        break;
    }
    */

    /* Create a new node with the format IBM-3270.XXX, number assigned by
     * the variable u_devnum */
    node = KeCreateFsNode("A:\\MODULES\\IBM-3270", &tmpbuf[0]);
    KeAddFsNodeToDriver(driver, node);
    node->driver_data = drive;

    u_devnum++;

    KeDebugPrint("x3270: Drive address is %i:%i\r\n", (int)drive->dev.schid.id,
        (int)drive->dev.schid.num);
    return 0;
}

int ModInitX3270(
    void)
{
    struct FsNode *node;

    KeDebugPrint("x3270: Initializing driver\r\n");
    driver = KeCreateFsDriver();
    driver->write = &ModWriteX3270;
    driver->read = &ModReadX3270;

    node = KeCreateFsNode("A:\\MODULES", "IBM-3270");
    /*KeAddFsNodeToDriver(driver, node);*/
    return 0;
}
