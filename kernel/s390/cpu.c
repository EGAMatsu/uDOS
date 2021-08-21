#include <stdint.h>
#include <s390/cpu.h>

unsigned int s390_cpuid(void) {
    uint16_t cpu;
    __asm__ __volatile__(
        "stap %0"
        : "=m"(cpu)
        :
        :
    );
    return (unsigned int)cpu;
}

unsigned int s390_store_then_or_system_mask(uint8_t mask) {
    uint8_t old_psw;
    __asm__ __volatile__(
        "stosm %0, %1"
        : "+d"(old_psw)
        : "r"(mask)
        : "cc"
    );
    return old_psw;
}

int s390_signal_processor(unsigned int cpu_addr, unsigned int param) {
    /* The s390 spec says that the next odd register number (in short, r1 + 1)
     * shall contain the parameter for the processor signal
     */
    register unsigned int r1 __asm__("1") = 0; /* Status */
    register unsigned int r2 __asm__("2") = param; /* Parameter (only lower 32 bits) */
    unsigned int order_code = 0;
    int cc = -1;

    __asm__ __volatile__(
        "sigp %%r1, %1, %2\n"
        "ipm %0"
        : "+d"(cc)
        : "r"(cpu_addr), "r"(order_code)
        : "cc", "memory"
    );
    return cc >> 28;
}