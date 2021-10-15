/* bsc.c
 *
 * BSC generic communication line driver (wraps around a raw device to establish
 * an stable ASCII protocol with the other party)
 */

#include <comm/bsc.h>
#include <string.h>
#include <vfs.h>
#include <alloc.h>

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

int bsc_write(
    struct vfs_handle *hdl,
    const void *buf,
    size_t n)
{
    struct vfs_node *node = (struct vfs_node *)hdl->node->driver_data;
    struct vfs_handle *tmphdl;

    unsigned char *ebcdic_buf;
    size_t i;

    ebcdic_buf = kzalloc(n);
    memcpy(ebcdic_buf, buf, n);

    /* Expand special characters and make everything uppercase */
    for(i = 0; i < n; i++) {
        /* Convert a newline into a CR/LF pair */
        if(ebcdic_buf[i] == '\n') {
            ebcdic_buf = krealloc(ebcdic_buf, n + 1);
            ++n;

            ebcdic_buf[i++] = '\n';
            ebcdic_buf[i] = '\r';
        }

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

    tmphdl = vfs_open_from_node(node, VFS_MODE_WRITE);
    if(tmphdl == NULL) {
        kfree(ebcdic_buf);
        return -1;
    }
    node->driver->write(tmphdl, ebcdic_buf, n);
    vfs_close(tmphdl);

    kfree(ebcdic_buf);
    return 0;
}

int bsc_read(
    struct vfs_handle *hdl,
    void *buf,
    size_t n)
{
    struct vfs_node *node = (struct vfs_node *)hdl->node->driver_data;
    struct vfs_handle *tmphdl;

    unsigned char *str_buf = (unsigned char *)buf;
    size_t i;

    tmphdl = vfs_open_from_node(node, VFS_MODE_READ);
    if(tmphdl == NULL) {
        return -1;
    }
    node->driver->read(tmphdl, buf, n);
    vfs_close(tmphdl);

    for(i = 0; i < n; i++) {
        str_buf[i] = asc2ebc[str_buf[i]];
    }
    return 0;
}

int bsc_init(
    void)
{
    struct vfs_node *node;
    struct vfs_driver *driver;

    driver = vfs_new_driver();
    driver->write = &bsc_write;
    driver->read = &bsc_read;

    node = vfs_new_node("\\SYSTEM\\COMM", "BSC.000");
    vfs_driver_add_node(driver, node);

    node->driver_data = vfs_resolve_path("\\SYSTEM\\DEVICES\\IBM-2703");
    if(node->driver_data == NULL) {
        kprintf("bsc: No available x2703 device\n");
        return -1;
    }

    kprintf("bsc: Line device wrapper initialized\n");
    return 0;
}