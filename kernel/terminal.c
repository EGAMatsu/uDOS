/* bsc.c
 *
 * BSC generic communication line driver (wraps around a raw device to establish
 * an stable ASCII protocol with the other party)
 */

#include <terminal.h>
#include <panic.h>
#include <memory.h>
#include <fs.h>
#include <mm.h>
#include <printf.h>
#include <asm.h>
#include <css.h>
#include <fs.h>

static struct fs_driver *bsc_driver;

const unsigned char asc2ebc[256] = {
    0x00, 0x01, 0x02, 0x03, 0x1A, 0x09, 0x1A, 0x7F, 0x1A, 0x1A, 0x1A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,
    0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F, 0x40, 0x4F, 0x7F, 0x7B,
    0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E,
    0x4C, 0x7E, 0x6E, 0x6F, 0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xE2,
    0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0x4A, 0xE0, 0x5A, 0x5F, 0x6D,
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92,
    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
    0xA7, 0xA8, 0xA9, 0xC0, 0x6A, 0xD0, 0xA1, 0x07, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F
};

const unsigned char ebc2asc[256] = {
    0x00, 0x01, 0x02, 0x03, 0x1A, 0x09, 0x1A, 0x7F, 0x1A, 0x1A, 0x1A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x1A, 0x1A, 0x08, 0x1A,
    0x18, 0x19, 0x1A, 0x1A, 0x1C, 0x1D, 0x1E, 0x1F, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x0A, 0x17, 0x1B, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x05, 0x06, 0x07,
    0x1A, 0x1A, 0x16, 0x1A, 0x1A, 0x1A, 0x1A, 0x04, 0x1A, 0x1A, 0x1A, 0x1A,
    0x14, 0x15, 0x1A, 0x1A, 0x20, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x5B, 0x2E, 0x3C, 0x28, 0x2B, 0x21, 0x26, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x5D, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
    0x2D, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x7C, 0x2C,
    0x25, 0x5F, 0x3E, 0x3F, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22, 0x1A, 0x61, 0x62, 0x63,
    0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
    0x51, 0x52, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x5C, 0x1A, 0x53, 0x54,
    0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A
};

static int ModWriteBsc(struct fs_handle *hdl, const void *buf, size_t n)
{
    struct fs_node *node = (struct fs_node *)hdl->node->driver_data;
    struct fs_handle *tmphdl;

    unsigned char *ebcdic_buf;
    size_t i;

    ebcdic_buf = MmAllocateZero(n);
    if(ebcdic_buf == NULL) {
        KePanic("Out of memory");
    }

    KeCopyMemory(ebcdic_buf, buf, n);

    /* Expand special characters and make everything uppercase */
    for(i = 0; i < n; i++) {
        /* Make characters be uppercase */
        if(ebcdic_buf[i] >= 'a' && ebcdic_buf[i] <= 'i') {
            ebcdic_buf[i] = 'A' + (ebcdic_buf[i] - 'a');
        } else if(ebcdic_buf[i] >= 'j' && ebcdic_buf[i] <= 'r') {
            ebcdic_buf[i] = 'J' + (ebcdic_buf[i] - 'j');
        } else if(ebcdic_buf[i] >= 's' && ebcdic_buf[i] <= 'z') {
            ebcdic_buf[i] = 'S' + (ebcdic_buf[i] - 's');
        }
    }

    /* Convert EBCDIC to ASCII */
    for(i = 0; i < n; i++) {
        ebcdic_buf[i] = ebc2asc[ebcdic_buf[i]];
    }

    tmphdl = KeOpenFromFsNode(node, VFS_MODE_WRITE);
    if(tmphdl == NULL) {
        MmFree(ebcdic_buf);
        return -1;
    }
    node->driver->write(tmphdl, ebcdic_buf, n);
    KeCloseFsNode(tmphdl);

    MmFree(ebcdic_buf);
    return 0;
}

static int ModReadBsc(struct fs_handle *hdl, void *buf, size_t n)
{
    struct fs_node *node = (struct fs_node *)hdl->node->driver_data;
    struct fs_handle *tmphdl;

    unsigned char *str_buf = (unsigned char *)buf;
    size_t i;

    tmphdl = KeOpenFromFsNode(node, VFS_MODE_READ);
    if(tmphdl == NULL) {
        return -1;
    }
    node->driver->read(tmphdl, buf, n);
    KeCloseFsNode(tmphdl);

    for(i = 0; i < n; i++) {
        str_buf[i] = asc2ebc[str_buf[i]];
    }
    return 0;
}

int ModInitBsc(void)
{
    struct fs_node *node;

    bsc_driver = KeCreateFsDriver();
    bsc_driver->write = &ModWriteBsc;
    bsc_driver->read = &ModReadBsc;

    node = KeCreateFsNode("A:\\MODULES", "BSC");
    KeAddFsNodeToDriver(bsc_driver, node);

    node->driver_data = KeResolveFsPath("A:\\MODULES\\IBM-2703");
    if(node->driver_data == NULL) {
        KeDebugPrint("bsc: No available x2703 device\r\n");
        return -1;
    }

    KeDebugPrint("bsc: Line device wrapper initialized\r\n");
    return 0;
}

/* Driver global for VFS */
static struct fs_driver *x2703_driver;
/* Device number allocation for VFS */
static size_t x2703u_devnum = 0;

struct DeviceX2703Info {
    struct css_device dev;
    struct css_request *write_req;
    struct css_request *read_req;
};

static int ModEnableX2703(struct DeviceX2703Info *info)
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

