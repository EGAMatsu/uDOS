ENTRY(start);
SECTIONS {
    .text.head : {
        *(.head);
        . = 1024;
    }

    .text : {
        *(.text.*);
    }
    .rodata : {
        *(.rodata*);
    }
    .data : {
        *(.data*);
    }

    bss_start = .;
    .bss : {
        *(COMMON);
        *(.bss*);

        stack_bottom = .;
        . += 64K;
        stack_top = .;

        heap_start = .;
        . += 0xfffff;
        heap_end = .;
    }
    bss_end = .;
}
