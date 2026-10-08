org 0x00300000
use32
PgStart:	push	esi
	push	edx
	push	ecx
	push	ebx
	push	eax
	call	PgMain
	add	esp,20
	iret
	jmp	PgStart

PgDmd:	mov	eax,cr2	;an error code is already pushed by processor
	push	eax
	push	100
	call	PgMain
	mov	esp,ebp	;also removes the error code pushed by the processor
	iret
	jmp	PgDmd

PgMain:

	