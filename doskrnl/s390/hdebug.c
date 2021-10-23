/* hdebug.c
 *
 * Hercules debug facility driver, implements a basic diagnostic output function
 * that does not work on real 3X0 machines, but it works on the emulator so it's
 * used for debugging purpouses
 */

#include <debug/printf.h>
#include <memory.h>
#include <mm/mm.h>
#include <fs/fs.h>

int hdebug_write(
    struct FsHandle *hdl,
    const void *buf,
    size_t n)
{
    char tmpbuf[255 + 6];
    size_t i;

    KeCopyMemory(&tmpbuf[0], "MSG * ", 6);
    KeCopyMemory(&tmpbuf[6], buf, n);

    /* Make it all uppercase for the aesthetics */
    for(i = 6; i < 6 + n - 1; i++) {
        if(tmpbuf[i] >= 'a' && tmpbuf[i] <= 'i') {
            tmpbuf[i] = 'A' + (tmpbuf[i] - 'a');
        } else if(tmpbuf[i] >= 'j' && tmpbuf[i] <= 'r') {
            tmpbuf[i] = 'J' + (tmpbuf[i] - 'j');
        } else if(tmpbuf[i] >= 's' && tmpbuf[i] <= 'z') {
            tmpbuf[i] = 'S' + (tmpbuf[i] - 's');
        }
    }

    /* Blank out newlines (so HERCULES logs are not messed up) */
    while(tmpbuf[6 + n - 1] == '\r' || tmpbuf[6 + n - 1] == '\n') {
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
    struct FsHandle *hdl,
    void *buf,
    size_t n)
{
    
    return 0;
}

int hdebug_init(
    void)
{
    struct FsDriver *driver;
    struct FsNode *node;

    driver = KeCreateFsDriver();
    driver->write = &hdebug_write;
    driver->read = &hdebug_read;

    node = KeCreateFsNode("A:\\DEVICES", "HDEBUG");
    node->driver = driver;
    return 0;
}