#include <s390/asm.h>
#include <s390/cpu.h>
#include <memory.h>

unsigned int HwS390Cpuid(void)
{
    uint16_t cpu;
    __asm__ __volatile__(
        "stap %0"
        : "=m"(cpu)
        :
        :);
    return (unsigned int)cpu;
}

PSW_DEFAULT_TYPE HwS390StoreOrSystemMask(
    unsigned int mask)
{
    PSW_DEFAULT_TYPE old_psw;
    __asm__ __volatile__(
        "stosm %0, %1"
        : "+d"(old_psw)
        : "r"(mask)
        : "cc");
    return old_psw;
}

int HwS390SignalProcessor(
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
        "sigp %%r1, %1, %2\r\n"
        "ipm %0"
        : "+d"(cc)
        : "r"(cpu_addr), "r"(order_code), "r"(r1), "r"(r2)
        : "cc", "memory");
    return cc >> 28;
}

/* Check if an address is valid - this only catches program exceptions to
 * determine if it's valid or not */
int HwS390CheckAddress(
    volatile const void *probe)
{
    PSW_DEFAULT_TYPE saved_psw;
    const PSW_DECL(pc_psw, &&invalid,
        PSW_DEFAULT_ARCHMODE
        | PSW_ENABLE_MCI);
    int r = 0;

#if (MACHINE >= M_ZARCH)
    KeCopyMemory(&saved_psw, (void *)PSA_FLCEPNPSW, sizeof(saved_psw));
    KeCopyMemory((void *)PSA_FLCEPNPSW, &pc_psw, sizeof(pc_psw));
#else
    KeCopyMemory(&saved_psw, (void *)PSA_FLCPNPSW, sizeof(saved_psw));
    KeCopyMemory((void *)PSA_FLCPNPSW, &pc_psw, sizeof(pc_psw));
#endif

    *((volatile const uint8_t *)probe);
    goto end;
invalid:
    r = -1;
end:
#if (MACHINE >= M_ZARCH)
    KeCopyMemory((void *)PSA_FLCEPNPSW, &saved_psw, sizeof(saved_psw));
#else
    KeCopyMemory((void *)PSA_FLCPNPSW, &saved_psw, sizeof(saved_psw));
#endif
    return r;
}

/* We are going to read in pairs of 1MiB and when we hit the memory limit we
 * will instantly catch the program exception and stop counting, then it's just
 * a matter of returning what we could count :) */
size_t HwS390GetMemorySize(
    void)
{
    const uint8_t *probe = (const uint8_t *)0x0 + PSA_SIZE;
    while(1) {
        int r;

        /* Do a "probe" read */
        r = HwS390CheckAddress(probe);
        if(r != 0) {
            KeDebugPrint("Done! %p\r\n", (uintptr_t)probe);
            break;
        }

        KeDebugPrint("Memory %p\r\n", (uintptr_t)probe);

        /* Go to next MiB */
        probe += 1048576;
    }
    return (size_t)probe;
}

/* Wait for an I/O response (overrides the I/O PSW) */
void HwS390WaitIo(
    void)
{
    const PSW_DECL(wait_io_psw, 0,
        PSW_DEFAULT_ARCHMODE
        | PSW_ENABLE_MCI
        | PSW_WAIT_STATE
        | PSW_IO_INT);
    
    const PSW_DECL(io_psw, &&after_wait,
        PSW_DEFAULT_ARCHMODE
        | PSW_ENABLE_MCI);

    /* The next I/O PSW to be set when the operation finishes */
#if (MACHINE >= M_ZARCH)
    KeCopyMemory((void *)PSA_FLCEINPSW, &io_psw, sizeof(io_psw));
    __asm__ __volatile__(
        "stosm %0, %1\r\n"
        :
        : "d"(PSA_FLCEINPSW), "d"(0x00)
        :);
#else
    KeCopyMemory((void *)PSA_FLCINPSW, &io_psw, sizeof(io_psw));
    __asm__ __volatile__(
        "stosm %0, %1\r\n"
        :
        : "d"(PSA_FLCINPSW), "d"(0x00)
        :);
#endif

    /* Set a PSW to wait the I/O response */
    __asm__ goto(
        "lpsw %0\r\n"
        :
        : "m"(wait_io_psw)
        :
        : after_wait);
    
    __builtin_unreachable();
after_wait:
    /* TODO: Restore old PSW */
    return;
}

/* Set a timer delta to trigger an interrupt in the specified ms */
#if (MACHINE >= M_ZARCH)
int64_t clock = 0;
#else
/* TODO: S390 can support the extended clock facility */
int32_t clock = 0;
#endif
int cpu_set_timer_delta_ms(
    int ms)
{
    __asm__ __volatile__(
        "stpt %0"
        : "=m"(clock)
        :
        :);
    
    clock = 64;

    __asm__ __volatile__(
        "spt %0"
        :
        : "m"(clock)
        :);
    return 0;
}