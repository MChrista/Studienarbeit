;###################################
; HELLO WORLD
; Christa, Polkehn
; execute an external c function
;###################################

SECTION .text
extern 	printHelloWorld	; mark symbol as external

global _start

_start:
	mov ebx,0
	mov eax,1
	int 0x80
	;call printHelloWorld ; Call C Function  	