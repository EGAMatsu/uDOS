/* irq.c
 *
 * Implements algorithms for managing IRQ lines of a system - not nescesarily
 * the x86 IRQ lines are the ones controlled by this system, those can also be
 * table-like-IRQ based systems (i.e RISC-V with the vector table)
 * 
 * The IRQ "hardware" assignation and other stuff that directly talks to the
 * hardware is dependent on the arch's irq.c and irq.h
 */

#include <mm.h>
#include <irq.h>
#include <panic.h>
#include <registry.h>

struct KeIrqTable {
    struct irq_range *ranges;
    size_t n_ranges;
};
static struct KeIrqTable g_irq_table = {0};

static struct irq_range *KeAddIrqRange(struct irq_range *range)
{
    g_irq_table.ranges = MmReallocateArray(g_irq_table.ranges, g_irq_table.n_ranges + 1, sizeof(struct irq_range));
    
    if(g_irq_table.ranges == NULL) {
        KePanic("Out of memory");
    }
    KeCopyMemory(&g_irq_table.ranges[g_irq_table.n_ranges++], range, sizeof(struct irq_range));
    return &g_irq_table.ranges[g_irq_table.n_ranges - 1];
}

struct irq_range * KeCreateIrqRange(size_t start, size_t n_lines, void (*alloc)(irq_t irq), void (*free)(irq_t irq))
{
    struct irq_range *range;
    range = MmAllocateZero(sizeof(struct irq_range));
    if(range == NULL) {
        KePanic("Out of memory");
    }

    range->n_lines = n_lines;
    range->start = start;

    range->lines = MmAllocateZeroArray(range->n_lines, sizeof(struct irq_line));
    if(range->lines == NULL) {
        KePanic("Out of memory");
    }
    range->alloc = alloc;
    range->free = free;
    return KeAddIrqRange(range);
}

static irq_handler_t * KeAddHandlerToIrqLine(struct irq_line *line, irq_handler_t *handler)
{
    line->handlers = MmReallocateArray(line->handlers, line->n_handlers + 1, sizeof(struct irq_line));
    if(line->handlers == NULL) {
        KePanic("Out of memory");
    }
    KeCopyMemory(&line->handlers[line->n_handlers++], handler, sizeof(irq_handler_t));
    return &line->handlers[line->n_handlers - 1];
}

static irq_t  KeAllocateIrqFromRange(struct irq_range *range, irq_handler_t *handler)
{
    size_t i;
    for(i = 0; i < range->n_lines; i++) {
        struct irq_line *line = &range->lines[i];

        /* We cannot allocate on private lines */
        if(line->type == IRQ_TYPE_PRIVATE) {
            continue;
        }

         KeAddHandlerToIrqLine(line, handler);
        return (irq_t)i + (irq_t)range->start;
    }
    return (irq_t)-1;
}

irq_t KeAllocateIrq(irq_handler_t *handler)
{
    size_t i;
    for(i = 0; i < g_irq_table.n_ranges; i++) {
        struct irq_range *range = &g_irq_table.ranges[i];
        irq_t irq;

        irq =  KeAllocateIrqFromRange(range, handler);
        if(irq == (irq_t)-1) {
            continue;
        }

        /* Sucessfully allocated handler - we are going to call the "alloc"
         * function for this range to notify whichever driver controls
         * this range of the new allocation */
        if(range->alloc == NULL) {
            KePanic("IRQ range has no allocator handler");
        }
        range->alloc(irq);
        return irq;
    }
    return (irq_t)-1;
}

void KeFreeIrq(irq_t irq)
{
    size_t i;
    for(i = 0; i < g_irq_table.n_ranges; i++) {
        struct irq_range *range = &g_irq_table.ranges[i];

        /* Check that id is between the range */
        if(irq >= range->start && irq <= range->start + range->n_lines) {
            MmFree(range->lines[irq - range->start].handlers);
            range->lines[irq - range->start].n_handlers = 0;
            range->lines[irq - range->start].type = IRQ_TYPE_SHARED;

            if(range->free == NULL) {
                KePanic("IRQ range has no deallocator handler");
            }
            range->free(irq);
            return;
        }
    }
}

void KeDestroyIrqRange(struct irq_range *range)
{
    MmFree(range->lines);
    MmFree(range);
    return;
}

