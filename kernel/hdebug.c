/* hdebug.c
 *
 * Hercules debug facility driver, implements a basic diagnostic output function
 * that does not work on real 3X0 machines, but it works on the emulator so it's
 * used for debugging purpouses
 */

#include <hdebug.h>
#include <printf.h>
#include <memory.h>
#include <mm.h>

int ModWriteHercDebug(struct fs_handle *hdl, const void *buf, size_t n)
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
    {
        size_t j = 0;
        while(j != i) {
            if(tmpbuf[j] == '\r' || tmpbuf[j] == '\n') {
                tmpbuf[j] = ' ';
            }

            j++;
        }
    }

    __DIAG8(&tmpbuf[0], i);
    return 0;
}

int ModInitHercDebug(void)
{
    struct fs_driver *driver;
    struct fs_node *node;

    driver = KeCreateFsDriver();
    driver->write = &ModWriteHercDebug;

    node = KeCreateFsNode("A:\\MODULES", "HDEBUG");
    node->driver = driver;
    return 0;
}
