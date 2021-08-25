ENTRY(start);
SECTIONS {
    .text : {
        *(.text.*);
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
        . += 64K;
        stack_top = .;

        heap_start = .;
        . += 0xffff;
        heap_end = .;
        bss_end = .;
    }
}
