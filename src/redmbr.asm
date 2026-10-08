;-----------------------------------------------------------------------------------------------------------------------;
;			REDSKY MBR for Hard Disks				;	
;-----------------------------------------------------------------------------------------------------------------------;
;segment 0x9000 is used to load Second Stage Booter
;segment 0x5000 is used to load bpb or mbr
;each segment is 64KB in size

;--------Assembler Directives--------;
org 0x7c00
use16
;------------------------------------------;

;---------------------------------------------------------------------------------;
;		Entry Procedure			  ;
;---------------------------------------------------------------------------------;
	push 	cs
	pop 	ds
	mov 	[bootdrv],dl
	mov	ax,0x9000
	mov 	ss,ax
	mov 	sp,0xFFFF
	mov	si,welMsg
	call	disp
	call	ChkBiosExt
	mov	ax,0x5000
	mov 	es,ax
	mov 	bx,0000
	xor 	ax,ax
	xor 	dx,dx
	mov 	cx,1
	call 	ReadSector		; push cs , pop ds
	call 	FindRedSky
	mov 	si,nSysMsg
	call 	disp
nf: 	jmp 	nf

nSysMsg		db	'REDSKY not found',00h
welMsg		db	'REDSKY ',00h
;---------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;Name	:	FindRedSky			;
;Input	:	es:bx->mbr			;
;		dx:ax=lba of mbr			;
;Output	:	same				;
;-------------------------------------------------------------------------------;
FindRedSky:	
	pusha
	cmp	word[es:bx+510],0xAA55	; 0xEB - jmp , 0x90 - nop		
	jnz 	NonBootDisk
	lea 	di,[bx+0x1AE]		; 0x1AE - 430 , 0x1BE - 446 (first partition entry)
	mov 	cx,5
NextEnt:	add 	di,0x10
	dec 	cx
	jz 	EntsOver
	push 	cx
	mov 	cl,[es:di]
	test 	cl,cl
	jnz	n1
	call 	InActP
n1:	cmp 	cl,0x80
	jnz	n2
	call 	ActP
n2:	pop	cx
	jmp 	NextEnt
EntsOver:	popa
	ret
;-------------------------------------------------------------------------------;


;-------------------------------------------------------------------------------;
;Name	:	ActP				;
;Input	:	es:di->partition entry			;
;		es:bx->mbr			;				
;		dx:ax=lba of mbr			;
;Output	:	same				;
;-------------------------------------------------------------------------------;
ActP:	
	cmp	byte[es:di+4],0x0B
	jnz	n3
	call  	DosP			; call on zero
n3:	ret
;-------------------------------------------------------------------------------;


;-------------------------------------------------------------------------------;
;Name	:	DosP				;
;Input	:	es:di->partition entry			;
;		es:bx->mbr			;				
;		dx:ax=lba of mbr			;
;Output	:	same				;
;-------------------------------------------------------------------------------;
DosP:
	pusha
	add 	ax,[es:di+08h]
	adc 	dx,[es:di+0Ah]
	add 	bx,512
	mov	cx,1
	call 	ReadSector
	cmp	word[es:bx+510],0xAA55	; 0xEB - jmp , 0x90 - nop		
	jnz 	NBpb
	mov 	cx,6
	mov	si,oemName
	lea 	di,[bx+3h]
	repe cmpsb
	je	OsFound
NBpb:	popa
	ret
;--static data--;
oemName		db 	'REDSKY'
;----------------;
;-------------------------------------------------------------------------------;


;-------------------------------------------------------------------------------;
;Name	:	OsFound				;
;Input	:	es:bx->bpb			;
;		dx:ax=lba of bpb			;
;Output	:	N/A				;
;-------------------------------------------------------------------------------;
OsFound:
	push	0x9000
	pop	es
	xor	bx,bx
	mov	cx,5
	call	ReadSector
	mov	fs,ax
	mov	ax,dx
	mov	gs,ax
	mov	dl,[bootdrv]
	push	es
	push	bx
	retf
;-------------------------------------------------------------------------------	;


