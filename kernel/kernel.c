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
        kprintf("B!\n");
    }
}

void kern_B(void) {
    while(1) {
        kprintf("A!\n");
    }
}

extern struct vfs_node *g_stdout_fd, *g_stdin_fd;
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
    kprintf("Initializing the registry key manager\n");
    reg_init();
    hlocal = reg_create_group(reg_get_root_group(), "HLOCAL");

    /* ********************************************************************** */
    /* USER AND GROUP AUTHORIZATION                                           */
    /* ********************************************************************** */
    kprintf("Creating users and groups\n");
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
    kprintf("Initializing the scheduler\n");
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
    kprintf("Initializing VFS");
    vfs_init();
    node = vfs_new_node("\\", "SYSTEM");
    node = vfs_new_node("\\SYSTEM", "CORES");
    node = vfs_new_node("\\SYSTEM", "STREAMS");
    node = vfs_new_node("\\SYSTEM", "DEVICES");
    node = vfs_new_node("\\", "DOCUMENTS");
    hdebug_init();

    /* ********************************************************************** */
    /* SYSTEM STREAMS                                                         */
    /* ********************************************************************** */
    node = vfs_new_node("\\SYSTEM\\STREAMS", "SYSOUT");
    node = vfs_new_node("\\SYSTEM\\STREAMS", "SYSIN");
    node = vfs_new_node("\\SYSTEM\\STREAMS", "SYSAUX");
    node = vfs_new_node("\\SYSTEM\\STREAMS", "SYSPRN");
    node = vfs_new_node("\\SYSTEM\\STREAMS", "SYSNUL");

    g_stdout_fd = vfs_open("\\SYSTEM\\DEVICES\\HDEBUG", O_WRITE);
    g_stdin_fd = vfs_open("\\SYSTEM\\STREAMS\\SYSIN", O_READ);
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
    //css_probe();
    x2703_init();
    x3270_init();
    x3390_init();
    //g_stdout_fd = vfs_open("\\SYSTEM\\DEVICES\\IBM-2703", O_READ | O_WRITE);
    //g_stdin_fd = vfs_open("\\SYSTEM\\DEVICES\\IBM-3270", O_READ);
    /* ********************************************************************** */

    /* ********************************************************************** */
    /* SYSTEM COMMUNICATIONS                                                  */
    /* ********************************************************************** */
    node = vfs_new_node("\\SYSTEM", "COMM");
    bsc_init();

    //g_stdout_fd = vfs_open("\\SYSTEM\\COMM\\BSC.000", O_READ | O_WRITE);
    /* ********************************************************************** */

    /*
    kprintf("VFS initialized\n");
    kprintf("UDOS on Enterprise System Architecture 390\n");
    kprintf("Welcome user %s!\n", user_from_uid(user_get_current())->name);
    kprintf("?>\n");
    */

    struct vfs_node *fd_node = vfs_open("\\SYSTEM\\COMM\\BSC.000", O_READ | O_WRITE);

    char tmpbuf[256];
    while(1) {
        //memset(&tmpbuf[curr], 0, 256);
        vfs_read(fd_node, &tmpbuf[0], 256);
        
        /*curr = strlen(&tmpbuf[curr]) - 1;
        if(tmpbuf[curr] != '\x11') {
            continue;
        }*/

        /*kprintf("%s", &tmpbuf[0]);*/

        kprintf("Echo: %s\n", &tmpbuf[0]);
    }

    vfs_close(fd_node);

    /*
    struct vfs_node *fd_node;

    fd_node = vfs_open("\\SYSTEM\\DEVICES\\IBM-3270", O_READ);

    const char *msg = "HELLO TELNET WORLD FROM THE UDOS OPERATING SYSTEM";
    vfs_write(fd_node, msg, 50);

    char tmpbuf[6 + (43 * 80) + 7];
    const char *msg = "Hello world :D";
    memset(&tmpbuf[0], ' ', sizeof(tmpbuf));
    memcpy(&tmpbuf[0], "\xC3\x11\x5D\x7F\x1D\xF0", 6);
    memcpy(&tmpbuf[6 + (43 * 80)], "\x1D\x00\x13\x3C\x5D\x7F\x00", 7);
    memset(&tmpbuf[6 + (1 * 80)], ' ', 80);
    memcpy(&tmpbuf[6 + (1 * 80)], msg, strlen(msg));
    vfs_write(fd_node, &tmpbuf[0], sizeof(tmpbuf));

    vfs_close(fd_node);
    */
    while(1);

    /*
    fd_node = vfs_open("\\SYSTEM\\DEVICES\\IBM3390", O_READ);
    if(fd_node == NULL) {
        kpanic("Cannot open disk");
    }
    struct vfs_fdscb fdscb = {0};
    zdsfs_get_file(fd_node, &fdscb, "LIBC.A");
    kprintf("Got file!\n");
    void *elf_data = (void *)((uintptr_t)&heap_start + 0xffff);
    vfs_read_fdscb(fd_node, &fdscb, elf_data, 32757);
    vfs_close(fd_node);
    */

    /* Save current registers into the control */
    /*
    __asm__ __volatile__(
        "stm 0, 15, %0"
        :
        : "d"(S390_FLCCRSAV));
    kprintf("Took system snapshot\n");
    */
    
    /*struct s390_svc_frame frame = {0};
    s390_do_svc(1, frame);*/

    kprintf("Welcome back\n");
    while(1);
}