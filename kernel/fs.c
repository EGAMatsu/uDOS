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
 * Virtual File (Dataset) System
 * UDOS is very lenient when it comes to paths, it takes in consideration that
 * files are case insensitive in order to encourage better naming of said files
 * it also does not care about the side the slash is facing at, both backslashes
 * and slashes are valid, even when both are used in the same path:
 * \System\Document.txt
 * /System/Document.txt
 * \SYSTEM\DOCUMENT.TXT
 * \System/DOCUMENT.txt
 * Dots (.) are also treated as separators between nodes.
 * 
 * Fast-Indexing in the VF(D)S
 * Fast indexing allows to index nodes via the usage of letters, which are
 * converted to numbers according to their position in the alphabet:
 * A:\DOCUMENT.TXT
 * b:\myasm.txt
 * 
 * Re-target Control of subsequent nodes
 * In case the children nodes of a node aren't visible for some reason (volatile
 * media or FTP servers) the control can be passed to the parent node by having
 * said node implement the request_node function. This will pass the path
 * (relative to the node) to the driver attached to the node.
 */

#include <mm.h>
#include <panic.h>
#include <printf.h>
#include <memory.h>
#include <fs.h>

static struct fs_node root_node = {0};
static struct {
    struct fs_driver *drivers;
    size_t n_driver;
}g_drvtab = {0};

int KeInitFs(
    void)
{
    return 0;
}

int KeAddFsNodeChild(
    struct fs_node *parent,
    struct fs_node *child)
{
    parent->children = MmReallocateArray(parent->children, parent->n_children + 1,
        sizeof(struct fs_node *));
    if(parent->children == NULL) {
        KePanic("Out of memory");
    }
    parent->children[parent->n_children++] = child;
    return 0;
}

static unsigned KeGetFsDrive(
    const char *path)
{
    unsigned letter = (unsigned)path[0];

    /* Case-insensitivity on letters */
    if(letter >= 'a' && letter <= 'i') {
        letter = 'A' + (letter - 'a');
    } else if(letter >= 'j' && letter <= 'r') {
        letter = 'J' + (letter - 'j');
    } else if(letter >= 's' && letter <= 'z') {
        letter = 'S' + (letter - 's');
    }

    /* Convert to a number */
    if(letter >= 'A' && letter <= 'I') {
        letter -= 'A';
    } else if(letter >= 'J' && letter <= 'R') {
        letter -= 'J';
    } else if(letter >= 'S' && letter <= 'Z') {
        letter -= 'S';
    }
    return letter;
}

/* Resolve the path and return a node */
struct fs_node *KeResolveFsPath(
    const char *path)
{
    const struct fs_node *root = &root_node;
    size_t filename_len = 0, i;
    const char *tmpbuf = path; /* The pointer based off buffer for name comparasions*/
    const char *filename_end; /* Pointer to the end of a filename */
    unsigned letter;

    /* An starting path separator means this is a path that does not employ
     * fast-indexing - so we skip the fast-indexing part */
    if(*tmpbuf == '/' || *tmpbuf == '\\' || *tmpbuf == '.') {
        tmpbuf++;
        goto find_file;
    }

    /* We will keep recursing the nodes using fast-indexing method
     * (single letter indexing) while there is alphabetical characters
     * on the path (a semicolon must be found) */
    while((*tmpbuf >= 'A' && *tmpbuf <= 'Z')
    || (*tmpbuf >= 'a' && *tmpbuf <= 'z')) {
        letter = KeGetFsDrive(tmpbuf);
        
        /* Only iterate to next child when the letter is valid */
        if(letter >= root->n_children) {
            return NULL;
        }
        root = root->children[letter];
        tmpbuf++;
    }

    /* Colon must proceed after the drive letter (to state that fast-indexing
     * is over and now normal-indexing is used) */
    if(*tmpbuf != ':') {
        goto found_file;
    }
    tmpbuf++;

    /* Skip the path separator (it is required to have it after the colon) */
    if(*tmpbuf != '/' && *tmpbuf != '\\' && *tmpbuf != '.') {
        goto found_file;
    }
    tmpbuf++;

find_file:
    /* File found */
    if(*tmpbuf == '\0') {
        goto found_file;
    }

    filename_end = KeBreakCharPtrString(tmpbuf, "/\\.");
    filename_len = (filename_end == NULL) ? KeStringLength(tmpbuf)
        : (size_t)((ptrdiff_t)filename_end - (ptrdiff_t)tmpbuf);
    
    /* TODO: Allow multiple asterisk nodes */
    for(i = 0; i < root->n_children; i++) {
        const struct fs_node *child = root->children[i];

        /* Retarget-control mode, ask the driver for the node instead */
        if(child->driver != NULL && child->driver->request_node != NULL) {
            /* Make path be relative to the current node :-) */
            tmpbuf += filename_len;
            if(*tmpbuf == '/' || *tmpbuf == '\\' || *tmpbuf == '.') {
                tmpbuf++;
            }
            return child->driver->request_node(child, tmpbuf);
        }

        /* Must be same length */
        if(KeStringLength(child->name) != filename_len) {
            continue;
        }

        /* TODO: Case insensitive node search */
        if(!KeCompareStringEx(tmpbuf, child->name, filename_len)) {
            root = child;

            /* If this is not the end of the filepath then we just advance
             * past the path separator */
            tmpbuf += filename_len;
            if(*tmpbuf == '/' || *tmpbuf == '\\' || *tmpbuf == '.') {
                tmpbuf++;
            }
            goto find_file;
        }
    }

    /* If none of the children matches then the file wasn't found */
    return NULL;
found_file:
    return (struct vfs_node *)root;
}

