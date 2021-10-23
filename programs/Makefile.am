bin_PROGRAMS = \
    cmdexec \
    forth

cmdexec_SOURCES = cmdexec.c
cmdexec_CFLAGS = -static -I$(top_srcdir)/dosrtl
cmdexec_LDFLAGS = -T$(top_srcdir)/programs/linker.ld
cmdexec_LDADD = $(top_builddir)/dosrtl/dosrtl.a

forth_SOURCES = forth.c
forth_CFLAGS = -static -I$(top_srcdir)/dosrtl
forth_LDFLAGS = -T$(top_srcdir)/programs/linker.ld
forth_LDADD = $(top_builddir)/dosrtl/dosrtl.a