;-------------------------------------------------------------------------------;
;Name	:	InActP				;
;Input	:	es:di->partition entry			;
;		es:bx->mbr			;				
;		dx:ax=lba of mbr			;
;Output	:	same				;
;-------------------------------------------------------------------------------;
InActP:
	cmp	byte[es:di+4],0x05
	jnz	n4
	call  	ExtndP
n4:	ret		
;-------------------------------------------------------------------------------;


;-------------------------------------------------------------------------------;
;Name	:	ExtndP				;
;Input	:	es:di->partition entry			;
;		es:bx->mbr			;				
;		dx:ax=lba of mbr			;
;Output	:	same				;
;-------------------------------------------------------------------------------;
ExtndP:
	pusha
	add 	ax,[es:di+08h]
	adc 	dx,[es:di+0Ah]
	add 	bx,512
	mov	cx,1
	call 	ReadSector
	call	FindRedSky
	popa
	ret
;-------------------------------------------------------------------------------;


;-------------------------------------------------------------------------------;
;Name	:	ChkBiosExt			;
;Input	:	N/A				;
;Output	:	N/A				;
;-------------------------------------------------------------------------------;
ChkBiosExt:
	mov	ah,0x41
	mov	bx,0x55AA
	mov	dl,[bootdrv]
	int	13h
	jc	NoExt
	cmp	bx,0xAA55
	jnz	NoExt
	ret
;------------------------------------------------------------------------------;


;-------------------------------------------------------------------------------;
;		Error Handlers			;
;-------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
NonBootDisk:
	mov	si,nbdMsg
	call	disp
	nbdf:	jmp	nbdf
nbdMsg	db	'Non-bootable Disk',00h
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
NoExt:
	mov	si,noExtMsg
	call	disp
	NoExtf:	jmp	NoExtf

noExtMsg		db	'Int 13h Extension is not available',00h
;------------------------------------------------------------------------------;

	;------------------------------------------------------------------------------------------------;
	;Name	: ReadSector

	;Input	:	dx:ax = lba                    
	;       		cx  = sector count         
	;		es : bx  -> buffer address 

	;Output	:	( Returns the same value )
	;------------------------------------------------------------------------------------------------;
ReadSector:
	pusha
	mov	[lbadx],dx
	mov	[lbaax],ax
	mov	ax,es
	mov	[bufseg],ax
	mov	[bufoff],bx
	mov	[num],cx
	mov	ah,42h
	mov	dl,[bootdrv]
	mov	si,drvPacket
	int	13h
	jc	ReadError
	popa
	ret
ReadError:
	mov	si,readErrMsg
	call	disp
readerrf:	jmp	readerrf

readErrMsg	db	'Read Error',00h

drvPacket:
	size	db	10h
	res	db	0h
	num	dw	0h	
	bufoff	dw	0h
	bufseg	dw	0h
	lbaax	dw	0h
	lbadx	dw	0h
	lbaun	dd	0h
	;------------------------------------------------------------------------------------------------;
	;Name	: disp

	;Input	: si -> string buffer

	;Output	: ( Undefined )	

	;------------------------------------------------------------------------------------------------;
disp:
				; Display some message
	pusha    
	cld                		
          	mov     	ah, 0x0E                            	; BIOS teletype
          	mov     	bx, 0x0007                            	; bh = display page ,bl = text attribute
	displaychar:
	lodsb 				; load next character
	or      	al, al                              	; test for NULL character
          	jz     	ddone
	pusha
          	int     	10h                                	; invoke BIOS
	popa
          	jmp     	displaychar
     	ddone:
	popa
          	ret
	;------------------------------------------------------------------------------------------------;
bootdrv		db	0
times	19	db	0
	;------------------------------------------------------------------------------------------------;

;-------------------------------------------------------------------------------;
;		Master Boot Table			;
;-------------------------------------------------------------------------------;
mbrent1	dq	0
	dq	0
mbrent2	dq	0
	dq	0
mbrent3	dq	0
	dq	0
mbrent4	db	80h
	db	0h
	dw	0h
	db	0Bh
	db	0h
	dw	0h
	dd	1h
	dd	1h
