/* S390 Disassembler and debugger kernel module */

#include <stdint.h>
#include <memory.h>
#include <printf.h>
#include <cpu.h>

struct dasm_breakpoint {
    void *addr;
};

int DbgInstIFormat(void **ptr, char *buf, const char *name)
{
    unsigned char **c_ptr = (unsigned char **)ptr;
    KeConcatString(buf, name);

    /*sprintf();*/
}

/* NOTE: Atleast a buffer of 40 characters needs to be passed */
int DbgDisasm(void *ptr, char *buf)
{
    unsigned char *c_ptr = (unsigned char *)ptr;
    uint8_t op = *(c_ptr++);

    buf[0] = '\0';

    switch(op) {
    case 0x1A: {
        KeCopyString(buf, "AR");
    } break;
    case 0x5A: {
        KeCopyString(buf, "A");
    } break;
    default:
        break;
    }
}

void DbgPrintFrame(cpu_context *frame)
{
    register size_t i;
    
    KeDebugPrint("GR00: %x GR01: %x GR02: %x GR03: %x\r\n", frame->r0, frame->r1, frame->r2, frame->r3);
    KeDebugPrint("GR04: %x GR05: %x GR06: %x GR07: %x\r\n", frame->r4, frame->r5, frame->r6, frame->r7);
    KeDebugPrint("GR08: %x GR09: %x GR10: %x GR11: %x\r\n", frame->r8, frame->r9, frame->r10, frame->r11);
    KeDebugPrint("GR12: %x GR13: %x GR14: %x GR15: %x\r\n", frame->r12, frame->r13, frame->r14, frame->r15);
    
    for(i = 0; i < 16; i++) {
        KeDebugPrint("GR%u: %x\r\n", (unsigned)i, (unsigned)frame->gp_regs[i]);
    }
}

/* Information of the stack taken from http://mvs380.sourceforge.net/System380.txt */
int DbgUnwindStack(cpu_context *frame)
{
    uint8_t *stack = (uint8_t *)frame->r13;
    uint32_t backchain;
    uint32_t top_stack;
    
    /* Pointer to top of the stack */
    backchain = (uint32_t *)(&stack[4]);
    top_stack = (uint32_t *)(&stack[76]);
    KeDebugPrint("%p %p\r\n", (unsigned int)backchain, (unsigned int)top_stack);
    KeDebugPrint("%u %u\r\n", (unsigned int)backchain, (unsigned int)top_stack);
    
    stack = &stack[top_stack];
    backchain = (uint32_t *)(&stack[4]);
    top_stack = (uint32_t *)(&stack[76]);
    KeDebugPrint("%p %p\r\n", (unsigned int)backchain, (unsigned int)top_stack);
    KeDebugPrint("%u %u\r\n", (unsigned int)backchain, (unsigned int)top_stack);
    return 0;
}