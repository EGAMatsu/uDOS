#include <s390/context.h>
#include <s390/asm.h>

/* In S390 we store the current context frame on the general register
 * save area present on the PSA */
arch_context_t *context_scratch_frame(
    void)
{
    return (arch_context_t *)S390_FLCGRSAV;
}