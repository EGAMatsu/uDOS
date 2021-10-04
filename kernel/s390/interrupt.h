#ifndef S390_INTERRUPT_H
#define S390_INTERRUPT_H

#include <stdint.h>

uintptr_t s390_do_svc(int code);

void s390_supervisor_call_handler(void);
void s390_program_check_handler(void);
void s390_external_handler(void);

/* Assembly stubs */
extern void s390_supervisor_call_handler_stub(void);
extern void s390_program_check_handler_stub(void);
extern void s390_external_handler_stub(void);

#endif