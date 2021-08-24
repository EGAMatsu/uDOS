/*
 * stivale1/2 protocol implementation
 * TODO: actually implement the protocol
 * TODO: load kernel from disk
 * TODO: load elf files
 * TODO: set up paging - or not :)
 * TODO: support multiboot?
 */

#include <stddef.h>
#include <stdint.h>
#include <stivale2.h>

#define AMBIT 0x80000000
#define FLCIOA 0xB8
#define FLCSNPSW 0x60
#define FLCPNPSW 0x68
#define FLCINPSW 0x78
#define FLCMNPSW 0x70
#define FLCCAW 0x00

struct css_ccw1 {
  uint8_t flags;
  uint8_t cmd;
  uint16_t length;
  uint32_t addr;
} __attribute__((packed));

struct rb_bbcchhr {
  struct {
    uint16_t bb;
    uint16_t cc1;
    uint16_t hh1;
  } bbcchh;
  struct {
    uint16_t cc2;
    uint16_t hh2;
    uint8_t record;
  } cchhr;
} __attribute__((packed));

struct s390_psw {
  uint32_t lo;
  uint32_t hi;
} __attribute__((packed, aligned(8)));

struct css_orb {
  uint32_t reserved1;
  uint32_t lpm;
  uint32_t addr;
  uint32_t reserved2[5];
} __attribute__((packed, aligned(4)));

struct css_irb {
  uint32_t reserved[24];
} __attribute__((packed, aligned(4)));

extern struct rb_bbcchhr read_block;

/* It works (kinda), so shut up */
__asm__(".align 8");
struct css_ccw1 rb_seek = {.cmd = 0x07,
                           .addr = (uint32_t)&read_block.bbcchh,
                           .flags = 0x40,
                           .length = 0x06};
struct css_ccw1 rb_search = {
    .cmd = 0x31,
    .addr = (uint32_t)&read_block.cchhr,
    .flags = 0x40,
    .length = 0x05,
};
struct css_ccw1 rb_search2 = {
    .cmd = 0x08, .addr = (uint32_t)&rb_search, .flags = 0x00, .length = 0x00};
struct css_ccw1 rb_ldccw = {
    .cmd = 0x0e, .addr = 0x00, .flags = 0x00, .length = 0x00};
struct rb_bbcchhr read_block = {0};
struct css_orb rb_orb = {
    .lpm = 0x0080FF00,
    .addr = (uint32_t)&rb_seek,
};
struct css_orb rb_irb = {0};

void *memcpy(void *dest, const void *src, size_t n) {
  const char *c_src = (const char *)src;
  char *c_dest = (char *)dest;
  while (n) {
    *(c_dest++) = *(c_src++);
    --n;
  }
  return dest;
}

int diag8_write(const void *buf, size_t size) {
  char tmpbuf[80];
  memcpy(&tmpbuf[0], "MSG * ", 6);
  memcpy(&tmpbuf[6], buf, size);
  tmpbuf[size + 6] = '\0';

  asm volatile("diag %0, %1, 8"
               :
               : "r"(&tmpbuf[0]), "r"(size + 6)
               : "cc", "memory");
  return 0;
}

/*
 * Read by CHR from disk given by SCHID
 */
void read_disk(unsigned int schid, uint16_t cyl, uint16_t head, uint8_t rec,
               void *buf, size_t n) {
  static struct s390_psw rb_wtnoer = {0x060E0000, AMBIT};
  struct s390_psw rb_newio = {0x000C0000, AMBIT + (uint32_t) && rb_count};

  volatile struct s390_psw *flcinpsw = (volatile struct s390_psw *)FLCINPSW;
  volatile uint32_t *flccaw = (volatile uint32_t *)FLCCAW;

  read_block.bbcchh.bb = 0;
  read_block.bbcchh.cc1 = cyl;
  read_block.bbcchh.hh1 = head;

  read_block.cchhr.cc2 = cyl;
  read_block.cchhr.hh2 = head;
  read_block.cchhr.record = rec;

  rb_ldccw.addr = (uint32_t)buf;
  rb_ldccw.length = n;

  flcinpsw->lo = rb_newio.lo;
  flcinpsw->hi = rb_newio.hi;

  *flccaw = (uint32_t)&rb_seek;

  diag8_write("WAITING FOR INTERRUPT", 22);
  {
    register unsigned int r1 __asm__("1") = schid;
    __asm__ __volatile__("tsch %0" : : "m"(rb_irb), "r"(r1) : "memory");

    __asm__ __volatile__("ssch %0" : : "m"(rb_orb), "r"(r1) : "memory");
  }

  diag8_write("WAITING FOR INTERRUPT", 22);
  __asm__ goto("lpsw %0\n" : : "m"(rb_wtnoer) : : rb_count);
rb_count:
  diag8_write("INTERRUPT DONE", 22);
  return;
}

extern unsigned char disk_buffer[];
int main(void) {
  diag8_write("S390X BOOTLOADER", 17);

  read_disk(0x00010001, 1, 0, 1, &disk_buffer, 2048);
  while (1)
    ;
}