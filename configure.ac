AC_INIT([uDOS], [1.0], [jesusantonio30122016@gmail.com])
LDFLAGS="-Wl,-static,--no-dynamic-linker,-ztext -nostdlib -z max-page-size=0x1000"

AC_SUBST([LDFLAGS])
CFLAGS="$CFLAGS -ffreestanding -fno-stack-protector -pipe"
CFLAGS="$CFLAGS -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations -Wdouble-promotion -Wredundant-decls -Wnested-externs -Winline -Wconversion -fexec-charset=IBM-1047 -O0 -m31 -g"
ASFLAGS="$ASFLAGS -Wall -Wextra -fexec-charset=IBM-1047 -m31 -g"

AM_INIT_AUTOMAKE([-Wall -Wextra foreign subdir-objects])

AM_PROG_AS
AC_PROG_CC

AC_CHECK_HEADERS([stdint.h stddef.h stdarg.h])

AM_CONDITIONAL(IS_X86_64, [test "${TARGET}" == "x86_64-elf"])
AM_CONDITIONAL(IS_I386, [test "${TARGET}" == "i686-elf" || "${TARGET}" == "i486-elf" || "${TARGET}" == "i386-elf"])
AM_CONDITIONAL(IS_XTENSA, [test "${TARGET}" == "xtensa-elf"])
AM_CONDITIONAL(IS_AARCH64, [test "${TARGET}" == "aarch64-elf-noabi"])
AM_CONDITIONAL(IS_S390X, [test "${TARGET}" == "s390-linux"])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile kernel/Makefile programs/Makefile ipl/Makefile])
AC_OUTPUT
