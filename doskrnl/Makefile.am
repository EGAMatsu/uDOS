# ESA 390 specific files
if TARGET_S390
arch_src = \
    s390/entry.S \
    s390/css.c \
    s390/cpu.c \
    s390/mmu.c \
    s390/interrupt.c \
    s390/hdebug.c \
    s390/context.c \
    s390/handlers.S \
    s390/kinit.c \
    s390/x2703.c \
    s390/x3390.c \
    s390/x3270.c \
    s390/disasm.c

arch_cflags = \
    -mwarn-framesize=8192 \
    -mwarn-dynamicstack \
    -mpacked-stack

arch_ldflags = \
    -T$(top_srcdir)/doskrnl/platform/esa390.ld
endif
if TARGET_XTENSA
arch_src = \
    xtensa/entry.S \
    xtensa/context.c \
    xtensa/kinit.c

arch_ldflags = \
    -T$(top_srcdir)/doskrnl/platform/xtensa-virt.ld
endif
if TARGET_X86
arch_src = \
    x86/entry.S \
    x86/context.c \
    x86/kinit.c

arch_ldflags = \
    -T$(top_srcdir)/doskrnl/platform/x86-pc.ld
endif

bin_PROGRAMS = doskrnl
doskrnl_SOURCES = \
    irq.c \
    kernel.c \
    memory.c \
    mutex.c \
    registry.c \
    user.c \
    scheduler.c \
    loader/elf.c \
    loader/pe.c \
    debug/panic.c \
    debug/printf.c \
    debug/ubsan.c \
    fs/fs.c \
    fs/zdsfs.c \
    comm/bsc.c \
    mm/mm.c \
    mm/pmm.c \
    $(arch_src)

doskrnl_CFLAGS = \
    -fno-stack-protector \
    $(arch_cflags)

doskrnl_LDFLAGS = \
    -Wl,-static,--no-dynamic-linker,-ztext \
    -z max-page-size=0x1000 \
    $(arch_ldflags)