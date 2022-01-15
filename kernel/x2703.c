/* x2703.c
 *
 * Driver for 2703 BSC communication line devices
 */

#include <mm/mm.h>
#include <debug/printf.h>
#include <debug/panic.h>
#include <asm.h>
#include <css.h>
#include <x2703.h>
#include <fs/fs.h>

/* Driver global for VFS */
static struct FsDriver *driver;
/* Device number allocation for VFS */
static size_t u_devnum = 0;

struct DeviceX2703Info {
    struct css_device dev;

    struct css_request *write_req;
    struct css_request *read_req;
};

static int ModEnableX2703(
    struct DeviceX2703Info *info)
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

static int ModWriteX2703(
    struct FsHandle *hdl,
    const void *buf,
    size_t n)
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

static int ModReadX2703(
    struct FsHandle *hdl,
    void *buf,
    size_t n)
{
    struct DeviceX2703Info *drive = hdl->node->driver_data;
    int r;
    
    drive->read_req = CssNewRequest(&drive->dev, 1);
    drive->read_req->flags = CSS_REQUEST_MODIFY | CSS_REQUEST_IGNORE_CC
        | CSS_REQUEST_WAIT_ATTENTION;
    
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

int ModAddX2703Device(
    struct css_schid schid,
    struct css_senseid *sensebuf)
{
    struct DeviceX2703Info *drive;
    struct FsNode *node;
    char tmpbuf[2] = {0};

    tmpbuf[0] = u_devnum % 10 + '0';

    drive = MmAllocateZero(sizeof(struct DeviceX2703Info));
    if(drive == NULL) {
        KePanic("Out of memory");
    }
    KeCopyMemory(&drive->dev.schid, &schid, sizeof(schid));

    /* Create a new node with the format IBM-2703.XXX, number assigned by
     * the variable u_devnum */
    node = KeCreateFsNode("A:\\MODULES\\IBM-2703", &tmpbuf[0]);
    KeAddFsNodeToDriver(driver, node);
    node->driver_data = drive;

    u_devnum++;

    KeDebugPrint("x3270: Drive address is %i:%i\r\n", (int)drive->dev.schid.id,
        (int)drive->dev.schid.num);
    return 0;
}

int ModInitX2703(
    void)
{
    struct FsNode *node;

    KeDebugPrint("x2703: Initializing driver\r\n");
    driver = KeCreateFsDriver();
    driver->write = &ModWriteX2703;
    driver->read = &ModReadX2703;

    node = KeCreateFsNode("A:\\MODULES", "IBM-2703");
    /*KeAddFsNodeToDriver(driver, node);*/
    return 0;
}
