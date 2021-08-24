#include <irq.h>
#include <malloc.h>
#include <panic.h>
#include <registry.h>

struct irq_table {
  struct irq_range *ranges;
  size_t n_ranges;
};

static struct irq_table g_irq_table = {NULL, 0};

static struct irq_range *irq_add_range(struct irq_range *range) {
  g_irq_table.ranges = krealloc_array(
      g_irq_table.ranges, g_irq_table.n_ranges + 1, sizeof(struct irq_range));
  if (g_irq_table.ranges == NULL) {
    kpanic("Out of memory");
  }
  memcpy(&g_irq_table.ranges[g_irq_table.n_ranges++], range,
         sizeof(struct irq_range));
  return &g_irq_table.ranges[g_irq_table.n_ranges - 1];
}

struct irq_range *irq_create_range(size_t start, size_t n_lines,
                                   void (*alloc)(irq_t irq),
                                   void (*free)(irq_t irq)) {
  struct irq_range *range;
  range = kzalloc(sizeof(struct irq_range));
  if (range == NULL) {
    kpanic("Out of memory");
  }

  range->n_lines = n_lines;
  range->start = start;

  range->lines = kzalloc_array(range->n_lines, sizeof(struct irq_line));
  if (range->lines == NULL) {
    kpanic("Out of memory");
  }
  range->alloc = alloc;
  range->free = free;
  return irq_add_range(range);
}

static irq_handler_t *irq_add_handler_to_line(struct irq_line *line,
                                              irq_handler_t *handler) {
  line->handlers = krealloc_array(line->handlers, line->n_handlers + 1,
                                  sizeof(struct irq_line));
  if (line->handlers == NULL) {
    kpanic("Out of memory");
  }
  memcpy(&line->handlers[line->n_handlers++], handler, sizeof(irq_handler_t));
  return &line->handlers[line->n_handlers - 1];
}

static irq_t irq_alloc_from_range(struct irq_range *range,
                                  irq_handler_t *handler) {
  size_t i;
  for (i = 0; i < range->n_lines; i++) {
    struct irq_line *line = &range->lines[i];

    /* We cannot allocate on private lines */
    if (line->type == IRQ_TYPE_PRIVATE) {
      continue;
    }

    irq_add_handler_to_line(line, handler);
    return (irq_t)i + (irq_t)range->start;
  }
  return (irq_t)-1;
}

irq_t irq_alloc(irq_handler_t *handler) {
  size_t i;
  for (i = 0; i < g_irq_table.n_ranges; i++) {
    struct irq_range *range = &g_irq_table.ranges[i];
    irq_t irq;

    irq = irq_alloc_from_range(range, handler);
    if (irq == (irq_t)-1) {
      continue;
    }

    /* Sucessfully allocated handler - we are going to call the "alloc"
     * function for this range to notify whichever driver controls
     * this range of the new allocation */
    if (range->alloc == NULL) {
      kpanic("IRQ range has no allocator handler");
    }
    range->alloc(irq);
    return irq;
  }
  return (irq_t)-1;
}

void irq_free(irq_t irq) {
  size_t i;
  for (i = 0; i < g_irq_table.n_ranges; i++) {
    struct irq_range *range = &g_irq_table.ranges[i];

    /* Check that id is between the range */
    if (irq >= range->start && irq <= range->start + range->n_lines) {
      kfree(range->lines[irq - range->start].handlers);
      range->lines[irq - range->start].n_handlers = 0;
      range->lines[irq - range->start].type = IRQ_TYPE_SHARED;

      if (range->free == NULL) {
        kpanic("IRQ range has no deallocator handler");
      }
      range->free(irq);
      return;
    }
  }
}

void irq_destroy_range(struct irq_range *range) {
  kfree(range->lines);
  kfree(range);
  return;
}