ENTRY(start);

SECTIONS {
    .text : ALIGN(4K) {
        *(.text.init);
        *(.text*);
    }

    .rodata : ALIGN(4K) {
        *(.rodata*);
    }
    
    .data : ALIGN(4K) {
        *(.data*);
    }

    .bss : ALIGN(4K) {
        PROVIDE(bss_start = .);
        *(COMMON);
        *(.bss*);

        PROVIDE(stack_bottom = .);
        . += 4K;
        PROVIDE(stack_top = .);
        PROVIDE(bss_end = .);
    }

    PROVIDE(heap_start = .);
}
