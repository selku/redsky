format COFF
use32
public _GetDrive
public _ReadHD
public _WriteHD
public _getRTCTime
public _getRTCDate
public _hangon
public _PS

_GetDrive:
	push	ebp
	mov	ebp,esp
	push	ecx
	push	edx
	push	edi
	pushfd
	cli
	xor	eax,eax
	call	hdcwait                                 	; Wait for HDC not busy.
        	jnc     	gn1
	mov     	al,01h                          	; Error 1: Controller busy...
	jmp	GetFin
gn1:	mov	dx,0x1F6
	mov     	al,[ebp+8]                        	;    .drive/head 
        	out     	dx,al                           	; ...done.
	call	hdcwait                                 	; Wait for HDC not busy.
        	jnc     	gn2
	mov     	al,01h                          	; Error 1: Controller busy...
	jmp	GetFin
gn2:	call	CheckDrive
	jz	GetFin
	mov     	edi,[ebp+12]
	mov     	dx,01F7h                        	; Send identify command...
        	mov     	al,0xEC                          	;    .
        	out     	dx,al                           	; ...done.
	cld
	call	CheckData
	jz	GetFin
        	mov     	cx,0100h                        	; Repeat count.
        	mov     	dx,01F0h
        	rep     	insw     
        	mov     	al,00h                          	; No error - return 0...
GetFin:	popfd
	clc
	pop	edi
	pop	edx
	pop	ecx
	pop	ebp
	ret	


_ReadHD:
	push	ebp
	mov	ebp,esp
	push	ecx
	push	edx
	push	edi
	pushfd
	cli
	xor	eax,eax
	call	PrepareHD
	jc	ReadFin
	xor     	ecx,ecx                           	; Get sector count...
        	mov     	cl,[ebp+12]                        	; ...done.
        	mov     	edi,[ebp+16]                        	; Get offset.
	mov     	dx,01F7h                        	; Send read command...
        	mov     	al,20h                          	;    .
        	out     	dx,al                           	; ...done.
	cld
ReadData:	call	CheckData
	jz	ReadFin
	push    	cx                              	; Save CX.
        	mov     	cx,0100h                        	; Repeat count.
        	mov     	dx,01F0h                        	; 16-bit transfer port.
        	rep     	insw                            	; Read data.
        	pop     	cx                              	; Restore CX.
        	loop    	ReadData                		; Loop until done.
        	mov     	al,00h                          	; No error - return 0...
ReadFin:	push	eax
	mov     	dx,01F7h 
	in	al,dx
	pop	eax
	popfd
	clc
	pop	edi
	pop	edx
	pop	ecx
	pop	ebp
	ret	


_WriteHD:
	push	ebp
	mov	ebp,esp
	push	ecx
	push	edx
	push	edi
	pushfd
	cli
	xor	eax,eax
	call	PrepareHD
	jc	WriteFin
	xor     	ecx,ecx                           	; Get sector count...
        	mov     	cl,[ebp+12]                        	; ...done.
        	mov     	edi,[ebp+16]                        	; Get offset.
	mov     	dx,01F7h                        	; Send write command...
        	mov     	al,30h                          	;    .
        	out     	dx,al                           	; ...done.
	cld
WriteData:	call	CheckData
	jz	WriteFin
	push    	cx                              	; Save CX.
        	mov     	cx,0100h                        	; Repeat count.
        	mov     	dx,01F0h                        	; 16-bit transfer port.
        	rep     	outsw                            	; Read data.
        	pop     	cx                              	; Restore CX.
        	loop    	WriteData                		; Loop until done.
        	mov     	al,00h                          	; No error - return 0...
WriteFin:	push	eax
	mov     	dx,01F7h 
	in	al,dx
	pop	eax
	popfd
	clc
	pop	edi
	pop	edx
	pop	ecx
	pop	ebp
	ret	


PrepareHD:                        		
        	call	hdcwait                                 	; Wait for HDC not busy.
        	jnc     	n1
	mov     	al,01h                          	; Error 1: Controller busy...
	jmp	fin
n1:	call	WriteParams
  	call	hdcwait                                 	; Wait for HDC not busy.
        	jnc     	n2
	mov     	al,01h                          	; Error 1: Controller busy...
	jmp	fin
n2:	call	CheckDrive
	jz	fin
	clc
	ret
fin:	stc
	ret


