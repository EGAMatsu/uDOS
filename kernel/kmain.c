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
#include <x2703.h>
#include <x3270.h>
#include <x3390.h>
#include <hdebug.h>
#include <bsc.h>
#include <zdsfs.h>
#include <mmu.h>
#include <memory.h>
#include <crypto.h>
#include <elf.h>
#include <pe.h>
#include <scheduler.h>
#include <interrupt.h>

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

int KeMain(void);

const PSW_DECL(svc_psw, &KeAsmSupervisorCallHandler, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);
const PSW_DECL(pc_psw, &KeAsmProgramCheckHandler, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);
const PSW_DECL(ext_psw, &KeAsmExternalHandler, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);
const PSW_DECL(mc_psw, &KeAsmMachineCheckHandler, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);
const PSW_DECL(io_psw, &KeAsmIOHandler, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT);

extern void *heap_start;
uint8_t int_stack[512] = {0};
int KeInit(void)
{
	uint8_t *facl = (uint8_t *)PSA_FLCFACL(0);
	struct registry_group *hsystem, *hlocal, *hsubgr;
    struct fs_node *node;
    user_t uid;
	struct scheduler_job *job;
    struct scheduler_task *task;
    struct scheduler_thread *thread;
    cpu_context* cr_ctx = (cpu_context *)PSA_FLCCRSAV;
    
    /* Some assertions... */
    DEBUG_ASSERT(sizeof(uint8_t) == 1);
    DEBUG_ASSERT(sizeof(uint16_t) == 2);
    DEBUG_ASSERT(sizeof(uint32_t) == 4);
    DEBUG_ASSERT(sizeof(uint64_t) == 8);
    
    DEBUG_ASSERT(sizeof(int8_t) == 1);
    DEBUG_ASSERT(sizeof(int16_t) == 2);
    DEBUG_ASSERT(sizeof(int32_t) == 4);
    DEBUG_ASSERT(sizeof(int64_t) == 8);
    
    /* Required because a lot of code depends upon this */
    DEBUG_ASSERT(sizeof(void*) == sizeof(unsigned int));
    
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
    KeDebugPrint("Initializing the physical memory manager\r\n");
    MmCreateRegion((void *)0xF00000, 0xFFFF * 16);

    KeDebugPrint("*******************************************************\r\n");
    KeDebugPrint("Server machine facility summary\r\n");
    KeDebugPrint("*******************************************************\r\n");
    
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
	
	/* ********************************************************************** */
    /* REGISTRY KEY AND VALUES MANAGER                                        */
    /* ********************************************************************** */
    KeDebugPrint("Initializing the registry key manager\r\n");
    KeInitRegistry();
    hsystem = KeCreateRegistryGroup(KeGetRegistryRootGroup(), "HSYSTEM");
    KeCreateRegistryGroup(hsystem, "DISK");
    KeCreateRegistryGroup(hsystem, "TAPE");
    KeCreateRegistryGroup(hsystem, "GRFX");
    KeCreateRegistryGroup(hsystem, "TERM");
    KeCreateRegistryGroup(hsystem, "PUNCH");
    hlocal = KeCreateRegistryGroup(KeGetRegistryRootGroup(), "HLOCAL");

    /* ********************************************************************** */
    /* USER AND GROUP AUTHORIZATION                                           */
    /* ********************************************************************** */
    KeDebugPrint("Creating users and groups\r\n");
    uid = KeCreateAccount("WRK001");
    uid = KeCreateAccount("WRK002");
    uid = KeCreateAccount("WRK003");
    uid = KeCreateAccount("WRK004");
    uid = KeCreateAccount("WRK005");
    uid = KeCreateAccount("WRK006");
    uid = KeCreateAccount("WRK007");
    uid = KeCreateAccount("WRK008");
    uid = KeCreateAccount("WRK009");
    uid = KeCreateAccount("WRK010");
    uid = KeCreateAccount("WRK011");
    uid = KeCreateAccount("WRK012");
    uid = KeCreateAccount("WRK013");
    uid = KeCreateAccount("WRK014");
    uid = KeCreateAccount("WRK015");
    uid = KeCreateAccount("WRK016");
    uid = KeCreateAccount("CLIENT01");
    KeSetCurrentAccount(uid);

    /* ********************************************************************** */
    /* MULTITASKING ENGINE                                                    */
    /* ********************************************************************** */
    KeDebugPrint("Initializing the scheduler\r\n");

    job = KeCreateJob("KERNEL", 1, 32757);
    task = KeCreateTask(job, "PRIMARY");

    thread = KeCreateThread(job, task, 8192);
    thread->pc = (unsigned int)&kern_A;
    thread->context.psw.address = thread->pc;
    thread->context.psw.flags = PSW_DEFAULT_ARCHMODE | PSW_IO_INT | PSW_EXTERNAL_INT | PSW_ENABLE_MCI;
    KeCopyMemory((void *)PSA_FLCSOPSW, &thread->context.psw, sizeof(struct s390_psw));
    KeCopyMemory(HwGetScratchContextFrame(), &thread->context, sizeof(thread->context));

    thread = KeCreateThread(job, task, 8192);
    thread->pc = (unsigned int)&kern_B;
    thread->context.psw.address = thread->pc;
    thread->context.psw.flags = PSW_DEFAULT_ARCHMODE | PSW_IO_INT | PSW_EXTERNAL_INT | PSW_ENABLE_MCI;
    
    thread = KeCreateThread(job, task, 8192);
    thread->pc = (unsigned int)&kern_A;
    thread->context.psw.address = thread->pc;
    thread->context.psw.flags = PSW_DEFAULT_ARCHMODE | PSW_IO_INT | PSW_EXTERNAL_INT | PSW_ENABLE_MCI;

    /* ********************************************************************** */
    /* VIRTUAL FILE SYSTEM                                                    */
    /* ********************************************************************** */
    KeDebugPrint("Initializing the VFS\r\n");
    KeInitFs();

    /* A: */
    node = KeCreateFsNode("\\", "SYSTEM");
    node = KeCreateFsNode("A:\\", "CORES");
    node = KeCreateFsNode("A:\\", "COMM");
    node = KeCreateFsNode("A:\\", "STREAMS");
    node = KeCreateFsNode("A:\\", "MODULES");

    /* B: */
    node = KeCreateFsNode("\\", "DEVICES");
    node = KeCreateFsNode("B:\\", "DISK");
    node = KeCreateFsNode("B:\\", "TERM");
    node = KeCreateFsNode("B:\\", "GRFX");
    node = KeCreateFsNode("B:\\", "PUNCH");
    node = KeCreateFsNode("B:\\", "TAPE");

    /* C: */
    node = KeCreateFsNode("\\", "DOCUMENTS");
	
    /* ********************************************************************** */
    /* SYSTEM STREAMS                                                         */
    /* ********************************************************************** */
    node = KeCreateFsNode("A:\\STREAMS", "SYSOUT");
    node = KeCreateFsNode("A:\\STREAMS", "SYSIN");
    node = KeCreateFsNode("A:\\STREAMS", "SYSAUX");
    node = KeCreateFsNode("A:\\STREAMS", "SYSPRN");
    node = KeCreateFsNode("A:\\STREAMS", "SYSNUL");
	
	/* ********************************************************************** */
    /* LOCAL DOCUMENTS                                                        */
    /* ********************************************************************** */
    node = KeCreateFsNode("C:\\", "SOURCE");
    node = KeCreateFsNode("C:\\", "BINARIES");
    node = KeCreateFsNode("C:\\", "LIBRARIES");
    node = KeCreateFsNode("C:\\", "INCLUDE");
	
	KeDebugPrint("VFS initialized\r\n");
	
	ModInitHercDebug();
    KeMain();
    return 0;
}