;-------------------------------------------------------------------------------;

	;------------------------------------------------------------------------------------------------;
bootsign		dw 	0xAA55


;segment 0x9000 is used to load Second Stage Booter
;segment 0x9000 is used as stack for Second Stage Booter
;segment 0x0050 is used to load Support Driver
;segment 0x1000 is used for Support Driver Stack in protected mode
;segment 0x2000 is used to load Third Stage Booter
;segment 0x3000 is used to load Third Stage Booter
;segment 0x4000 is used to load FAT32
;segment 0x5000 is used to load bpb or mbr
;segment 0x6000 is used for Paging System stack in protected mode
;segment 0x7000 is used to create TSS for Support Driver and Third Stage Booter
;segment 0x8000 is used for TSB stack in protected mode
;GDT entries are as follows: code,data,SDstack,TSBstack,SDTSS,TSBTSS
;each segment is 64KB in size

org 0x0000
use16
	jmp    	start
        	nop
	;------------------------------------------------------------------------------------------------;
				; Boot program name
	bsOemName          		db      	'REDSKY  '
	;------------------------------------------------------------------------------------------------;
				; Bios Parameter Block
	bpbBytesPerSector  		dw      	0x200	; 512
	bpbSectorsPerCluster   	db      	8              
	bpbReservedSectors      	dw      	32               
	bpbNumberOfFATs       	db      	2               
	bpbRootEntries         	dw      	0  
	bpbTotalSectors       	dw      	0  
	bpbMedia                		db     	0xF8	; non-removable disk              
	bpbSectorsPerFAT     	dw      	0              
	bpbSectorsPerTrack    	dw     	63             
	bpbHeadsPerCylinder  	dw      	16              
	bpbHiddenSectors       	dd      	31               
	bpbTotalSectorsBig     	dd      	129024
	bpbSectsPerFAT32		dd          	126
	bpbExtFlags		dw	0
	bpbFSVer			dw	0
	bpbRootClus		dd	2
	bpbFSInfo		dw	1
	bpbBkBootSec		dw	6
	bpbReserved1		dd	0
	bpbReserved2		dd	0
	bpbReserved3		dd	0
	;------------------------------------------------------------------------------------------------;
				; Extended Bios Parameter Block
	bsDriveNumber          	db      	?          
	bsUnused               	 	db      	?              
	bsExtBootSignature      	db      	?               
	bsSerialNumber          	dd  	?              
	bsVolumeLabel           	db      	'NO NAME    '    
	bsFileSystem            	db      	'FAT32   '       
	;------------------------------------------------------------------------------------------------;
				; Start of boot program
	baselbaax	dw	0
	baselbadx	dw	0
	fatlbaax	dw	0
	fatlbadx	dw	0
	datalbaax	dw	0
	datalbadx	dw	0	

start:
	cli
	push	cs
	pop	ds
	mov	si,entrymsg
	call	disp2
	mov	[bsDriveNumber],dl
	mov	ax,gs
	mov	[baselbadx],ax
	mov	dx,ax
	mov	ax,fs
	mov	[baselbaax],ax
	add	ax,[bpbReservedSectors]
	adc	dx,0
	mov	[fatlbaax],ax
	mov	[fatlbadx],dx
	mov	ax,[bpbBytesPerSector]
	xor	cx,cx
	mov	cl,[bpbSectorsPerCluster]
	mul	cx
	mov	cx,0x4
	div	cx
	mov	cx,ax
	mov	ax,word[bpbTotalSectorsBig]
	mov	dx,word[bpbTotalSectorsBig+2]
	call	Div32By16		;dx:ax->numofsectperfat
	test	cx,cx
	jz	NoRem
	add	ax,1
	adc	dx,0
NoRem:	
	call	VerifyFATnsects
	mov	word[bpbSectsPerFAT32],ax
	mov	word[bpbSectsPerFAT32+2],dx
	xor	cx,cx
	mov	cl,[bpbNumberOfFATs]
	call	Mul32By16		;dx:ax->numofsectperfat*numoffat
	add	ax,[fatlbaax]
	adc	dx,[fatlbadx]
	mov	[datalbaax],ax
	mov	[datalbadx],dx
	jmp	nextstart