#include <fs.h>
#include <mm.h>
#include <pmm.h>
#include <schedule.h>
#include <cpu.h>
#include <printf.h>
void KeSupervisorCallHandler(void)
{
    cpu_context *frame = (cpu_context *)HwGetScratchContextFrame();
    int16_t code;
    int8_t ilc;
#if (MACHINE > 390u)
#error No handling
#else
    struct s390_psw *old_psw = (struct s390_psw *)PSA_FLCSOPSW;
#endif

    code = (*(volatile int16_t *)PSA_FLCSVCN);
    ilc = (*(volatile int8_t *)PSA_FLCSVILC);

#if defined(DEBUG)
    KeDebugPrint("SVC call (id %i) (len=%i) from %p\r\n", (int)code, (int)ilc, old_psw->address);
#endif
    /* MVS Compatibility */
    if(code == 120 || code == 10) {
        /* getmain */
        if((code == 10 && (signed int)frame->r1 < 0) || (code == 120 && frame->r1 == 0)) {
            size_t len;
            void *p;

            len = (size_t)frame->r0;
            if(code == 10) {
                len &= 0xffffff;
            }

            p = MmAllocate(len);
            frame->r15 = (p == NULL) ? 4 : 0;
            frame->r1 = (register_t)p;
        }
        /* freemain */
        else {
            MmFree((void *)frame->r1);
        }
        return;
    }

    switch((uint16_t)frame->r4) {
    case 50:
        KeSchedule();
        break;
    /* Request simple prompt */
    case 90: {
        KeReadFsNode(g_stdin_fd, (const char *)frame->r1, (size_t)frame->r2);
    } break;
    /* Print debug */
    case 100: {
        KeDebugPrint("%s\r\n", (const char *)frame->r1);
    } break;
    /* Yield to scheduling */
    case 180: {
        
    } break;
    /* Get storage */
    case 190: {
        frame->r4 = (unsigned int)MmAllocate((size_t)frame->r1);
    } break;
    /* Drop storage */
    case 191: {
        MmFree((void *)frame->r1);
    } break;
    /* Resize storage */
    case 192: {
        void **p = (void **)frame->r1;
        *p = MmReallocate(*p, (size_t)frame->r2);
    } break;
    /* Open VFS node */
    case 200: {
        struct fs_handle *hdl;
        hdl = KeOpenFsNode((const char *)frame->r1, (int)frame->r2);
        frame->r4 = (unsigned int)hdl;
    } break;
    /* Close VFS handle */
    case 201: {
        KeCloseFsNode((struct fs_handle *)frame->r1);
    } break;
    /* Read FDSCB-mode in handle */
    case 202: {
        struct fs_handle *hdl = (struct fs_handle *)frame->r1;
        struct fs_fdscb fdscb;
        KeCopyMemory(&fdscb, (struct fs_fdscb *)frame->r3, sizeof(struct fs_fdscb));
        /*KeReadWithFdscbFsNode(hdl, &fdscb, (size_t)frame->r2);*/
    } break;
    default:
        break;
    }
    return;
}

#include <panic.h>
#include <printf.h>
#include <cpu.h>
#include <asm.h>
#include <user.h>
#include <mm.h>
#include <fs.h>
void KeProgramCheckHandler(void)
{
    cpu_context *frame = (cpu_context *)HwGetScratchContextFrame();
#if (MACHINE > 390u)
    PSW_DEFAULT_TYPE *old_pc_psw = (PSW_DEFAULT_TYPE *)PSA_FLCEPOPSW;
#else
    PSW_DEFAULT_TYPE *old_pc_psw = (PSW_DEFAULT_TYPE *)PSA_FLCPOPSW;
#endif
    KeDebugPrint("Program Exception occoured at %p\r\n", (unsigned int)old_pc_psw->address);
    
    switch(*((uint16_t *)PSA_FLCPICOD)) {
    case 0x0001:
        KeDebugPrint("Operation\r\n");
        break;
    case 0x0002:
        KeDebugPrint("Privileged operation\r\n");
        break;
    case 0x0003:
        KeDebugPrint("Execute\r\n");
        break;
    case 0x0004:
        KeDebugPrint("Protection\r\n");
        break;
    case 0x0005:
        KeDebugPrint("Addressing\r\n");
        break;
    case 0x0006:
        KeDebugPrint("Specification\r\n");
        break;
    case 0x0007:
        KeDebugPrint("Data\r\n");
        break;
    case 0x0008:
        KeDebugPrint("Fixed-point Overflow\r\n");
        break;
    case 0x0009:
        KeDebugPrint("Fixed-point Divide\r\n");
        break;
    case 0x000A:
        KeDebugPrint("Decimal Overflow\r\n");
        break;
    case 0x000B:
        KeDebugPrint("Decimal Divide\r\n");
        break;
    case 0x000C:
        KeDebugPrint("HFP Overflow\r\n");
        break;
    case 0x000D:
        KeDebugPrint("HFP Underflow\r\n");
        break;
    case 0x000E:
        KeDebugPrint("HFP Significance\r\n");
        break;
    case 0x000F:
        KeDebugPrint("HFP Divide\r\n");
        break;
    case 0x0010:
        KeDebugPrint("Segment Translation\r\n");
        break;
    case 0x0011:
        KeDebugPrint("Page Translation\r\n");
        break;
    case 0x0012:
        KeDebugPrint("Translation Specification\r\n");
        break;
    case 0x0013:
        KeDebugPrint("Special Operation\r\n");
        break;
    default:
        KeDebugPrint("Unknown %zu\r\n", 0);
        break;
    }
    
    DbgPrintFrame(frame);
    DbgUnwindStack(frame);
    
    KePanic("*** PC Exception\r\n");
    return;
}

void KeMachineCheckHandler(void)
{
    KeDebugPrint("*** Machine check ***\r\n");
    return;
}

void KeExternalHandler(void)
{
    KeDebugPrint("*** Timer event fired up\r\n");
    return;
}

void KeIOHandler(void)
{
    KeDebugPrint("*** I/O Interruption!\r\n");
    return;
}
