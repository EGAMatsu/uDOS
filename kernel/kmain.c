#include <mm.h>
#include <irq.h>
#include <panic.h>
#include <pmm.h>
#include <registry.h>
#include <user.h>
#include <fs.h>
#include <mutex.h>
#include <css.h>
#include <cpu.h>
#include <hdebug.h>
#include <dev.h>
#include <zdsfs.h>
#include <mmu.h>
#include <memory.h>
#include <crypto.h>
#include <exec.h>
#include <scheduler.h>
#include <assert.h>

void kern_A(void)
{
    while(1) {
        KeDebugPrint("Hello A!\r\n");
        HwDoSVC(50, 0, 0, 0);
    }
}

void kern_B(void)
{
    while(1) {
        KeDebugPrint("Hello B!\r\n");
        HwDoSVC(50, 0, 0, 0);
    }
}

const PSW_DECL(svc_psw, &KeAsmSupervisorCallHandler, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);
const PSW_DECL(pc_psw, &KeAsmProgramCheckHandler, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);
const PSW_DECL(ext_psw, &KeAsmExternalHandler, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);
const PSW_DECL(mc_psw, &KeAsmMachineCheckHandler, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);
const PSW_DECL(io_psw, &KeAsmIOHandler, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);

/* TODO: On z/Arch the IPL psw can't be 128-bits, we need to change the PSW_DECL macro!!! */
extern void _smptrmp(void);
const PSW_DECL(mp_psw, &_smptrmp, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);