entrymsg		db	'Loading... ',00h

ChkBiosExt2:
	mov	ah,0x41
	mov	bx,0x55AA
	mov	dl,[bsDriveNumber]
	int	13h
	jc	NoExt2
	cmp	bx,0xAA55
	jnz	NoExt2
	ret
NoExt2:
	mov	si,noExtMsg2
	call	disp
	NoExtf2:	jmp	NoExtf2

noExtMsg2	db	'Int 13h Ext is not available',00h

	;------------------------------------------------------------------------------------------------;
	;Name	: ReadSector2

	;Input	:	dx:ax = lba                    
	;       		cx  = sector count         
	;		es : bx  -> buffer address 

	;Output	:	( Returns the same value )
	;------------------------------------------------------------------------------------------------;
ReadSector2:
	pusha
	mov	[lbadx2],dx
	mov	[lbaax2],ax
	mov	ax,es
	mov	[bufseg2],ax
	mov	[bufoff2],bx
	mov	[num2],cx
	mov	ah,42h
	mov	dl,[bsDriveNumber]
	mov	si,drvPacket2
	int	13h
	jc	ReadError2
	popa
	ret
ReadError2:
	mov	si,readErrMsg2
	call	disp2
readerrf2:	jmp	readerrf2

readErrMsg2	db	'Read Error',00h

drvPacket2:
	size2	db	10h
	res2	db	0h
	num2	dw	0h	
	bufoff2	dw	0h
	bufseg2	dw	0h
	lbaax2	dw	0h
	lbadx2	dw	0h
	lbaun2	dd	0h
	;------------------------------------------------------------------------------------------------;
	;Name	: disp

	;Input	: si -> string buffer

	;Output	: ( Undefined )	

	;------------------------------------------------------------------------------------------------;
disp2:
				; Display some message
	pusha    
	cld                		
          	mov     	ah, 0x0E                            	; BIOS teletype
          	mov     	bx, 0x0007                            	; bh = display page ,bl = text attribute
	displaychar2:
	lodsb 				; load next character
	or      	al, al                              	; test for NULL character
          	jz     	ddone2
	pusha
          	int     	10h                                	; invoke BIOS
	popa
          	jmp     	displaychar2
     	ddone2:
	popa
          	ret

GetNextClust:
	push	cx
	mov	cx,[bpbBytesPerSector]
	call	Div32By16
	push	cx
	mov	cx,0x4
	call	Mul32By16
	add	ax,[fatlbaax]
	adc	dx,[fatlbadx]
	mov	cx,1
	call	ReadSector2
	pop	ax
	mov	cx,0x4
	mul	cx
	mov	cx,bx
	add	bx,ax
	mov	ax,[es:bx]
	mov	dx,[es:bx+2]
	mov	bx,cx
	pop	cx
	ret

Div32By16:
	push	bx
	push	di
	push	si
	xchg	ax,dx
	mov	di,dx
	xor	dx,dx
	div	cx
	push	ax
	mov	bx,dx
	mov	ax,0xFFFF
	xor	dx,dx
	div	cx
	inc	dx
	push	di
	mov	di,dx
	mul	bx
	mov	si,ax
	mov	ax,di
	mul	bx
	div	cx
	add	si,ax
	mov	di,dx
	pop	ax
	xor	dx,dx
	div	cx
	add	si,ax
	mov	ax,di
	add	ax,dx
	xor	dx,dx
	div	cx
	add	ax,si
	mov	cx,dx
	pop	dx
	pop	si
	pop	di
	pop	bx
	ret

Mul32By16:
	push	di
	push	si
	mov	si,dx
	mul	cx
	xchg	si,ax
	mov	di,dx
	mul	cx
	mov	cx,dx
	add	ax,di
	mov	dx,ax
	mov	ax,si
	pop	si
	pop	di
	ret


dw	0xAA55

