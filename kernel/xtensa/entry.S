.section .text.init
.globl start
.type start, @function
start:
    # Clear the bss section
    movi a8, ((bss_end - bss_start) / 8)
    l32r a9, bss_start
    movi.n a10, 0
    slli a8, a8, 3
    loop a8, .end_loop
.loop:
    s32i.n a10, a9, 0
    addi.n a9, a9, 4
.end_loop:

    # Set the stack
    movi sp, stack_top

    l32r a8, kinit
    callx8 a8