static int ModWriteX2703(struct fs_handle *hdl, const void *buf, size_t n)
{
    struct DeviceX2703Info *drive = hdl->node->driver_data;
    int r;
    
    drive->write_req = CssNewRequest(&drive->dev, 1);
    drive->write_req->flags = CSS_REQUEST_MODIFY;

    drive->write_req->ccws[0].cmd = CSS_CMD_WRITE;
    CSS_SET_ADDR(&drive->write_req->ccws[0], buf);
    drive->write_req->ccws[0].flags = 0;
    drive->write_req->ccws[0].length = (uint16_t)n;

    drive->dev.orb.flags = 0x0080FF00;

    CssSendRequest(drive->write_req);
    r = CssPerformRequest(drive->write_req);
    CssDestroyRequest(drive->write_req);
    return r;
}

static int ModReadX2703(struct fs_handle *hdl, void *buf, size_t n)
{
    struct DeviceX2703Info *drive = hdl->node->driver_data;
    int r;
    
    drive->read_req = CssNewRequest(&drive->dev, 1);
    drive->read_req->flags = CSS_REQUEST_MODIFY | CSS_REQUEST_IGNORE_CC | CSS_REQUEST_WAIT_ATTENTION;
    
    /* Using the generic CSS_CMD_READ is not valid, a special version
     **must** be used - otherwise it won't work */
    drive->read_req->ccws[0].cmd = 0x0A;
    CSS_SET_ADDR(&drive->read_req->ccws[0], buf);
    drive->read_req->ccws[0].flags = 0x20;
    drive->read_req->ccws[0].length = (uint16_t)n;

    drive->dev.orb.flags = 0x0080FF00;

    CssSendRequest(drive->read_req);
    r = CssPerformRequest(drive->read_req);
    CssDestroyRequest(drive->read_req);
    return r;
}

int ModAdd2703Device(struct css_schid schid, struct css_senseid *sensebuf)
{
    struct DeviceX2703Info *drive;
    struct fs_node *node;
    char tmpbuf[2] = {0};

    tmpbuf[0] = x2703u_devnum % 10 + '0';

    drive = MmAllocatePhysical(sizeof(struct DeviceX2703Info), 8);
    if(drive == NULL) {
        KePanic("Out of memory");
    }
    KeSetMemory(&drive, 0, sizeof(struct DeviceX2703Info));
    KeCopyMemory(&drive->dev.schid, &schid, sizeof(schid));
	ModEnableX2703(drive);

    /* Create a new node with the format IBM-2703.XXX, number assigned by
     * the variable x2703u_devnum */
    node = KeCreateFsNode("A:\\MODULES\\IBM-2703", &tmpbuf[0]);
    KeAddFsNodeToDriver(x2703_driver, node);
    node->driver_data = drive;

    x2703u_devnum++;

    KeDebugPrint("x3270: Drive address is %i:%i\r\n", (int)drive->dev.schid.id, (int)drive->dev.schid.num);
    return 0;
}

int ModInit2703(void)
{
    struct fs_node *node;

    KeDebugPrint("x2703: Initializing driver\r\n");
    x2703_driver = KeCreateFsDriver();
    x2703_driver->write = &ModWriteX2703;
    x2703_driver->read = &ModReadX2703;

    node = KeCreateFsNode("A:\\MODULES", "IBM-2703");
    /*KeAddFsNodeToDriver(x2703_driver, node);*/
    return 0;
}

/* Driver global for VFS */
static struct fs_driver *x3270_driver;
/* Device number allocation for VFS */
static size_t x3270u_devnum = 0;

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

static unsigned int ModGetX3270Address(int addr)
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

static int ModEnableX3270(struct DeviceX3270Info *info)
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

static int ModWriteX3270(struct fs_handle *hdl, const void *buf, size_t n)
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

static int ModReadX3270(struct fs_handle *hdl, void *buf, size_t n)
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

int ModAdd3270Device(struct css_schid schid, struct css_senseid *sensebuf)
{
    struct DeviceX3270Info *drive;
    struct fs_node *node;
    char tmpbuf[2] = {0};

    tmpbuf[0] = x3270u_devnum % 10 + '0';

    drive = MmAllocatePhysical(sizeof(struct DeviceX3270Info), 8);
    if(drive == NULL) {
        KePanic("Out of memory\r\n");
    }
    KeSetMemory(&drive, 0, sizeof(struct DeviceX3270Info));
    KeCopyMemory(&drive->dev.schid, &schid, sizeof(schid));
    ModEnableX3270(drive);
    
    if(sensebuf != NULL) {
        switch(sensebuf->cu_type) {
        case 0x3274:
            drive->cols = 80;
            drive->rows = 24;
            break;
        default:
            KePanic("Unknown model %x\r\n", (unsigned int)sensebuf->cu_type);
            break;
        }
    }

    /* Create a new node with the format IBM-3270.XXX, number assigned by
     * the variable x3270u_devnum */
    node = KeCreateFsNode("A:\\MODULES\\IBM-3270", &tmpbuf[0]);
    KeAddFsNodeToDriver(x3270_driver, node);
    node->driver_data = drive;

    x3270u_devnum++;

    KeDebugPrint("x3270: Drive address is %i:%i\r\n", (int)drive->dev.schid.id, (int)drive->dev.schid.num);
    return 0;
}

int ModInit3270(void)
{
    struct fs_node *node;

    KeDebugPrint("x3270: Initializing driver\r\n");
    x3270_driver = KeCreateFsDriver();
    x3270_driver->write = &ModWriteX3270;
    x3270_driver->read = &ModReadX3270;

    node = KeCreateFsNode("A:\\MODULES", "IBM-3270");
    /*KeAddFsNodeToDriver(x3270_driver, node);*/
    return 0;
}
