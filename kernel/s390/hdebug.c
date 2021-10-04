/* HDEBUG - Hercules debug facility driver */

#include <printf.h>
#include <string.h>
#include <alloc.h>
#include <vfs.h>

int hdebug_write(
    struct vfs_node *node,
    const void *buf,
    size_t n)
{
    char tmpbuf[80 + 6];
    memcpy(&tmpbuf[0], "MSG * ", 6);
    memcpy(&tmpbuf[6], buf, n);

    if(tmpbuf[n + 5] == '\n') {
        n--;
    }

    __asm__ __volatile__(
        "diag %0, %1, 8"
        :
        : "r"(&tmpbuf[0]), "r"(n + 6)
        : "cc", "memory");
    return 0;
}

int hdebug_read(
    struct vfs_node *node,
    void *buf,
    size_t n)
{
    
    return 0;
}

int hdebug_init(
    void)
{
    struct vfs_driver *driver;
    struct vfs_node *node;

    driver = vfs_new_driver();
    driver->write = &hdebug_write;
    driver->read = &hdebug_read;

    node = vfs_new_node("\\SYSTEM\\DEVICES", "HDEBUG");
    node->driver = driver;
    return 0;
}