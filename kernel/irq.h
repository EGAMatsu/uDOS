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

struct irq_range * KeCreateIrqRange(size_t start, size_t n_lines, void (*alloc)(irq_t irq), void (*free)(irq_t irq));
irq_t  KeAllocateIrq(irq_handler_t *handler);
void KeFreeIrq(irq_t id);
void KeDestroyIrqRange(struct irq_range *range);

/* Assembly stubs */
#define KeAsmSupervisorCallHandler _Asvchdl
extern void KeAsmSupervisorCallHandler(void);
#define KeAsmProgramCheckHandler _Apchdl
extern void KeAsmProgramCheckHandler(void);
#define KeAsmExternalHandler _Aexthdl
extern void KeAsmExternalHandler(void);
#define KeAsmMachineCheckHandler _Amchdl
extern void KeAsmMachineCheckHandler(void);
#define KeAsmIOHandler _Aiohdl
extern void KeAsmIOHandler(void);

/* C counterparts */
#define KeSupervisorCallHandler _Zhsvc
void KeSupervisorCallHandler(void);
#define KeProgramCheckHandler _Zhpc
void KeProgramCheckHandler(void);
#define KeExternalHandler _Zhext
void KeExternalHandler(void);
#define KeMachineCheckHandler _Zhmc
void KeMachineCheckHandler(void);
#define KeIOHandler _Zhio
void KeIOHandler(void);

#endif
