#include <asm.h>
#include <cpu.h>
#include <memory.h>
#include <printf.h>

void HwSwitchThreadContext(struct scheduler_thread *old_thread, struct scheduler_thread *new_thread)
{
    /* Set the new reload address */
#if (MACHINE > 390u)
    KeCopyMemory(&old_thread->context.psw, (void *)PSA_FLCESOPSW, sizeof(struct s390x_psw));
    KeCopyMemory((void *)PSA_FLCESOPSW, &new_thread->context.psw, sizeof(struct s390x_psw));
#else
    KeCopyMemory(&old_thread->context.psw, (void *)PSA_FLCSOPSW, sizeof(struct s390_psw));
    KeCopyMemory((void *)PSA_FLCSOPSW, &new_thread->context.psw, sizeof(struct s390_psw));
#endif

    /* Save context to current thread (obtained from the scratch frame) */
    KeCopyMemory(&old_thread->context, HwGetScratchContextFrame(), sizeof(cpu_context));
    
    /* Load new thread context into the scratch frame */
    KeCopyMemory(HwGetScratchContextFrame(), &new_thread->context, sizeof(cpu_context));
    return;
}

/* We are going to read in pairs of 1MiB and when we hit the memory limit we
 * will instantly catch the program exception and stop counting, then it's just
 * a matter of returning what we could count :) */
size_t HwGetMemorySize(void)
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
void HwWaitIO(void)
{
    return;
}
