/* S390 Disassembler and debugger kernel module */

#include <stdint.h>
#include <memory.h>
#include <printf.h>
#include <context.h>

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