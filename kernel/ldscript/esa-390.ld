ENTRY(start);

MEMORY {
    psa : ORIGIN = 0x0000, LENGTH = 8192
    ram (rwx) : ORIGIN = 0x9000, LENGTH = 64K
}

SECTIONS {
    /* Low memory data, for example here we have the exception register scratch
     * area - since stmg and lmg can only address 12 bits of data at a time we have
     * to place low-memory symbols on an a below 8K barrier */

    .text : ALIGN(4) {
        *(.text.init);
        *(.text);
        *(.text.*);
    } >ram

    .rodata : ALIGN(4) {
        *(.rodata);
        *(.rodata.*);
    } >ram
    
    .data : ALIGN(4) {
        *(.data);
        *(.data.*);
        *(.sdata);
        *(.sdata.*);
    } >ram

    .bss : ALIGN(4) {
        bss_start = .;
        *(COMMON);
        *(.bss);
        *(.bss.*);

        stack_bottom = .;
        . += 4K;
        stack_top = .;

        bss_end = .;
    } >ram

    heap_start = .;
}
