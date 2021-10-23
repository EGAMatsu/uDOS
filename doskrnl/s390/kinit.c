#include <memory.h>
#include <mm/pmm.h>

#include <s390/interrupt.h>
#include <s390/asm.h>
#include <s390/cpu.h>

const S390_PSW_DECL(svc_psw, &s390_supervisor_call_handler_stub,
    S390_PSW_ENABLE_ARCHMODE
    | S390_PSW_ENABLE_MCI
    | S390_PSW_IO_INT
    | S390_PSW_EXTERNAL_INT);

const S390_PSW_DECL(pc_psw, &s390_program_check_handler_stub,
    S390_PSW_ENABLE_ARCHMODE
    | S390_PSW_ENABLE_MCI
    | S390_PSW_IO_INT
    | S390_PSW_EXTERNAL_INT);

const S390_PSW_DECL(ext_psw, &s390_external_handler_stub,
    S390_PSW_ENABLE_ARCHMODE
    | S390_PSW_ENABLE_MCI
    | S390_PSW_IO_INT
    | S390_PSW_EXTERNAL_INT);

/* First make our current context allow the execution of interrupts */
static void s390_enable_all_int(
    void)
{
    const S390_PSW_DECL(all_int_psw, &&after_enable,
        S390_PSW_ENABLE_ARCHMODE
        | S390_PSW_ENABLE_MCI
        | S390_PSW_EXTERNAL_INT
        | S390_PSW_IO_INT);
#if (MACHINE >= M_ZARCH)
    uint64_t cr0 = S390_CR0_TIMER_MASK_CTRL;
#endif

#if (MACHINE >= M_ZARCH)
    /* Then we will set the control register accordingly to allow timers */
    __asm__ __volatile__(
        "lctl 0, 0, %0"
        :
        : "m"(cr0)
    );
#endif
    
    /* Enable all interrupts because we can handle them ;) */
    __asm__ goto(
        "lpsw %0\r\n"
        :
        : "m"(all_int_psw)
        :
        : after_enable);
    __builtin_unreachable();
after_enable:
    return;
}

static void s390_enable_dat(
    void)
{
    const S390_PSW_DECL(new_psw, &&after_enable,
        S390_PSW_ENABLE_ARCHMODE
        | S390_PSW_ENABLE_MCI
        | S390_PSW_EXTERNAL_INT
        | S390_PSW_IO_INT);
    
    __asm__ goto(
        "lpsw %0\r\n"
        :
        : "m"(new_psw)
        :
        : after_enable);
    __builtin_unreachable();
after_enable:
    return;
}

extern void *heap_start;

int kinit(
    void)
{
    /* ********************************************************************** */
    /* INTERRUPTION HANDLERS                                                  */
    /* ********************************************************************** */
#if (MACHINE >= M_ZARCH)
    KeCopyMemory((void *)PSA_FLCESNPSW, &svc_psw, sizeof(svc_psw));
    KeCopyMemory((void *)PSA_FLCEPNPSW, &pc_psw, sizeof(pc_psw));
    KeCopyMemory((void *)PSA_FLCEENPSW, &ext_psw, sizeof(ext_psw));
#else
    KeCopyMemory((void *)PSA_FLCSNPSW, &svc_psw, sizeof(svc_psw));
    KeCopyMemory((void *)PSA_FLCPNPSW, &pc_psw, sizeof(pc_psw));
    KeCopyMemory((void *)PSA_FLCENPSW, &ext_psw, sizeof(ext_psw));
#endif

    //s390_enable_all_int();

    /*kprintf("CPU#%zu\r\n", (size_t)HwS390Cpuid());*/

    /* ********************************************************************** */
    /* PHYSICAL MEMORY MANAGER                                                */
    /* ********************************************************************** */
    /*kprintf("Initializing the physical memory manager\r\n");*/
    MmCreateRegion(&heap_start, 0xFFFF * 16);

    //HwTurnOnMmu(NULL);
    //s390_enable_dat();

    kmain();
    return 0;
}