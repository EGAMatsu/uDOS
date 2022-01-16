/*
 * TODO: Use flatboot because some important data is chopped off the kernel
 */

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

void do_cmd(void)
{
    return;
}

int stream_sysnul_read(struct fs_node *node, void *buf, size_t size)
{
    KeSetMemory(buf, 0, size);
    return 0;
}

#include <scheduler.h>

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

#include <elf.h>
#include <pe.h>

int KeMain(void)
{
    static struct registry_group *hsystem, *hlocal, *hsubgr;
    struct fs_node *node;
    static user_t uid;
    struct scheduler_job *job;
    struct scheduler_task *task;
    struct scheduler_thread *thread;
	
	struct css_schid schid = { 1, 0 };
	
	unsigned char key[3] = {
        0x4b, 0x65, 0x79
    };
    unsigned char bitstream[9] = {
        0x50, 0x6c, 0x61, 0x69, 0x6e, 0x74, 0x65, 0x78, 0x74
    };
    const unsigned char *cipher = CryptoARC4Encode(&bitstream[0], 9, &key[0], 3);
    struct fs_handle *fdh;
    struct fs_fdscb fdscb = {0};

    struct PeReader *pe_reader;
    struct ElfReader *elf_reader;
    void *data_buffer;
	
	register size_t i;

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
    thread->context.psw.flags = PSW_DEFAULT_ARCHMODE | PSW_IO_INT
        | PSW_EXTERNAL_INT | PSW_ENABLE_MCI;
    KeCopyMemory((void *)PSA_FLCSOPSW, &thread->context.psw, sizeof(struct s390_psw));
    KeCopyMemory(HwGetScratchContextFrame(), &thread->context, sizeof(thread->context));

    thread = KeCreateThread(job, task, 8192);
    thread->pc = (unsigned int)&kern_B;
    thread->context.psw.address = thread->pc;
    thread->context.psw.flags = PSW_DEFAULT_ARCHMODE | PSW_IO_INT
        | PSW_EXTERNAL_INT | PSW_ENABLE_MCI;
    
    thread = KeCreateThread(job, task, 8192);
    thread->pc = (unsigned int)&kern_A;
    thread->context.psw.address = thread->pc;
    thread->context.psw.flags = PSW_DEFAULT_ARCHMODE | PSW_IO_INT
        | PSW_EXTERNAL_INT | PSW_ENABLE_MCI;
    
    /*unsigned a = 0;
    while(a < 800) {
        KeDebugPrint("Hello %u!\r\n", (unsigned)a);
        a++;
        HwDoSVC(50, 0, 0, 0);
        HwSetCPUTimerDelta(10);
    }*/

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

    ModInitHercDebug();
    /*g_stdout_fd = KeOpenFsNode("A:\\MODULES\\HDEBUG", VFS_MODE_WRITE);*/

    /* ********************************************************************** */
    /* SYSTEM STREAMS                                                         */
    /* ********************************************************************** */
    node = KeCreateFsNode("A:\\STREAMS", "SYSOUT");
    node = KeCreateFsNode("A:\\STREAMS", "SYSIN");
    node = KeCreateFsNode("A:\\STREAMS", "SYSAUX");
    node = KeCreateFsNode("A:\\STREAMS", "SYSPRN");
    node = KeCreateFsNode("A:\\STREAMS", "SYSNUL");

    /*g_stdin_fd = KeOpenFsNode("A:\\STREAMS\\SYSIN", VFS_MODE_READ);*/

    /* ********************************************************************** */
    /* SYSTEM MODULES                                                         */
    /* ********************************************************************** */
    ModInitX2703();
    ModInitX3270();
    ModInitX3390();
    ModInitBsc();
    ModProbeCss();
	
    ModAddX2703Device(schid, NULL);

    schid.num = 1;
    ModAddX3390Device(schid, NULL);

    schid.num = 2;
    ModAddX3390Device(schid, NULL);
    
    /* ********************************************************************** */
    /* LOCAL DOCUMENTS                                                        */
    /* ********************************************************************** */
    node = KeCreateFsNode("C:\\", "SOURCE");
    node = KeCreateFsNode("C:\\", "BINARIES");
    node = KeCreateFsNode("C:\\", "LIBRARIES");
    node = KeCreateFsNode("C:\\", "INCLUDE");

    do_cmd();

    /* If the telnet does not work for some reason uncomment/comment this as
     * needed, either gcc is a horrible code generator or my code is not good
     * enough, i'm putting my money on the latter - the compiler is (almost)
     * never wrong - If gcc is truly fucked well... fuck */
    /*KeDebugPrint("What a new thing!?\r\n");*/

    /* ********************************************************************** */
    /* SYSTEM DEVICES                                                         */
    /* ********************************************************************** */
    /*
    g_stdout_fd = KeOpenFsNode("A:\\MODULES\\IBM-2703.0", VFS_MODE_WRITE);
    if(g_stdout_fd == NULL) {
        KePanic("Unable to forward STDOUT to the BSC line\r\n");
    }
    */

    g_stdin_fd = KeOpenFsNode("A:\\MODULES\\IBM-2703.0", VFS_MODE_READ);
    if(g_stdin_fd == NULL) {
        KePanic("Unable to forward STDIN from the BSC line\r\n");
    }

    /*
    g_stdin_fd = KeOpenFsNode("A:\\MODULES\\BSC", VFS_MODE_READ);
    if(g_stdin_fd == NULL) {
        KePanic("Unable to forward STDIN from the BSC line\r\n");
    }
    */

    KeDebugPrint("VFS initialized\r\n");
    KeDebugPrint("UDOS on Enterprise System Architecture 390\r\n");
    KeDebugPrint("OS is ready - connect your terminals now!\r\n");
    KeDebugPrint("Welcome user %s!\r\n", KeGetAccountById(KeGetCurrentAccount())->name);

    KeDebugPrint("*** ARC4 OUTPUT ***\r\n");
    for(i = 0; i < 9; i++) {
        KeDebugPrint("C[%zu] = %x\r\n", i, (unsigned int)cipher[i]);
    }

    KeDebugPrint("%s>\r\n", KeGetAccountById(KeGetCurrentAccount())->name);

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

    while(1) {
        char *write_ptr;
        char linebuf[80];
        
        KeSetMemory(&linebuf[0], 0, 80);
        KeReadFsNode(g_stdin_fd, &linebuf[0], 80);

        KeWriteFsNode(g_stdout_fd, &linebuf[4], KeStringLength(&linebuf[4]));

        /* TODO: Fix memory not being freed */
        for(i = 0; i < 65535 * 32; i++) {
			
		}
    }

    /*
    fdh = KeOpenFsNode("A:\\COMM\\BSC.000", VFS_MODE_READ | VFS_MODE_WRITE);
    KeWriteFsNode(fdh, "LIST\r\n", 6);
    char *tmpbuf;
    tmpbuf = MmAllocateZero(4096);
    KeReadFsNode(fdh, tmpbuf, 4096);
    KeDebugPrint("READ TELNET\n%s\r\n", tmpbuf);
    KeCloseFsNode(fdh);
    */

    /*
    {
        struct fs_handle *fdh;
        fdh = KeOpenFsNode("A:\\DEVICES\\IBM-3270", VFS_MODE_READ);
        if(fdh == NULL) {
            KePanic("Cannot open 3270");
        }
        KeDebugPrint("Opened %s\r\n", fdh->node->name);

        const char *msg = "Hello world";
        char *tmpbuf;
        size_t len = 6 + (24 * 80) + 7;

        tmpbuf = MmAllocateZero(len);
        if(tmpbuf == NULL) {
            KePanic("Out of memory");
        }
        KeSetMemory(&tmpbuf[0], '.', len);
        KeCopyMemory(&tmpbuf[0], "\xC3\x11\x5D\x7F\x1D\xF0", 6);
        KeCopyMemory(&tmpbuf[6 + (24 * 80)], "\x1D\x00\x13\x3C\x5D\x7F\x00", 7);
        KeCopyMemory(&tmpbuf[6 + (1 * 80)], msg, KeStringLength(msg));
        KeWriteFsNode(fdh, &tmpbuf[0], len);

        const char *clear_pg = "\x0C";
        KeWriteFsNode(fdh, &clear_pg, KeStringLength(clear_pg));

        const char *test = "Hello world\r\n";
        KeWriteFsNode(fdh, &test[0], KeStringLength(test));

        KeCloseFsNode(fdh);
    }
    */

    KeDebugPrint("Welcome back\r\n");
    while(1);
}