WriteParams:
	mov     	dx,01F2h                        	; Write parameters...
        	mov     	al,[ebp+12]                        	;    .sector count
        	out     	dx,al                           	;    .
        	inc     	dx                              	;    .
        	mov     	al,[ebp+8]                        	;    .sector
        	out     	dx,al                           	;    .
        	inc     	dx                              	;    .
        	mov     	al,[ebp+10]                        	;    .cylinder low
        	out     	dx,al                           	;    .
        	inc     	dx                              	;    .
        	mov     	al,[ebp+11]                        	;    .cylinder high
        	out     	dx,al                           	;    .
        	inc     	dx                              	;    .
	mov     	al,[ebp+9]                        	;    .drive/head 
        	out     	dx,al                           	; ...done.
	ret

CheckDrive:
	mov     	ecx,000C0000h                   	; 3s delay.
        	mov     	dx,01F7h                        	; HDC status register.
 ir_l1: 	in      	al,dx                           	; Read status.
        	test    	al,40h                          	; Drive ready?
        	jnz     	ir_ok2                          	; Continue if so.
        	loop   	ir_l1                           	; Loop for 3s.
        	mov     	al,02h                          	; Error 2: Drive not ready...
        	ret
 ir_ok2:	test    	al,10h                          	; Drive seek complete?
        	jnz     	seekfin
        	loop   	ir_l1                           	; Loop for 3s.
        	mov     	al,03h                          	; Error 3: Cannot read/write data...
seekfin:	ret



CheckData:
 ir_l2: 	mov     	dx,01F7h                        	; Get status port.
        	call	delay   
        	in      	al,dx                           	; Get status.
        	test    	al,80h                          	; Busy?
        	jnz     	ir_l2                           	; Loop if so.
     	test    	al,29h                          	; Loop if no change...
        	jz      	ir_l2                           	; ...done.
        	test    	al,08h                          	; Ready for data?
        	jnz     	chkdtfin
        	test    	al,1h                          	; Error in command?
        	jz     	ir_l2
	call	GetError
	xor	al,al			;set zero bit
	mov	al,4h
chkdtfin:	ret

GetError:
	mov     	dx,01F1h                        	; Get error code...
        	in      	al,dx                           	;    .
        	mov     	ah,al                        		; ...done.
        	mov     	dx,01F6h                        	; Recalibrate head...
	mov	al,[ebp+9]
        	out     	dx,al                           	;    .
        	inc     	dx                              	;    .
        	mov     	al,10h                          	;    .
        	out     	dx,al                           	; ...done.
	ret

hdcwait:                        		; Wait for HDC to finish commands.
        	mov     	ecx,00040000h                   	; 1s delay on 486DX-50.
        	mov     	dx,01F7h                        	; HDC status register.
        	in      	al,dx                           	; Read status.
        	test    	al,80h                          	; Is the HDC busy?
        	jz      	hdwok                             	; If not, end immediately.
hdwloop: 	call	delay
	in      	al,dx                           	; Read status.
        	test    	al,80h                          	; Is the HDC busy?
        	jz      	hdwok                             	; If not, end loop.
        	loop   	hdwloop                           	; Otherwise, continue.
   	stc                                     		; Set CF.
        	jmp	hdwerr	                            	; Exit with error.
 hdwok: 	clc                                     		; Clear CF.
 hdwerr:	ret

delay:
	push    	cx                              	; Save CX.
        	mov     	cx,000Ah                            	; Get repeat count.
dloop1: 	loop 	dloop1                              	; Loop 0Ah times. 50-100 clock delay.
        	pop     	cx                              	; Restore CX.
	ret	

_getRTCTime:
	xor	eax,eax
	mov	al,0xB
	out	0x70,al
	in	al,0x71
	shl	eax,8
	mov	al,4h
	out	0x70,al
	in	al,0x71
	shl	eax,8
	mov	al,2h
	out	0x70,al
	in	al,0x71
	shl	eax,8
	mov	al,0h
	out	0x70,al
	in	al,0x71
	ret

_getRTCDate:
	xor	eax,eax
	mov	al,9h
	out	0x70,al
	in	al,0x71
	shl	eax,8
	mov	al,8h
	out	0x70,al
	in	al,0x71
	shl	eax,8
	mov	al,7h
	out	0x70,al
	in	al,0x71
	ret

_hangon:
	mov	eax,0
	add	eax,1
hh:	jmp	hh

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

