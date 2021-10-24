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

int ModWriteHercDebug(
    struct FsHandle *hdl,
    const void *buf,
    size_t n)
{
    const char *cmd = "MSG * ";
    char tmpbuf[50 + 6];
    size_t i;

    /* Truncate to 50-characters */
    if(n >= 50) {
        n = 50;
    }

    KeCopyMemory(&tmpbuf[0], cmd, 6);
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
    while(tmpbuf[i] == '\r' || tmpbuf[i] == '\n') {
        i--;
    }
    i++;

    __asm__ __volatile__(
        "diag %0, %1, 8"
        :
        : "r"(&tmpbuf[0]), "r"(i)
        : "cc", "memory");
    return 0;
}

int ModInitHercDebug(
    void)
{
    struct FsDriver *driver;
    struct FsNode *node;

    driver = KeCreateFsDriver();
    driver->write = &ModWriteHercDebug;

    node = KeCreateFsNode("A:\\MODULES", "HDEBUG");
    node->driver = driver;
    return 0;
}