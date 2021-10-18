#ifndef X86_CONTEXT_H
#define X86_CONTEXT_H

#include <stdint.h>

typedef struct x86_context {
    union {
        uint64_t gp_regs[16];
        struct {
            uint64_t r0;
            uint64_t r1;
            uint64_t r2;
            uint64_t r3;
            uint64_t r4;
            uint64_t r5;
            uint64_t r6;
            uint64_t r7;
            uint64_t r8;
            uint64_t r9;
            uint64_t r10;
            uint64_t r11;
            uint64_t r12;
            uint64_t r13;
            uint64_t r14;
            uint64_t r15;
        };
    };
}arch_context_t;

/* The scratch frame is an abstract memory area representing where the
 * registers at the time of an interruption are stored at, this is so
 * the scheduler can retrieve them */
arch_context_t *context_scratch_frame(void);

#endif