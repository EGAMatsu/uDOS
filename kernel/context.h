#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>
#include <asm.h>

#if (MACHINE > 390u)
typedef uint64_t register_t;
#else
/* TODO: This is wrong */
typedef uint32_t register_t;
#endif

typedef struct _cpu_context {
    union {
        register_t gp_regs[16];
        struct {
            register_t r0;
            register_t r1;
            register_t r2;
            register_t r3;
            register_t r4;
            register_t r5;
            register_t r6;
            register_t r7;
            register_t r8;
            register_t r9;
            register_t r10;
            register_t r11;
            register_t r12;
            register_t r13;
            register_t r14;
            register_t r15;
        };
    };

    PSW_DEFAULT_TYPE psw;
}cpu_context;

/* The scratch frame is an abstract memory area representing where the
 * registers at the time of an interruption are stored at, this is so
 * the scheduler can retrieve them */
cpu_context *HwGetScratchContextFrame(void);

#include <scheduler.h>
void HwSwitchThreadContext(struct scheduler_thread *old_thread, struct scheduler_thread *new_thread);

#endif
