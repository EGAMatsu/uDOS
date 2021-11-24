/* S390 Disassembler and debugger kernel module */

#include <stdint.h>
#include <memory.h>
#include <debug/printf.h>

struct dasm_breakpoint {
    void *addr;
};

int dasm_inst_i_fmt(
    void **ptr,
    char *buf,
    const char *name)
{
    unsigned char **c_ptr = (unsigned char **)ptr;
    KeConcatString(buf, name);

    //sprintf();
}

/* NOTE: Atleast a buffer of 40 characters needs to be passed */
int dasm_at_addr(
    void *ptr,
    char *buf)
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
