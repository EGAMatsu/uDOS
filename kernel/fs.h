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
struct fs_fdscb {
    uint16_t cyl;
    uint16_t head;
    uint8_t rec;
};

/* A node representing a directory (has children) or a file (no children) */
struct fs_node {
    unsigned char flags;
    char *name;

    struct fs_driver *driver;
    void *driver_data;

    struct fs_node **children;
    size_t n_children;
};

int KeInitFs(void);

#define KeAddFsNodeChild _Zfsadnch
int KeAddFsNodeChild(struct fs_node *parent, struct fs_node *child);
struct fs_node *KeResolveFsPath(const char *path);
#define KeCreateFsNode _Zfscnode
struct fs_node *KeCreateFsNode(const char *path, const char *name);

/* Handle for opening a node, not viewed by the caller and only used internally
 * by the functions below */
struct fs_handle {
    struct fs_node *node;
    int mode;

    void *read_buf;
    size_t read_buf_size;

    void *write_buf;
    size_t write_buf_size;

    int flags;
};

#define KeOpenFromFsNode _Zfsofn
struct fs_handle *KeOpenFromFsNode(struct fs_node *node, int flags);
#define KeOpenFsNode _Zfson
struct fs_handle *KeOpenFsNode(const char *path, int flags);
#define KeCloseFsNode _Zfs0cos
void KeCloseFsNode(struct fs_handle *hdl);
#define KeWriteFsNode _Zfswfn
int KeWriteFsNode(struct fs_handle *hdl, const void *buf, size_t n);
#define KeReadFsNode _Zfsrfn
int KeReadFsNode(struct fs_handle *hdl, void *buf, size_t n);
#define KeWriteWithFdscbFsNode _Zfswffn
int KeWriteWithFdscbFsNode(struct fs_handle *hdl, struct fs_fdscb *fdscb, const void *buf, size_t n);
#define KeReadWithFdscbFsNode _Zfsrffn
int KeReadWithFdscbFsNode(struct fs_handle *hdl, struct fs_fdscb *fdscb, void *buf, size_t n);
#define KeIoControlFsNode _Zfs0ctl
int KeIoControlFsNode(struct fs_handle *hdl, int cmd, ...);
#define KeFlushFsNode _Zfsffn
int KeFlushFsNode(struct fs_handle *hdl);

/* Manage nodes via ownership - This is used so drivers can register nodes and
 * when they need to terminate they can just destroy their registered nodes
 * without worrying about leftovers */
struct fs_driver {
    struct fs_node **nodes;
    size_t n_nodes;

    int (*open)(struct fs_handle *hdl);
    int (*close)(struct fs_handle *hdl);
    int (*write)(struct fs_handle *hdl, const void *buf, size_t n);
    int (*read)(struct fs_handle *hdl, void *buf, size_t n);
    int (*seek)(struct fs_handle *hdl, int whence, long offset);
    int (*flush)(struct fs_handle *hdl);
    int (*ioctl)(struct fs_handle *hdl, int cmd, va_list args);

    int (*write_fdscb)(struct fs_handle *hdl, struct fs_fdscb *fdscb, const void *buf, size_t n);
    int (*read_fdscb)(struct fs_handle *hdl, struct fs_fdscb *fdscb, void *buf, size_t n);
    
    /* Retargeted-control handlers */
    void *(*request_node)(const struct fs_node *root, const char *path);
    int (*add_node)(struct fs_node *node, const char *path);
    int (*remove_node)(const char *path);
};

#define KeCreateFsDriver _Zfscfd
struct fs_driver *KeCreateFsDriver(void);
#define KeAddFsNodeToDriver _Zfsantd
int KeAddFsNodeToDriver(struct fs_driver *driver, struct fs_node *node);

#if defined(DEBUG)
void KeDumpFs(void);
#endif

#endif
