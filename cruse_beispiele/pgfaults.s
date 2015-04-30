//-----------------------------------------------------------------
//	pgfaults.s	(Adapted from our 'vrampage.s' example)
//
//	This example illustrates several 'page-fault' error-codes
//	which result from attempts to access a 'Not-Present' page
//	or from attempts to access a 'Supervisor-Only' page while 
//	executing code at privilege-level 3.  Here the page-fault 
//	handler displays the address of the faulting instruction,
//	obtained from register CR2, and the 'error-code' that was
//	pushed onto the ring0 stack in response to the exception.
//	(A user then hits any key to continue the demonstration.)
//
//	 to assemble:  $ as pgfaults.s -o pgfaults.o
//	 and to link:  $ ld pgfaults.o -T ldscript -o pgfaults.b
//	 and install:  $ dd if=pgfaults.b of=/dev/sda4 seek=1
//
//	NOTE: This code begins executing with CS:IP = 1000:0002.
//
//	programmer: ALLAN CRUSE
//	written on: 10 NOV 2008
//-----------------------------------------------------------------

	.equ	realCS, 0x1000		# arena's segment-address 

	.section	.text
#------------------------------------------------------------------
	.word	0xABCD			# our loader expects this
#------------------------------------------------------------------

main:	.code16				# begins in x86 real-mode
	
	mov	%sp, %cs:ipltos+0	# preserve loader's SP
	mov	%ss, %cs:ipltos+2	# preserve loader's SS

	mov	%cs, %ax		# address program arena
	mov	%ax, %ss		#   using SS register
	lea	tos, %sp		# establish a new stack

	call	init_IDT_descriptors
	call	create_paging_tables
	call	enter_protected_mode
	call	execute_program_demo 
	call	leave_protected_mode

	lss	%cs:ipltos, %sp		# recover the former stack
	lret				# return control to loader
#------------------------------------------------------------------
ipltos:	.word	0, 0
#------------------------------------------------------------------
theGDT:	.quad	0x0000000000000000	# required null-descriptor

	.equ	sel_cs, (.-theGDT)	# code-segment's selector
	.quad	0x00009A010000FFFF	# code-segment descriptor

	.equ	sel_ds, (.-theGDT)	# data-segment's selector
	.quad	0x000092010000FFFF	# data-segment descriptor

	.equ	sel_es, (.-theGDT)	# vram-segment's selector
	.quad	0x000092000000FFFF	# vram-segment descriptor

	.equ	sel_fs, (.-theGDT)	# flat-segment's selector
	.quad	0x008F92000000FFFF	# flat-segment descriptor

	.equ	privCS, (.-theGDT)+0	# code-segment's selector
	.quad	0x00409A010000FFFF	# code-segment descriptor

	.equ	privDS, (.-theGDT)+0	# data-segment's selector
	.quad	0x004092010000FFFF	# data-segment descriptor

	.equ	userCS, (.-theGDT)+3	# code-segment's selector
	.quad	0x0040FA014000FFFF	# code-segment descriptor

	.equ	userDS, (.-theGDT)+3	# data-segment's selector
	.quad	0x0040F2015000FFFF	# data-segment descriptor

	.equ	userES, (.-theGDT)+3	# vram-segment's selector
	.quad	0x0040F2000000FFFF	# vram-segment descriptor

	.equ	userSS, (.-theGDT)+3	# stak-segment's selector
	.quad	0x0040F6016000FFFF	# stak-segment descriptor

	.equ	ring0, (.-theGDT)	# selector for call-gate
	.word	finish, sel_cs, 0xEC00, 0x0000	# gate-descriptor

	.equ	selTSS, (.-theGDT)	# task-segment's selector
	.word	limTSS, theTSS, 0x8901, 0x0000	# task-descriptor

	.equ	limGDT, (.-theGDT)-1	# our GDT's segment-limit 
