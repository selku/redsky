format COFF
use32
public _hangon
public _getDW
public _LGDTR
public _LTR
public _CallTask
public _STR
public _SGDTRL
public _SGDTRB
public _LLDTR
public _SLDTR
public _GetEFlags
public _SetEFlags
public _DM_LoadSect
public _LIDTR
public _DisableCursor
public _EnablePaging
public _PS
public _DM
public _TM
public _FS
public _DMP
public _GData

_hangon:
	mov	eax,0
	add	eax,1
hh:	jmp	hh

_getDW:
	mov	eax,[esp+4]
	mov	eax,[eax]
	ret

_LGDTR:
	mov	ax,[esp+4]
	mov	[esp+6],ax
	lea	eax,[esp+6]
	lgdt	[eax]
	ret
	
_LTR:
	mov	ax,[esp+4]
	ltr	ax
	ret

_CallTask:
	call	far [esp]
	ret


_STR:
	xor	eax,eax
	str 	ax
	ret

_SGDTRL:
	mov	eax,ktemp
	sgdt 	[eax]
	xor	eax,eax
	mov	ax,[ktemp]
	ret

_SGDTRB:
	mov	eax,ktemp
	sgdt 	[eax]
	lea	eax,[ktemp+2]
	mov	eax,[eax]
	ret

ktemp	dw	3

_LLDTR:
	lldt	[esp+4]
	ret

_SLDTR:
	xor	eax,eax
	sldt	ax
	ret

_GetEFlags:
	pushfd
	pop	eax
	ret

_SetEFlags:
	mov	eax,[esp+4]
	push	eax
	popfd
	ret

_DM_LoadSect:
	mov     eax,[esp+20]
	pusha
	push	eax
	mov     dx,1f6h         ;Drive and head port
	mov     al,0a0h         ;Drive 0, head 0
	out     dx,al

	mov     dx,1f2h         ;Sector count port
	mov     al,1            ;Read one sector
	out     dx,al

	mov     dx,1f3h         ;Sector number port
	mov     al,1            ;Read sector one
	out     dx,al

	mov     dx,1f4h         ;Cylinder low port
	mov     al,0            ;Cylinder 0
	out     dx,al

	mov     dx,1f5h         ;Cylinder high port
	mov     al,0            ;The rest of the cylinder 0
	out     dx,al

	mov     dx,1f7h         ;Command port
	mov     al,20h          ;Read with retry.
	out     dx,al
still_going:
	in      al,dx
	test    al,8            ;This means the sector buffer requires
				;servicing.
	jz      still_going     ;Don't continue until the sector buffer
				;is ready.

	mov     ecx,512/2        ;One sector /2
	pop	eax
	mov     edi,eax
	mov     dx,1f0h         ;Data port - data comes in and out of here.
	xor bx,bx
	cld
	rep     insw
	popa
	ret

_LIDTR:
	mov	ax,[esp+4]
	mov	[esp+6],ax
	lea	eax,[esp+6]
	lidt	[eax]
	ret

_DisableCursor:
	push	edx
	mov	dx,0x03CC
	in	al,dx
	or	al,0x01
	mov	dx,0x03C2
	out	dx,al
	mov	al,0x0A
	mov	dx,0x03D4
	out	dx,al
	mov	dx,0x03D5
	in	al,dx
	or	al,0x20
	out	dx,al
	pop	edx
	ret

_EnablePaging:
	mov	eax,[esp+4]
	mov	cr3,eax
	mov	eax,cr0
	or	eax,0x80000000
	mov	cr0,eax
	ret

_PS:
	push	ebp
	mov	ebp,esp
	push	ebx
	push	ecx
	push	edx
	push	esi
	mov	eax,[ebp+8]
	mov	ebx,[ebp+12]
	mov	ecx,[ebp+16]
	mov	edx,[ebp+20]
	mov	esi,[ebp+24]
	push	120	; (Task10 +5)*8
	push	0
	call	far [esp]
	add	esp,8
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	pop	ebp
	ret 

_DM:
	push	ebp
	mov	ebp,esp
	push	ebx
	push	ecx
	push	edx
	push	esi
	mov	eax,[ebp+8]
	mov	ebx,[ebp+12]
	mov	ecx,[ebp+16]
	mov	edx,[ebp+20]
	mov	esi,[ebp+24]
	push	80	; (Task5 +5)*8
	push	0
	call	far [esp]
	add	esp,8
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	pop	ebp
	ret 

_TM:
	mov	eax,[esp+4]
	push	56	;(Task2 +5)*8
	push	0
	call	far[esp]
	add	esp,8
	ret

_FS:
	push	ebp
	mov	ebp,esp
	push	ebx
	push	ecx
	push	edx
	push	esi
	push	edi
	mov	eax,[ebp+8]
	mov	ebx,[ebp+12]
	mov	ecx,[ebp+16]
	mov	edx,[ebp+20]
	mov	esi,[ebp+24]
	mov	edi,[ebp+28]
	push	144	; (Task13 +5)*8
	push	0
	call	far [esp]
	add	esp,8
	pop	edi
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	pop	ebp
	ret

_DMP:
	push	ebp
	mov	ebp,esp
	push	ebx
	push	ecx
	push	edx
	push	esi
	mov	eax,[ebp+8]
	mov	ebx,[ebp+12]
	mov	ecx,[ebp+16]
	mov	edx,[ebp+20]
	mov	esi,[ebp+24]
	push	128	; (Task11 +5)*8
	push	0
	call	far [esp]
	add	esp,8
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	pop	ebp
	ret 

_GData	dd	0