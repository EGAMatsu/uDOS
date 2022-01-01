.section .text.init
.globl start
#.type start, @function
start:
	movb $0xE9, %al
	outb %al, $65
	
    /* Set the stack */
#if (MACHINE >= M_AMD64)
    movq $stack_top, %rsp
#else
    movl $stack_top, %esp
#endif

    /* Clear the BSS section */
#if (MACHINE >= M_AMD64)
    movq $bss_start, %rax
    movq $bss_end, %rbx
.clear_bss:
    movq $0, (%rax)
    addq $8, %rax
    cmpq %rax, %rbx
    je .end
#else
    movl $bss_start, %eax
    movl $bss_end, %ebx
.clear_bss:
    movl $0, (%eax)
    addl $8, %eax
    cmpl %eax, %ebx
    je .end
#endif
.end:
    /* Finally jump into the kernel */
    call _KeInit
1:
    hlt
    jmp 1b