#------------------------------------------------------------------
regGDT:	.word	limGDT, theGDT, 0x0001	# image for GDTR register
#------------------------------------------------------------------
regCR3:	.long	16*realCS + pgdir	# image for CR3 register 
#------------------------------------------------------------------
create_paging_tables:

	# setup segment-register DS to address program's arena
	mov	%cs, %ax		# address paging tables
	mov	%ax, %ds		#   using DS register

	#------------------------------------
	# initialize five page-table entries 
	#------------------------------------

	# setup mapping for virtual-address 0x00000000 
	mov	$0x00, %edi		# for virtual page 0x00 
	mov	$0xB8000, %eax		# physical page-address
	or	$0x003, %eax		# present+writable+user
	mov	%eax, pgtbl(,%edi,4)	# write the table-entry

	# setup mapping for virtual-address 0x00010000 
	mov	$0x10, %edi		# for virtual page 0x10
	mov	$0x10000, %eax		# physical page-address
	or	$0x003, %eax		# present and writable
	mov	%eax, pgtbl(,%edi,4)	# write the table-entry

	# setup mapping for virtual-address 0x00011000 
	mov	$0x11, %edi		# for virtual page 0x11
	mov	$0x11000, %eax		# physical page-address
	or	$0x003, %eax		# present and writable
	mov	%eax, pgtbl(,%edi,4)	# write the table-entry

	# setup mapping for virtual-address 0x00012000 
	mov	$0x12, %edi		# for virtual page 0x12
	mov	$0x12000, %eax		# physical page-address
	or	$0x003, %eax		# present and writable
	mov	%eax, pgtbl(,%edi,4)	# write the table-entry

	# setup mapping for virtual-address 0x00013000 
	mov	$0x13, %edi		# for virtual page 0x13
	mov	$0x13000, %eax		# physical page-address
	or	$0x003, %eax		# present and writable
	mov	%eax, pgtbl(,%edi,4)	# write the table-entry

	#------------------------------------
	# initialize one page-drectory entry  
	#------------------------------------

	# setup the page-directory to use our page-table
	mov	$0x10000, %ebx		# arena physical-address
	lea	pgtbl(%ebx), %eax	# pgtbl physical-address
	or	$0x007, %eax		# present+writable+user 
	mov	%eax, pgdir		# page-directory entry 0

	ret
#------------------------------------------------------------------
enter_protected_mode:

	cli				# interrupts disabled

	mov	%cr0, %eax		# current machine status
	bts	$0, %eax		# turn on PE-bit's image
	mov	%eax, %cr0		# enable protected mode

	lgdt	%cs:regGDT		# establish our GDT
	lidt	%cs:regIDT		# establish our GDT

	ljmp	$sel_cs, $pm		# reload CS register
pm:	
	mov	$sel_ds, %ax
	mov	%ax, %ss		# reload SS register

	xor	%ax, %ax		# purge invalid values
	mov	%ax, %ds		#   from DS register
	mov	%ax, %es		#   from ES register
	mov	%ax, %fs		#   from FS register
	mov	%ax, %gs		#   from GS register
	ret
#------------------------------------------------------------------
execute_program_demo:

	mov	%esp, %ss:tossav+0	# preserve ESP
	mov	%ss,  %ss:tossav+4	# preserve SS

	mov	$privDS, %ax
	mov	%ax, %ss
	and	$0xFFFFFFF0, %esp
	mov	%esp, %ss:theTSS+4	# setup image for ESP0
	mov	%ss,  %ss:theTSS+8	# setup image for SS0
	
	# setup page-directory address in control register CR3
	mov	%cs:ptdb, %eax		# page-directory address
	mov	%eax, %cr3		# goes into CR3 register

	# turn on paging (by setting bit #31 in register CR0)
	mov	%cr0, %eax		# current machine status
	bts	$31, %eax		# turn on PG-bit's image
	mov	%eax, %cr0		# enable page-mappings  
	jmp	.+2			# flush prefetch queue 

	# establish Task-State Segment
	mov	$selTSS, %ax
	ltr	%ax

	# now transfer control to 32-bit code in 'ring3'  
	pushl	$userSS			# image for SS
	pushl	$0			# image for ESP
	pushl	$userCS			# image for CS
	pushl	$draw_message		# image for EIP
	lretl				# transfer to ring3
finish:
	# disable paging (by clearing bit #31 in register CR0)
	mov	%cr0, %eax		# current machine status
	btr	$31, %eax		# reset PG-bit's image
	mov	%eax, %cr0		# disable page-mapping
	jmp	.+2			# flush prefetch queue

	# invalidate the CPU's Translation Lookaside Buffer
	xor	%eax, %eax		# setup "dummy" value
	mov	%eax, %cr3		# and write it to CR3

	# recover the saved 16-bit stack (for return to 'main')
	lss	%cs:tossav, %esp	# recover 16-bit stack
	ret
