bin_PROGRAMS = \
    hello \
    cmdexec \
    forth \
    bfuck

hello_SOURCES = hello.c
hello_CFLAGS = -static -I$(top_srcdir)/dosrtl
hello_LDFLAGS = -T$(top_srcdir)/programs/linker.ld
hello_LDADD = $(top_builddir)/dosrtl/dosrtl.a

cmdexec_SOURCES = cmdexec.c
cmdexec_CFLAGS = -static -I$(top_srcdir)/dosrtl
cmdexec_LDFLAGS = -T$(top_srcdir)/programs/linker.ld
cmdexec_LDADD = $(top_builddir)/dosrtl/dosrtl.a

forth_SOURCES = forth.c
forth_CFLAGS = -static -I$(top_srcdir)/dosrtl
forth_LDFLAGS = -T$(top_srcdir)/programs/linker.ld
forth_LDADD = $(top_builddir)/dosrtl/dosrtl.a

bfuck_SOURCES = bfuck.c
bfuck_CFLAGS = -static -I$(top_srcdir)/dosrtl
bfuck_LDFLAGS = -T$(top_srcdir)/programs/linker.ld
bfuck_LDADD = $(top_builddir)/dosrtl/dosrtl.a
