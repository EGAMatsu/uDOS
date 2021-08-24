ENTRY(start);
SECTIONS {
    . = 0x1000;
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
        . += 8K;
        stack_top = .;
    }
    bss_end = .;
    disk_buffer = .;
}
