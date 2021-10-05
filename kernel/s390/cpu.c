#include <s390/asm.h>
#include <s390/cpu.h>
#include <string.h>

unsigned int s390_cpuid(void)
{
    uint16_t cpu;
    __asm__ __volatile__(
        "stap %0"
        : "=m"(cpu)
        :
        :);
    return (unsigned int)cpu;
}

unsigned int s390_store_then_or_system_mask(
    uint8_t mask)
{
    uint8_t old_psw;
    __asm__ __volatile__(
        "stosm %0, %1"
        : "+d"(old_psw)
        : "r"(mask)
        : "cc");
    return old_psw;
}

int s390_signal_processor(
    unsigned int cpu_addr,
    unsigned int param)
{
    /* The s390 spec says that the next odd register number (in short, r1 + 1)
   * shall contain the parameter for the processor signal */
    register unsigned int r1 __asm__("1") = 0; /* Status */
    register unsigned int r2 __asm__("2") =
        param; /* Parameter (only lower 32 bits) */
    unsigned int order_code = 0;
    int cc = -1;

    __asm__ __volatile__(
        "sigp %%r1, %1, %2\n"
        "ipm %0"
        : "+d"(cc)
        : "r"(cpu_addr), "r"(order_code), "r"(r1), "r"(r2)
        : "cc", "memory");
    return cc >> 28;
}

int s390_set_timer_delta(
    int ms)
{
    __attribute__((aligned(8))) int64_t clock = 0;
    __asm__ __volatile__(
        "stpt %0"
        : "=m"(clock)
        :
        :);
    kprintf("Old clock %i\n", (int)clock);
    
    clock = (int64_t)ms;

    __asm__ __volatile__(
        "spt %0"
        :
        : "m"(clock)
        :);
    kprintf("New clock %i\n", (int)clock);
    return 0;
}

/* Check if an address is valid - this only catches program exceptions to
 * determine if it's valid or not */
int s390_address_is_valid(
    volatile const void *probe)
{
    int r = 0;
#if (MACHINE >= M_ZARCH)
    struct s390x_psw pc_psw = {
        0x00040000 | S390_PSW_AM64, S390_PSW_DEFAULT_AMBIT, 0,
        (uint32_t)&&invalid
    };
    struct s390x_psw old_pc_psw;
#else
    struct s390_psw pc_psw = {
        0x000C0000, (uint32_t)&&invalid + S390_PSW_DEFAULT_AMBIT
    };
    struct s390_psw old_pc_psw;
#endif

#if (MACHINE >= M_ZARCH)
    memcpy(&old_pc_psw, (void *)S390_FLCEPNPSW, sizeof(struct s390x_psw));
    memcpy((void *)S390_FLCEPNPSW, &pc_psw, sizeof(struct s390x_psw));
#else
    memcpy(&old_pc_psw, (void *)S390_FLCPNPSW, sizeof(struct s390_psw));
    memcpy((void *)S390_FLCPNPSW, &pc_psw, sizeof(struct s390_psw));
#endif

    *((volatile const uint8_t *)probe);
    goto end;
invalid:
    r = -1;
end:
#if (MACHINE >= M_ZARCH)
    memcpy((void *)S390_FLCEPNPSW, &old_pc_psw, sizeof(struct s390x_psw));
#else
    memcpy((void *)S390_FLCPNPSW, &old_pc_psw, sizeof(struct s390_psw));
#endif
    return r;
}

/* We are going to read in pairs of 1MiB and when we hit the memory limit we
 * will instantly catch the program exception and stop counting, then it's just
 * a matter of returning what we could count :) */
size_t s390_get_memsize(void)
{
    const uint8_t *probe = (const uint8_t *)0x0;
    while(1) {
        int r;
        /* Do a "probe" read */
        r = s390_address_is_valid(probe);
        if(r != 0) {
            kprintf("Done! %p\n", (uintptr_t)probe);
            break;
        }

        kprintf("Memory %p\n", (uintptr_t)probe);

        /* Go to next MiB */
        probe += 1048576;
    }
    return (size_t)probe;
}

/* Wait for an I/O response (overrides the I/O PSW) */
void s390_wait_io(void)
{
    struct s390_psw wait_io_psw = {
        S390_PSW_WAIT_STATE(1) | S390_PSW_IO_INT(1), S390_PSW_DEFAULT_AMBIT
    };

#if (MACHINE >= M_ZARCH)
    struct s390x_psw io_psw = {
        0x04040000 | S390_PSW_AM64, S390_PSW_AM31, 0,
        (uint32_t)&&after_wait
    };
#else
    struct s390_psw io_psw = {
        0x000C0000,
        (uint32_t)&&after_wait + S390_PSW_DEFAULT_AMBIT
    };
#endif

    /* The next I/O PSW to be set when the operation finishes */
#if (MACHINE >= M_ZARCH)
    memcpy((void *)S390_FLCEINPSW, &io_psw, sizeof(struct s390x_psw));
    __asm__ __volatile__(
        "stosm %0, %1"
        :
        : "d"(S390_FLCEINPSW), "d"(0x00)
        :);
#else
    memcpy((void *)S390_FLCINPSW, &io_psw, sizeof(struct s390_psw));
    __asm__ __volatile__(
        "stosm %0, %1"
        :
        : "d"(S390_FLCINPSW), "d"(0x00)
        :);
#endif

    /* Set a PSW to wait the I/O response */
    __asm__ goto(
        "lpsw %0\n"
        :
        : "m"(wait_io_psw)
        :
        : after_wait);
    
    __builtin_unreachable();
after_wait:
    return;
}