.globl s390_service_call_handler_stub
s390_service_call_handler_stub:
    brasl %r14, s390_service_call_handler
    br %r14

.globl s390_program_check_handler_stub
s390_program_check_handler_stub:
    brasl %r14, s390_program_check_handler
    br %r14