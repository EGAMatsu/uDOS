/*
 * TODO: Use flatboot because some important data is chopped off the kernel
 */

#include <alloc.h>
#include <irq.h>
#include <panic.h>
#include <pmm.h>
#include <registry.h>
#include <user.h>
#include <vfs.h>

#include <mutex.h>
#include <s390/css.h>

#include <s390/x2703.h>
#include <s390/x3270.h>
#include <s390/x3390.h>

#include <arch/mmu.h>

extern unsigned int dist_kernel_bin_len;
extern unsigned char dist_kernel_bin[];

int stream_sysnul_read(
    struct vfs_node *node,
    void *buf,
    size_t size)
{
    memset(buf, 0, size);
    return 0;
}

#include <elf.h>
#include <scheduler.h>

void kern_A(void) {
    while(1) {
        kprintf("B!\r\n");
    }
}

void kern_B(void) {
    while(1) {
        kprintf("A!\r\n");
    }
}

int kmain(
    void)
{
    static struct reg_group *hlocal;
    struct vfs_node *node;
    static user_t uid;
    struct scheduler_job *job;
    struct scheduler_task *task;
    struct scheduler_thread *thread;

    /* ********************************************************************** */
    /* REGISTRY KEY AND VALUES MANAGER                                        */
    /* ********************************************************************** */
    kprintf("Initializing the registry key manager\r\n");
    reg_init();
    hlocal = reg_create_group(reg_get_root_group(), "HLOCAL");

    /* ********************************************************************** */
    /* USER AND GROUP AUTHORIZATION                                           */
    /* ********************************************************************** */
    kprintf("Creating users and groups\r\n");
    uid = user_create("SYSADMIN");
    uid = user_create("MASTER");
    uid = user_create("CLIENT");
    uid = user_create("REMOTE");
    uid = user_create("LOCAL");
    uid = user_create("REMOTE");
    user_set_current(uid);

    /* ********************************************************************** */
    /* MULTITASKING ENGINE                                                    */
    /* ********************************************************************** */
    kprintf("Initializing the scheduler\r\n");
    job = scheduler_new_job("KERNEL", 1, 32757);
    task = scheduler_new_task(job, "PRIMARY");
    thread = scheduler_new_thead(job, task, 4096);
    thread->pc = &kern_A;
    thread = scheduler_new_thead(job, task, 4096);
    thread->pc = &kern_B;
    cpu_set_timer_delta_ms(100);

    /* ********************************************************************** */
    /* VIRTUAL FILE SYSTEM                                                    */
    /* ********************************************************************** */
    kprintf("Initializing the VFS\r\n");
    vfs_init();
    node = vfs_new_node("\\", "SYSTEM");
    node = vfs_new_node("\\SYSTEM", "CORES");
    node = vfs_new_node("\\SYSTEM", "COMM");
    node = vfs_new_node("\\SYSTEM", "STREAMS");
    node = vfs_new_node("\\SYSTEM", "DEVICES");
    node = vfs_new_node("\\", "DOCUMENTS");

    hdebug_init();
    g_stdout_fd = vfs_open("\\SYSTEM\\DEVICES\\HDEBUG", VFS_MODE_WRITE);

    /* ********************************************************************** */
    /* SYSTEM STREAMS                                                         */
    /* ********************************************************************** */
    node = vfs_new_node("\\SYSTEM\\STREAMS", "SYSOUT");
    node = vfs_new_node("\\SYSTEM\\STREAMS", "SYSIN");
    node = vfs_new_node("\\SYSTEM\\STREAMS", "SYSAUX");
    node = vfs_new_node("\\SYSTEM\\STREAMS", "SYSPRN");
    node = vfs_new_node("\\SYSTEM\\STREAMS", "SYSNUL");

    //g_stdin_fd = vfs_open("\\SYSTEM\\STREAMS\\SYSIN", VFS_MODE_READ);
    /* ********************************************************************** */

    /* ********************************************************************** */
    /* LOCAL DOCUMENTS                                                        */
    /* ********************************************************************** */
    node = vfs_new_node("\\DOCUMENTS", "DATASETS");
    node = vfs_new_node("\\DOCUMENTS", "SOURCE");
    node = vfs_new_node("\\DOCUMENTS", "BINARIES");
    node = vfs_new_node("\\DOCUMENTS", "LIBRARIES");
    node = vfs_new_node("\\DOCUMENTS", "INCLUDE");
    /* ********************************************************************** */

    /* ********************************************************************** */
    /* SYSTEM DEVICES                                                         */
    /* ********************************************************************** */
    //x2703_init();
    x3270_init();
    x3390_init();
    //bsc_init();
    css_probe();

    //g_stdout_fd = vfs_open("\\SYSTEM\\DEVICES\\IBM-3270", VFS_MODE_WRITE);
    //g_stdin_fd = vfs_open("\\SYSTEM\\DEVICES\\IBM-3270", VFS_MODE_READ);

    //g_stdout_fd = vfs_open("\\SYSTEM\\DEVICES\\IBM-2703", VFS_MODE_READ | VFS_MODE_WRITE);
    //g_stdout_fd = vfs_open("\\SYSTEM\\COMM\\BSC.000", VFS_MODE_READ | VFS_MODE_WRITE);
    /* ********************************************************************** */

    kprintf("VFS initialized\r\n");
    kprintf("UDOS on Enterprise System Architecture 390\r\n");
    kprintf("Welcome user %s!\r\n", user_from_uid(user_get_current())->name);
    kprintf("%s>\r\n", user_from_uid(user_get_current())->name);

    while(1) {
        char *write_ptr;
        char linebuf[80];
        
        memset(&linebuf[0], 0, 80);
        vfs_read(g_stdin_fd, &linebuf[0], 80);

        vfs_write(g_stdout_fd, &linebuf[4], strlen(&linebuf[4]));

        /* TODO: Fix memory not being freed */
        for(size_t i = 0; i < 65535 * 32; i++) {};
    }

    /*
    fdh = vfs_open("\\SYSTEM\\COMM\\BSC.000", VFS_MODE_READ | VFS_MODE_WRITE);
    vfs_write(fdh, "LIST\r\n", 6);
    char *tmpbuf;
    tmpbuf = kzalloc(4096);
    vfs_read(fdh, tmpbuf, 4096);
    kprintf("READ TELNET\n%s\r\n", tmpbuf);
    vfs_close(fdh);
    */

    /*
    {
        struct vfs_handle *fdh;
        fdh = vfs_open("\\SYSTEM\\DEVICES\\IBM-3270", VFS_MODE_READ);
        if(fdh == NULL) {
            kpanic("Cannot open 3270");
        }
        kprintf("Opened %s\r\n", fdh->node->name);

        const char *msg = "Hello world";
        char *tmpbuf;
        size_t len = 6 + (24 * 80) + 7;

        tmpbuf = kzalloc(len);
        if(tmpbuf == NULL) {
            kpanic("Out of memory");
        }
        memset(&tmpbuf[0], '.', len);
        memcpy(&tmpbuf[0], "\xC3\x11\x5D\x7F\x1D\xF0", 6);
        memcpy(&tmpbuf[6 + (24 * 80)], "\x1D\x00\x13\x3C\x5D\x7F\x00", 7);
        memcpy(&tmpbuf[6 + (1 * 80)], msg, strlen(msg));
        vfs_write(fdh, &tmpbuf[0], len);

        const char *clear_pg = "\x0C";
        vfs_write(fdh, &clear_pg, strlen(clear_pg));

        const char *test = "Hello world\r\n";
        vfs_write(fdh, &test[0], strlen(test));

        vfs_close(fdh);
    }
    */

    /*
    fdh = vfs_open("\\SYSTEM\\DEVICES\\IBM3390", VFS_MODE_READ);
    if(fdh == NULL) {
        kpanic("Cannot open disk");
    }
    struct vfs_fdscb fdscb = {0};
    zdsfs_get_file(fdh, &fdscb, "LIBC.A");
    kprintf("Got file!\r\n");
    void *elf_data = (void *)((uintptr_t)&heap_start + 0xffff);
    vfs_read_fdscb(fdh, &fdscb, elf_data, 32757);
    vfs_close(fdh);
    */

    /* Save current registers into the control */
    /*
    __asm__ __volatile__(
        "stm 0, 15, %0"
        :
        : "d"(S390_FLCCRSAV));
    kprintf("Took system snapshot\r\n");
    */

    kprintf("Welcome back\r\n");
    while(1);
}