VerifyFATnsects:
	push	cx
	push	bx
	mov	cx,word[bpbSectsPerFAT32]
	mov	bx,word[bpbSectsPerFAT32+2]
	cmp	bx,dx
	jg	getFromTbl
	cmp	cx,ax
	jg	getFromTbl
	jmp	VFFin
getFromTbl:
	mov	dx,bx
	mov	ax,cx
VFFin:	pop	bx
	pop	cx
	ret
FileMissing:
	test	byte[sdl],1
	jnz	m1
	mov	si,sdMMsg
	call	disp2
m1:	test	byte[tsl],1
	jnz	m2
	mov	si,tsMMsg
	call	disp2
m2:	jmp	m2
	
ChkEOC:
	push	dx
	push	ax
	clc
	and	dx,0x0FFF
	xor	dx,0x0FFF	; z - if equal
	jnz	ChkEOCFin
	and	ax,0xFFF8
	xor	ax,0xFFF8
	jnz	ChkEOCFin
	stc
ChkEOCFin: pop	ax
	pop	dx
	ret


FindFiles:
	pusha
	mov	bp,0
	mov	ax,0x5000
	mov	es,ax
	xor	bx,bx
	xor	cx,cx
	mov	cl,[bpbSectorsPerCluster]
	mov	ax,word[bpbRootClus]
	mov	dx,word[bpbRootClus+2]
LoadNextCluster:
	push	ax
	push	dx
	sub	ax,2
	sbb	dx,0
	add	ax,[datalbaax]
	adc	dx,[datalbadx]
	call	ReadSector2
	test	byte[sdl],1
	jnz	s1
	call	SearchSupDrv
s1:	test	byte[tsl],1
	jnz	s2
	call	SearchTSB
s2:	test	bp,2
	jnz	SearchFin
	pop	dx
	pop	ax
	call	GetNextClust
	call	ChkEOC
	jc	FileMissing
	jmp	LoadNextCluster
SearchFin:	
	test	byte[sdl],1
	jz	FileMissing
	test	byte[tsl],1
	jz	FileMissing
	pop	dx
	pop	ax
	popa
	ret

SearchSupDrv:
	mov     	si, supDrvName                      	; image name to find
	call	FindFile
	jnc	SupDrvNF
	mov	byte[sdl],0x1
	call	ChkEOC
	jc	EmpSD
	mov	word[supdrv],ax
	mov	word[supdrv+2],dx
SupDrvNF:
	ret



EmpSD:
	mov	si,empSDMsg
	call	disp2
EmpSDf: jmp	EmpSDf

SearchTSB:
	mov     	si, TSBName                      	; image name to find
	call	FindFile
	jnc	TSBNF
	mov	byte[tsl],0x1
	call	ChkEOC
	jc	EmpTSB
	mov	word[tsb],ax
	mov	word[tsb+2],dx
TSBNF:
	ret


EmpTSB:
	mov	si,empTSBMsg
	call	disp2
EmpTSBf: jmp	EmpTSBf


FindFile:
	clc
	push	cx
	push	bx
	mov	ax,[bpbBytesPerSector]
	shr	ax,5
	xor	cx,cx
	mov     	cl,[bpbSectorsPerCluster]    	; load loop counter
	mul	cx
	mov	cx,ax
          	mov	di,0	                          	; locate first root entry
     	searchloop:
          	mov	bx,cx			; save the value of cx in bx
          	mov     	cx, 0x000B                          	; eleven character name
	test	byte[es:di],0xFF		; test whether first character is NULL(entries end)
	jz	FileNotFound		; if true, this is the end of root directory entries
	push	si
          	push    	di			
     	repe  cmpsb                                       	; test for entry match
          	pop     	di
	pop	si
          	je      	FileFound
          	add     	di, 0x0020                          	; queue next directory entry
	mov	cx,bx			; restore cx value
          	loop    	searchloop
        	jmp     	FileNotFoundT		
	FileFound:	
	mov	dx,word[es:di+0x14]		; save first cluster of file in dx:ax
	mov     	ax,word[es:di+0x1A]		; 0x14=20 , 0x1A=26
	stc
