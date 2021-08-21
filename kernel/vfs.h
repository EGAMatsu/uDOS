#ifndef VFS_H
#define VFS_H
#ifdef __cplusplus
extern "C" {
#endif

enum vfs_node_type {
	VFS_NODE_STREAM,
	VFS_NODE_PORTAL,
	VFS_NODE_GROUP,
};

#define IS_BUFFERED(t) (t & 1)
#define ENABLE_BUFFER(t) t |= 1
#define DISABLE_BUFFER(t) t &= ~(1)

#define O_READ (1 << 0)
#define O_WRITE (1 << 1)
#define O_TRUNC (1 << 2)
#define O_APPEND (1 << 3)

#include <stdarg.h>
struct vfs_hooks {
	unsigned char flags;
	int (*write)(int fd, const void *buf, size_t n);
	int (*read)(int fd, void *buf, size_t n);
	int (*seek)(int fd, int whence, long offset);
	int (*ioctl)(int fd, int cmd, va_list args);
	int (*flush)(int fd);
};

struct vfs_node {
	char *name;
	struct vfs_hooks hooks;

	struct vfs_node **children;
	size_t n_children;
};

int vfs_init(void);
int vfs_add_child(struct vfs_node *parent, struct vfs_node *child);
struct vfs_node *vfs_resolve_path(const char *path);
struct vfs_node *vfs_new_node(const char *name, const char *path);
struct vfs_node *vfs_find_node(const char *name);

int vfs_open(const char *path, int mode);
void vfs_close(int fd);
int vfs_write(int fd, const void *buf, size_t n);
int vfs_read(int fd, void *buf, size_t n);
int vfs_ioctl(int fd, int cmd, ...);
int vfs_flush(int fd);

void vfs_dump(void);

#ifdef __cplusplus
}
#endif
#endif
