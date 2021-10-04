ENTRY(start);

MEMORY {
    psa : ORIGIN = 0x0000, LENGTH = 8192
    ram (rwx) : ORIGIN = 0x4000, LENGTH = 256K
}

SECTIONS {
    /* Low memory data, for example here we have the exception register scratch
     * area - since stmg and lmg can only address 12 bits of data at a time we have
     * to place low-memory symbols on an a below 8K barrier */

    .text : ALIGN(4K) {
        *(.text.init);
        *(.text*);
    } >ram

    .rodata : ALIGN(4K) {
        *(.rodata*);
    } >ram
    .data : ALIGN(4K) {
        *(.data*);
    } >ram

    .bss : ALIGN(4K) {
        bss_start = .;
        *(COMMON);
        *(.bss*);

        stack_bottom = .;
        . += 16K;
        stack_top = .;
        bss_end = .;
    } >ram

    heap_start = .;
}
