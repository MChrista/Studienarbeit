//-----------------------------------------------------------------
//	memsize.s
//
//	This boot-sector program demonstrates use of the ROM-BIOS
//	'Get Memory Size' service (int 0x12) in order to find out
//	the upper limit on memory available for use in real-mode.
//
//	  to assemble:  $ as memsize.s -o memsize.o
//	  and to link:  $ ld memsize.o -T ldscript -o memsize.b  
//	  and install:  $ dd if=memsize.b of=/dev/sda4 
//
//	NOTE: This code begins executing with CS:IP = 0000:7C00.
//
//	programmer: ALLAN CRUSE
//	written on: 01 SEP 2008 
//-----------------------------------------------------------------

	.code16				# for 'real mode' on x86

	.section	.text
#------------------------------------------------------------------
start:	ljmp	$0x07C0, $main		# renormalizing CS and IP
#------------------------------------------------------------------
ten:	.short	10			# decimal-system's radix
msg:	.ascii	"\r\n Real-Mode Memory: "	# message legend
buf:	.ascii	"    0 KB \r\n\n"		# size to report
len:	.short	. - msg				# message length
att:	.byte	0x0D			# use bright purple text 
#------------------------------------------------------------------
main:	# setup segment-registers to address our program data
	
	mov	%cs, %ax		# address code segment
	mov	%ax, %ds		#   with DS register
	mov	%ax, %es		#   also ES register

	# invoke ROM-BIOS service to obtain memory-size (in KB)

	int	$0x12			# get ram's size into AX

	# use repeated division by ten to convert the value found
	# in AX to a decimal digit-string (without leading zeros)

	mov	$5, %di			# initialize buffer-index
nxdgt:	xor	%dx, %dx		# extend AX to doubleword
	divw	ten			# divide by decimal radix
	add	$'0', %dl		# convert number to ascii
	dec	%di			# buffer-index moved left
	mov	%dl, buf(%di)		# store numeral in buffer
	or	%ax, %ax		# was the quotient zero?
	jnz	nxdgt			# no, get another numeral	

	# now use ROM-BIOS video services to display our report
	
	mov	$0x0F, %ah		# get video-page into BH
	int	$0x10			# invoke BIOS service

	mov	$0x03, %ah		# cursor (row,col) to DX
	int	$0x10			# invoke BIOS service

	lea	msg, %bp		# point ES:BP to string
	mov	len, %cx		# setup CX with length
	mov	att, %bl		# setup BL with colors
	mov	$0x1301, %ax		# write_string function
	int	$0x10			# invoke BIOS service

	# then wait for the user to presses any key

	mov	$0x00, %ah		# get_keystroke
	int	$0x16			# invoke BIOS service

	# reboot this computer

	int	$0x19			# invoke BIOS service
#------------------------------------------------------------------
	.org	510			# offset of boot-signature
	.byte	0x55, 0xAA		# value for boot-signature
#------------------------------------------------------------------
	.end				# nothing more to assemble
