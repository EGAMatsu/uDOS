#include <Context.h>
#include <Asm.h>
#include <cpu.h>
#include <Memory.h>

/* In S390 we store the current context frame on the general register
 * save area present on the PSA */
cpu_context *HwGetScratchContextFrame(
    void)
{
    return (cpu_context *)PSA_FLCGRSAV;
}

void HwSwitchThreadContext(
    struct scheduler_thread *old_thread,
    struct scheduler_thread *new_thread)
{
    /* Set the new reload address */
#if (MACHINE > 390u)
    KeCopyMemory(&old_thread->context.psw, (void *)PSA_FLCESOPSW,
        sizeof(struct s390x_psw));
    KeCopyMemory((void *)PSA_FLCESOPSW, &new_thread->context.psw,
        sizeof(struct s390x_psw));
#else
    KeCopyMemory(&old_thread->context.psw, (void *)PSA_FLCSOPSW,
        sizeof(struct s390_psw));
    KeCopyMemory((void *)PSA_FLCSOPSW, &new_thread->context.psw,
        sizeof(struct s390_psw));
#endif

    /* Save context to current thread (obtained from the scratch frame) */
    KeCopyMemory(&old_thread->context, HwGetScratchContextFrame(),
        sizeof(cpu_context));
    
    /* Load new thread context into the scratch frame */
    KeCopyMemory(HwGetScratchContextFrame(), &new_thread->context,
        sizeof(cpu_context));
    return;
}
