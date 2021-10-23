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
struct FsFdscb {
    uint16_t cyl;
    uint16_t head;
    uint8_t rec;
};

/* A node representing a directory (has children) or a file (no children) */
struct FsNode {
    unsigned char flags;
    char *name;

    struct FsDriver *driver;
    void *driver_data;

    struct FsNode **children;
    size_t n_children;
};

int KeInitFs(void);
int KeAddFsNodeChild(struct FsNode *parent, struct FsNode *child);
struct FsNode *KeResolveFsPath(const char *path);
struct FsNode *KeCreateFsNode(const char *path, const char *name);

/* Handle for opening a node, not viewed by the caller and only used internally
 * by the functions below */
struct FsHandle {
    struct FsNode *node;
    int mode;

    void *read_buf;
    size_t read_buf_size;

    void *write_buf;
    size_t write_buf_size;

    int flags;
};

struct FsHandle *KeOpenFromFsNode(struct FsNode *node, int flags);
struct FsHandle *KeOpenFsNode(const char *path, int flags);
void KeCloseFsNode(struct FsHandle *hdl);
int KeWriteFsNode(struct FsHandle *hdl, const void *buf, size_t n);
int KeReadFsNode(struct FsHandle *hdl, void *buf, size_t n);
int KeWriteWithFdscbFsNode(struct FsHandle *hdl, struct FsFdscb *fdscb,
    const void *buf, size_t n);
int KeReadWithFdscbFsNode(struct FsHandle *hdl, struct FsFdscb *fdscb,
    void *buf, size_t n);
int KeIoControlFsNode(struct FsHandle *hdl, int cmd, ...);
int KeFlushFsNode(struct FsHandle *hdl);

/* Manage nodes via ownership - This is used so drivers can register nodes and
 * when they need to terminate they can just destroy their registered nodes
 * without worrying about leftovers */
struct FsDriver {
    struct FsNode **nodes;
    size_t n_nodes;

    int (*open)(struct FsHandle *hdl);
    int (*close)(struct FsHandle *hdl);
    int (*write)(struct FsHandle *hdl, const void *buf, size_t n);
    int (*read)(struct FsHandle *hdl, void *buf, size_t n);
    int (*seek)(struct FsHandle *hdl, int whence, long offset);
    int (*flush)(struct FsHandle *hdl);
    int (*ioctl)(struct FsHandle *hdl, int cmd, va_list args);

    int (*write_fdscb)(struct FsHandle *hdl, struct FsFdscb *fdscb,
        const void *buf, size_t n);
    int (*read_fdscb)(struct FsHandle *hdl, struct FsFdscb *fdscb, void *buf,
        size_t n);
    
    /* Retargeted-control handlers */
    struct FsNode *(*request_node)(const char *path);
    int (*add_node)(struct FsNode *node, const char *path);
    int (*remove_node)(const char *path);
};

struct FsDriver *KeCreateFsDriver(void);
int KeAddFsNodeToDriver(struct FsDriver *driver, struct FsNode *node);

#if defined(DEBUG)
void KeDumpFs(void);
#endif

#endif
