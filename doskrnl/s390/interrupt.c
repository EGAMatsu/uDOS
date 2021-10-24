#include <debug/panic.h>
#include <debug/printf.h>
#include <s390/interrupt.h>
#include <s390/context.h>

#include <s390/asm.h>
#include <user.h>

#include <mm/mm.h>
#include <fs/fs.h>

uintptr_t s390_do_svc(
    int code)
{
    __asm__ __volatile__(
        "svc %0"
        :
        : "d"(code)
        :);
}

void s390_supervisor_call_handler(
    void)
{
    int16_t code;
    int8_t ilc;
    arch_context_t *frame = (arch_context_t *)PSA_FLCGRSAV;
#if (MACHINE >= M_ZARCH)
#error No handling
#else
    struct s390_psw *old_psw = (struct s390_psw *)PSA_FLCSOPSW;
#endif

    code = (*(volatile int16_t *)PSA_FLCSVCN);
    ilc = (*(volatile int8_t *)PSA_FLCSVILC);

#if defined(DEBUG)
    kprintf("SVC call (id %i) (len=%i) from %p\r\n", (int)code, (int)ilc,
        old_psw->address);
    kprintf("R0: %p R1: %p R2: %p R3: %p R4: %p\r\n", frame->r0, frame->r1,
        frame->r2, frame->r3, frame->r4);
    kprintf("R5: %p R6: %p R7: %p R8: %p R9: %p\r\n", frame->r5, frame->r6,
        frame->r7, frame->r8, frame->r9);
    kprintf("SC: %p FP: %p GP: %p BP: %p RA: %p SP: %p\r\n", frame->r10,
        frame->r11, frame->r12, frame->r13, frame->r14, frame->r15);
#endif

    switch((uint16_t)frame->r4) {
    case 1:
        KeSchedule();
        kprintf("Yielding!\r\n");
        break;
    /* Request simple prompt */
    case 90: {
        KeReadFsNode(g_stdin_fd, (const char *)frame->r1, (size_t)frame->r2);
    } break;
    /* Print debug */
    case 100: {
        kprintf("%s\r\n", (const char *)frame->r1);
    } break;
    /* Yield to scheduling */
    case 180: {
        
    } break;
    /* Get storage */
    case 190: {
        size_t size = (size_t)frame->r1;
        void *p;
        p = MmAllocate(size);
        frame->r4 = (uintptr_t)p;
    } break;
    /* Drop storage */
    case 191: {
        void *p = (void *)frame->r1;
        MmFree(p);
    } break;
    /* Resize storage */
    case 192: {
        size_t new_size = (size_t)frame->r2;
        void **p = (void **)frame->r1;

        *p = MmReallocate(*p, new_size);
    } break;
    /* Open VFS node */
    case 200: {
        struct FsHandle *hdl;
        hdl = KeOpenFsNode((const char *)frame->r1, (int)frame->r2);
        frame->r4 = (uintptr_t)hdl;
    } break;
    /* Close VFS handle */
    case 201: {
        struct FsHandle *hdl = (struct FsHandle *)frame->r1;
        KeCloseFsNode(hdl);
    } break;
    /* Read FDSCB-mode in handle */
    case 202: {
        struct FsHandle *hdl = (struct FsHandle *)frame->r1;
        struct FsFdscb fdscb;
        KeCopyMemory(&fdscb, (struct FsFdscb *)frame->r3, sizeof(struct FsFdscb));
        /*KeReadWithFdscbFsNode(hdl, &fdscb, (size_t)frame->r2);*/
    } break;
    default:
        break;
    }
    return;
}

void s390_program_check_handler(
    void)
{
    arch_context_t *frame = (arch_context_t *)HwGetScratchContextFrame();

#if (MACHINE >= M_ZARCH)
    PSW_DEFAULT_TYPE *old_pc_psw = (PSW_DEFAULT_TYPE *)PSA_FLCEPOPSW;
#else
    PSW_DEFAULT_TYPE *old_pc_psw = (PSW_DEFAULT_TYPE *)PSA_FLCPOPSW;
#endif

    kprintf("Program Exception occoured at %p\r\n",
        (uintptr_t)old_pc_psw->address);

    switch(*((uint16_t *)PSA_FLCPICOD)) {
    case 0x0001:
        kprintf("Operation\r\n");
        break;
    case 0x0002:
        kprintf("Privileged operation\r\n");
        break;
    case 0x0003:
        kprintf("Execute\r\n");
        break;
    case 0x0004:
        kprintf("Protection\r\n");
        break;
    case 0x0005:
        kprintf("Addressing\r\n");
        break;
    case 0x0006:
        kprintf("Specification\r\n");
        break;
    case 0x0007:
        kprintf("Data\r\n");
        break;
    case 0x0008:
        kprintf("Fixed-point Overflow\r\n");
        break;
    case 0x0009:
        kprintf("Fixed-point Divide\r\n");
        break;
    case 0x000A:
        kprintf("Decimal Overflow\r\n");
        break;
    case 0x000B:
        kprintf("Decimal Divide\r\n");
        break;
    case 0x000C:
        kprintf("HFP Overflow\r\n");
        break;
    case 0x000D:
        kprintf("HFP Underflow\r\n");
        break;
    case 0x000E:
        kprintf("HFP Significance\r\n");
        break;
    case 0x000F:
        kprintf("HFP Divide\r\n");
        break;
    case 0x0010:
        kprintf("Segment Translation\r\n");
        break;
    case 0x0011:
        kprintf("Page Translation\r\n");
        break;
    case 0x0012:
        kprintf("Translation Specification\r\n");
        break;
    case 0x0013:
        kprintf("Special Operation\r\n");
        break;
    default:
        kprintf("Unknown %zu\r\n", 0);
        break;
    }

    kprintf("R0: %p R1: %p R2: %p R3: %p R4: %p\r\n", frame->r0, frame->r1,
        frame->r2, frame->r3, frame->r4);
    kprintf("R5: %p R6: %p R7: %p R8: %p R9: %p\r\n", frame->r5, frame->r6,
        frame->r7, frame->r8, frame->r9);
    kprintf("SC: %p FP: %p GP: %p BP: %p RA: %p SP: %p\r\n", frame->r10,
        frame->r11, frame->r12, frame->r13, frame->r14, frame->r15);

    while(1);
    return;
}

void s390_external_handler(
    void)
{
    kprintf("Timer event fired up\r\n");
    while(1);
    return;
}