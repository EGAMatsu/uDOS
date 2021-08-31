ENTRY(start);
SECTIONS {
    .text : {
        *(.text.init);
        *(.text*);
    }
    .rodata : {
        *(.rodata*);
    }
    .data : {
        *(.data*);
    }

    .bss : {
        bss_start = .;
        *(COMMON);
        *(.bss*);

        stack_bottom = .;
        . += 8K;
        stack_top = .;
        bss_end = .;
    }

    heap_start = .;
    . += 0x80000;
    heap_end = .;
}
