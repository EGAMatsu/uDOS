#include <stdint.h>
#include <memory.h>
#include <pmm.h>
#include <panic.h>
#include <printf.h>
#include <interrupt.h>
#include <asm.h>
#include <cpu.h>

const PSW_DECL(svc_psw, &KeAsmSupervisorCallHandler, PSW_DEFAULT_ARCHMODE
    | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);

const PSW_DECL(pc_psw, &KeAsmProgramCheckHandler, PSW_DEFAULT_ARCHMODE
    | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);

const PSW_DECL(ext_psw, &KeAsmExternalHandler, PSW_DEFAULT_ARCHMODE
    | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);

const PSW_DECL(mc_psw, &KeAsmMachineCheckHandler, PSW_DEFAULT_ARCHMODE
    | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);

const PSW_DECL(io_psw, &KeAsmIOHandler, PSW_DEFAULT_ARCHMODE
    | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT | PSW_WAIT_STATE);

/* First make our current context allow the execution of interrupts */
static void s390_enable_all_int(
    void)
{
    /*
    const PSW_DECL(all_int_psw, &&after_enable, PSW_DEFAULT_ARCHMODE
        | PSW_ENABLE_MCI | PSW_EXTERNAL_INT | PSW_IO_INT);
    
    uint64_t cr0 = S390_CR0_TIMER_MASK | 0xFF000000;
    */

    /* Then we will set the control register accordingly to allow timers */
    /*
    __asm__ __volatile__(
        "LCTL 0, 0, %0"
        :
        : "m"(cr0)
    );
    */
    
    /* Enable all interrupts because we can handle them ;) */
    /*
    __asm__ goto(
        "LPSW %0\r\n"
        :
        : "m"(all_int_psw)
        :
        : after_enable);
    */

    /*__builtin_unreachable();*/
after_enable:
    return;
}

static void s390_enable_dat(
    void)
{
    /*
    const PSW_DECL(new_psw, &&after_enable, PSW_DEFAULT_ARCHMODE
        | PSW_ENABLE_MCI | PSW_EXTERNAL_INT | PSW_IO_INT | PSW_DAT);
    */
    
    /*
    __asm__ goto(
        "LPSW %0\r\n"
        :
        : "m"(new_psw)
        :
        : after_enable);
    */
    /*__builtin_unreachable();*/
after_enable:
    return;
}

extern void *heap_start;

int HwGenerateFacilitySummary(
    void)
{
    uint8_t *facl = (uint8_t *)PSA_FLCFACL(0);
    size_t i;

    KeDebugPrint("*******************************************************\r\n");
    KeDebugPrint("Server machine facility summary\r\n");
    KeDebugPrint("*******************************************************\r\n");

    /* Sense-id */
    for(i = 0; i < 6; i++) {
        facl[i] = 0;
    }
    
    KeDebugPrint("N3 Facility: %s\r\n", (facl[0] & PSA_FLCFACL0_N3) ? "yes" : "no");
    KeDebugPrint("z/Arch Install: %s\r\n", (facl[0] & PSA_FLCFACL0_ZARCH_INSTALL) ? "yes" : "no");
    KeDebugPrint("z/Arch Active: %s\r\n", (facl[0] & PSA_FLCFACL0_ZARCH_ACTIVE) ? "yes" : "no");
#if (MACHINE > 390u)
    KeDebugPrint("IDTE Facility: %s\r\n", (facl[0] & PSA_FLCFACL0_IDTE) ? "yes" : "no");
    KeDebugPrint("IDTE Clear Segment: %s\r\n", (facl[0] & PSA_FLCFACL0_IDTE_CLEAR_SEGMENT) ? "yes" : "no");
    KeDebugPrint("IDTE Clear Region: %s\r\n", (facl[0] & PSA_FLCFACL0_IDTE_CLEAR_REGION) ? "yes" : "no");
#endif
    KeDebugPrint("ASN and LX Reuse Facility: %s\r\n", (facl[0] & PSA_FLCFACL0_ASN_LX_REUSE) ? "yes" : "no");
    KeDebugPrint("STFLE Facility: %s\r\n", (facl[0] & PSA_FLCFACL0_STFLE) ? "yes" : "no");
    
    KeDebugPrint("DAT Facility: %s\r\n", (facl[1] & PSA_FLCFACL1_DAT) ? "yes" : "no");
    KeDebugPrint("Sense Running Status: %s\r\n", (facl[1] & PSA_FLCFACL1_SRS) ? "yes" : "no");
    KeDebugPrint("SSKE Instruction Installed: %s\r\n", (facl[1] & PSA_FLCFACL1_SSKE) ? "yes" : "no");
    KeDebugPrint("STSI Enhancement: %s\r\n", (facl[1] & PSA_FLCFACL1_CTOP) ? "yes" : "no");
#if (MACHINE > 390u)
    KeDebugPrint("110524 Facility: %s\r\n", (facl[1] & PSA_FLCFACL1_QCIF) ? "yes" : "no");
    KeDebugPrint("IPTE Facility: %s\r\n", (facl[1] & PSA_FLCFACL1_IPTE) ? "yes" : "no");
    KeDebugPrint("NQ-Key Setting Facility: %s\r\n", (facl[1] & PSA_FLCFACL1_NQKEY) ? "yes" : "no");
    KeDebugPrint("APFT Facility: %s\r\n", (facl[1] & PSA_FLCFACL1_APFT) ? "yes" : "no");
#endif

    KeDebugPrint("Extended Translation 2 Facility: %s\r\n", (facl[2] & PSA_FLCFACL2_ETF2) ? "yes" : "no");
    KeDebugPrint("Cryptographic Assist Facility: %s\r\n", (facl[2] & PSA_FLCFACL2_CRYA) ? "yes" : "no");
    KeDebugPrint("Long Displacement Facility: %s\r\n", (facl[2] & PSA_FLCFACL2_LONGDISP) ? "yes" : "no");
    KeDebugPrint("Long Displacement (High Performance) Facility: %s\r\n", (facl[2] & PSA_FLCFACL2_LONGDISPHP) ? "yes" : "no");
    KeDebugPrint("Hardware FP Multiply-Subtraction: %s\r\n", (facl[2] & PSA_FLCFACL2_HFP_MULSUB) ? "yes" : "no");
    KeDebugPrint("Extended Immediate Facility: %s\r\n", (facl[2] & PSA_FLCFACL2_EIMM) ? "yes" : "no");
    KeDebugPrint("Extended Translation 3 Facility: %s\r\n", (facl[2] & PSA_FLCFACL2_ETF3) ? "yes" : "no");
    KeDebugPrint("Hardware FP Unnormalized Extension: %s\r\n", (facl[2] & PSA_FLCFACL2_HFP_UN) ? "yes" : "no");
    
    KeDebugPrint("FLCFACL[0-6]: %x, %x, %x, %x, %x, %x\r\n", facl[0], facl[1], facl[2], facl[3], facl[4], facl[5], facl[6]);

    KeDebugPrint("*******************************************************\r\n");
}