extern void *heap_start;
int KeInit(void)
{
    uint8_t *facl = (uint8_t *)PSA_FLCFACL(0);
    struct fs_node *node;
    user_t uid;
    struct scheduler_job *job;
    struct scheduler_task *task;
    struct scheduler_thread *thread;
    cpu_context* cr_ctx = (cpu_context *)PSA_FLCCRSAV;
    struct css_schid ipl_schid, schid;
    size_t i;

    /* Register the interrupt handler PSWs so they are used when something happens and we
     * can handle that accordingly */
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
#if defined(DEBUG)
    KeDebugPrint("SVC Handler => %p, %p\r\n", &KeAsmSupervisorCallHandler, &KeSupervisorCallHandler);
    KeDebugPrint("PC Handler => %p, %p\r\n", &KeAsmProgramCheckHandler, &KeProgramCheckHandler);
    KeDebugPrint("EXT Handler => %p, %p\r\n", &KeAsmExternalHandler, &KeExternalHandler);
    KeDebugPrint("MC Handler => %p, %p\r\n", &KeAsmMachineCheckHandler, &KeMachineCheckHandler);
    KeDebugPrint("IO Handler => %p, %p\r\n", &KeAsmIOHandler, &KeIOHandler);
#endif

    /* Start the early memory manager - with only one memory partition enough to fit early boot */
    KeDebugPrint("Initializing the physical memory manager\r\n");
    MmCreateRegion((void *)0xF0000, 0xFFFF + 0xFFFF);

    /* Copy the MP trampoline PSW into the IPL psw so when we start the other CPUs
     * they will go into our trampoline */
    KeCopyMemory((void *)0, &mp_psw, sizeof(mp_psw)); /* Restart PSW */
    KeCopyMemory((void *)8, &mp_psw, sizeof(mp_psw)); /* IPL PSW */
    KePrint("Waking up the other CPUs\r\n");
    /*for(i = 0; i < 32; i++) {
        int r;
        r = HwSignalCPU(i, S390_SIGP_START);
        if(!r) {
            KePrint("Started CPU#%zu\r\n", i);
        } else {
            KePrint("CC=%i\r\n", r);
        }
    }*/

    /* Multitasking engine */
    KeDebugPrint("Initializing the scheduler\r\n");

    job = KeCreateJob("KERNEL", 1, 32757);
    task = KeCreateTask(job, "PRIMARY");

    thread = KeCreateThread(job, task, 1024);
    thread->pc = (unsigned int)&kern_B;
    thread->context.r15 = (unsigned int)&kern_B;
    thread->context.psw.address = thread->pc;
    thread->context.psw.flags = PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT;
    KeCopyMemory((void *)PSA_FLCSOPSW, &thread->context.psw, sizeof(PSW_DEFAULT_TYPE));
    KeCopyMemory(HwGetScratchContextFrame(), &thread->context, sizeof(thread->context));

    thread = KeCreateThread(job, task, 1024);
    thread->pc = (unsigned int)&kern_A;
    thread->context.r15 = (unsigned int)&kern_A;
    thread->context.psw.address = thread->pc;
    thread->context.psw.flags = PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT;

    /*KeSchedule();*/
    HwDoSVC(50, 0, 0, 0);

    /* Read FLCCAW schid */
    /*ipl_schid.num = ((struct css_schid *)PSA_FLCCAW)->num;
    ipl_schid.id = ((struct css_schid *)PSA_FLCCAW)->id;*/

    /* Print statments from this point onwards should go to a console or a device */
    KeDebugPrint("Hello world!\r\n");

    /* Recopile some information about the system */
    KeDebugPrint("CPU#%zu\r\n", (size_t)HwCPUID());
    /* KeDebugPrint("Memory: %zu\r\n", (size_t)HwGetMemorySize()); */
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

    /* Basic user and group authorization */
    KeDebugPrint("Creating users and groups\r\n");
    uid = KeCreateAccount("SYSTEM01");
    KeSetCurrentAccount(uid);

    /* Virtual filesystem */
    KeDebugPrint("Initialize early virtual filesystem\r\n");
    KeInitFs();

    /* A: */
    node = KeCreateFsNode("\\", "SYSTEM");
    node = KeCreateFsNode("A:\\", "MODULES");
    node = KeCreateFsNode("A:\\", "CORES");
    node = KeCreateFsNode("A:\\", "COMM");

    node = KeCreateFsNode("A:\\", "STREAMS");
    node = KeCreateFsNode("A:\\STREAMS", "SYSOUT");
    node = KeCreateFsNode("A:\\STREAMS", "SYSIN");
    node = KeCreateFsNode("A:\\STREAMS", "SYSAUX");

    /* B: */
    node = KeCreateFsNode("\\", "DEVICES");
    node = KeCreateFsNode("B:\\", "DISK");
    node = KeCreateFsNode("B:\\", "TERM");
    node = KeCreateFsNode("B:\\", "PUNCH");
    node = KeCreateFsNode("B:\\", "TAPE");

    /* C: */
    node = KeCreateFsNode("\\", "USER");

    /* System streams */
    node = KeCreateFsNode("A:\\STREAMS", "SYSPRN");
    node = KeCreateFsNode("A:\\STREAMS", "SYSNUL");

    /* User documents */
    node = KeCreateFsNode("C:\\", "ASM");
    node = KeCreateFsNode("C:\\", "LINK");
    node = KeCreateFsNode("C:\\", "SOURCE");
    node = KeCreateFsNode("C:\\", "INCLUDE");

    /* Initialize the registry (relational database of k=v pairs) engine */
    KeDebugPrint("Initializing the registry key manager\r\n");
    KeInitRegistry();
    KeCreateRegistryGroup(KeGetRegistryRootGroup(), "HSYSTEM");
    KeCreateRegistryGroup(KeGetRegistryRootGroup(), "HLOCAL");

    KePrint("Kernel fully initialized!\r\n");

    KePrint("         888888ba   .88888.  .d88888b  \r\n");
    KePrint("         88    `8b d8'   `8b 88.       \r\n");
    KePrint("dP    dP 88     88 88     88 `Y88888b. \r\n");
    KePrint("88    88 88     88 88     88       `8b \r\n");
    KePrint("88.  .88 88    .8P Y8.   .8P d8'   .8P \r\n");
    KePrint("`88888P' 8888888P   `8888P'   Y88888P  \r\n");
#if (MACHINE == 390u)
    KePrint("On ESA 390!\r\n");
#elif (MACHINE == 370u)
    KePrint("On ESA 370!\r\n");
#elif (MACHINE == 360u)
    KePrint("On ESA 360!\r\n");
#elif (MACHINE == 380u)
    KePrint("On Hercules/380!\r\n");
#elif (MACHINE > 390u)
    KePrint("On z/Arch!\r\n");
#else
    KePrint("On a mainframe!\r\n");
#endif

    KePrint("OS is ready - connect your user terminals now!\r\n");
    KePrint("Welcome user %s!\r\n", KeGetAccountById(KeGetCurrentAccount())->name);

    while(1);
    return 0;
}
