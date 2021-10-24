#include <x86/context.h>
#include <stddef.h>

arch_context_t *HwGetScratchContextFrame(
    void)
{
    return NULL;
}

void HwSwitchThreadContext(
    struct SchedulerThread *old_thread,
    struct SchedulerThread *new_thread)
{
    return;
}