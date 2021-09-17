bin_PROGRAMS = kernel
kernel_SOURCES = \
    entry.S \
    irq.c \
    kernel.c \
    alloc.c \
    mutex.c \
    panic.c \
    pmm.c \
    printf.c \
    registry.c \
    string.c \
    user.c \
    vfs.c \
    cpu.c \
    s390/css.c \
    s390/cpu.c \
    s390/mmu.c \
    s390/dasd.c \
    s390/interrupt.c \
    s390/int_handler.S

kernel_LDFLAGS = -T$(top_srcdir)/kernel/linker.ld