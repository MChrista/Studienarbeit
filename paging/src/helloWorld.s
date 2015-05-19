;###################################
; HELLO WORLD
; Christa, Polkehn
; execute an external c function
;###################################

SECTION .text

extern 	printHelloWorld	; mark symbol as external

global _start
.word   0xABCD

_start:
	call printHelloWorld ; Call C Function  	
	mov ebx,0
	mov eax,1
	int 0x80
