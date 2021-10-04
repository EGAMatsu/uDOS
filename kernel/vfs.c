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
    parent->children[parent->n_children++] = child;
    return 0;
}

struct vfs_node *vfs_resolve_path(
    const char *path)
{
    const struct vfs_node *root = &root_node;
    size_t filename_len = 0, i;
    const char *tmpbuf = path; /* The pointer based off buffer for name comparasions*/
    const char *filename_end; /* Pointer to the end of a filename */

    if(*tmpbuf != '/' && *tmpbuf != '\\') {
        return NULL;
    }

    /* Skip the path separator */
    tmpbuf++;

find_file:
    /* File found */
    if(*tmpbuf == '\0') {
        return (struct vfs_ndoe *)root;
    }

    filename_end = strpbrk(tmpbuf, "/\\");
    filename_len = (filename_end == NULL) ? strlen(tmpbuf)
        : (size_t)((ptrdiff_t)filename_end - (ptrdiff_t)tmpbuf);
    for(i = 0; i < root->n_children; i++) {
        const struct vfs_node *child = root->children[i];

        if(strlen(child->name) != filename_len) {
            continue;
        }

        if(!strncmp(tmpbuf, child->name, filename_len)) {
            root = child;

            /* If this is not the end of the filepath then we just advance
             * past the path separator */
            tmpbuf += filename_len;
            if(*tmpbuf == '/' || *tmpbuf == '\\') {
                tmpbuf++;
            }
            goto find_file;
        }
    }
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
    if(node->name == NULL) {
        kpanic("Out of memory");
    }
    strcpy(node->name, name);

    root = vfs_resolve_path(path);
    if(root == NULL) {
        root = &root_node;
    }
    vfs_add_child(root, node);
    return node;
}

struct vfs_node *vfs_open(
    const char *path,
    int mode)
{
    struct vfs_node *node = vfs_resolve_path(path);
    if(node == NULL) {
        return NULL;
    }

    if(node->driver == NULL || node->driver->open == NULL) {
        goto end;
    }
    node->driver->open(node);
end:
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
    memset(driver, 0, sizeof(struct vfs_driver));
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

    for(i = 0; i < (size_t)level * 4; i++) {
        kputc('-');
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
