#include <string.h>
#include <pmm.h>

#include <s390/interrupt.h>
#include <s390/asm.h>
#include <s390/cpu.h>

#if (MACHINE >= M_ZARCH)
struct s390x_psw svc_psw = {
    0x00040000 | S390_PSW_AM64,
    S390_PSW_DEFAULT_AMBIT,
    0,
    (uint32_t)&s390_supervisor_call_handler_stub
};

struct s390x_psw pc_psw = {
    0x00040000 | S390_PSW_AM64,
    S390_PSW_DEFAULT_AMBIT,
    0,
    (uint32_t)&s390_program_check_handler_stub
};

struct s390x_psw ext_psw = {
    0x00040000 | S390_PSW_AM64,
    S390_PSW_DEFAULT_AMBIT,
    0,
    (uint32_t)&s390_external_handler_stub
};
#else
struct s390_psw svc_psw = {
    0x000C0000,
    (uint32_t)&s390_supervisor_call_handler_stub + S390_PSW_DEFAULT_AMBIT
};

struct s390_psw pc_psw = {
    0x000C0000,
    (uint32_t)&s390_program_check_handler_stub + S390_PSW_DEFAULT_AMBIT
};

struct s390_psw ext_psw = {
    0x000C0000,
    (uint32_t)&s390_external_handler_stub + S390_PSW_DEFAULT_AMBIT
};
#endif

/* First make our current context allow the execution of interrupts */
static void s390_enable_all_int(
    void)
{
    const S390_PSW_DECL(all_int_psw, &&after_enable,
        S390_PSW_ENABLE_ARCHMODE
        | S390_PSW_ENABLE_MCI
        | S390_PSW_EXTERNAL_INT
        | S390_PSW_IO_INT);
    uint64_t cr0 = S390_CR0_TIMER_MASK_CTRL;
    
    /* Enable all interrupts because we can handle them ;) */
    __asm__ goto(
        "lpsw %0\n"
        :
        : "m"(all_int_psw)
        :
        : after_enable);
    __builtin_unreachable();
after_enable:

    /* Then we will set the control register accordingly to allow timers */
    __asm__ __volatile__(
        "lctl 0, 0, %0"
        :
        : "m"(cr0)
    );
    return;
}

extern void *heap_start;

int kinit(
    void)
{
    //s390_enable_all_int();

    /* ********************************************************************** */
    /* INTERRUPTION HANDLERS                                                  */
    /* ********************************************************************** */
#if (MACHINE >= M_ZARCH)
    memcpy((void *)S390_FLCESNPSW, &svc_psw, sizeof(svc_psw));
    memcpy((void *)S390_FLCEPNPSW, &pc_psw, sizeof(pc_psw));
    memcpy((void *)S390_FLCEENPSW, &ext_psw, sizeof(ext_psw));
#else
    memcpy((void *)S390_FLCSNPSW, &svc_psw, sizeof(svc_psw));
    memcpy((void *)S390_FLCPNPSW, &pc_psw, sizeof(pc_psw));
    memcpy((void *)S390_FLCENPSW, &ext_psw, sizeof(ext_psw));
#endif

    /*kprintf("CPU#%zu\n", (size_t)s390_cpuid());*/

    /* ********************************************************************** */
    /* PHYSICAL MEMORY MANAGER                                                */
    /* ********************************************************************** */
    /*kprintf("Initializing the physical memory manager\n");*/
    pmm_create_region(&heap_start, 0xffff);

    kmain();
    return 0;
}