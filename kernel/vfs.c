#include <alloc.h>
#include <panic.h>
#include <printf.h>
#include <string.h>
#include <vfs.h>

static struct vfs_node root_node = {0};

int vfs_init(void) {
    root_node.children   = NULL;
    root_node.n_children = 0;
    root_node.name       = NULL;
    return 0;
}

int vfs_add_child(struct vfs_node *parent, struct vfs_node *child) {
    parent->children = krealloc_array(
        parent->children, parent->n_children + 1, sizeof(struct vfs_node *));
    if (parent->children == NULL) {
        kpanic("Out of memory");
    }
    parent->children[parent->n_children] = child;
    parent->n_children++;
    return 0;
}

struct vfs_node *vfs_resolve_path(const char *path) {
    struct vfs_node *root = &root_node;
    size_t filename_len   = 0, i;
    char *buffer;

    if (*path != '\\') {
        return NULL;
    }

    buffer = kmalloc(strlen(path) + 1);
    if (buffer == NULL) {
        kpanic("Out of memory");
    }
    strcpy(buffer, path);
    buffer++;

find_file:
    /* File found */
    if (*buffer == '\0') {
        kfree(buffer);
        return root;
    }

    filename_len = 0;
    while (buffer[filename_len] != '\\' && buffer[filename_len] != '\0') {
        filename_len++;
    }

    for (i = 0; i < root->n_children; i++) {
        struct vfs_node *child = root->children[i];
        if (!strncmp(buffer, child->name, filename_len)) {
            root = child;

            /* Skip the filename (and the slash following it) */
            buffer += filename_len + 1;

            if (buffer[-1] == '\0') {
                kfree(buffer);
                return root;
            }
            goto find_file;
        }
    }
    kfree(buffer);
    return NULL;
}

struct vfs_node *vfs_new_node(const char *name, const char *path) {
    struct vfs_node *node, *root;
    node = kmalloc(sizeof(struct vfs_node));
    memset(node, 0, sizeof(struct vfs_node));
    node->name = kmalloc(strlen(name) + 1);
    strcpy(node->name, name);

    root = vfs_resolve_path(path);
    vfs_add_child(root, node);
    return node;
}

struct vfs_node *vfs_find_node(const char *name) {
    size_t i;
    for (i = 0; i < root_node.n_children; i++) {
        struct vfs_node *node = root_node.children[i];
        if (!strcmp(node->name, name)) {
            return node;
        }
    }
    return NULL;
}

static struct vfs_node *fd_table[512] = {0};
int vfs_open(const char *path, int mode) {
    size_t i;
    for (i = 0; i < 512; i++) {
        if (fd_table[i] != NULL) {
            continue;
        }

        fd_table[i] = vfs_resolve_path(path);
        return (int)i;
    }
    return -1;
}

void vfs_close(int fd) {
    fd_table[fd] = NULL;
    return;
}

int vfs_write(int fd, const void *buf, size_t n) {
    if (fd_table[fd] == NULL || fd_table[fd]->hooks.write == NULL) {
        return -1;
    }

    return fd_table[fd]->hooks.write(fd, buf, n);
}

int vfs_read(int fd, void *buf, size_t n) {
    if (fd_table[fd] == NULL || fd_table[fd]->hooks.read == NULL) {
        return -1;
    }

    return fd_table[fd]->hooks.read(fd, buf, n);
}

int vfs_ioctl(int fd, int cmd, ...) {
    int r;
    va_list args;

    if (fd_table[fd] == NULL || fd_table[fd]->hooks.ioctl == NULL) {
        return -1;
    }

    va_start(args, cmd);
    r = fd_table[fd]->hooks.ioctl(fd, cmd, args);
    va_end(args);
    return r;
}

int vfs_flush(int fd) {
    if (fd_table[fd] == NULL || fd_table[fd]->hooks.flush == NULL) {
        return -1;
    }

    return fd_table[fd]->hooks.flush(fd);
}

static void vfs_dump_node(struct vfs_node *node, int level) {
    size_t i;

    if (node == NULL) {
        return;
    }

    for (i = 0; i < (size_t)level; i++) {
        kputc(' ');
    }
    kprintf("%s\n", node->name);

    for (i = 0; i < node->n_children; i++) {
        struct vfs_node *child = node->children[i];
        vfs_dump_node(child, level + 1);
    }
}

void vfs_dump(void) { vfs_dump_node(&root_node, 0); }
