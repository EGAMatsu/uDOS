/*
 * TODO: Use flatboot because some important data is chopped off the kernel
 */

#include <mm/mm.h>
#include <irq.h>
#include <debug/panic.h>
#include <mm/pmm.h>
#include <registry.h>
#include <user.h>
#include <fs/fs.h>

#include <mutex.h>
#include <s390/css.h>

#include <s390/x2703.h>
#include <s390/x3270.h>
#include <s390/x3390.h>

#include <arch/mmu.h>

int stream_sysnul_read(
    struct FsNode *node,
    void *buf,
    size_t size)
{
    KeSetMemory(buf, 0, size);
    return 0;
}

#include <scheduler.h>

void kern_A(void) {
    while(1) {
        kprintf("Hello A!\r\n");
        __asm__ __volatile__("svc 1");
    }
}

void kern_B(void) {
    while(1) {
        kprintf("Hello B!\r\n");
        __asm__ __volatile__("svc 1");
    }
}

#include <loader/elf.h>
#include <loader/pe.h>

int kmain(
    void)
{
    static struct RegistryGroup *hsystem, *hlocal, *hsubgr;
    struct FsNode *node;
    static user_t uid;
    struct SchedulerJob *job;
    struct SchedulerTask *task;
    struct SchedulerThread *thread;

    /* ********************************************************************** */
    /* REGISTRY KEY AND VALUES MANAGER                                        */
    /* ********************************************************************** */
    kprintf("Initializing the registry key manager\r\n");
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
    kprintf("Creating users and groups\r\n");
    uid = KeCreateUser("WRK001");
    uid = KeCreateUser("WRK002");
    uid = KeCreateUser("WRK003");
    uid = KeCreateUser("WRK004");
    uid = KeCreateUser("WRK005");
    uid = KeCreateUser("WRK006");
    uid = KeCreateUser("WRK007");
    uid = KeCreateUser("WRK008");
    uid = KeCreateUser("WRK009");
    uid = KeCreateUser("WRK010");
    uid = KeCreateUser("WRK011");
    uid = KeCreateUser("WRK012");
    uid = KeCreateUser("WRK013");
    uid = KeCreateUser("WRK014");
    uid = KeCreateUser("WRK015");
    uid = KeCreateUser("WRK016");
    uid = KeCreateUser("CLIENT01");
    KeSetCurrentUser(uid);

    /* ********************************************************************** */
    /* MULTITASKING ENGINE                                                    */
    /* ********************************************************************** */
    kprintf("Initializing the scheduler\r\n");

    job = KeCreateJob("KERNEL", 1, 32757);
    task = KeCreateTask(job, "PRIMARY");

    /*
    thread = KeCreateThread(job, task, 8192);
    thread->pc = (uintptr_t)&kern_A;
    thread->context.psw.address = thread->pc;
    thread->context.psw.flags = S390_PSW_DEFAULT_ARCHMODE
        | S390_PSW_IO_INT
        | S390_PSW_EXTERNAL_INT
        | S390_PSW_ENABLE_MCI;
    KeCopyMemory((void *)PSA_FLCSOPSW, &thread->context.psw, sizeof(struct s390_psw));
    KeCopyMemory(HwGetScratchContextFrame(), &thread->context, sizeof(thread->context));

    thread = KeCreateThread(job, task, 8192);
    thread->pc = (uintptr_t)&kern_B;
    thread->context.psw.address = thread->pc;
    thread->context.psw.flags = S390_PSW_DEFAULT_ARCHMODE
        | S390_PSW_IO_INT
        | S390_PSW_EXTERNAL_INT
        | S390_PSW_ENABLE_MCI;
    
    __asm__ __volatile__("svc 1");
    */

    //cpu_set_timer_delta_ms(100);

    /* ********************************************************************** */
    /* VIRTUAL FILE SYSTEM                                                    */
    /* ********************************************************************** */
    kprintf("Initializing the VFS\r\n");
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

#if defined(TARGET_S390)
    hdebug_init();
    g_stdout_fd = KeOpenFsNode("A:\\MODULES\\HDEBUG", VFS_MODE_WRITE);
#endif

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
    //x2703_init();
    ModInitX3270();
    ModInitX3390();
    //bsc_init();
    ModProbeCss();
    
    struct css_schid schid = { 0, 0 };
    ModAddX3270Device(schid, NULL);

    /* ********************************************************************** */
    /* HARDWARE DEVICES                                                       */
    /* ********************************************************************** */
    node = KeCreateFsNode("B:\\", "?");
    
    /* ********************************************************************** */
    /* LOCAL DOCUMENTS                                                        */
    /* ********************************************************************** */
    node = KeCreateFsNode("C:\\", "SOURCE");
    node = KeCreateFsNode("C:\\", "BINARIES");
    node = KeCreateFsNode("C:\\", "LIBRARIES");
    node = KeCreateFsNode("C:\\", "INCLUDE");

    /* ********************************************************************** */
    /* SYSTEM DEVICES                                                         */
    /* ********************************************************************** */
#if defined(TARGET_S390)
    //g_stdout_fd = KeOpenFsNode("A:\\DEVICES\\IBM-3270.0", VFS_MODE_WRITE);
    //if(g_stdout_fd == NULL) {
    //    KePanic("Unable to connect to 3270\r\n");
    //}
    //g_stdin_fd = KeOpenFsNode("A:\\DEVICES\\IBM-3270", VFS_MODE_READ);

    //g_stdout_fd = KeOpenFsNode("A:\\DEVICES\\IBM-2703", VFS_MODE_READ | VFS_MODE_WRITE);
    //g_stdout_fd = KeOpenFsNode("A:\\COMM\\BSC.000", VFS_MODE_READ | VFS_MODE_WRITE);
#endif

    kprintf("VFS initialized\r\n");
    kprintf("UDOS on Enterprise System Architecture 390\r\n");
    kprintf("Welcome user %s!\r\n", KeGetUserById(KeGetCurrentUser())->name);

    kprintf("%s>\r\n", KeGetUserById(KeGetCurrentUser())->name);

#if defined(TARGET_S390)
    struct FsHandle *fdh;
    struct FsFdscb fdscb = {0};

    struct PeReader *pe_reader;
    struct ElfReader *elf_reader;
    void *data_buffer;

    fdh = KeOpenFsNode("A:\\MODULES\\IBM-3390.0", VFS_MODE_READ);
    if(fdh == NULL) {
        KePanic("Cannot open disk");
    }

    ModGetZdsfsFile(fdh, &fdscb, "FORTH");
    kprintf("Loading FORTH\r\n");
    data_buffer = (void *)0x100000;
    KeReadWithFdscbFsNode(fdh, &fdscb, data_buffer, 32757);
    ExLoadElfFromBuffer(data_buffer, 32757);

    /*
    ModGetZdsfsFile(fdh, &fdscb, "DOSRTL.A");
    kprintf("Loading runtime library\r\n");
    data_buffer = (void *)0x100000;
    KeReadWithFdscbFsNode(fdh, &fdscb, data_buffer, 32757);
    elf_reader = ExCreateElfReader();
    ExReadElfFromBuffer(elf_reader, data_buffer, 32757);

    ModGetZdsfsFile(fdh, &fdscb, "PDPNNPT.EXE");
    kprintf("Loading network program\r\n");
    data_buffer = (void *)0x100000;
    KeReadWithFdscbFsNode(fdh, &fdscb, data_buffer, 32757);
    pe_reader = ExCreatePeReader();
    ExReadPeFromBuffer(pe_reader, (void *)0x100004, 32757);
    */

    KeCloseFsNode(fdh);
#endif

#if defined(TARGET_S390)
    while(1) {
        char *write_ptr;
        char linebuf[80];
        
        KeSetMemory(&linebuf[0], 0, 80);
        KeReadFsNode(g_stdin_fd, &linebuf[0], 80);

        KeWriteFsNode(g_stdout_fd, &linebuf[4], KeStringLength(&linebuf[4]));

        /* TODO: Fix memory not being freed */
        for(size_t i = 0; i < 65535 * 32; i++) {};
    }
#endif

    /*
    fdh = KeOpenFsNode("A:\\COMM\\BSC.000", VFS_MODE_READ | VFS_MODE_WRITE);
    KeWriteFsNode(fdh, "LIST\r\n", 6);
    char *tmpbuf;
    tmpbuf = MmAllocateZero(4096);
    KeReadFsNode(fdh, tmpbuf, 4096);
    kprintf("READ TELNET\n%s\r\n", tmpbuf);
    KeCloseFsNode(fdh);
    */

    /*
    {
        struct FsHandle *fdh;
        fdh = KeOpenFsNode("A:\\DEVICES\\IBM-3270", VFS_MODE_READ);
        if(fdh == NULL) {
            KePanic("Cannot open 3270");
        }
        kprintf("Opened %s\r\n", fdh->node->name);

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

    kprintf("Welcome back\r\n");
    while(1);
}