AC_INIT([uDOS],[0.1.1])

LDFLAGS="-nostdlib"
AC_SUBST([LDFLAGS])

# General options and warnings
CFLAGS="$CFLAGS -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations -Wdouble-promotion -Wredundant-decls -Wnested-externs -Winline -Wconversion -Wfatal-errors -pipe -std=c99"
CFLAGS="$CFLAGS -ffreestanding -fexec-charset=IBM-1047 -O2 -g"

ASFLAGS="$ASFLAGS -Wall -Wextra -g"

AM_INIT_AUTOMAKE([-Wall -Wextra foreign subdir-objects])

AC_PROG_CPP

# Store the original target in another variable :)
in_target=$target

# Obtain the target machine from the target environment variable this means we
# will override the target variable in order to define a more
# "autoconf"-friendly variable
case "$in_target" in
    x86_64-* | amd64*)
        target=x86; machine=amd64; machine_def=M_AMD64;
    ;;
    i*86-*)
        target=x86; machine=i386; machine_def=M_I386;
    ;;
    s3*0-* | s390x-* | zarch-*)
        # Kernel uses 31-bit mode regardless if it's an 360, 390 or a z/Arch
        # machine
        # TODO: We should use 24-bit and 64-bit mode as appropriate
        ASFLAGS="$ASFLAGS -m31"
        CFLAGS="$CFLAGS -m31"
        target=s390;
        case ${in_target} in
            s360*)
                machine=esa360; machine_def=M_S360;
            ;;
            s370*)
                machine=esa370; machine_def=M_S370;
            ;;
            s380*)
                machine=esa380; machine_def=M_S380;
            ;;
            s390*)
                machine=esa390; machine_def=M_S390;
            ;;
            s390x* | zarch*)
                machine=zarch; machine_def=M_ZARCH;
            ;;
            *)
                AC_MSG_ERROR([Unknown ESA S390 machine $in_target])
            ;;
        esac
    ;;
    mips*-*)
        target=mips; machine=mips32;
        case ${in_target} in
            mips32* | mips*)
                machine=mips32; machine_def=M_MIPS32;
            ;;
            mips64*)
                machine=mips64; machine_def=M_MIPS64;
            ;;
            *)
                AC_MSG_ERROR([Unknown MIPS machine $in_target])
            ;;
        esac
    ;;
    parisc*-* | hppa*-*)
        target=hppa; machine=ppc32;
        case ${in_target} in
            parisc32* | parisc* | hppa32* | hppa*)
                machine=ppc32; machine_def=M_HPPA32;
            ;;
            parisc64* | hppa64*)
                machine=ppc64; machine_def=M_HPPA64;
            ;;
            *)
                AC_MSG_ERROR([Unknown PA-RISC machine $in_target])
            ;;
        esac
    ;;
    ppc*-* | powerpc*-*)
        target=powerpc; machine=ppc32;
        case ${in_target} in
            ppc32* | ppc* | powerpc32* | powerpc*)
                machine=ppc32; machine_def=M_PPC32;
            ;;
            ppc64* | powerpc64*)
                machine=ppc64; machine_def=M_PPC64;
            ;;
            *)
                AC_MSG_ERROR([Unknown PowerPC machine $in_target])
            ;;
        esac
    ;;
    riscv*-*)
        target=riscv;
        case ${in_target} in
            riscv32* | riscv*)
                machine=riscv32; machine_def=M_RISCV32;
            ;;
            riscv64*)
                machine=riscv64; machine_def=M_RISCV64;
            ;;
            riscv128*)
                machine=riscv128; machine_def=M_RISCV128;
            ;;
            *)
                AC_MSG_ERROR([Unknown RISC-V machine $in_target])
            ;;
        esac
    ;;
    xtensa*-*)
        target=xtensa;
        machine=xtensa-esp32; machine_def=M_XTENSA_ESP32;
        #case ${in_target} in
        #    xtensa* | xtensa-esp32*)
        #        machine=xtensa-esp32; machine_def=M_XTENSA_ESP32;
        #    ;;
        #    xtensa-lx106*)
        #        machine=xtensa-lx106; machine_def=M_XTENSA_LX106;
        #    ;;
        #    *)
        #        AC_MSG_ERROR([Unknown xtensa machine $in_target])
        #    ;;
        #esac
    ;;
    *)
        AC_MSG_ERROR([Unknown target $in_target])
    ;;
esac

