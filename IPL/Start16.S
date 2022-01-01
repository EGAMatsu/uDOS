.code16
.text

.global start
start:
	cli
	# Set segments
	xorw %ax, %ax
	movw %ax, %ss
	movw %ax, %ds
	
	# Set stack
	movw $stack_end, %sp
	cld
	
	# Old bioses improperly set DL
	cmp $0x00, %dl
	je old_bios
	
	movb %dl, disk_devnum
	
	# Query disk information
	movb $0x08, %ah
	int $0x13
	jc disk_err
	andw $0x3F, %cx
	movw %cx, disk_sectors_pt
	movzx %dh, %dx
	add $1, %dx
	mov %dx, disk_sides
old_bios:
	mov $0x00, %eax
loop:
	
	jmp loop

print:
	pushw %ax
	movb $0x0E, %ah
2:
	lodsb
	test %al, %al
	jz 1f
	int $0x10
	jmp 2b
1:
	popw %ax
	ret

disk_err:
	movw $disk_err_msg, %si
	call print
	
	cli
	hlt
1:	jmp 1b

disk_err_msg:
	.asciz "Disk error"

disk_devnum:
	.byte 0
disk_sectors_pt:
	.word 0
disk_sides:
	.word 0

.fill 0x1fe - (. - start), 1, 0
#.org 510
.word 0xAA55

stack_start:
.fill 4096, 1, 0
stack_end: