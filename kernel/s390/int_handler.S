.globl s390_supervisor_call_handler_stub
s390_supervisor_call_handler_stub:
    j s390_supervisor_call_handler_stub
    stm %r0, %r15, 0x180
    lm %r0, %r15, 0x1C0

    larl %r15, int_stack_bottom
    brasl %r14, s390_supervisor_call_handler

    lm %r0, %r15, 0x180
    lpsw 0x140

.globl s390_program_check_handler_stub
s390_program_check_handler_stub:
    j s390_program_check_handler_stub
    stm %r0, %r15, 0x180
    lm %r0, %r15, 0x1C0

    larl %r15, int_stack_bottom
    brasl %r14, s390_program_check_handler
    
    lm %r0, %r15, 0x180
    lpsw 0x150

.globl s390_external_handler_stub
s390_external_handler_stub:
    j s390_external_handler_stub
    stm %r0, %r15, 0x180
    lm %r0, %r15, 0x1C0

    larl %r15, int_stack_bottom
    brasl %r14, s390_external_handler

    lm %r0, %r15, 0x180
    lpsw 0x130

.section .bss
int_stack_bottom:
    .skip 4096