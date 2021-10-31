#include <s390/psa.h>

#if (MACHINE >= M_ZARCH)
#error "Unknown handling"
#else
.globl s390_supervisor_call_handler_stub
s390_supervisor_call_handler_stub:
    STM %r0, %r15, PSA_FLCGRSAV
    LM %r0, %r15, PSA_FLCCRSAV

    LARL %r15, int_stack_bottom
    BRASL %r14, s390_supervisor_call_handler

    LM %r0, %r15, PSA_FLCGRSAV
    LPSW PSA_FLCSOPSW

.globl s390_program_check_handler_stub
s390_program_check_handler_stub:
    STM %r0, %r15, PSA_FLCGRSAV
    LM %r0, %r15, PSA_FLCCRSAV

    LARL %r15, int_stack_bottom
    BRASL %r14, s390_program_check_handler
    
    LM %r0, %r15, PSA_FLCGRSAV
    LPSW PSA_FLCPOPSW

.globl s390_external_handler_stub
s390_external_handler_stub:
    STM %r0, %r15, PSA_FLCGRSAV
    LM %r0, %r15, PSA_FLCCRSAV

    LARL %r15, int_stack_bottom
    BRASL %r14, s390_external_handler

    LM %r0, %r15, PSA_FLCGRSAV
    LPSW PSA_FLCEOPSW
#endif

.section .bss
    .skip 4096
int_stack_bottom: