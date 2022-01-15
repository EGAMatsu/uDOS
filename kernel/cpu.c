#include <asm.h>
#include <cpu.h>
#include <memory.h>
#include <printf.h>

/* We are going to read in pairs of 1MiB and when we hit the memory limit we
 * will instantly catch the program exception and stop counting, then it's just
 * a matter of returning what we could count :) */
size_t HwGetMemorySize(
    void)
{
    const uint8_t *probe = (const uint8_t *)0x0 + PSA_SIZE;
    while(1) {
        int r;

        /* Do a "probe" read */
        r = HwCheckAddress(probe);
        if(r != 0) {
            KeDebugPrint("Done! %p\r\n", (unsigned int)probe);
            break;
        }

        KeDebugPrint("Memory %p\r\n", (unsigned int)probe);

        /* Go to next MiB */
        probe += 1048576;
    }
    return (size_t)probe;
}

/* Wait for an I/O response (overrides the I/O PSW) */
void HwWaitIO(
    void)
{
    return;
}
