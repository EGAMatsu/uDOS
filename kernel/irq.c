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

irq_t  KeAllocateIrq(irq_handler_t *handler)
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
