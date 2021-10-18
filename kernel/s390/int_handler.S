#include <s390/asm.h>

#if (MACHINE >= M_ZARCH)
#error "Unknown handling"
#else
.globl s390_supervisor_call_handler_stub
s390_supervisor_call_handler_stub:
    stm %r0, %r15, S390_FLCGRSAV
    lm %r0, %r15, S390_FLCCRSAV

    larl %r15, int_stack_bottom
    brasl %r14, s390_supervisor_call_handler

    lm %r0, %r15, S390_FLCGRSAV
    lpsw S390_FLCSOPSW

.globl s390_program_check_handler_stub
s390_program_check_handler_stub:
    stm %r0, %r15, S390_FLCGRSAV
    lm %r0, %r15, S390_FLCCRSAV

    larl %r15, int_stack_bottom
    brasl %r14, s390_program_check_handler
    
    lm %r0, %r15, S390_FLCGRSAV
    lpsw S390_FLCPOPSW

.globl s390_external_handler_stub
s390_external_handler_stub:
    stm %r0, %r15, S390_FLCGRSAV
    lm %r0, %r15, S390_FLCCRSAV

    larl %r15, int_stack_bottom
    brasl %r14, s390_external_handler

    lm %r0, %r15, S390_FLCGRSAV
    lpsw S390_FLCEOPSW
#endif

.section .bss
int_stack_bottom:
    .skip 4096