FileNotFound: inc	bp
FileNotFoundT:
	pop	bx
	pop	cx
	ret

 
	

LoadFile:
	pusha
	xor	si,si
LoadNC:	mov	ax,[fs:si]
	mov	dx,[fs:si+2]
	sub	ax,2
	sbb	dx,0
	push	cx
	xor	cx,cx
	mov	cl,[bpbSectorsPerCluster]
	push	cx
	call	Mul32By16
	pop	cx
	add	ax,[datalbaax]
	adc	dx,[datalbadx]
	call	ReadSector2
	mov	ax,[bpbBytesPerSector]
	mul	cx
	add	bx,ax
	add	si,4
	pop	cx
	loop	LoadNC
	popa
	ret


LoadClusters:
	pusha
	mov	ax,0x1000
	mov	fs,ax
	xor	di,di
	mov	ax,0x5000
	mov	es,ax
	xor	bx,bx
	xor	cx,cx
	mov	ax,word[supdrv]
	mov	dx,word[supdrv+2]
	call	LCloop
	mov	[NSd],cx
	nop		
	nop
	mov	ax,0x8000
	mov	fs,ax
	xor	di,di
	mov	ax,0x5000
	mov	es,ax
	xor	bx,bx
	xor	cx,cx
	mov	ax,word[tsb]
	mov	dx,word[tsb+2]
	call	LCloop
	mov	[NTs],cx
LCFin:	popa
	ret


LCloop:	mov	[fs:di],ax
	mov	[fs:di+2],dx
	add	di,4
	inc	cx
	call	GetNextClust
	call	ChkEOC
	jc	LCloopFin
	jmp	LCloop
LCloopFin:  
	ret

ChkHardDisk:
	pusha
	call	ChkBiosExt2
	xor	ax,ax
	mov	ah,0x48
	mov	dl,[bsDriveNumber]
	push	ds
	push	word[0x5000]
	pop	ds
	xor	si,si
	mov	word[ds:si],0x42
	mov	word[ds:si+2],0x0
	pop	ds
	popa
	ret
	int	13h
	jc	ChkHDErr
	mov	ax,[ds:si+0x10]
	mov	dx,[ds:si+0x12]
	cmp	word[ds:si+0x14],0x0
	jg	ChkHDSizeOK
	cmp	word[ds:si+0x16],0x0
	jg	ChkHDSizeOK
	mov	bx,word[ds:si+0x18]
	pop	ds
	cmp	dx,word[bpbTotalSectorsBig+2]
	jg	ChkSectSize
	cmp	ax,word[bpbTotalSectorsBig]
	jl	InvHD2
	jmp	ChkSectSize
ChkHDSizeOK:
	pop	ds
ChkSectSize:
	cmp	bx,[bpbBytesPerSector]
	jne	InvSectSize
	popa
	ret

InvSectSize:
	mov	si,invSectSizeMsg
	call	disp2
InvSectSizef: jmp	InvSectSizef



ChkHDErr:
	mov	si,chkHDErrMsg
	call	disp2
ChkHDf:	jmp	ChkHDf



InvHD2:
	mov	si,invHDSizeMsg
	call	disp2
InvHDSizef: jmp	InvHDSizef

CorrectClustAddr:
	mov	byte[sdl],0
	mov	byte[tsl],0
	push	ax
	mov	ax,word[supdrv+2]
	and	ax,0x0FFF
	test	ax,0xFFFF
	jnz	sd2ok
	inc	byte[sdl]
sd2ok:	mov	word[supdrv+2],ax
	mov	ax,word[supdrv]
	test	ax,0xFFFF
	jnz	sd1ok
	inc	byte[sdl]
sd1ok:	mov	ax,word[tsb+2]
	and	ax,0x0FFF
	test	ax,0xFFFF
	jnz	tsb2ok
	inc	byte[tsl]
tsb2ok:	mov	word[tsb+2],ax
	mov	ax,word[tsb]
	test	ax,0xFFFF
	jnz	tsb1ok
	inc	byte[tsl]
tsb1ok:	test	byte[sdl],2
	jz	sdvok
	jmp	EmpSD