#------------------------------------------------------------------
tossav:	.long	0, 0			# storage for SS and ESP
#------------------------------------------------------------------
leave_protected_mode:

	mov	$sel_ds, %ax		# insure 'writable' 64KB
	mov	%ax, %ds		#   in DS hidden cache
	mov	%ax, %es		#   in ES hidden cache
	mov	%ax, %fs		#   in FS hidden cache
	mov	%ax, %gs		#   in GS hidden cache

	mov	%cr0, %eax		# current machine status
	btr	$0, %eax		# reset PE-bit's image
	mov	%eax, %cr0		# disable protected-mode

	ljmp	$realCS, $rm		# reload CS register
rm:	mov	%cs, %ax
	mov	%ax, %ss		# reload SS register

	lidt	%cs:regIVT		# real-mode vector-table
	sti				# reenable interrupts
	ret
#------------------------------------------------------------------
ptdb:	.long	16*realCS + pgdir 	# pgdir: physical address 
#------------------------------------------------------------------
theIDT:	.zero	16 * 8			# enough for 16 IDT gates
	.equ	limIDT, (.-theIDT)-1	# our IDT's segment-limit 
#------------------------------------------------------------------
regIDT:	.word	limIDT, theIDT, 0x0001	# image for IDTR register 
regIVT:	.word	0x03FF, 0x0000, 0x0000	# image for IDTR register
#------------------------------------------------------------------
init_IDT_descriptors:

	# install interrupt-gate for Page-Fault exceptions
	mov	$0x0E, %ebx		# ID-number for exception
	lea	theIDT(,%ebx,8), %di	# gate-descriptor offset
	movw	$isrPGF, %ss:0(%di)	# entry-point lower-word
	movw	$privCS, %ss:2(%di)	# code-segment selector
	movw	$0x8E00, %ss:4(%di)	# 80386 interrupt-gate
	movw	$0x0000, %ss:6(%di)	# entry-point upper-word

	# install interrupt-gate for General Protection faults
	mov	$0x0D, %ebx		# ID-number for exception
	lea	theIDT(,%ebx,8), %di	# gate-descriptor offset
	movw	$isrGPF, %ss:0(%di)	# entry-point lower-word
	movw	$privCS, %ss:2(%di)	# code-segment selector
	movw	$0x8E00, %ss:4(%di)	# 80386 interrupt-gate
	movw	$0x0000, %ss:6(%di)	# entry-point upper-word

	ret
#------------------------------------------------------------------
hex:	.ascii	"0123456789ABCDEF"	# array of hex numerals
msg0:	.ascii	" nnn=xxxxxxxx "	# buffer for stack element
len0:	.int	. - msg0		# length of message-buffer
att0:	.byte	0x70			# attribute for characters
dst0:	.int	(8*80 - 15)*2		# screen-position's base
names:	.ascii	" CR2 err EIP  CS EFL" 	# string of element-names
nelts:	.int	(. - names)/4		# number of stack elements
#------------------------------------------------------------------
isrPGF:	.code32	# our exception-handler for page-fault exceptions

	sub	$4, %esp		# allocate space for CR2
	enter	$0, $0			# setup error-code access
	pushal				# preserve registers
	pushl	%ds
	pushl	%es

	# save the value found in Control Register CR2
	mov	%cr2, %eax		# get register-value CR2
	mov	%eax, 4(%ebp)		# save CR2-value on stack

	# setup segment-registers 
	mov	$privDS, %ax		# address program data
	mov	%ax, %ds		#   with DS register
	mov	$sel_es, %ax		# address video memory
	mov	%ax, %es		#   with ES register

	# loop to show the stackframe and the faulting address
	xor	%ebx, %ebx		# initialize array-index
nxelt:	
	# put element-name into buffer
	mov	names(,%ebx,4), %eax
	mov	%eax, msg0

	# put element-value into buffer
	mov	4(%ebp,%ebx,4), %eax
	lea	msg0+5, %edi
	call	eax2hex

	# compute the screen-position
	mov	dst0, %edi
	imul	$160, %ebx, %eax
	sub	%eax, %edi

	# draw the buffer-contents onscreen
	cld
	lea	msg0, %esi
	mov	len0, %ecx
	mov	att0, %ah
nxchr:	lodsb
	stosw
	loop	nxchr

	inc	%ebx
	cmp	nelts, %ebx
	jb	nxelt

	# wait for the user to press and release a key