int KeMain(void)
{
	struct css_schid schid;
    struct fs_handle *fdh;
    struct fs_fdscb fdscb = {0};
    struct PeReader *pe_reader;
    struct ElfReader *elf_reader;
    void *data_buffer;
	
	register size_t i;

    /* ********************************************************************** */
    /* SYSTEM MODULES                                                         */
    /* ********************************************************************** */
    ModInitX2703();
    ModInitX3270();
    ModInitX3390();
    /*ModInitBsc();*/
    /*ModProbeCss();*/
    
    schid.id = 1;
    schid.num = 0;
    ModAddX3270Device(schid, NULL);
    
    schid.id = 1;
    schid.num = 1;
    ModAddX3270Device(schid, NULL);
    
    schid.id = 1;
    schid.num = 2;
    ModAddX3270Device(schid, NULL);
    
    schid.id = 1;
    schid.num = 3;
    ModAddX3270Device(schid, NULL);
    
    schid.id = 1;
    schid.num = 4;
    ModAddX3270Device(schid, NULL);
    
    schid.id = 1;
    schid.num = 5;
    ModAddX3390Device(schid, NULL);
    
    /* ********************************************************************** */
    /* SYSTEM DEVICES                                                         */
    /* ********************************************************************** */
    g_stdout_fd = KeOpenFsNode("A:\\MODULES\\IBM-3270.0", VFS_MODE_WRITE);
    if(g_stdout_fd == NULL) {
        KePanic("Unable to forward SYSOUT from the 2703 line\r\n");
    }
    
    g_stdin_fd = KeOpenFsNode("A:\\MODULES\\IBM-3270.0", VFS_MODE_READ);
    if(g_stdin_fd == NULL) {
        KePanic("Unable to forward SYSIN from the 2703 line\r\n");
    }
    
    KePrint("UDOS on Enterprise System Architecture 390\r\n");
    KePrint("OS is ready - connect your terminals now!\r\n");
    KePrint("Welcome user %s!\r\n", KeGetAccountById(KeGetCurrentAccount())->name);
    while(1) {
        char *write_ptr;
        char linebuf[80];
        
        KePrint("%s>\r\n", KeGetAccountById(KeGetCurrentAccount())->name);
        KeSetMemory(&linebuf[0], 0, 80);
        KeReadFsNode(g_stdin_fd, &linebuf[0], 80);
        KeWriteFsNode(g_stdout_fd, &linebuf[4], KeStringLength(&linebuf[4]));
        
        /* TODO: Fix memory not being freed */
        for(i = 0; i < 65535 * 32; i++) {
            
        }
    }
    
    fdh = KeOpenFsNode("A:\\MODULES\\IBM-3390.0", VFS_MODE_READ);
    if(fdh == NULL) {
        KePanic("Cannot open disk");
    }
    if(ModGetZdsfsFile(fdh, &fdscb, "HERC02.ZIP") != 0) {
        KePanic("File not found");
    }
    KeDebugPrint("Loading NEWSGRP\r\n");
    data_buffer = (void *)0x100000;
    KeReadWithFdscbFsNode(fdh, &fdscb, data_buffer, 32757);
    ExLoadElfFromBuffer(data_buffer, 32757);
    KeCloseFsNode(fdh);
}
