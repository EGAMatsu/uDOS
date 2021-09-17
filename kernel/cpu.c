#include <s390/cpu.h>
#include <cpu.h>

struct cpu_table g_cpu_info_table = {0};
struct cpu_info *cpu_get_current(void) {
    return &g_cpu_info_table.cpus[s390_cpuid()];
}