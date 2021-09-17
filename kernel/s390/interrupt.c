#include <s390/interrupt.h>
#include <panic.h>
#include <printf.h>

void s390_service_call_handler(void) {
    kprintf("HELLO SVC WORLD!\n");
    while (1)
        ;
    return;
}

void s390_program_check_handler(void) {
    kprintf("HELLO PCE WORLD!\n");
    while (1)
        ;
    return;
}