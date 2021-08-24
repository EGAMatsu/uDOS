#include <irq.h>
#include <malloc.h>
#include <panic.h>
#include <pmm.h>
#include <registry.h>
#include <user.h>
#include <vfs.h>

static struct reg_group *hlocal;
static user_t admin, udos;
extern void *heap_start;
struct vfs_node *node;

struct s390x_perm_storage_assign {
  uint32_t ipl_psw;
  uint32_t ipl_ccw[2];
  uint32_t external_old_psw;
  uint32_t svc_old_psw;
  uint32_t program_old_psw;
  uint32_t mcheck_old_psw;
  uint32_t io_old_psw;
  uint32_t channel_status;
  uint16_t channel_address;
  uint16_t unused1;
  uint16_t timer;
  uint16_t unused2;
  uint32_t ext_new_psw;
  uint32_t svc_new_psw;
  uint32_t program_new_psw;
  uint32_t mcheck_new_psw;
  uint32_t io_new_psw;
};

static void test(void) {
  kprintf("HELLO INTERRUPT WORLD!\n");
  while (1)
    ;
}

#include <cpu.h>
#include <mutex.h>
#include <s390/cpu.h>
#include <s390/css.h>
#include <s390/mmu.h>

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
  char tmpbuf[80];
  memcpy(&tmpbuf, "MSG * ", 5);
  memcpy(&tmpbuf, buf, size);
  tmpbuf[size + 5] = '\0';

  __asm__ __volatile__("diag %0, %1, 8"
                       :
                       : "r"(&tmpbuf[0]), "r"(size)
                       : "cc", "memory");
  return 0;
}

extern int g_stdio_fd;
int kmain(void) {
  pmm_create_region(&heap_start, 0x8fffff);

  vfs_init();
  node = vfs_new_node("SYSTEM", "\\");
  node = vfs_new_node("ORG", "\\");
  node = vfs_new_node("SYSIO", "\\");
  node->hooks.write = &diag8_write;
  // node->hooks.read = &hercules_diag_read;

  g_stdio_fd = vfs_open("\\SYSIO", O_WRITE);

  mutex_lock(&g_cpu_info_table.lock);
  for (size_t i = 0; i < 248; i++) {
    int r;
    r = s390_signal_processor(i, S390_SIGP_INIT_RESET);
    if (r != 0) {
      continue;
    }

    kprintf("CPU#%zu %x\n", (size_t)i, r);
    g_cpu_info_table.cpus[i].is_present = 1;
  }
  mutex_unlock(&g_cpu_info_table.lock);
  s390_mmu.turn_on(&s390_mmu);
  kprintf("This cpu -> %zu\n", (size_t)s390_cpuid());

  kprintf("Registry key manager\n");
  reg_init();
  hlocal = reg_create_group(reg_get_root_group(), "HLOCAL");

  kprintf("Users and groups\n");
  admin = user_create("ADMIN");
  udos = user_create("UDOS");
  user_set_current(1);

  kprintf("Hello world\n");
  ucli_init();
  while (1)
    ;
}
