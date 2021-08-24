.section .text
.ascii "\xE7\xE7\xE7"

.global start
.type start, @function
start:
1:  lhi %r6, 0x66
    j 1b

    # Set the stack pointer
    larl %r15, stack_top

    # Clear bss
    lhi %r0, 0
    larl %r1, bss_start
    larl %r2, bss_end
.loop:
    stc %r0, %r1
    ahi %r1, 1
    cr %r1, %r2
    brne .loop

    brasl %r14, kmain
1:
    j 1b
