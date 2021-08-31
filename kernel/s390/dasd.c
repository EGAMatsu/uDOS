#include <s390/dasd.h>
#include <s390/css.h>
#include <alloc.h>
#include <vfs.h>

struct css_ccw1 *cmd_chain = NULL;

int dasd_write(int fd, const void *buf, size_t n) {
    return 0;
}

int dasd_read(int fd, void *buf, size_t n) {
    return 0;
}

int dasd_seek(int fd, int whence, long offset) {
    return 0;
}

int dasd_ioctl(int fd, int cmd, va_list args) {
    return 0;
}

int dasd_flush(int fd) {
    return 0;
}

int dasd_init(void) {
    struct vfs_node* node;
    node = vfs_new_node("DASD", "\\");
    return 0;
}