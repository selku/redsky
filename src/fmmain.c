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

int main()
{
	int retvalue;
	unsigned int code,param1,param2,param3,param4;
	struct TSSStruct *tss=(struct TSSStruct *)(0x00100000+128);
	unsigned short myTaskId=10,prevTaskId;
	prevTaskId=(tss+myTaskId)->Back_Link/8-5;
	code=(tss+prevTaskId)->Eax;
	param1=(tss+prevTaskId)->Ebx;
	param2=(tss+prevTaskId)->Ecx;
	param3=(tss+prevTaskId)->Edx;
	param4=(tss+prevTaskId)->Esi;
	//setInt();
	retvalue=FileManager(code,param1,param2,param3,param4);
	//clrInt();
	(tss+prevTaskId)->Eax=retvalue;
	return(retvalue);
}