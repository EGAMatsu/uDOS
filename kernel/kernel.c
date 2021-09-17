#include <irq.h>
#include <alloc.h>
#include <panic.h>
#include <pmm.h>
#include <registry.h>
#include <user.h>
#include <vfs.h>

static struct reg_group *hlocal;
static user_t admin, udos;
extern void *heap_start;
struct vfs_node *node;

#include <cpu.h>
#include <mutex.h>
#include <s390/cpu.h>
#include <s390/css.h>
#include <s390/mmu.h>
#include <s390/dasd.h>
#include <s390/asm.h>

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

int diag8_write(int fd, const void *buf, size_t size) {
    char tmpbuf[80 + 6];
    memcpy(&tmpbuf[0], "MSG * ", 6);
    memcpy(&tmpbuf[6], buf, size);

    __asm__ __volatile__("diag %0, %1, 8"
                         :
                         : "r"(&tmpbuf[0]), "r"(size + 6)
                         : "cc", "memory");
    return 0;
}

int stream_sysnul_read(int fd, void *buf, size_t size) {
    memset(buf, 0, size);
    return 0;
}

#define MAX_CPUS 248

#include <s390/interrupt.h>
extern int g_stdout_fd, g_stdin_fd;
int kmain(void) {
    /*
    struct s390x_psw svc_psw = {
        PSW_ENABLE_MCI | PSW_AM64,
        PSW_DEFAULT_AMBIT,
        0,
        (uint32_t)&s390_service_call_handler_stub
    };
    struct s390x_psw pc_psw = {
        PSW_ENABLE_MCI | PSW_AM64,
        PSW_DEFAULT_AMBIT,
        0,
        (uint32_t)&s390_program_check_handler_stub
    };

    memcpy((void *)FLCEPNPSW, &pc_psw, sizeof(pc_psw));
    memcpy((void *)FLCESNPSW, &svc_psw, sizeof(svc_psw));
    __asm__ __volatile__("svc 3");
    */

    pmm_create_region(&heap_start, 0x80000);

    vfs_init();
    node              = vfs_new_node("\\", "SYSTEM");
    node              = vfs_new_node("\\SYSTEM", "CORES");
    node              = vfs_new_node("\\SYSTEM", "DEVICES");

    node              = vfs_new_node("\\SYSTEM", "STREAMS");
    
    /* System output */
    node              = vfs_new_node("\\SYSTEM\\STREAMS", "SYSOUT");
    node->hooks.write = &diag8_write;
    g_stdout_fd       = vfs_open("\\SYSTEM\\STREAMS\\SYSOUT", O_WRITE);

    /* System input */
    node              = vfs_new_node("\\SYSTEM\\STREAMS", "SYSIN");
    g_stdin_fd        = vfs_open("\\SYSTEM\\STREAMS\\SYSOUT", O_READ);

    /* System auxilliar output/input */
    node              = vfs_new_node("\\SYSTEM\\STREAMS", "SYSAUX");

    /* System printer */
    node              = vfs_new_node("\\SYSTEM\\STREAMS", "SYSPRN");

    /* System null device */
    node              = vfs_new_node("\\SYSTEM\\STREAMS", "SYSNUL");
    node->hooks.read  = &stream_sysnul_read;

    node              = vfs_new_node("\\", "DOCUMENTS");
    node              = vfs_new_node("\\DOCUMENTS", "DATASETS");
    node              = vfs_new_node("\\DOCUMENTS", "SOURCE");
    node              = vfs_new_node("\\DOCUMENTS", "BINARIES");
    node              = vfs_new_node("\\DOCUMENTS", "LIBRARIES");
    node              = vfs_new_node("\\DOCUMENTS", "INCLUDE");

    kprintf("VFS initialized\n");
    int r = dasd_init();

    int fd = vfs_open("\\SYSTEM\\DASD", O_WRITE);
    char tmpbuf[512];
    kprintf("vfs_open\n");
    vfs_read(fd, &tmpbuf[0], 16);
    kprintf("vfs_read\n");
    vfs_close(fd);
    kprintf("vfs_close\n");
    while (1)
        ;

    /*mutex_lock(&g_cpu_info_table.lock);
    for (size_t i = 0; i < 248; i++) {
      int r;
      kprintf("CPU#%zu %x\n", (size_t)i, r);
      r = s390_signal_processor(i, S390_SIGP_INIT_RESET);
      if (r != 0) {
        continue;
      }
      g_cpu_info_table.cpus[i].is_present = 1;
    }
    mutex_unlock(&g_cpu_info_table.lock);*/
    /* s390_mmu.turn_on(&s390_mmu); */
    kprintf("This cpu -> [%s] [%zu] [%s]\n", "xd", (size_t)s390_cpuid(), "xd");

    kprintf("Registry key manager\n");
    reg_init();
    hlocal = reg_create_group(reg_get_root_group(), "HLOCAL");

    kprintf("Users and groups\n");
    admin = user_create("ADMIN");
    udos  = user_create("UDOS");
    user_set_current(1);

    kprintf("Hello world\n");
    while (1)
        ;
}