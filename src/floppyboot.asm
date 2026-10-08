;------------------------------------------------------------------------------------------------------------------------------;
;		 REDSKY BOOT PROGRAM FOR FAT12 FLOPPIES		        ;
;------------------------------------------------------------------------------------------------------------------------------;

	;------------------------------------------------------------------;
	; Assembler : FASM    ( Flat Assembler )		;
	; Date	   : 21 May 2005			;
	;					;
	;Description : 				;
	;	Checks for virtual-8086 mode.		;	
	;	Checks for intel 80386 processor.	;
	;	Loads the file 'REDBOOT.BIN' to the	;
	;	memory location 0D5FF0h.		;
	;					;
	;------------------------------------------------------------------;

;----------------------------------------------------------------;
		; Assembler Directives
org 0x7C00
use16
;----------------------------------------------------------------;

	;------------------------------------------------------------------------------------------------;
	;************************MAIN PROCEDURE**********************;
	;------------------------------------------------------------------------------------------------;
	;------------------------------------------------------------------------------------------------;
				; Make a short jump to start of the boot program
	jmp    	start
        	nop
	;------------------------------------------------------------------------------------------------;
				; Boot program name
	bsOemName          		db      	'Red Boot'
	;------------------------------------------------------------------------------------------------;
				; Bios Parameter Block
	bpbBytesPerSector  		dw      	?              
	bpbSectorsPerCluster   	db      	?              
	bpbReservedSectors      	dw      	?               
	bpbNumberOfFATs       	db      	?               
	bpbRootEntries         	dw      	?              
	bpbTotalSectors       	dw      	?              
	bpbMedia                		db     	?              
	bpbSectorsPerFAT     	dw      	?              
	bpbSectorsPerTrack    	dw     	?              
	bpbHeadsPerCylinder  	dw      	?              
	bpbHiddenSectors       	dd      	?               
	bpbTotalSectorsBig     	dd      	?               
	;------------------------------------------------------------------------------------------------;
				; Extended Bios Parameter Block
	bsDriveNumber          	db      	?          
	bsUnused               	 	db      	?              
	bsExtBootSignature      	db      	?               
	bsSerialNumber          	dd  	?              
	bsVolumeLabel           	db      	'NO NAME    '    
	bsFileSystem            	db      	'FAT12   '       
	;------------------------------------------------------------------------------------------------;
				; Start of boot program
start:
	;------------------------------------------------------------------------------------------------;
				; initialise memory
	cli
	push	0x5000	
	pop	ss
	push	cs
	pop	ds
	mov	sp,0FFFFh
	mov     	[bsDriveNumber], dl
	mov	si,entrmsg
	call	disp
	;------------------------------------------------------------------------------------------------;
				; check for virtual-8086 mode
	smsw	ax                            
       	test  	al,01h                          
       	jnz	v86mode                   
	;------------------------------------------------------------------------------------------------;
				; Detect whether the current processor is 386
	pushf
        	xor 	ah,ah
        	push 	ax
        	popf
        	pushf
        	pop 	ax
        	and 	ah,0f0h
        	cmp 	ah,0f0h
        	je  	not386
        	mov 	ah,0f0h
        	push 	ax
       	popf
        	pushf
        	pop ax
        	and 	ah,0f0h
        	jz  	not386
        	popf
	;------------------------------------------------------------------------------------------------;
				; Load FAT12 image into memory
	mov     	cx, [bpbSectorsPerFAT] 
	push	0D000h
	pop	es	
	xor     	bx, bx
	mov     	ax, word[bpbHiddenSectors]
        	mov     	dx, word[bpbHiddenSectors+2]
        	add     	ax, [bpbReservedSectors]
        	adc     	dx, bx                
        	call    	ReadSector
	;------------------------------------------------------------------------------------------------;
				; Load root directory into memory
	mov     	bx, ax
	mov     	di, dx
	push	es
	mov	ax,cx
        	mul     	word[bpbBytesPerSector]
	shr	ax,4h
	mov	si,es
	add	ax,si
	mov	es,ax
	push	ax	
	mov     	ax, 32
        	mov     	si, [bpbRootEntries]
        	mul     	si
        	div     	word[bpbBytesPerSector]
        	mov     	cx, ax              
	mov     	al, [bpbNumberOfFATs]
        	cbw
        	mul     	word [bpbSectorsPerFAT]
        	add     	ax, bx
        	adc     	dx, di             
	xor     	bx, bx                 
	call    	ReadSector
	add     	ax, cx
        	adc     	dx, bx
	push    	dx
        	push    	ax
	;------------------------------------------------------------------------------------------------;
				; Search for the file 'REDBOOT.BIN'
	mov     	di, bx                 
        	mov     	dx, si                 
        	mov     	si, ProgramName
        	FindName:
        	mov     	cx, 11
	FindNameCycle:
        	cmp     	byte [es:di], ch
        	je      	FindNameFailed          
        	pusha
        	repe    	cmpsb
        	popa
        	je      	FindNameFound
        	add     	di, 32
        	dec     	dx
        	jnz     	FindNameCycle         
	FindNameFailed:
        	mov	si,filemissmsg
	call	disp
	filemissfail:
	jmp	filemissfail
	FindNameFound:
        	mov     	si, [es:di+1Ah]        
	;------------------------------------------------------------------------------------------------;
				; Load the file 'REDBOOT.BIN'
	push	0D5FFh
	pop	es
	xor	bx,bx
	ReadNextCluster:
       	call    	ReadCluster
        	cmp     	si, 0FF8h
        	jc      	ReadNextCluster         
	;------------------------------------------------------------------------------------------------;
				; Execute the file 'REDBOOT.BIN'
	add	sp,04h
	jmp	dword[es:0h]
	hlt
	;------------------------------------------------------------------------------------------------;
				;The boot program ends here
	;------------------------------------------------------------------------------------------------;
	
	
	;------------------------------------------------------------------------------------------------;
	;*************************SUB PROCEDURES*********************;
	;------------------------------------------------------------------------------------------------;

	;------------------------------------------------------------------------------------------------;
	;Name    	: ReadCluster

	;Input	:	es : bx  -> buffer 
     	;		si = cluster no 

	;Output	:	si = next cluster  
         	;		es : bx  -> next addr
	;------------------------------------------------------------------------------------------------;
