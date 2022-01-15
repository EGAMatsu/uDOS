#ifndef IRQ_H
#define IRQ_H

#include <stddef.h>
#include <mutex.h>

typedef unsigned int irq_t;
typedef void (*irq_handler_t)(size_t id);

typedef enum _irq_type {
    IRQ_TYPE_SHARED = 0,
    IRQ_TYPE_PRIVATE = 1
}irq_type_t;

struct irq_line {
    irq_type_t type;
    irq_handler_t *handlers;
    size_t n_handlers;
};

struct irq_range {
    struct irq_line *lines;
    size_t n_lines;
    size_t start;
    void (*alloc)(irq_t irq);
    void (*free)(irq_t irq);
};

struct irq_range * KeCreateIrqRange(size_t start, size_t n_lines,
    void (*alloc)(irq_t irq), void (*free)(irq_t irq));
irq_t  KeAllocateIrq(irq_handler_t *handler);
void KeFreeIrq(irq_t id);
void KeDestroyIrqRange(struct irq_range *range);

#endif
