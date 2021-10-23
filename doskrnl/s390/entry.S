/* ESA 390 boot code, this entry code is independent from z/Arch or 360
 * it should run on whatever machine flawlessly */
#include <s390/cpu.h>

.section .text.init
.globl start
.type start, @function
start:
    # Clear bss
    lhi %r0, 0
    larl %r1, bss_start
    larl %r2, bss_end
.loop:
    stc %r0, %r1
    ahi %r1, 1
    cr %r1, %r2
    brne .loop

    # Set the stack pointer
    larl %r15, stack_bottom
    la %r11, %r15
    brasl %r14, kinit
1:
    j 1b