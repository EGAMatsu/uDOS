.section .text.init
.globl start
.type start, @function
start:
#if (MACHINE >= M_AMD64)
/* 64-bit */
    movq $bss_start, %rax
    movq $bss_end, %rbx
.clear_bss:
    movq $0, (%rax)
    addq $8, %rax
    cmpq %rax, %rbx
    je .end
.end:
    movq $stack_top, %rsp
#else
/* 32-bit */
    movl $bss_start, %eax
    movl $bss_end, %ebx
.clear_bss:
    movl $0, (%eax)
    addl $8, %eax
    cmpl %eax, %ebx
    je .end
.end:
    movl $stack_top, %esp
#endif

    /* Finally jump into the kernel */
    jmp kinit