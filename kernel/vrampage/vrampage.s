//-----------------------------------------------------------------
//	vrampage.s
//
//	This program uses the processor's page-mapping capability
//	to "relocate" the first page of text-mode video memory to
//	the bottom of the CPU's "virtual" address-space (i.e., to
//	address zero), as a simple instance of using page-tables. 
//
//	 to assemble:  $ as vrampage.s -o vrampage.o
//	 and to link:  $ ld vrampage.o -T ldscript -o vrampage.b
//	 and install:  $ dd if=vrampage.b of=/dev/sda4 seek=1
//
//	NOTE: This code begins executing with CS:IP = 1000:0002.
//
//	programmer: ALLAN CRUSE
//	written on: 08 NOV 2008
//-----------------------------------------------------------------

	.equ	realCS, 0x1000		# arena's segment-address 

	.section	.text
#------------------------------------------------------------------
	.word	0xABCD			# our loader expects this
#------------------------------------------------------------------
	.globl	main
main:	.code16				# begins in x86 real-mode
	
	mov	%sp, %cs:ipltos+0	# preserve loader's SP
	mov	%ss, %cs:ipltos+2	# preserve loader's SS

	mov	%cs, %ax		# address program arena
	mov	%ax, %ss		#   using SS register
	lea	tos, %sp		# establish a new stack

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

	.equ	limGDT, (.-theGDT)-1	# our GDT's segment-limit 
#------------------------------------------------------------------
#------------------------------------------------------------------
regGDT:	.word	limGDT, theGDT, 0x0001	# image for GDTR register
#------------------------------------------------------------------
regCR3:	.long	16*realCS + pgdir	# image for CR3 register 
#------------------------------------------------------------------
create_paging_tables:

	# setup segment-register DS to address program's arena
	mov	%cs, %ax		# address paging tables
	mov	%ax, %ds		#   using DS register

	#-----------------------------------
	# initialize two page-table entries 
	#-----------------------------------

	# setup mapping for virtual-address 0x00000000 
	mov	$0x00, %edi		# for virtual page 0x00 
	mov	$0xB8000, %eax		# physical page-address
	or	$0x003, %eax		# present and writable
	mov	%eax, pgtbl(,%edi,4)	# write the table-entry

	# setup mapping for virtual-address 0x00010000 
	mov	$0x10, %edi		# for virtual page 0x10
	mov	$0x10000, %eax		# physical page-address
	or	$0x003, %eax		# present and writable
	mov	%eax, pgtbl(,%edi,4)	# write the table-entry

	#------------------------------------
	# initialize one page-drectory entry  
	#------------------------------------

	# setup the page-directory to use our page-table
	mov	$0x10000, %ebx		# arena physical-address
	lea	pgtbl(%ebx), %eax	# pgtbl physical-address
	or	$0x003, %eax		# present and writable 
	mov	%eax, pgdir		# page-directory entry 0

	ret
#------------------------------------------------------------------
enter_protected_mode:

	cli				# interrupts disabled

	mov	%cr0, %eax		# current machine status
	bts	$0, %eax		# turn on PE-bit's image
	mov	%eax, %cr0		# enable protected mode

	lgdt	%cs:regGDT		# establish our GDT

	ljmp	$sel_cs, $pm		# reload CS register
pm:	
	mov	$sel_ds, %ax
	mov	%ax, %ss		# reload SS register

	ret
#------------------------------------------------------------------
#------------------------------------------------------------------
execute_program_demo:

	# setup page-directory address in control register CR3
	mov	ptdb, %eax		# page-directory address
	mov	%eax, %cr3		# goes into CR3 register

	# turn on paging (by setting bit #31 in register CR0)
	mov	%cr0, %eax		# current machine status
	bts	$31, %eax		# turn on PG-bit's image
	mov	%eax, %cr0		# enable page-mappings  
	jmp	.+2			# flush prefetch queue 

	# now write a message to the "virtual" video memory
	mov	$sel_es, %ax		# address segment zero
	mov	%ax, %es		#   with ES register
	mov	dst, %di		# point ES:DI to screen
	cld				# do forward processing
	lea	msg, %si		# point DS:SI to string
	mov	len, %cx		# string's length in CX
	mov	att, %ah		# text's colors into AH
nxpel:	lodsb				# fetch next character
	stosw				# store char w/ colors
	loop	nxpel			# draw complete string

	# disable paging (by clearing bit #31 in register CR0)
	mov	%cr0, %eax		# current machine status
	btr	$31, %eax		# reset PG-bit's image
	mov	%eax, %cr0		# disable page-mapping
	jmp	.+2			# flush prefetch queue

	# invalidate the CPU's Translation Lookaside Buffer
	xor	%eax, %eax		# setup "dummy" value
	mov	%eax, %cr3		# and write it to CR3

	ret
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

	sti				# reenable interrupts
	ret
#------------------------------------------------------------------
#------------------------------------------------------------------
ptdb:	.long	16*realCS + pgdir 	# pgdir: physical address 
#------------------------------------------------------------------
msg:	.ascii	" Hello from 'virtual' video memory " 
len:	.short	. - msg			# number of message bytes
att:	.byte	0x1F			# message text-attributes
dst:	.short	(12 * 80 + 22) * 2	# message screen-position
#------------------------------------------------------------------
	.align	16			# insures stack alignment
	.space	256			# space for stack's usage
tos:					# labels our top-of-stack
#------------------------------------------------------------------
	.align	0x1000			# tables are page-aligned
pgtbl:	.zero	0x1000			# reserved for page-table
pgdir:	.zero	0x1000			# also for page-directory
#------------------------------------------------------------------
	.end				# nothing more to assemble