sdvok:	test	byte[tsl],2
	jz	tsbvok
	jmp	EmpTSB
tsbvok:	pop	ax
	ret

nextstart:
	call	ChkCPU
	call	FindFiles
	call	CorrectClustAddr
	call	LoadClusters
	mov	ax,0x0050
	mov	es,ax
	xor	bx,bx
	mov	ax,0x1000
	mov	fs,ax
	mov	cx,[NSd]
	call	LoadFile
	mov	ax,0x2000
	mov	es,ax
	xor	bx,bx
	mov	ax,0x8000
	mov	fs,ax
	mov	cx,[NTs]
	call	LoadFile
	jmp 	newentry

enableA20:
	call	empty_8042
	mov	al,0xD1		; command write
	out	0x64,al
	call	empty_8042
	mov	al,0xDF		; A20 on
	out	0x60,al
	call	empty_8042
	ret

kill_motor:
	push dx
	mov dx,0x3f2
	mov al,0
	out dx,al
	pop dx
	ret

empty_8042:
	mov	ax,ax
	mov	bx,bx
	in	al,0x64	; 8042 status port
	test	al,2		; is input buffer full?
	jnz	empty_8042	; yes - loop
	ret

verify_a20:
	push ax
	push ds
	push es
		xor ax,ax
		mov ds,ax
		dec ax
		mov es,ax

		mov ax,[es:10h]		; read word at FFFF:0010 (1 meg)
		not ax			; 1's complement
		push word [0]		; save word at 0000:0000 (0)
			mov [0],ax	; word at 0 = ~(word at 1 meg)
			mov ax,[0]	; read it back
			cmp ax,[es:10h]	; fail if word at 0 == word at 1 meg
		pop word [0]
	pop es
	pop ds
	pop ax
	ret		; if ZF=1, the A20 gate is NOT enabled

msgint0D		db	'Some Protection Exception',00h

trap:	mov ax,0xB800
	mov fs,ax
	push	ds
	push	si
	push	cs
	pop	ds
	mov	si,msgint0D
	call	disp2
	pop	si
	pop	ds
	pop ax				; point stacked IP beyond...
	add ax,5			; ...the offending instruction
	push ax
	iret

newentry:
	sti
	jmp	w1
w1:	jmp	w2
w2:	call 	kill_motor
eA20:	call 	enableA20
	call	verify_a20
	jz	eA20
	cli
	mov	ax,cs	
	mov 	[es:0x0D * 4 + 2],ax		; INT 0Dh vector 
	lea 	ax,[trap]
	mov 	[es:0x0D * 4],ax
	push	cs
	pop	ds
	xor 	ax,ax
	mov 	es,ax
	mov	si,nsp
	mov	di,0x9000
	mov	cx,50
	cld
	rep movsd
	jmp	0x0000:0x9000

ChkCPU:

check_8086:
	pushf			;Bits 12-15 are always set on the 8086 processor
        	pushf                           	;save FLAGS
        	pop     	bx                      	;store FLAGS in BX
        	mov     	ax, 0fffh               	;clear bits 12-15
        	and     	ax, bx                  	;  in FLAGS
        	push    	ax                      	;store new FLAGS calue on stack
        	popf                            	;replace current FLAGS value
        	pushf                           	;set new flags
        	pop     	ax                      	;store new flags in AX
        	and     	ax, 0f000h              	;if bits 12-15 are set, then CPU
        	cmp     	ax, 0f000h              	;  is an 8086/8088
        	je      	CPU8086Err
	popf

check_80286:	
	pushf	 		;Bits 12-15 are always clear on the 80286 processor;
        	or      	bx, 0f000h              	;try to set bits 12-15
        	push    	bx
        	popf
        	pushf
        	pop     	ax
        	and     	ax, 0f000h              	; if bits 12-15 are cleared,CPU=Intel 286
        	je      	CPU80286Err         
	popf	
				; 386/386+
ChkV86:
	smsw	ax                            
       	test  	al,01h                          
       	jnz	v86mode  
	ret		           	

