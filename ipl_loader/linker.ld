ENTRY(start);

SECTIONS {
    .text : {
        . = 0;
        *(.ipl_boot);
        . = 1024;
        *(.text.*);
    }
    .rodata : {
        *(.rodata*);
    }
    .data : {
        *(.data*);
    }
    .bss : {
        *(COMMON);
        *(.bss*);
    }
}
