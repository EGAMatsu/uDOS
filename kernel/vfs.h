#ifndef VFS_H
#define VFS_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

enum vfs_mode {
    VFS_MODE_READ = 0x01,
    VFS_MODE_WRITE = 0x02,
    VFS_MODE_BUFFERED = 0x04
};

#define SEEK_SET 0
#define SEEK_BEGIN 1
#define SEEK_END 2

/* File dataset control block, used for tapes and disks and tracking files */
struct vfs_fdscb {
    uint16_t cyl;
    uint16_t head;
    uint8_t rec;
};

/* A node representing a directory (has children) or a file (no children) */
struct vfs_node {
    unsigned char flags;
    char *name;

    struct vfs_driver *driver;
    void *driver_data;

    struct vfs_node **children;
    size_t n_children;
};

int vfs_init(void);
int vfs_add_child(struct vfs_node *parent, struct vfs_node *child);
struct vfs_node *vfs_resolve_path(const char *path);
struct vfs_node *vfs_new_node(const char *path, const char *name);

/* Handle for opening a node, not viewed by the caller and only used internally
 * by the functions below */
struct vfs_handle {
    struct vfs_node *node;
    int mode;

    void *read_buf;
    size_t read_buf_size;

    void *write_buf;
    size_t write_buf_size;

    int flags;
};

struct vfs_handle *vfs_open_from_node(struct vfs_node *node, int flags);
struct vfs_handle *vfs_open(const char *path, int flags);
void vfs_close(struct vfs_handle *hdl);
int vfs_write(struct vfs_handle *hdl, const void *buf, size_t n);
int vfs_read(struct vfs_handle *hdl, void *buf, size_t n);
int vfs_write_fdscb(struct vfs_handle *hdl, struct vfs_fdscb *fdscb,
    const void *buf, size_t n);
int vfs_read_fdscb(struct vfs_handle *hdl, struct vfs_fdscb *fdscb,
    void *buf, size_t n);
int vfs_ioctl(struct vfs_handle *hdl, int cmd, ...);
int vfs_flush(struct vfs_handle *hdl);

/* Manage nodes via ownership - This is used so drivers can register nodes and
 * when they need to terminate they can just destroy their registered nodes
 * without worrying about leftovers */
struct vfs_driver {
    struct vfs_node **nodes;
    size_t n_nodes;

    int (*open)(struct vfs_handle *hdl);
    int (*close)(struct vfs_handle *hdl);

    int (*write)(struct vfs_handle *hdl, const void *buf, size_t n);
    int (*read)(struct vfs_handle *hdl, void *buf, size_t n);
    int (*seek)(struct vfs_handle *hdl, int whence, long offset);

    int (*write_fdscb)(struct vfs_handle *hdl, struct vfs_fdscb *fdscb,
        const void *buf, size_t n);
    int (*read_fdscb)(struct vfs_handle *hdl, struct vfs_fdscb *fdscb, void *buf,
        size_t n);

    int (*flush)(struct vfs_handle *hdl);
    int (*ioctl)(struct vfs_handle *hdl, int cmd, va_list args);
};

struct vfs_driver *vfs_new_driver(void);
int vfs_driver_add_node(struct vfs_driver *driver, struct vfs_node *node);

#if defined(DEBUG)
void vfs_dump(void);
#endif

#endif
