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

extern void *heap_start;

int kinit(
    void)
{
    /* ********************************************************************** */
    /* INTERRUPTION HANDLERS                                                  */
    /* ********************************************************************** */
#if (MACHINE >= M_ZARCH)
    memcpy((void *)S390_FLCESNPSW, &svc_psw, sizeof(struct s390x_psw));
    memcpy((void *)S390_FLCEPNPSW, &pc_psw, sizeof(struct s390x_psw));
    memcpy((void *)S390_FLCEENPSW, &ext_psw, sizeof(struct s390x_psw));
#else
    memcpy((void *)S390_FLCSNPSW, &svc_psw, sizeof(struct s390_psw));
    memcpy((void *)S390_FLCPNPSW, &pc_psw, sizeof(struct s390_psw));
    memcpy((void *)S390_FLCENPSW, &ext_psw, sizeof(struct s390_psw));
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