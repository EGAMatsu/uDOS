#include <alloc.h>
#include <irq.h>
#include <panic.h>
#include <pmm.h>
#include <registry.h>
#include <user.h>
#include <vfs.h>

extern void *heap_start;

#include <mutex.h>
#include <s390/css.h>

#include <s390/ibm3270.h>
#include <s390/ibm3390.h>

#include <arch/mmu.h>

extern unsigned int dist_kernel_bin_len;
extern unsigned char dist_kernel_bin[];

const unsigned char ebc2asc[256] = {
    0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f, 0x16, 0x05, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26,
    0x18, 0x19, 0x3f, 0x27, 0x22, 0x1d, 0x35, 0x1f, 0x40, 0x5a, 0x7f, 0x7b,
    0x5b, 0x6c, 0x50, 0x7d, 0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0x7a, 0x5e,
    0x4c, 0x7e, 0x6e, 0x6f, 0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xad, 0xe0, 0xbd, 0x5f, 0x6d,
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92,
    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6,
    0xa7, 0xa8, 0xa9, 0xc0, 0x6a, 0xd0, 0xa1, 0x07, 0x9f, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x41, 0xaa, 0x4a, 0xb1, 0x00, 0xb2, 0x6a, 0xb5,
    0xbd, 0xb4, 0x9a, 0x8a, 0x5f, 0xca, 0xaf, 0xbc, 0x90, 0x8f, 0xea, 0xfa,
    0xbe, 0xa0, 0xb6, 0xb3, 0x9d, 0xda, 0x9b, 0x8b, 0xb7, 0xb8, 0xb9, 0xab,
    0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9e, 0x68, 0x74, 0x71, 0x72, 0x73,
    0x78, 0x75, 0x76, 0x77, 0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9c, 0x48,
    0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57, 0x8c, 0x49, 0xcd, 0xce,
    0xcb, 0xcf, 0xcc, 0xe1, 0x70, 0xdd, 0xde, 0xdb, 0xdc, 0x8d, 0x8e, 0x8f,
};

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
    /* PHYSICAL MEMORY MANAGER                                                */
    /* ********************************************************************** */
    kprintf("Initializing the physical memory manager\n");
    pmm_create_region(&heap_start, 0xffff);

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
    ibm3270_init();
    ibm3390_init();
    //g_stdout_fd = vfs_open("\\SYSTEM\\DEVICES\\IBM3270", O_WRITE);
    //g_stdin_fd = vfs_open("\\SYSTEM\\DEVICES\\IBM3270", O_READ);
    /* ********************************************************************** */
    kprintf("VFS initialized\n");

    struct vfs_node *fd_node;

    fd_node = vfs_open("\\SYSTEM\\DEVICES\\IBM3270", O_READ);
    char tmpbuf[6 + (43 * 80) + 7];
    const char *msg = "Hello world :D";
    memset(&tmpbuf[0], ' ', sizeof(tmpbuf));
    memcpy(&tmpbuf[0], "\xC3\x11\x5D\x7F\x1D\xF0", 6);
    memcpy(&tmpbuf[6 + (43 * 80)], "\x1D\x00\x13\x3C\x5D\x7F\x00", 7);
    memset(&tmpbuf[6 + (1 * 80)], ' ', 80);
    memcpy(&tmpbuf[6 + (1 * 80)], msg, strlen(msg));
    vfs_write(fd_node, &tmpbuf[0], sizeof(tmpbuf));
    vfs_close(fd_node);

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

    /* Save current registers into the control */
    /*
    __asm__ __volatile__(
        "stm 0, 15, %0"
        :
        : "d"(S390_FLCCRSAV));
    kprintf("Took system snapshot\n");
    */

    kprintf("Welcome to UDOS!\n");
    
    /*struct s390_svc_frame frame = {0};
    s390_do_svc(1, frame);*/

    kprintf("Welcome back\n");
    while(1);
}