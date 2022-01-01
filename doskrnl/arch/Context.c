#include <S390/Context.h>
#include <S390/Asm.h>
#include <S390/Psa.h>
#include <Memory.h>

/* In S390 we store the current context frame on the general register
 * save area present on the PSA */
arch_context_t *HwGetScratchContextFrame(
    void)
{
    return (arch_context_t *)PSA_FLCGRSAV;
}

void HwSwitchThreadContext(
    struct SchedulerThread *old_thread,
    struct SchedulerThread *new_thread)
{
    /* Set the new reload address */
#if (MACHINE >= M_ZARCH)
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
        sizeof(arch_context_t));
    
    /* Load new thread context into the scratch frame */
    KeCopyMemory(HwGetScratchContextFrame(), &new_thread->context,
        sizeof(arch_context_t));
    return;
}