uint8_t int_stack[512] = {0};

extern void KeMain(void);
int KeInit(
    void)
{
    cpu_context* cr_ctx = (cpu_context *)PSA_FLCCRSAV;
    /* ********************************************************************** */
    /* INTERRUPTION HANDLERS                                                  */
    /* ********************************************************************** */

    /* Initialize CR registers */
    /*
         L 13,=A(@@STACK)
         LA 5,180(13)
         ST 5,76(13)
    */
    KeSetMemory(cr_ctx, 0, sizeof(cr_ctx));
    cr_ctx->r13 = (unsigned int)&int_stack;
    *((uint32_t *)(&int_stack[76])) = *((uint32_t *)(&int_stack[180]));

    KeDebugPrint("Setting interrupts\r\n");
#if (MACHINE > 390u)
    KeCopyMemory((void *)PSA_FLCESNPSW, &svc_psw, sizeof(svc_psw));
    KeCopyMemory((void *)PSA_FLCEPNPSW, &pc_psw, sizeof(pc_psw));
    KeCopyMemory((void *)PSA_FLCEENPSW, &ext_psw, sizeof(ext_psw));
    KeCopyMemory((void *)PSA_FLCEMNPSW, &mc_psw, sizeof(mc_psw));
    KeCopyMemory((void *)PSA_FLCEINPSW, &io_psw, sizeof(io_psw));
#else
    KeCopyMemory((void *)PSA_FLCSNPSW, &svc_psw, sizeof(svc_psw));
    KeCopyMemory((void *)PSA_FLCPNPSW, &pc_psw, sizeof(pc_psw));
    KeCopyMemory((void *)PSA_FLCENPSW, &ext_psw, sizeof(ext_psw));
    KeCopyMemory((void *)PSA_FLCMNPSW, &mc_psw, sizeof(mc_psw));
    KeCopyMemory((void *)PSA_FLCINPSW, &io_psw, sizeof(io_psw));
#endif
    KeDebugPrint("SVC Handler => %p, %p\r\n", &KeAsmSupervisorCallHandler, &KeSupervisorCallHandler);
    KeDebugPrint("PC Handler => %p, %p\r\n", &KeAsmProgramCheckHandler, &KeProgramCheckHandler);
    KeDebugPrint("EXT Handler => %p, %p\r\n", &KeAsmExternalHandler, &KeExternalHandler);
    KeDebugPrint("MC Handler => %p, %p\r\n", &KeAsmMachineCheckHandler, &KeMachineCheckHandler);
    KeDebugPrint("IO Handler => %p, %p\r\n", &KeAsmIOHandler, &KeIOHandler);
    
    /*s390_enable_all_int();*/
    KeDebugPrint("CPU#%zu\r\n", (size_t)HwCPUID());

    /* ********************************************************************** */
    /* PHYSICAL MEMORY MANAGER                                                */
    /* ********************************************************************** */
    /*KeDebugPrint("Initializing the physical memory manager\r\n");*/
    MmCreateRegion((void *)0xF00000, 0xFFFF * 16);

    /*HwTurnOnMmu(NULL);*/
    /*s390_enable_all_int();*/
    /*s390_enable_dat();*/

    HwGenerateFacilitySummary();

    KeMain();
    return 0;
}
