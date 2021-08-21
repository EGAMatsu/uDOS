#ifndef IRQ_H
#define IRQ_H
#ifdef __cplusplus
extern "C" {
#endif

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

struct irq_range *irq_create_range(size_t start, size_t n_lines, void (*alloc)(irq_t irq), void (*free)(irq_t irq));
irq_t irq_alloc(irq_handler_t *handler);
void irq_free(irq_t id);
void irq_destroy_range(struct irq_range *range);

#ifdef __cplusplus
}
#endif
#endif