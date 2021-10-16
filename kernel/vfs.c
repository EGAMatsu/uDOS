/* vfs.c
 *
 * Implements a BTree+ virtual filesystem that can be managed by the kernel
 * and the drivers to make a nice representation of a node-based file management
 * subsystem. This virtual filesystem has 2 modes of operation:
 * 
 * FDSCB Operation Mode
 * In this mode the files are accessed with a FDSCB descriptor describing head,
 * cylinder and record index of a file, normally this would just be allowed only
 * on legacy disks and S390 DASD tapes
 * 
 * *NIX Operation Mode
 * In this mode the files are accessed via the usual read/write operations
 * present on *NIX systems
 * 
 * UDOS is very lenient when it comes to paths, it takes in consideration that
 * files are case insensitive in order to encourage better naming of said files
 * it also does not care about the side the slash is facing at, both backslashes
 * and slashes are valid, even when both are used in the same path:
 * \System\Document.txt
 * /System/Document.txt
 * \SYSTEM\DOCUMENT.TXT
 * \System/DOCUMENT.txt
 */

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

/* Resolve the path and return a node */
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

        /* Must be same length */
        if(strlen(child->name) != filename_len) {
            continue;
        }

        /* TODO: Case insensitive node search */
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

struct vfs_handle *vfs_open_from_node(
    struct vfs_node *node,
    int flags)
{
    struct vfs_handle *hdl;
    int r;

    if(node == NULL || node->driver == NULL) {
        return NULL;
    }
    
    hdl = kzalloc(sizeof(struct vfs_handle));
    if(hdl == NULL) {
        return NULL;
    }
    
    hdl->node = node;
    if(hdl->node->driver->open != NULL) {
        r = hdl->node->driver->open(hdl);
        if(r != 0) {
            kfree(hdl);
            return NULL;
        }
    }
    hdl->flags = flags;
    return hdl;
}

struct vfs_handle *vfs_open(
    const char *path,
    int flags)
{
    struct vfs_handle *hdl;
    hdl = vfs_open_from_node(vfs_resolve_path(path), flags);
    return hdl;
}

void vfs_close(
    struct vfs_handle *hdl)
{
    if(hdl->node->driver->close != NULL) {
        hdl->node->driver->close(hdl);
    }
    
    kfree(hdl);
    return;
}

int vfs_write(
    struct vfs_handle *hdl,
    const void *buf,
    size_t n)
{
    if(hdl->node->driver->write == NULL) {
        return -1;
    }

    /* Concat the buffer to our write buffer when we later flush it */
    if(hdl->flags & VFS_MODE_BUFFERED) {
        hdl->write_buf = krealloc(hdl->write_buf, hdl->write_buf_size + n);
        if(hdl->write_buf == NULL) {
            return -1;
        }

        memcpy(&((unsigned char *)hdl->write_buf)[hdl->write_buf_size], buf, n);
        hdl->write_buf_size += n;
        return n;
    }
    /* On no-buffer mode the data is directly written instead of being buffered
     * by the handler buffer holders */
    else {
        return hdl->node->driver->write(hdl, buf, n);
    }
}

int vfs_read(
    struct vfs_handle *hdl,
    void *buf,
    size_t n)
{
    if(hdl->node->driver->read == NULL) {
        return -1;
    }
    return hdl->node->driver->read(hdl, buf, n);
}

int vfs_write_fdscb(
    struct vfs_handle *hdl,
    struct vfs_fdscb *fdscb,
    const void *buf,
    size_t n)
{
    if(hdl->node->driver->write_fdscb == NULL) {
        return -1;
    }
    return hdl->node->driver->write_fdscb(hdl, fdscb, buf, n);
}

int vfs_read_fdscb(
    struct vfs_handle *hdl,
    struct vfs_fdscb *fdscb,
    void *buf,
    size_t n)
{
    if(hdl->node->driver->read_fdscb == NULL) {
        return -1;
    }
    return hdl->node->driver->read_fdscb(hdl, fdscb, buf, n);
}

int vfs_ioctl(
    struct vfs_handle *hdl,
    int cmd,
    ...)
{
    int r;
    va_list args;

    if(hdl->node->driver->ioctl == NULL) {
        return -1;
    }

    va_start(args, cmd);
    r = hdl->node->driver->ioctl(hdl, cmd, args);
    va_end(args);
    return r;
}

int vfs_flush(
    struct vfs_handle *hdl)
{
    int r = 0;

    /* Only flush when there is a write buffer and the mode is buffered */
    if(hdl->flags & VFS_MODE_BUFFERED) {
        /* There must be a write callback */
        if(hdl->node->driver->write == NULL && hdl->write_buf_size != 0) {
            return -1;
        }

        /* Write it all at once in a single call */
        r = hdl->node->driver->write(hdl, hdl->write_buf, hdl->write_buf_size);

        /* Free up buffer */
        kfree(hdl->write_buf);
        hdl->write_buf = NULL;
        hdl->read_buf_size = 0;
    }
    return r;
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

    node->driver = driver;
    return 0;
}

#if defined(DEBUG)
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
    kprintf("%s\r\n", node->name);

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
#endif