/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */
 
/* 
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .text.init
.globl start
#.type start, @function
start:
	movb $0x378, %al
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