struct fs_node *KeCreateFsNode(
    const char *path,
    const char *name)
{
    struct fs_node *node, *root;

    node = MmAllocateZero(sizeof(struct fs_node));
    if(node == NULL) {
        KePanic("Out of memory");
    }

    node->name = MmAllocate(KeStringLength(name) + 1);
    if(node->name == NULL) {
        KePanic("Out of memory");
    }
    KeCopyString(node->name, name);

    root = KeResolveFsPath(path);
    if(root == NULL) {
        root = &root_node;
    }
    KeAddFsNodeChild(root, node);
    return node;
}

struct fs_handle *KeOpenFromFsNode(
    struct fs_node *node,
    int flags)
{
    struct fs_handle *hdl;
    int r;

    if(node == NULL || node->driver == NULL) {
        return NULL;
    }
    
    hdl = MmAllocateZero(sizeof(struct fs_handle));
    if(hdl == NULL) {
        return NULL;
    }
    
    hdl->node = node;
    if(hdl->node->driver->open != NULL) {
        r = hdl->node->driver->open(hdl);
        if(r != 0) {
            MmFree(hdl);
            return NULL;
        }
    }
    hdl->flags = flags;
    return hdl;
}

struct fs_handle *KeOpenFsNode(
    const char *path,
    int flags)
{
    struct fs_handle *hdl;
    hdl = KeOpenFromFsNode(KeResolveFsPath(path), flags);
    return hdl;
}

void KeCloseFsNode(
    struct fs_handle *hdl)
{
    if(hdl->node->driver->close != NULL) {
        hdl->node->driver->close(hdl);
    }
    
    MmFree(hdl);
    return;
}

int KeWriteFsNode(
    struct fs_handle *hdl,
    const void *buf,
    size_t n)
{
    if(n == 0) {
        return 0;
    }

    if(hdl->node->driver->write == NULL) {
        return -1;
    }

    /* Concat the buffer to our write buffer when we later flush it */
    if(hdl->flags & VFS_MODE_BUFFERED) {
        hdl->write_buf = MmReallocate(hdl->write_buf, hdl->write_buf_size + n);
        if(hdl->write_buf == NULL) {
            return -1;
        }

        KeCopyMemory(&((unsigned char *)hdl->write_buf)[hdl->write_buf_size], buf, n);
        hdl->write_buf_size += n;
        return n;
    }
    /* On no-buffer mode the data is directly written instead of being buffered
     * by the handler buffer holders */
    else {
        return hdl->node->driver->write(hdl, buf, n);
    }
}

int KeReadFsNode(
    struct fs_handle *hdl,
    void *buf,
    size_t n)
{
    if(n == 0) {
        return 0;
    }

    if(hdl->node->driver->read == NULL) {
        return -1;
    }
    return hdl->node->driver->read(hdl, buf, n);
}

int KeWriteWithFdscbFsNode(
    struct fs_handle *hdl,
    struct fs_fdscb *fdscb,
    const void *buf,
    size_t n)
{
    if(n == 0) {
        return 0;
    }

    if(hdl->node->driver->write_fdscb == NULL) {
        return -1;
    }
    return hdl->node->driver->write_fdscb(hdl, fdscb, buf, n);
}

int KeReadWithFdscbFsNode(
    struct fs_handle *hdl,
    struct fs_fdscb *fdscb,
    void *buf,
    size_t n)
{
    if(n == 0) {
        return 0;
    }
    
    if(hdl->node->driver->read_fdscb == NULL) {
        return -1;
    }
    return hdl->node->driver->read_fdscb(hdl, fdscb, buf, n);
}

int KeIoControlFsNode(
    struct fs_handle *hdl,
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

int KeFlushFsNode(
    struct fs_handle *hdl)
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
        MmFree(hdl->write_buf);
        hdl->write_buf = NULL;
        hdl->read_buf_size = 0;
    }
    return r;
}

struct fs_driver *KeCreateFsDriver(
    void)
{
    struct fs_driver *driver = &g_drvtab.drivers[g_drvtab.n_driver];

    g_drvtab.drivers = MmReallocateArray(g_drvtab.drivers, g_drvtab.n_driver + 1,
        sizeof(struct fs_driver));
    if(g_drvtab.drivers == NULL) {
        return -1;
    }
    driver = &g_drvtab.drivers[g_drvtab.n_driver++];
    KeSetMemory(driver, 0, sizeof(struct fs_driver));
    return driver;
}

int KeAddFsNodeToDriver(
    struct fs_driver *driver,
    struct fs_node *node)
{
    driver->nodes = MmReallocateArray(driver->nodes, driver->n_nodes + 1,
        sizeof(struct fs_node *));
    if(driver->nodes == NULL) {
        return -1;
    }
    driver->nodes[driver->n_nodes] = node;
    driver->n_nodes++;

    node->driver = driver;
    return 0;
}

#if defined(DEBUG)
static void KeDumpFsNode(
    struct fs_node *node,
    int level)
{
    size_t i;

    if(node == NULL) {
        return;
    }

    for(i = 0; i < (size_t)level * 4; i++) {
        kputc('-');
    }
    KeDebugPrint("%s\r\n", node->name);

    for(i = 0; i < node->n_children; i++) {
        struct fs_node *child = node->children[i];
        KeDumpFsNode(child, level + 1);
    }
    return;
}

void KeDumpFs(
    void)
{
    KeDumpFsNode(&root_node, 0);
}
#endif
