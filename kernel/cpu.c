#include <Asm.h>
#include <Cpu.h>
#include <Psa.h>
#include <Memory.h>
#include <Debug\Printf.h>

/* Check if an address is valid - this only catches program exceptions to
 * determine if it's valid or not */
int HwCheckAddress(
    volatile const void *probe)
{
#if 0
    const PSW_DECL(pc_psw, &&invalid, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI);
    PSW_DEFAULT_TYPE saved_psw;
    int r = 0;

#if (MACHINE > 390u)
    KeCopyMemory(&saved_psw, (void *)PSA_FLCEPNPSW, sizeof(PSW_DEFAULT_TYPE));
    KeCopyMemory((void *)PSA_FLCEPNPSW, &pc_psw, sizeof(PSW_DEFAULT_TYPE));
#else
    KeCopyMemory(&saved_psw, (void *)PSA_FLCPNPSW, sizeof(PSW_DEFAULT_TYPE));
    KeCopyMemory((void *)PSA_FLCPNPSW, &pc_psw, sizeof(PSW_DEFAULT_TYPE));
#endif

    *((volatile const uint8_t *)probe);
    goto end;
invalid:
    r = -1;
end:
#if (MACHINE > 390u)
    KeCopyMemory((void *)PSA_FLCEPNPSW, &saved_psw, sizeof(PSW_DEFAULT_TYPE));
#else
    KeCopyMemory((void *)PSA_FLCPNPSW, &saved_psw, sizeof(PSW_DEFAULT_TYPE));
#endif
    return r;
#endif
    return 0;
}

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
    /* The next I/O PSW to be set when the operation finishes */
    while(1) {
        size_t i, j;
        KeDebugPrint("Waiting for IO...\r\n");
        for(i = 0; i < 65535; i++) {
			for(j = 0; j < 65535; j++) {
				// Wait...
			}
		}
    }
    return;
}
