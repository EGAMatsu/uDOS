#ifndef CPU_H
#define CPU_H

#include <mutex.h>
#include <stddef.h>
#include <stdint.h>

struct cpu_info {
  int is_present;
  struct mmu_dev *mmu;
  uint64_t cr[16]; /* Control registers */
  mutex_t lock;
};

struct cpu_table {
  struct cpu_info cpus[255];
  mutex_t lock;
};

extern struct cpu_table g_cpu_info_table;
struct cpu_info *cpu_get_current(void);

#endif