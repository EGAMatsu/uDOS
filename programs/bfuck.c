/* bfuck.c
 *
 * Sample Brainfuck interpreter
 */

#include <Rtl.h>
#include <Error.h>

LIST_TYPE(JumpStack, int);

/* Interpret a dataset containing brainfuck code */
int BfInterpret(
    Dataset *ds,
    unsigned char *tape,
    size_t max_size)
{
    JumpStack jmp_stack = {0};
    int curr_idx = 0;
    int ch;

    while((ch = RtlReadDatasetChar(ds)) != EOF) {
        switch(ch) {
        /* Incremenent and decrement operators */
        case '-':
            tape[curr_idx]--;
            break;
        case '+':
            tape[curr_idx]++;
            break;
        /* Pointer switching */
        case '>':
            curr_idx++;
            if(curr_idx >= (int)max_size) {
                curr_idx = 0;
            }
            break;
        case '<':
            curr_idx--;
            if(curr_idx < 0) {
                curr_idx = (int)max_size - 1;
            }
            break;
        case '[':
            LIST_PUSH(jmp_stack, &curr_idx, 0);
            if(jmp_stack.n_elems >= 1000) {
                RtlPrintError("Jump stack overflow\r\n");
                return -1;
            }

            /* If the pointer at the tape is zero go to the next matching ]
             * command on the stream */
            if(tape[curr_idx] == 0) {
                int depth = 1;

                /* Find matching ] */
                while((ch = RtlReadDatasetChar(ds)) != EOF
                && depth > 0) {
                    if(ch == '[') {
                        depth++;
                    } else if(ch ==']') {
                        depth--;
                    }
                }

                /* No match found (EOF found) */
                if(depth != 0) {
                    RtlPrintError("No matching ] found\r\n");
                    return -1;
                }
            }
            break;
        case ']':
            /* If the value is non-zero jump back to the matching [ */
            if(tape[curr_idx] != 0) {
                curr_idx = jmp_stack.elems[jmp_stack.n_elems - 1];
                RtlSeekDataset(ds, curr_idx);
            }

            if(jmp_stack.n_elems == 0) {
                RtlPrintError("Jump stack underflow\r\n");
                return -1;
            }
            jmp_stack.n_elems--;
            break;
        default:
            break;
        }
    }
    return 0;
}

int PgMain(
    ExecParams *exec)
{
    size_t i;

    RtlDebugPrint("SAMPLE.001 - Brainfuck interpreter\r\n");

    if(exec->n_dsnames == 0) {
        RtlDebugPrint("Please specify atleast 1 dataset\r\n");
    }

    /* Iterate over the execution given dataset names */
    for(i = 0; i < exec->n_dsnames; i++) {
        Dataset *ds;
        unsigned char *tape;
        size_t max_size = 8192;

        tape = RtlAllocateMemory((size_t)max_size);
        if(tape == NULL) {
            RtlPrintError("No memory for tape\r\n");
            return -1;
        }

        /* Open datasets in read-only mode */
        ds = RtlOpenDataset(exec->dsnames[i], "R");

        BfInterpret(ds, tape, max_size);

        RtlCloseDataset(ds);
        RtlFreeMemory(tape);
    }
    return 0;
}
