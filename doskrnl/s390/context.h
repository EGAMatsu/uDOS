#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>
#include <s390/asm.h>

#if (MACHINE >= M_ZARCH)
typedef uint64_t S390Register;
#else
/* TODO: This is wrong */
typedef uint32_t S390Register;
#endif

typedef struct S390Context {
    union {
        S390Register gp_regs[16];
        struct {
            S390Register r0;
            S390Register r1;
            S390Register r2;
            S390Register r3;
            S390Register r4;
            S390Register r5;
            S390Register r6;
            S390Register r7;
            S390Register r8;
            S390Register r9;
            S390Register r10;
            S390Register r11;
            S390Register r12;
            S390Register r13;
            S390Register r14;
            S390Register r15;
        };
    };

    PSW_DEFAULT_TYPE psw;
}arch_context_t;

/* The scratch frame is an abstract memory area representing where the
 * registers at the time of an interruption are stored at, this is so
 * the scheduler can retrieve them */
arch_context_t *HwGetScratchContextFrame(void);

#endif