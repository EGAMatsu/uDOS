ENTRY(start);

MEMORY {
    ram (rwx) : ORIGIN = 0xF000, LENGTH = 256K
}

SECTIONS {
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
        . += 8K;
        stack_top = .;

        bss_end = .;
    } >ram

    heap_start = .;
}
