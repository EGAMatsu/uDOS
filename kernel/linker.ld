ENTRY(start);

MEMORY {
    ram (rwx) : ORIGIN = 0x2000, LENGTH = 256K
}

SECTIONS {
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
        . += 8K;
        stack_top = .;
        bss_end = .;
    } >ram

    /* Heap */
    .data : ALIGN(4K) {
        heap_start = .;
        . += 0x8000;
        heap_end = .;
    } >ram
}
