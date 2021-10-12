bin_PROGRAMS = kernel
kernel_SOURCES = \
    irq.c \
    kernel.c \
    string.c \
    alloc.c \
    mutex.c \
    panic.c \
    pmm.c \
    printf.c \
    registry.c \
    user.c \
    vfs.c \
    scheduler.c \
    fs/zdsfs.c \
    s390/entry.S \
    s390/css.c \
    s390/cpu.c \
    s390/mmu.c \
    s390/ibm3390.c \
    s390/ibm3270.c \
    s390/interrupt.c \
    s390/hdebug.c \
    s390/context.c \
    s390/int_handler.S \
    s390/kinit.c

kernel_CFLAGS = \
    -fno-stack-protector \
    -mwarn-framesize=8192 \
    -mwarn-dynamicstack \
    -mpacked-stack

kernel_LDFLAGS = \
    -Wl,-static,--no-dynamic-linker,-ztext \
    -z max-page-size=0x1000 \
    -T$(top_srcdir)/kernel/linker.ld