# Now define the macros in the C files depending on the machine we selected
# we obtained the target from the code above - we also have to do this
# since we need to define macros and we can only do it once otherwise
# the script may break, plus code duplication is not good
#
# So this is mostly for avoiding code duplication
case "$target" in
    x86)
        AC_DEFINE(TARGET_X86, 1,
            [Define if target architecture is an x86, x86_64 or any of the
            x86-type processors])
        
        AC_DEFINE(M_I386, 32, [x86_32 machine])
        AC_DEFINE(M_AMD64, 64, [x86_64 machine])
    ;;
    s390)
        AC_DEFINE(TARGET_S390, 1,
            [Define if target architecture is an ESA S3X0 (also includes z/Arch)])
        
        # Machines
        AC_DEFINE(M_S360, 360, [ESA S/360 machine])
        AC_DEFINE(M_S370, 370, [ESA S/370 machine])
        AC_DEFINE(M_S380, 380, [ESA S/380 machine])
        AC_DEFINE(M_S390, 390, [ESA S/390 machine])
        AC_DEFINE(M_ZARCH, 400, [z/Arch machine])
        
        # Generate the hercules configuration file
        memory_size="16"
        n_cpus="2"
        disk_file="udos00.cckd"
        case "$machine" in
            esa360)
                archmode="ESA S/360";
            ;;
            esa370)
                archmode="ESA S/370";
            ;;
            # Only available on Hercules-380
            esa380)
                archmode="ESA S/380";
            ;;
            esa390)
                archmode="ESA S/390";
            ;;
            zarch)
                archmode="Z/ARCH";
            ;;
        esac

        cat $srcdir/udos.cnf | awk '!/^#/ {
            if($1 == "NUMCPU") print "NUMCPU      '"$n_cpus"'";
            else if($1 == "ARCHLVL") print "ARCHLVL     '"$archmode"'"
            else if($1 == "MAINSIZE") print "MAINSIZE    '"$memory_size"'";
            else if($2 == "3390") print "01B9        3390        '"$disk_file"'";
            else if($1 != "") print $0; }' >hercules.cnf
        cat hercules.cnf
    ;;
    mips)
        AC_DEFINE(TARGET_MIPS, 1,
            [Define if target arch is MIPS])
        
        # Machines
        AC_DEFINE(M_MIPS32, 32, [MIPS 32-bit machine])
        AC_DEFINE(M_MIPS64, 64, [MIPS 64-bit machine])
    ;;
    riscv)
        AC_DEFINE(TARGET_RISCV, 1,
            [Define if target arch is RISC-V])
        
        # Machines
        AC_DEFINE(M_RISCV32, 32, [RISC-V 32-bit machine])
        AC_DEFINE(M_RISCV64, 64, [RISC-V 64-bit machine])
        AC_DEFINE(M_RISCV128, 128, [RISC-V 128-bit machine])
    ;;
    hppa)
        AC_DEFINE(TARGET_HPPA, 1,
            [Define if target arch is PA-RISC])
        
        # Machines
        AC_DEFINE(M_HPPA32, 32, [PA-RISC 32-bit machine])
        AC_DEFINE(M_HPPA64, 64, [PA-RISC 64-bit machine])
    ;;
    powerpc)
        AC_DEFINE(TARGET_POWERPC, 1,
            [Define if target arch is PowerPC])
        
        # Machines
        AC_DEFINE(M_PPC32, 32, [PPC 32-bit machine])
        AC_DEFINE(M_PPC64, 64, [PPC 64-bit machine])
    ;;
    xtensa)
        AC_DEFINE(TARGET_XTENSA, 1,
            [Define if target arch is Xtensa])
        
        # Machines
        AC_DEFINE(M_XTENSA_ESP32, 32, [Xtensa microchip])
        AC_DEFINE(M_XTENSA_LX106, 106, [Xtensa microchip])
    ;;
    *)
        AC_MSG_ERROR([Unknown target $target])
    ;;
esac

AM_CONDITIONAL(TARGET_X86, [test $target = x86])
AM_CONDITIONAL(TARGET_S390, [test $target = s390])
AM_CONDITIONAL(TARGET_MIPS, [test $target = mips])
AM_CONDITIONAL(TARGET_RISCV, [test $target = riscv])
AM_CONDITIONAL(TARGET_POWERPC, [test $target = powerpc])
AM_CONDITIONAL(TARGET_HPPA, [test $target = hppa])
AM_CONDITIONAL(TARGET_XTENSA, [test $target = xtensa])

AC_DEFINE_UNQUOTED(MACHINE, [$machine_def],
    [The current machine type we are using from the architecture])

# We are going to use flatboot's IPL for more convenience
case "$target" in
    s390)
        mkdir -p wget_cache

        flatboot_ver=0.1.2
        case "$machine_def" in
            M_ZARCH)
                ipl_arch=ZARCH
            ;;
            M_S390)
                ipl_arch=S390
            ;;
            *)
                AC_MSG_ERROR([No flatboot IPL available for $machine_def])
            ;;
        esac

        flatboot_stage1=stage1_$ipl_arch.txt
        flatboot_stage2=stage2_$ipl_arch.bin

        # Obtain the file only if it does not already exist
        if [ ! -f wget_cache/$flatboot_stage1 ]; then
            wget https://github.com/udos-project/flatboot/releases/download/$flatboot_ver/$flatboot_stage1
            mv $flatboot_stage1 wget_cache/$flatboot_stage1
        fi
        cp wget_cache/$flatboot_stage1 stage1.txt

        if [ ! -f wget_cache/$flatboot_stage2 ]; then
            wget https://github.com/udos-project/flatboot/releases/download/$flatboot_ver/$flatboot_stage2
            mv $flatboot_stage2 wget_cache/$flatboot_stage2
        fi
        cp wget_cache/$flatboot_stage2 stage2.bin
    ;;
    *)
        #AC_MSG_ERROR([No IPL available for $target])
    ;;
esac

AC_PROG_RANLIB
AM_PROG_AS
AC_PROG_CC

# Check for the prescense of essential freestanding headers
m4_warn([obsolete],
[The preprocessor macro `STDC_HEADERS' is obsolete.
  Except in unusual embedded environments, you can safely include all
  ISO C90 headers unconditionally.])dnl
# Autoupdate added the next two lines to ensure that your configure
# script's behavior did not change.  They are probably safe to remove.
AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP

# Create symbolic link for current architecture
AC_CONFIG_LINKS([doskrnl/arch:$srcdir/doskrnl/$target])
CFLAGS="$CFLAGS -Iarch -I$srcdir/../libc"

AC_CONFIG_FILES([Makefile dosrtl/Makefile doskrnl/Makefile programs/Makefile])
AC_OUTPUT
