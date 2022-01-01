ENTRY(start);

SECTIONS {
	. = 1M;
    .text : ALIGN(4K) {
		*(.multiboot);
        *(.text.init);
        *(.text);
        *(.text.*);
    }

    .rodata : ALIGN(4K) {
        *(.rodata);
        *(.rodata.*);
    }
    
    .data : ALIGN(4K) {
        *(.data);
        *(.data.*);
        *(.sdata);
        *(.sdata.*);
    }
	
	bss_start = .;
    .bss : ALIGN(4K) {
        *(COMMON);
        *(.bss);
        *(.bss.*);

        stack_bottom = .;
        . += 8K;
        stack_top = .;
    }
	bss_end = .;
    heap_start = .;
}
