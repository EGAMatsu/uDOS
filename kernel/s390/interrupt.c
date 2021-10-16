#include <panic.h>
#include <printf.h>
#include <s390/interrupt.h>
#include <s390/context.h>

uintptr_t s390_do_svc(
    int code)
{
    __asm__ __volatile__(
        "svc %0"
        :
        : "d"(code)
        :);
}

#include <s390/asm.h>
#include <user.h>
void s390_supervisor_call_handler(
    void)
{
    int code = (int)(*(volatile uint16_t *)S390_FLCESICODE);
    struct s390_context *frame = (struct s390_context *)S390_FLCGRSAV;

    kprintf("SVC call (id %i)\r\n", code);

    /*switch(code) {
    case 1:
        scheduler_schedule();
        break;
    }*/
    return;
}

void s390_program_check_handler(
    void)
{
    const struct s390_context *frame = (const struct s390_context *)0x180;

    kprintf("Program exception!\r\n");

    kprintf("R0: %p R1: %p R2: %p R3: %p R4: %p\r\n", frame->r0, frame->r1,
        frame->r2, frame->r3, frame->r4);
    kprintf("R5: %p R6: %p R7: %p R8: %p R9: %p\r\n", frame->r5, frame->r6,
        frame->r7, frame->r8, frame->r9);
    kprintf("Static chain/R10: %p\r\n", frame->r10);
    kprintf("Frame Pointer/R11: %p\r\n", frame->r11);
    kprintf("GOT Pointer/R12: %p\r\n", frame->r12);
    kprintf("Base Pointer/R13: %p\r\n", frame->r13);
    kprintf("Return Address/R14: %p\r\n", frame->r14);
    kprintf("Stack Pointer/R15: %p\r\n", frame->r15);
    while(1);
}

void s390_external_handler(void)
{
    kprintf("Timer event fired up\r\n");
    while(1);
    return;
}