spin:	in	$0x64, %al		# read keyboard status
	test	$0x01, %al		# test for OUTB=1 
	jz	spin			# spin if OUTB<>1
	in	$0x60, %al		# read keyboard scancode
	test	$0x80, %al		# was a key released?
	jz	spin			# no, spin till released

	#----------------------------------------------------------
	# Now, if the faulting address is below 0x00400000 (4-MB),
	# we will modify its entry in our page-table, as follows: 
	# if the error-code's P-bit is clear (i.e., page was 'Not 
	# Present'), we add a new entry into the page-table which
	# maps the page to our program-arena at 0x00010000; else,
	# if the page already was 'present', then we make it both
	# 'writable' and 'user-accessible'.
	#----------------------------------------------------------

	# sanity check: is faulting-address within table's bounds?
	mov	4(%ebp), %eax		# get the faulting address
	cmp	$0x00400000, %eax	# within page-table bounds?
	jb	intbl			# yes, we will handle it
	ljmp	$sel_cs, $finish	# else bail out from demo
intbl:
	# prepare to modify the faulting-address's page-table entry
	mov	$sel_fs, %ax		# address 'flat' segment
	mov	%ax, %ds		#    with DS register
	mov	%cr3, %edx		# page-directory address

	# lookup page-table address in the page-directory
	mov	4(%ebp), %ecx		# get page-fault address
	and	$0xFFC00000, %ecx	# isolate pgdir index
	shr	$22, %ecx		# get bits 31..22
	mov	(%edx,%ecx,4), %ebx	# page-directory entry
	and	$0xFFFFF000, %ebx	# page-table's address 

	# compute index of page-table entry for faulting address
	mov	4(%ebp), %ecx		# get page-fault address
	and	$0x003FF000, %ecx	# isolate pgtbl index
	shr	$12, %ecx		# get bits 21..12

	# examine error-code's P-bit to determine which action
	btl	$0, 8(%ebp)		# page already present?
	jc	setWU			# yes, then W-bit,U-bit

	mov	$0x00010000, %eax	# arena physical address
	mov	%eax, (%ebx, %ecx, 4)	# new page-table entry
setWU:	orl	$0x007, (%ebx, %ecx, 4)	# set entry attributes

	popl	%es			# recover registers
	popl	%ds
	popal				
	leave				# discard stackframe
	add	$4, %esp		# discard CR2 space
	add	$4, %esp		# discard error-code
	iret				# retry faulting opcode
#------------------------------------------------------------------
eax2hex: .code32	# converts EAX to hex string at DS:EDI
	pushal
	mov	$8, %ecx
nxnyb:	rol	$4, %eax
	mov	%al, %bl
	and	$0xF, %ebx
	mov	hex(%ebx), %dl
	mov	%dl, (%edi)
	inc	%edi
	loop	nxnyb
	popal
	ret
#------------------------------------------------------------------
isrGPF:	.code32	# General protection exception

	pushal
	pushl	%ds
	pushl	%es

	mov	$sel_es, %ax
	mov	%ax, %es
	xor	%edi, %edi
	cld
	mov	$0x50, %ecx
	mov	$0x4F46, %ax
	rep	stosw
	
	popl	%es
	popl	%ds
	popal
	jmp	isrPGF
#------------------------------------------------------------------
	.align	16			# insures stack alignment
	.space	256			# space for stack's usage
tos:					# labels our top-of-stack
#------------------------------------------------------------------
msg:	.ascii	" Hello from 'ring3' virtual memory " 
len:	.long	. - msg			# number of message bytes
att:	.byte	0x5F			# message text-attributes
dst:	.long	(12 * 80 + 22) * 2	# message screen-position
#------------------------------------------------------------------
draw_message:	.code32

	pushl	%ds
	pushl	%es

	# now write a message to the "virtual" video memory
	mov	$userDS, %ax		# address program data
	mov	%ax, %ds		#   with DS register
	mov	$userES, %ax		# address segment zero
	mov	%ax, %es		#   with ES register
	mov	dst, %edi		# point ES:DI to screen
	cld				# do forward processing
	lea	msg, %esi		# point DS:SI to string
	mov	len, %ecx		# string's length in CX
	mov	att, %ah		# text's colors into AH
nxpel:	lodsb				# fetch next character
	stosw				# store char w/ colors
	loop	nxpel			# draw complete string

	popl	%es
	popl	%ds
	lcall	$ring0, $0		# go through call-gate
#------------------------------------------------------------------
	.align	0x1000			# tables are page-aligned
pgtbl:	.zero	0x1000			# reserved for page-table
pgdir:	.zero	0x1000			# also for page-directory
#------------------------------------------------------------------
theTSS:	.long	0x00000000		# back-link (not used)
	.long	0x00000000		# reserved for ESP0
	.long	0x00000000		# reserved for SS0
	.equ	limTSS, (.-theTSS)-1	# segment-limit for TSS
#------------------------------------------------------------------
	.end				# nothing more to assemble
