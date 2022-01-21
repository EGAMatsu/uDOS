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
        KeDebugPrint("GR%u: %x\r\n", (unsigned int)i, (unsigned int)frame->gp_regs[i]);
    }
}

/* Symbols of the kernel (last updated 17:10 on January 16, 2022)*/
struct DebugSymData {
    void *addr;
    const char *name;
};

#include <memory.h>
#include <mm.h>

/* NOTE: These must be ordered in ascending order!! */
static struct DebugSymData sym_tab[] = {
    { (void *)0x0000, "FLCRNPSW" },
    { (void *)0x0008, "FLCROPSW" },

    { (void *)&_Zcrcpym, "KeCopyMemory" },
    { (void *)&_Zcrmovm, "KeMoveMemory" },
    { (void *)&_Zcrsetm, "KeSetMemory" },
    { (void *)&_Zcrcmpm, "KeCompareMemory" },
    { (void *)&_Zcrslen, "KeStringLength" },
    { (void *)&_Zcrcpstr, "KeCompareString" },
    { (void *)&_Zcrcmpse, "KeCompareStringEx" },
    { (void *)&_Zcrgcs, "KeGetCharPtrString" },
    { (void *)&_Zcrspns, "KeSpanString" },
    { (void *)&_Zcrbrkc, "KeBreakCharPtrString" },
    { (void *)&_Zcrcpys, "KeCopyString" },
    { (void *)&_Zcrbrkc, "KeCopyStringEx" },
    { (void *)&_Zcrcats, "KeConcatString" },
    { (void *)&_Zcrctse, "KeConcatStringEx" },
    { (void *)&_Zcrfss, "KeFindStringString" },
    { (void *)&_Zcrcvti, "KeConvertStringToInt" },

    { (void *)&_Zma, "MmAllocate" },
    { (void *)&_Zmaa, "MmAllocateArray" },
    { (void *)&_Zmaz, "MmAllocateZero" },
    { (void *)&_Zmalza, "MmAllocateZeroArray" },
    { (void *)&_Zmreal, "MmReallocate" },
    { (void *)&_Zmra, "MmReallocateArray" },
    { (void *)&_Zmfee, "MmFree" },

    { NULL, NULL }
};

static struct DebugSymData *DbgGetSymbol(void *addr)
{
    struct DebugSymData *sym = &sym_tab[0];
    struct DebugSymData *r_sym = &sym_tab[0];
    size_t i = 0;

    while(sym->name != NULL) {
        if((unsigned int)sym->addr < (unsigned int)addr && (unsigned int)sym->addr > (unsigned int)r_sym->addr) {
            r_sym = sym;
            continue;
        }
        sym = &sym_tab[++i];
    }
    return r_sym;
}

/* Information of the stack taken from http://mvs380.sourceforge.net/System380.txt
0 - unused by C, but PL/I or Cobol might use it
4 - backchain to previous save area
8 - forward chain to next save area
12 - R14
16 - R15
20 - R0
24 - R1
28 - R2
32 - R3
36 - R4
40 - R5
44 - R6
48 - R7
52 - R8
56 - R9
60 - R10
64 - R11
68 - R12
72 - unused but could be used to store a CRAB
76 - pointer to the top of the stack
80 - work area for compiler-generated code (CONVLO)
84 - work area for compiler-generated code (CONVHI)
88 - local variables begin
*/
int DbgUnwindStack(cpu_context *ctx_frame)
{
    extern uint32_t __stack[1024];
    uint8_t *frame = (uint8_t *)ctx_frame->r13;

    /* Corrupt stack pointer? */
    if((ptrdiff_t)frame < (ptrdiff_t)&__stack) {
        KeDebugPrint("Potential corrupt stack, (SP=%x)\r\n", (unsigned int)frame);
        frame = (uint8_t *)&__stack;
    }

    /* Pointer to top of the stack */
    while(frame != NULL) {
        /* R14 is stored on frame+12; and holds the address of the return point of the caller */
        uint32_t retaddr = *((uint32_t *)&frame[12]);
        struct DebugSymData *retsym = DbgGetSymbol((void *)retaddr);
        /* And R15 is also stored on frame+16, and contains the branching/base address of the callee */
        uint32_t calladdr = *((uint32_t *)&frame[16]);
        struct DebugSymData *callsym = DbgGetSymbol((void *)calladdr);

        if(retsym != NULL) {
            KeDebugPrint("RET=(%i=%s:%i)\r\n", (unsigned int)retaddr, retsym->name, (int)((ptrdiff_t)retaddr - (ptrdiff_t)retsym->addr));
        }

        if(callsym != NULL) {
            KeDebugPrint("CALL=(%x=%s:%i)\r\n", (unsigned int)calladdr, callsym->name, (int)((ptrdiff_t)calladdr - (ptrdiff_t)callsym->addr));
        }

        /* Get the forward chain */
        KeDebugPrint("%p\r\n", (unsigned int)frame);
        frame = *((uint8_t **)(&frame[8]));
    }
    return 0;
}
