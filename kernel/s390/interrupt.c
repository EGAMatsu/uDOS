#include <panic.h>
#include <printf.h>
#include <s390/interrupt.h>
#include <s390/context.h>

uintptr_t s390_do_svc(
    int code)
{
    __asm__ __volatile__(
        "svc %0"
        :
        : "d"(code)
        :);
}

#include <s390/asm.h>
#include <user.h>

#include <alloc.h>
#include <vfs.h>
void s390_supervisor_call_handler(
    void)
{
    int code = (int)(*(volatile uint16_t *)S390_FLCESICODE);
    struct s390_context *frame = (struct s390_context *)S390_FLCGRSAV;

    kprintf("SVC call (id %i)\r\n", code);

    switch(code) {
    case 1:
        kprintf("Yielding!\r\n");
        scheduler_schedule();
        break;
    /* Yield to scheduling */
    case 180: {
        
    } break;
    /* Get storage */
    case 190: {
        size_t size = (size_t)frame->r1;
        void *p;
        p = kmalloc(size);
        frame->r4 = (uintptr_t)p;
    } break;
    /* Drop storage */
    case 191: {
        void *p = (void *)frame->r1;
        kfree(p);
    } break;
    /* Resize storage */
    case 192: {
        size_t new_size = (size_t)frame->r2;
        void **p = (void **)frame->r1;

        *p = krealloc(*p, new_size);
    } break;
    /* Open VFS node */
    case 200: {
        struct vfs_handle *hdl;
        hdl = vfs_open((const char *)frame->r1, (int)frame->r2);
        frame->r4 = (uintptr_t)hdl;
    } break;
    /* Close VFS handle */
    case 201: {
        struct vfs_handle *hdl = (struct vfs_handle *)frame->r1;
        vfs_close(hdl);
    } break;
    /* Read FDSCB-mode in handle */
    case 202: {
        struct vfs_handle *hdl = (struct vfs_handle *)frame->r1;
        struct vfs_fdscb fdscb;
        memcpy(&fdscb, (struct vfs_fdscb *)frame->r3, sizeof(struct vfs_fdscb));
        /*vfs_read_fdscb(hdl, &fdscb, (size_t)frame->r2);*/
    } break;
    default:
        break;
    }
    return;
}

void s390_program_check_handler(
    void)
{
    arch_context_t *frame = (arch_context_t *)context_scratch_frame();

#if (MACHINE >= M_ZARCH)
    S390_PSW_DEFAULT_TYPE *old_pc_psw = (S390_PSW_DEFAULT_TYPE *)S390_FLCEPOPSW;
#else
    S390_PSW_DEFAULT_TYPE *old_pc_psw = (S390_PSW_DEFAULT_TYPE *)S390_FLCPOPSW;
#endif

    kprintf("Program Exception occoured at %p\r\n",
        (uintptr_t)old_pc_psw->address);

    switch(*((uint16_t *)S390_FLCPICOD)) {
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

void s390_external_handler(void)
{
    kprintf("Timer event fired up\r\n");
    return;
}