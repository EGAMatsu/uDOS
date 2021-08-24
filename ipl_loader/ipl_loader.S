#
# This is a gnu assembler re-implementation of PDOS's sapstart.asm
# with a special touch from me, of course ;)
#

.macro ccw0 cmd=0, addr=0, flags=0, length=0
    .byte \cmd
    .short (\addr & 0xffff)
    .byte ((\addr & 0xff0000) >> 16)
    .byte \flags
    .byte 0
    .short \length
.endm

.macro ccw1 cmd=0, addr=0, flags=0, length=0
    .byte \cmd
    .byte \flags
    .short \length
    .long \addr
.endm

.equ AMBIT, 0x80000000
.equ FLCIOA, 0xB8

.equ FLCSNPSW, 0x60
.equ FLCPNPSW, 0x68
.equ FLCINPSW, 0x78
.equ FLCMNPSW, 0x70

# TODO: Set this value to something correct
.equ FLCCAW, 0x00

.section .ipl_boot
.quad 0x000C000080000400

.section .text
.global start
.type start, @function
start:
    mvc FLCINPSW(8), waiter1
    mvc FLCMNPSW(8), waiter2
    mvc FLCSNPSW(8), waiter3
    mvc FLCPNPSW(8), waiter4

    # Obtain the IPL device address and save it on r1 because store
    # subchannel uses r1 as device address register which uses the
    # higher half for ID and the lower half for the device number
    lhi %r1, 0
    icm %r1, 0b1111, FLCIOA
#ifdef __s390__
    lctl 6, 6, all_io_int
#endif

io_int_jmp:
    lhi %r2, 0
    lhi %r3, 1
    lhi %r4, 1
    larl %r5, disk_buffer
    lhi %r6, 32767

# Read a block from the DASD
# r1 = Device number (subchannel id)
# r2 = Head
# r3 = Cylinder
# r4 = Record
# r5 = Buffer
# r6 = Size of buffer
read_block:
    # Store from stack
    lr %r12, %r15
    lr %r10, %r1
    stcm %r2, 0b0011, rb_cc1 # Cylinder
    stcm %r2, 0b0011, rb_cc2
    stcm %r3, 0b0011, rb_hh1 # Head
    stcm %r3, 0b0011, rb_hh2
    stc %r4, rb_record # Record
    lr %r2, %r5 # Buffer
    lr %r7, %r6 # Bytes to read
#ifdef __s390__
    st %r2, rb_ldccw + 4
    sth %r7, rb_ldccw + 2
/* s360, s370 and s380 */
#else
    stcm %r2, 0b0111, rb_ldccw + 1
    sth %r7, rb_ldccw + 6
#endif
    mvc FLCINPSW(8), rb_newio
    stosm FLCINPSW, 0x00
    la %r3, rb_seek
    st %r3, FLCCAW
#ifdef __s390__
    lr %r1, %r10
    la %r9, rb_irb
    tsch 0(%r9)
    la %r10, rb_orb
    ssch 0(%r10)
#else
    sio 0(%r10)
#endif
    lpsw rb_wtnoer

# New IO code will jump here
.align 4
rb_count:
#ifdef __s390__
    tsch 0(%r9)
    sh %r7, 10(%r9)
    lr %r15, %r7
    clc 4(4, %r9), rb_end_chain
#else
    sh %r7, FLCCSW + 6
    lr %r15, %r7
    clc FLCCSW(4), rb_end_chain
#endif
    be rb_end
rb_end:
    b disk_buffer

# Reports fatal error then hangs
fatal_error:
    larl %r0, .err_msg
    lhi %r1, 16
    diag %r0, %r1, 8
.short 0

.align 8
/* s390 */
#if defined(__s390__)
rb_seek:
    ccw1 7, rb_bbcchh, 0x40, 6
rb_search:
    ccw1 0x31, rb_cchhr, 0x40, 5
    ccw1 8, rb_search, 0, 0
rb_ldccw:
    ccw1 0x0e, 0, 0x20, 32767
/* s360, s370 and s380 */
#else
rb_seek:
    ccw0 7, rb_bbcchh, 0x40, 6
rb_search:
    ccw0 0x31, rb_cchhr, 0x40, 5
    ccw0 8, rb_search, 0, 0
rb_ldccw:
    ccw0 0x0e, 0, 0x20, 32767
#endif
rb_end_chain:

.align 2
rb_bbcchh:
    .short 0
rb_cc1:
    .short 0 # 2 uninit bytes
rb_hh1:
    .short 0 # 2 uninit bytes
rb_cchhr:
rb_cc2:
    .short 0 # 2 uninit bytes
rb_hh2:
    .short 0 # 2 uninit bytes
rb_record:
    .byte 0

.align 8
rb_wtnoer:
    .long 0x060E0000
    .long AMBIT

rb_newio:
    .long 0x000C0000
    .long (AMBIT + rb_count)

waiter1:
    .long 0x000E0000
    .long (AMBIT + 0x00000111)
waiter2:
    .long 0x000E0000
    .long (AMBIT + 0x00000222)
waiter3:
    .long 0x000E0000
    .long (AMBIT + 0x00000333)
waiter4:
    .long 0x000E0000
    .long (AMBIT + 0x00000444)

.align 4
all_io_int:
    .long 0xFF000000

.align 4
rb_orb:
    .long 0x00
    .long 0x0080FF00
    .long rb_seek
    .long 0 # 5, 4 bytes of 0
    .long 0
    .long 0
    .long 0
    .long 0

.align 4
rb_irb:
    .long 0 # 24, 4 bytes of 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0

# EBCDIC message of "MSG * Disk error"
.err_msg:
.ascii "\xD4\xE2\xC7\x40\x5C\x40\xC4\x89\xA2\x92\x40\x85\x99\x99\x96\x99"

.align 4096
disk_buffer:
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .quad 0
