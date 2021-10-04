#include <alloc.h>
#include <panic.h>
#include <printf.h>
#include <string.h>
#include <vfs.h>

static struct vfs_node root_node = {0};

static struct {
    struct vfs_driver *drivers;
    size_t n_driver;
}g_drvtab = {0};

int vfs_init(
    void)
{
    g_drvtab.drivers = NULL;
    g_drvtab.n_driver = 0;
    return 0;
}

int vfs_add_child(
    struct vfs_node *parent,
    struct vfs_node *child)
{
    parent->children = krealloc_array(parent->children, parent->n_children + 1,
        sizeof(struct vfs_node *));
    if(parent->children == NULL) {
        kpanic("Out of memory");
    }
    parent->children[parent->n_children] = child;
    parent->n_children++;
    return 0;
}

struct vfs_node *vfs_resolve_path(
    const char *path)
{
    struct vfs_node *root = &root_node;
    size_t filename_len = 0, i;
    char *buffer, *filename_end;

    if(*path != '\\') {
        return NULL;
    }

    buffer = kmalloc(strlen(path) + 1);
    if(buffer == NULL) {
        kpanic("Out of memory");
    }
    strcpy(buffer, path);
    buffer++;

find_file:
    /* File found */
    if(*buffer == '\0') {
        kfree(buffer);
        return root;
    }

    filename_end = strchr(buffer, '\\');
    filename_len = (filename_end == NULL)
        ? strlen(buffer)
        : (size_t)((ptrdiff_t)filename_end - (ptrdiff_t)buffer);
    for(i = 0; i < root->n_children; i++) {
        struct vfs_node *child = root->children[i];
        if(!strncmp(buffer, child->name, filename_len)) {
            root = child;

            /* Skip the filename (and the slash following it) */
            buffer += filename_len + 1;
            if(buffer[-1] == '\0') {
                kfree(buffer);
                return root;
            }
            goto find_file;
        }
    }
    kfree(buffer);
    return NULL;
}

struct vfs_node *vfs_new_node(
    const char *path,
    const char *name)
{
    struct vfs_node *node, *root;
    node = kzalloc(sizeof(struct vfs_node));
    if(node == NULL) {
        kpanic("Out of memory");
    }
    node->name = kmalloc(strlen(name) + 1);
    strcpy(node->name, name);

    root = vfs_resolve_path(path);
    vfs_add_child(root, node);
    return node;
}

struct vfs_node *vfs_find_node(
    const char *name)
{
    size_t i;
    for(i = 0; i < root_node.n_children; i++) {
        struct vfs_node *node = root_node.children[i];
        if(!strcmp(node->name, name)) {
            return node;
        }
    }
    return NULL;
}

struct vfs_node *vfs_open(
    const char *path,
    int mode)
{
    struct vfs_node *node = vfs_resolve_path(path);
    if(node == NULL) {
        return NULL;
    }

    if(node->driver != NULL && node->driver->open != NULL) {
        node->driver->open(node);
    }
    return node;
}

void vfs_close(
    struct vfs_node *node)
{
    if(node->driver == NULL || node->driver->close == NULL) {
        return;
    }

    node->driver->close(node);
    return;
}

int vfs_write(
    struct vfs_node *node,
    const void *buf,
    size_t n)
{
    if(node->driver == NULL || node->driver->write == NULL) {
        return -1;
    }

    return node->driver->write(node, buf, n);
}

int vfs_read(
    struct vfs_node *node,
    void *buf,
    size_t n)
{
    if(node->driver == NULL || node->driver->read == NULL) {
        return -1;
    }

    return node->driver->read(node, buf, n);
}

int vfs_write_fdscb(
    struct vfs_node *node,
    struct vfs_fdscb *fdscb,
    const void *buf,
    size_t n)
{
    if(node->driver == NULL || node->driver->write_fdscb == NULL) {
        return -1;
    }

    return node->driver->write_fdscb(node, fdscb, buf, n);
}

int vfs_read_fdscb(
    struct vfs_node *node,
    struct vfs_fdscb *fdscb,
    void *buf,
    size_t n)
{
    if(node->driver == NULL || node->driver->read_fdscb == NULL) {
        return -1;
    }

    return node->driver->read_fdscb(node, fdscb, buf, n);
}

int vfs_ioctl(
    struct vfs_node *node,
    int cmd,
    ...)
{
    int r;
    va_list args;

    if(node->driver == NULL || node->driver->ioctl == NULL) {
        return -1;
    }

    va_start(args, cmd);
    r = node->driver->ioctl(node, cmd, args);
    va_end(args);
    return r;
}

int vfs_flush(
    struct vfs_node *node)
{
    if(node->driver == NULL || node->driver->flush == NULL) {
        return -1;
    }

    return node->driver->flush(node);
}

struct vfs_driver *vfs_new_driver(
    void)
{
    struct vfs_driver *driver = &g_drvtab.drivers[g_drvtab.n_driver];

    g_drvtab.drivers = krealloc_array(g_drvtab.drivers, g_drvtab.n_driver + 1,
        sizeof(struct vfs_driver));
    if(g_drvtab.drivers == NULL) {
        return -1;
    }
    driver = &g_drvtab.drivers[g_drvtab.n_driver++];
    return driver;
}

int vfs_driver_add_node(
    struct vfs_driver *driver,
    struct vfs_node *node)
{
    driver->nodes = krealloc_array(driver->nodes, driver->n_nodes + 1,
        sizeof(struct vfs_node *));
    if(driver->nodes == NULL) {
        return -1;
    }
    driver->nodes[driver->n_nodes] = node;
    driver->n_nodes++;
    return 0;
}

static void vfs_dump_node(
    struct vfs_node *node,
    int level)
{
    size_t i;

    if(node == NULL) {
        return;
    }

    for(i = 0; i < (size_t)level; i++) {
        kputc(' ');
    }
    kprintf("%s\n", node->name);

    for(i = 0; i < node->n_children; i++) {
        struct vfs_node *child = node->children[i];
        vfs_dump_node(child, level + 1);
    }
    return;
}

void vfs_dump(
    void)
{
    vfs_dump_node(&root_node, 0);
}
