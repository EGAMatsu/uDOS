#ifndef INTERRUPT_H
#define INTERRUPT_H

void s390_service_call_handler(void);
void s390_program_check_handler(void);

/* Assembly stubs */
extern void s390_service_call_handler_stub(void);
extern void s390_program_check_handler_stub(void);

#endif