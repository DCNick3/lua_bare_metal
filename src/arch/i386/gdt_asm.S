.section .text

.global gdt_flush
.type gdt_flush, @function

gdt_flush:
	mov 4(%esp), %eax
	lgdt 0(%eax)

	# Init all data segments
	mov $0x10, %eax 
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	
	ljmp $0x08, $gdt_flush_exit
gdt_flush_exit:
	ret

.global idt_flush
.type idt_flush, @function

idt_flush:
	mov 4(%esp), %eax
	lidt 0(%eax)
	ret

.macro isr_noerr id=0
.global isr\id
.type isr\id , @function
isr\id :
	cli
	push $0
	push $\id
	jmp isr_common_handler
.endm

.macro isr_err id=0
.global isr\id
.type isr\id , @function
isr\id :
	cli
	push $\id
	jmp isr_common_handler
.endm

isr_noerr 0
isr_noerr 1
isr_noerr 2
isr_noerr 3
isr_noerr 4
isr_noerr 5
isr_noerr 6
isr_noerr 7
isr_err 8
isr_noerr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr_noerr 15
isr_noerr 16
isr_noerr 17
isr_noerr 18
isr_noerr 19
isr_noerr 20
isr_noerr 21
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_noerr 29
isr_noerr 30
isr_noerr 31

.altmacro

.macro  isrs from=0, to=5

.global isr\from
.type isr\from , @function
isr\from :
	cli
	push $0
	push $\from
	jmp isr_common_handler

.if     \to-\from
isrs     %(\from+1) , \to
.endif
.endm

isrs 32, 132
isrs 133, 233
isrs 234, 255

.macro  isrns from=0, to=5
.long isr\from
.if     \to-\from
isrns     %(\from+1) , \to
.endif
.endm

.global isrns
isrns:
	isrns 0, 100
	isrns 101, 201
	isrns 202, 255


isr_common_handler:
	pusha

	xor %eax, %eax
	mov %ds, %ax
	push %eax

	mov $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %gs
	mov %ax, %fs

	mov 0x24(%esp), %eax
	push %eax
	mov 0x2c(%esp), %eax
	push %eax

	call isr_handler

	add $0x8, %esp
	pop %eax

	mov %ds, %ax
	mov %es, %ax
	mov %fs, %ax
	mov %gs, %ax

	popa
	add $0x8, %esp

	sti
	iret

