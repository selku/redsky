# define	word	unsigned short
# define	dword	unsigned int

struct DebugWordStruct
{
	 word	Trap_Bit:1;
	 word	Res000:15;	// 0
};

struct TSSStruct
{
	word	Back_Link;
	word	Res0;	// 0
	dword	Esp0;
	word	Ss0;
	word	Res1;	// 0
	dword	Esp1;
	word	Ss1;
	word	Res2;	// 0
	dword	Esp2;
	word	Ss2;
	word	Res3;	// 0
	dword	Cr3;
	dword	Eip;
	dword	Eflags;
	dword	Eax;
	dword	Ecx;
	dword 	Edx;
	dword 	Ebx;
	dword 	Esp;
	dword 	Ebp;
	dword 	Esi;
	dword 	Edi;
	word	Es;
	word	Res00;	// 0
	word	Cs;
	word	Res01;	// 0
	word	Ss;
	word	Res02;	// 0
	word	Ds;
	word	Res03;	// 0
	word	Fs;
	word	Res04;	// 0
	word	Gs;
	word	Res05;	// 0
	word	Ldt;
	word	Res06;	// 0
	struct DebugWordStruct Debug;
	word	Bitmap_Offset;
	dword	ResKey[5];
	dword	ProcessId;
};

void main()
{
	int code;
	unsigned int retvalue;
	struct TSSStruct *tss=(struct TSSStruct *)(0x00100000+128);
	unsigned short myTaskId=2,prevTaskId;
	prevTaskId=(tss+myTaskId)->Back_Link/8-5;
	code=(tss+prevTaskId)->Eax;
	
	if(code==1)
		retvalue=initTM();
	else if(code==2)
		retvalue=getKey();
	else if(code==3)
		retvalue=getCh();
	else if(code==4)
		retvalue=getTime();
	else if(code==5)
		retvalue=getDate();
	else
		retvalue=0;
	
	(tss+prevTaskId)->Eax=retvalue;
	return;
}