v86mode:				; Inform about virtual-8086 mode
	mov 	si,v86msg
	call	disp2
	v86fail:
	jmp	v86fail



CPU8086Err:
	mov	si,c86Msg
	call	disp2
C86f:	jmp	C86f



CPU80286Err:
	mov	si,c286Msg
	call	disp2
C286f:	jmp	C286f



supdrv	dd	0
NSd	dw	0
tsb	dd	0
NTs	dw	0
sdl	db	0
tsl	db	0

c86Msg		db	' CPU : Intel 8088/8086/80188/80186. Atleast Intel 80386 is required.',00h

v86msg			db	' Cannot boot in Virtual 8086 mode.',00h

c286Msg		db	' CPU : Intel 80286. Atleast Intel 80386 is required.',00h

sdMMsg	db	' Support Driver, SUPDRV.BIN missing',00h
tsMMsg	db	' Third Stage Booter, THRDBOOT.BIN missing',00h

supDrvName	db	'SUPDRV  BIN'

empSDMsg	db	' Support Driver is empty',00h

TSBName		db	'THRDBOOTBIN'

empTSBMsg	db	' Third Stage Booter is empty'

invSectSizeMsg	db	' Sector size in hard disk is different than indicated during formatting.',00h

chkHDErrMsg	db	' Error encountered during hard disk access. Check hard disk and its settings.',00h

invHDSizeMsg	db	' Less number of sectors in hard disk than indicated during formatting.',00h

nsp:
org 0x9000
use16
	jmp	newentry2

GDTvalue		dw 	27h, GDT, 0

GDT:

D0:
	dw	0
	dw	0
	dw	0
	dw	0

DD1:
	dw	0xFFFF
	dw	0
	db	0
	db	10011010b
	db	0xFF
	db	0

DD2:
	dw	0xFFFF
	dw	0
	db	0
	db	10010010b
	db	0xFF
	db	0

DD3:
	dw	0
	dw	0
	db	0
	db	10010110b
	db	0xF0
	db	0

DD4:
	dw	0xFFFF
	dw	0
	db	0
	db	10000110b
	db	0x1F
	db	0
	


newentry2:
	mov 	ax,0h
	mov 	ds,ax
	mov	fs,ax
	mov	gs,ax
	push	cs
	pop	es
	mov	bx,GDTvalue
	lgdt 	[es:bx]
	mov	es,ax
	mov 	eax,cr0
	inc 	ax
	mov 	cr0,eax
	jmp	08:clrpipe
use32
clrpipe:
	cli
	pushfd
	pop	eax
	or	eax,0x8000	; set the resume flag at bit 16 of EFLAGS
	push	eax
	popfd
	xor	eax,eax
	mov 	ax,0010h
	mov 	ds,ax
	mov 	es,ax
	mov 	ax,0018h
	mov 	ss,ax
	mov 	ebp,0x0008FFFF
	mov	esp,0x0008FFFF
	mov al,' '
	mov ah,111
	call clrscr
	jmp 0x00020000

clrscr:
	push 	ecx
	push	ax
	push	edi
	push	dx
	mov	ecx,2000
	mov	edi,0B8000h
clloop: 	mov	word[edi],ax
	add	edi,2
	loop	clloop
	mov	al,14
	mov	dx,0x3D4
	out	dx,al
	xor	al,al
	inc	dx
	out	dx,al
	mov	al,15
	dec	dx
	out	dx,al
	xor	al,al
	inc	dx
	out	dx,al
	pop	dx
	pop	edi
	pop	ax
	pop	ecx
	ret



times	71	db	0x90
times	3873	db	0x90
times	4*32768	db	0
times	2*4096	db	0			; 145920 - first root directory cluster


supdrvent		db	'THRDBOOTBIN',00h
times	8	db	0
		dw	0x0
		dd	0
		dw	0x0
		dd	0

thrdbootent	db	'SUPDRV  BIN',00h
times	8	db	0
		dw	0x0
		dd	0
		dw	0x8
		dd	0

times	448	db	0
times	7*512	db	0			; 150016 - end of root dir first clust