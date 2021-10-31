/* ESA 390 boot code, this entry code is independent from z/Arch or 360
 * it should run on whatever machine flawlessly */
#include <s390/cpu.h>

.section .text.init
.globl start
.type start, @function
start:
    # Clear bss
    LHI %r0, 0
    LARL %r1, bss_start
    LARL %r2, bss_end
.loop:
    STC %r0, %r1
    AHI %r1, 1
    CR %r1, %r2
    BRNE .loop

    # Set the stack pointer
    LARL %r15, stack_bottom
    LA %r11, %r15
    BRASL %r14, kinit
1:
    J 1b