ReadCluster:
				; Reads the si cluster
      	mov     	bp, sp
        	lea     	ax, [si-2]
        	xor     	ch, ch
        	mov     	cl, [bpbSectorsPerCluster]
                mul     	cx
	add     	ax, [ss:bp+1*2]
        	adc     	dx, [ss:bp+2*2]
                call    	ReadSector
	mov     	ax, [bpbBytesPerSector]         
        	mul     	cx               
	add	bx,ax      
        	mov     	ax, 3
        	mul     	si
        	shr     	ax, 1
        	xchg    	ax, si                
        	push    	ds
        	mov     	ds,[ss:bp+4*2]   
        	mov     	si, [ds:si]            
        	pop     	ds
	jnc     	ReadClusterEven
	shr     	si, 4
	ReadClusterEven:
        	and     si, 0FFFh               
	ReadClusterDone:
        	ret
	;------------------------------------------------------------------------------------------------;

	;------------------------------------------------------------------------------------------------;
	;Name	: ReadSector

	;Input	:	dx : ax = lba                    
	;       		cx  = sector count         
	;		es : bx  -> buffer address 

	;Output	:	( Returns the same value )
	;------------------------------------------------------------------------------------------------;
ReadSector:
       				; Reads cx numberofsectors from dx:ax
	pusha
	ReadSectorNext:
        	mov     	di, 5              
	ReadSectorRetry:
        	pusha
        	div     	word[bpbSectorsPerTrack]   
        	mov     	cx, dx
        	inc     	cx       
        	xor     	dx, dx
        	div     	word[bpbHeadsPerCylinder]          
        	mov     	ch, al  
        	shl     	ah, 6
        	or      	cl, ah
          	mov     	dh, dl
	mov     	dl, [bsDriveNumber]
                mov     	ax, 201h
                int     	13h                  
        	jnc     	ReadSectorDone          
	xor     	ah, ah                
        	int     	13h                 
	popa
        	dec     	di
        	jnz     	ReadSectorRetry      
	jmp     	ErrRead
	ReadSectorDone:
        	popa    
        	add     	bx, [bpbBytesPerSector] 
        	add     	ax, 1
        	adc     	dx, 0                
        	loop   	ReadSectorNext
	popa
        	ret
	;------------------------------------------------------------------------------------------------;

	;------------------------------------------------------------------------------------------------;
	;Name	: disp

	;Input	: si -> string buffer

	;Output	: ( Undefined )	

	;------------------------------------------------------------------------------------------------;
disp:
				; Display some message
	xor	cx,cx
	mov 	ah,0Eh
	mov 	bx,0F00h
	mov	cl,[si]
	disploop:
	mov 	al,cl
	int	10h
	inc	si
	mov	cl,[si]
	inc	cl
	loop	disploop
	ret
	;------------------------------------------------------------------------------------------------;

	;------------------------------------------------------------------------------------------------;
	;*************************Error Handlers***************************;
	;------------------------------------------------------------------------------------------------;

	;------------------------------------------------------------------------------------------------;
ErrRead:
				; Inform about disk error
	mov	si,diskmsg
	call	disp
	errreadfail:
	jmp	errreadfail
	;------------------------------------------------------------------------------------------------;
not386:
				; Inform about processor requirement
	mov	si,not386msg
	call	disp
	not386fail:
	jmp	not386fail
	;------------------------------------------------------------------------------------------------;

	;------------------------------------------------------------------------------------------------;
v86mode:
				; Inform about virtual-8086 mode
	mov 	si,v86msg
	call	disp
	v86fail:
	jmp	v86fail
	;------------------------------------------------------------------------------------------------;
	
	;------------------------------------------------------------------------------------------------;
	;********************Static Memory Declarations*****************;
	;------------------------------------------------------------------------------------------------;

	;------------------------------------------------------------------------------------------------;	
				; Messages
	entrmsg			db	'REDSKY',0Ah,0Dh,00h
	v86msg			db	'Err: V86 mode',00h
	not386msg		db	'Err: No 386+',00h
	diskmsg			db	'Err: disk fails',00h
	filemissmsg		db	'Err: REDBOOT.BIN?',00h
	;------------------------------------------------------------------------------------------------;
				; Boot file name
	ProgramName     		db      	'REDBOOT BIN'
	;------------------------------------------------------------------------------------------------;

	;------------------------------------------------------------------------------------------------;
	;***************************Boot Magic***************************;
	;------------------------------------------------------------------------------------------------;
	bootsign			dw      	0AA55h
	;------------------------------------------------------------------------------------------------;

;------------------------------------------------------------------------------------------------------------------------------;