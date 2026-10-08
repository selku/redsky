# define dword unsigned int
# define word unsigned short
# define byte unsigned char

struct GDTaccessrightsstruct
{
	byte type:4;	// 0x9=Inactive Task,0xB=Active Task
	byte notsystem:1;
	byte dpl:2;
	byte present:1;
};

struct GDTattribstruct
{
	byte limit_hi_nib:4;
	byte avl:1;
	byte res0:1;
	byte D:1;
	byte G:1;
};

struct GDTstruct
{
	word limit_lo_word;
	word base_lo_word;
	byte base_med_byte;
	struct GDTaccessrightsstruct accessrights;
	struct GDTattribstruct attrib;
	byte base_hi_byte;
};

struct IDTattribstruct
{
	unsigned char 	type:5;
	unsigned char	dpl:2;
	unsigned char	present:1;
};

struct IDTstruct
{
	unsigned short 	res0;
	unsigned short 	tss;
	unsigned char 	res1;	
	struct IDTattribstruct	attrib;
	unsigned short	res2;
};

/*D0:
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
	db	0*/

struct GDTCstruct
{
	word limit_lo_word;
	word base_lo_word;
	byte base_med_byte;
	byte accessrights;
	byte attrib;
	byte base_hi_byte;
};

# define PIC1 0x20
# define PIC2 0xA0
# define ICW1 0x11
# define ICW4 0x01
/* init_pics()
* init the PICs and remap them
*/

extern unsigned char inline inb(unsigned short _port)
{
	unsigned char result;
	__asm__ ("in %%dx, %%al" : "=a" (result) : "d" (_port));
	return result;
}
extern void inline outb(unsigned short _port, unsigned char _data)
{
	__asm__ ("out %%al, %%dx" : : "a" (_data), "d" (_port));
}

void init_pics(int pic1, int pic2)
{
	/* send ICW1 */
	outb(PIC1, ICW1);
	outb(PIC2, ICW1);
	/* send ICW2 */
	outb(PIC1 + 1, pic1); /* remap */
	outb(PIC2 + 1, pic2); /* pics */
	/* send ICW3 */
	outb(PIC1 + 1, 4); /* IRQ2 -> connection to slave */
	outb(PIC2 + 1, 2);
	/* send ICW4 */
	outb(PIC1 + 1, ICW4);
	outb(PIC2 + 1, ICW4);
	/* disable all IRQs */
	outb(PIC1 + 1, 0xFC);
}

//init_pics(0x20, 0x28);

void inline prtDelay()
{
	inb(PIC1+1);
}
void inline clrInt()
{
	__asm__("cli");
}
void inline setInt()
{
	__asm__("sti");
}	

void disableallirqs()
{
	outb(PIC1 + 1, 0xFF);
}
void enablef2irqs()
{
	outb(PIC1 + 1, 0xFC);
}

extern int printk(const char *, ...);

void makeGDTEntry(struct GDTstruct *gdt,unsigned int type,unsigned int prev,unsigned int base,unsigned int limit)
{
	struct GDTCstruct *gdtc=(struct GDTCstruct *)gdt;
	gdtc->limit_lo_word=(unsigned short) limit;
	limit=limit>>16;
	limit=limit&0x0F;
	type=type&0x001F;
	if(type==0x10)
	{
		type=0;
	}
	else
	{
		if(type<10) limit=limit|0x70;
		else limit=limit|0xF0;	
		prev=prev&0x03;
		prev=prev|0x04;
		prev=prev<<5;
		type=type|prev;
	}	
	gdtc->attrib=(unsigned char) limit;
	gdtc->accessrights=(unsigned char) type;
	gdtc->base_lo_word=(unsigned short) base;
	base=base>>16;
	gdtc->base_med_byte=(unsigned char) base;
	base=base>>8;
	gdtc->base_hi_byte=(unsigned char) base;
	return;
}


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

static unsigned int EFlags;

void makeTSS(struct TSSStruct *tss)
{
	tss->Back_Link=0;
	tss->Res0=0;
	tss->Esp0=0;
	tss->Ss0=3*8;
	tss->Res1=0;
	tss->Esp1=0;
	tss->Ss1=3*8;
	tss->Res2=0;
	tss->Esp2=0;
	tss->Ss2=3*8;
	tss->Res3=0;
	tss->Cr3=0;
	tss->Eip=0;
	tss->Eflags=EFlags;
	tss->Eax=0;
	tss->Ecx=0;
	tss->Edx=0;
	tss->Ebx=0;
	tss->Esp=0;
	tss->Ebp=0;
	tss->Esi=0;
	tss->Edi=0;
	tss->Es=2*8;
	tss->Res00=0;
	tss->Cs=1*8;
	tss->Res01=0;
	tss->Ss=3*8;
	tss->Res02=0;
	tss->Ds=2*8;
	tss->Res03=0;
	tss->Fs=0;
	tss->Res04=0;
	tss->Gs=0;
	tss->Res05=0;
	tss->Ldt=4*8;
	tss->Res06=0;
	tss->Debug.Res000=0;
	tss->Debug.Trap_Bit=0;
	tss->Bitmap_Offset=0;
}

# define	IDTTSS	5

void makeIDTEntry(struct IDTstruct *idt,unsigned int prev,unsigned int present,unsigned short gdtentry)
{
	idt->res0=0;
	idt->res1=0;
	idt->res2=0;
	idt->attrib.dpl=(prev&0x3);
	idt->attrib.present=(present&0x1);
	idt->attrib.type=IDTTSS;
	idt->tss=gdtentry;
}

extern unsigned int getDW(unsigned int *);

extern unsigned int getPhyMem()
{
	int i;
	const unsigned int dis=0x00100000;	//1048576;
	unsigned int *mem=0x0;			
	for(i=1;i<4096;i++)
	{
		*(mem+(i*dis))=1;
		if(getDW(mem+(i*dis))!=1) break;
		*(mem+(i*dis))='A';
		if(getDW(mem+(i*dis))!='A') break;
	}
	return i*4;
}

validChar(char Ch)
{
	int i;
	static char vc[15]={'$','%','-','_','@','~','`','!','(',')','{','}','^','#','&'};
	if(((Ch>='A')&&(Ch<='Z'))||((Ch>='a')&&(Ch<='z'))||((Ch>='1')&&(Ch<='9'))) return 1;
	for(i=0;i<15;i++)
	{
		if(Ch==vc[i]) return 1;
	}
	return 0;
}

formalisePath(char *Path,char *NewPath)
{
	int i,j,k,l,x;
	
	   // totally there can be 372*11 byte entries = 4092 bytes
	
	for(i=0,k=0;i<40;i++)
	{
		if(k==4092) return -1;
		for(x=0;x<12;x++)
		{
			*(NewPath+k+x)=' ';
		}
		*(NewPath+k+x)=0;
		for(j=0;j<8;j++)
		{
			if((*(Path+i+j)==' ')&&(j==0)) return -1;
			if(!validChar(*(Path+i+j)))
			{
			 	if(j==0) return -1;
				if((*(Path+i+j)=='/')||(*(Path+i+j)=='\\'))
				{
					k=k+11;
					i=i+j;
					break;
				}
				else if(*(Path+i+j)=='.')
				{
					k=k+8;
					i++;
					for(l=0;l<3;l++)
					{
						if((*(Path+i+j+l)==' ')&&(l==0)) return -2;
						if(!validChar(*(Path+i+j+l)))
						{
							if(l==0) return -3;
							if((*(Path+i+j+l)=='/')||(*(Path+i+j+l)=='\\'))
							{
								k=k+3;
								i=i+l;
								i=i+j;
								break;
							}
							else if(*(Path+i+j+l)==0) return 0;
							else return -4;
						}
						*(NewPath+k+l)=((*(Path+i+j+l)>='a')&&(*(Path+i+j+l)<='z')) ? (*(Path+i+j+l)+'A'-'a') : *(Path+i+j+l);
					}
					if(l==3)
					{
						i=i+3;
						i=i+j;
						k=k+3;
						if(*(Path+i)==0) return 0;
						if((*(Path+i)!='/')&&(*(Path+i)!='\\')) { /*printk("%c",*(Path+i));*/ return -5; }
					}
					break;
				}
				else if(*(Path+i+j)==0) return 0;
				else return -6;
			}
			*(NewPath+k+j)=((*(Path+i+j)>='a')&&(*(Path+i+j)<='z')) ? (*(Path+i+j)+'A'-'a') : *(Path+i+j);
		}
		if(j==8)
		{
			if(*(Path+i+j)=='.')
			{
				k=k+8;
				i++;
				for(l=0;l<3;l++)
				{
					if((*(Path+i+j+l)==' ')&&(l==0)) return -1;
					if(!validChar(*(Path+i+j+l)))
					{
						if(l==0) return -1;
						if((*(Path+i+j+l)=='/')||(*(Path+i+j+l)=='\\'))
						{
							k=k+3;
							i=i+l;
							break;
						}
						else if(*(Path+i+j+l)==0) return 0;
						else return -1;
					}
					*(NewPath+k+l)=((*(Path+i+j+l)>='a')&&(*(Path+i+j+l)<='z')) ? (*(Path+i+j+l)+'A'-'a') : *(Path+i+j+l);
				}
				if(l==3)
				{
					i=i+3;
					k=k+3;
					if(*(Path+i+j)==0) return 0;
					if((*(Path+i+j)!='/')&&(*(Path+i+j)!='\\')) { /*printk("%c",*(Path+i+j));*/ return -1; }
				}
			}
			else
			{
				k=k+11;
			}
			i=i+8;
			if(*(Path+i)==0) return 0;
			if((*(Path+i)!='/')&&(*(Path+i)!='\\')) { /*printk("vv: %c",*(Path+i));*/ return -1; }
		}
	}
	return 0;
}

# define	NULLSEG	16
# define 	CODESEG	30	// 26-non conforming
# define 	DATASEG	18
# define 	STACKSEG	22
# define	AVLTSS		9
# define	BUSYTSS	11
# define	LDTSEG		2

extern void LGDTR(int,struct GDTstruct *);
extern void LIDTR(int,struct IDTstruct *);
extern void LLDTR(int);

extern void LTR(unsigned int);

extern int CallTask(unsigned int);

extern int STR();
extern int SGDTRL();
extern int SGDTRB();
extern int SLDTR();
extern int GetEFlags();
extern void SetEFlags(unsigned int);

extern unsigned char in(unsigned short);
extern void out(unsigned short, unsigned char);

static int NGdt;

struct SectStruct
{
	unsigned char data[512];
};

struct TSSStruct *tss;

extern int GData;

# define	READ	3
# define	WRITE	4


# define DIR	0x10

extern inline int strncmp(const char * cs,const char * cd,int count)
{
	int i,r;
	for(i=0;i<count;i++)
	{
		if(r=*(cs+i)-*(cd+i)) break;
	}
	return(r);
}

struct fatdirstruct
{
	char name[11];
	unsigned char attrib;
	unsigned char res0;
	unsigned char CTime10mS;
	unsigned short CTime;
	unsigned short CDate;
	unsigned short ADate;
	unsigned short ClustHigh;
	unsigned short UTime;
	unsigned short UDate;
	unsigned short ClustLow;
	unsigned int Size;
};

static unsigned int *table;

static unsigned int pagefile,filesys,filemngr,pagingsys,taskmngr,scm;

struct bpbstruct
{
	unsigned char jmpCode[3];
	char bsOemName[8];
	unsigned short bpbBytesPerSector;
	unsigned char bpbSectorsPerCluster;              
	unsigned short bpbReservedSectors;               
	unsigned char bpbNumberOfFATs;               
	unsigned short bpbRootEntries;  
	unsigned short bpbTotalSectors;  
	unsigned char bpbMedia;              
	unsigned short bpbSectorsPerFAT;             
	unsigned short bpbSectorsPerTrack;             
	unsigned short bpbHeadsPerCylinder;              
	unsigned int bpbHiddenSectors;               
	unsigned int bpbTotalSectorsBig;
	unsigned int bpbSectsPerFAT32;
	unsigned short bpbExtFlags;
	unsigned short bpbFSVer;
	unsigned int bpbRootClus;
	unsigned short bpbFSInfo;
	unsigned short bpbBkBootSec;
	unsigned int bpbReserved1;
	unsigned int bpbReserved2;
	unsigned int bpbReserved3;
	unsigned char bsDriveNumber;          
	unsigned char bsUnused;             
	unsigned char bsExtBootSignature;               
	unsigned int bsSerialNumber;             
	char bsVolumeLabel[11];
	char bsFileSystem[8];
	char bootcode[420];
	unsigned short bootsign;
};


static unsigned int fatstartlba;
static unsigned int fatsects;
static unsigned int datastartlba;

static unsigned char bpbSectorsPerCluster;
static unsigned short bpbBytesPerSector;
static unsigned int bpbRootClus;


int DiskManager(unsigned int cmd,unsigned int lba,unsigned int nsects,unsigned int drive,unsigned int sect,unsigned int process)
{
	nsects=nsects&0xFFFF;
	drive=(drive<<16)&0xFFFF0000;
	nsects=nsects|drive;
/*	(tss+5)->Eax=cmd;
	(tss+5)->Ebx=lba;
	(tss+5)->Ecx=nsects;
	(tss+5)->Edx=sect;
	CallTask(10*8);
	return((tss+5)->Eax);*/
	return(DM(cmd,lba,nsects,sect,process));
}



void initSectBuffer()
{
	int i;
	table=(unsigned int *)&GData;
	if(((unsigned int)table)%4096) table=(unsigned int *)((((unsigned int)table)/4096)*4096+4096);
	for(i=0;i<20;i++)
	{
		*(table+i)=0;		//table+((i+1)*128);
	}
	//printk("\nGdata : 0x%x\nTable : 0x%x",(unsigned int)&GData,(unsigned int)table);
}

unsigned int * LoadFATSect(unsigned int fatsect)
{
	fatsect=fatstartlba+fatsect;
	int i;
	static n=0;
	for(i=0;i<20;i++)
	{
		if(*(table+i)==fatsect) return(table+((i+1)*128));
		if(*(table+i)==0)
		{
			DiskManager(READ,fatsect,1,(unsigned int) -1,(unsigned int)(table+((i+1)*128)),0);
			*(table+i)=fatsect;
			return(table+((i+1)*128));
		}
	}
	DiskManager(READ,fatsect,1,(unsigned int) -1,(unsigned int)(table+((n+1)*128)),0);
	*(table+n)=fatsect;
	n++;
	if(n==20) n=0;
	return(table+((n+1)*128));
}



unsigned int getNextClust(unsigned int dirclust)
{
	unsigned int *fat;
	unsigned int fatsect,item;
	fatsect=dirclust/128;
	item=dirclust%128;
	fat=(unsigned int *)LoadFATSect(fatsect);
	return((*(fat+item))&0x0FFFFFFF);
}

extern unsigned int PS(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);

void LoadFileIntoVM(unsigned int fileclust,unsigned int processId,unsigned int page)
{
	int i;
	i=0;
	unsigned int count;
	unsigned int *p_aPage;
	p_aPage=(unsigned int *)(0x00100000+8192+8192);
	count=bpbSectorsPerCluster/8;
	if(bpbSectorsPerCluster%8) { printk("\nCluster size not a multiple of 4KB is not allowed..."); hangon(); }
	while((fileclust)&&(fileclust<0x0FFFFFF7))
	{
		//printk("\nFM clust =0x%x",fileclust);

			/*DiskManager(READ,datastartlba+(fileclust-2)*bpbSectorsPerCluster,8,(unsigned int)-1,(unsigned int) p_aPage,0);
			//printk("\nContents of FM : \n%s",(char *)p_aPage);
			for(i=0;i<count;i++)
			{
				PS(9,(((unsigned int)p_aPage)/4096),processId,page,0);
			}
			//p_aPage=(unsigned int *) 0x01FDF000;
			//printk("\nContents of FM : \n%s",(char *)p_aPage);*/

		for(i=0;i<count;i++)
		{
			DiskManager(5,datastartlba+(fileclust-2)*bpbSectorsPerCluster,8,(unsigned int)-1,page*4096,processId);
			page++;
		}
		fileclust=getNextClust(fileclust);
	}
}

void LoadFile(unsigned int addr,unsigned int fileclust)
{
	while((fileclust)&&(fileclust<0x0FFFFFF7))
	{
		DiskManager(READ,datastartlba+(fileclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,(unsigned int)-1,addr,0);
		addr=addr+(bpbBytesPerSector*bpbSectorsPerCluster);
		fileclust=getNextClust(fileclust);
	}
}

static unsigned int pageclust;
static unsigned int n_pageclust;
static unsigned int PhysicalMemory;

static char fname[100]="prgfilej.iur/turbo.dir/tt.exe";

extern void EnablePaging(unsigned int);
void formalisePageFile();

/*struct FTSslot
{
	unsigned short tick;
	unsigned char second;
	unsigned char minute;
	unsigned char hour;
	unsigned char day;
	unsigned char month;
	unsigned char year;
	unsigned char fgpKeyStart;
	unsigned char fgpKeyEnd;
	unsigned short fgpKey[16];
	unsigned char bgpKeyStart;
	unsigned char bgpKeyEnd;
	unsigned short bgpKey[16];
	unsigned char schstd;
	unsigned int tickcount;
	unsigned int schdtime;
};

static struct FTSslot *fts;*/

extern unsigned int TM(int);

unsigned int getTime()
{
	return(TM(4));
}

unsigned int getDate()
{
	return(TM(5));	
}

short getKey()
{
	return(TM(2));
}

short getCh()
{
	return(TM(3));
}

char getChar()
{
	char ch=0;
	while(!ch)
		ch=getCh();
	//printk("%c",ch);
	return(ch);
}

char * gets(char *str)
{
	char ch=0;
	int i=0;
	while(ch!='\n')
	{
		ch=getChar();
		str[i]=ch; i++;
	}
	str[i]=0;
	return(str);
}

int getNumber()
{
	char ch=0,overflowed=0;
	unsigned int oldresult,result=0;
	while(ch!='\n')
	{
		ch=getCh();
		if((ch>='0')&&(ch<='9'))
		{
			//printk("%c",ch);
			oldresult=result;
			result=result*10+ch;
			if(oldresult>result) overflowed=1;
		}
	}
	if(overflowed) return(-1);
	return(result);
}

int FS(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);

void initMe()
{
	unsigned char hdtype;
	int i,temp;
	unsigned int pdbr;
	struct GDTstruct *gdt;
	struct GDTstruct *ldt;
	struct IDTstruct *idt;
	struct SectStruct *sect;
	struct bpbstruct *bpb;
	unsigned int sysdir,dirclust;
	static const char filenames[6][12]={"REDPAGE SYS","PAGNGSYSBIN","FILEMNGRBIN","DFILESYSBIN","TASKMNGRBIN","SYSCTMODBIN"};
	//fts=(struct FTSslot *) 0x00100000;
	printk("\nReinitialising GDT...");
	tss=(struct TSSStruct *) (0x00100000+128);	//first slot is sysinfoslot... only 16 TSS slots are allowed...
	gdt=(struct GDTstruct *)(0x00100000+2048);	// only 64 entries are allowed...
	ldt=(struct GDTstruct *)(0x00100000+2048+512);	// only 64 entries are allowed...
	idt=(struct IDTstruct *)(0x00100000+2048+1024);	// only 128 entries are allowed...
	EFlags=GetEFlags();
	makeGDTEntry(gdt,NULLSEG,0,0,0);		// gdt,type,prev,base,limit
	makeGDTEntry(ldt,NULLSEG,0,0,0);
	makeGDTEntry(gdt+1,CODESEG,0,0,0xFFFFF);
	makeGDTEntry(gdt+2,DATASEG,0,0,0xFFFFF);
	makeGDTEntry(gdt+3,STACKSEG,0,0,0);
	makeGDTEntry(gdt+4,LDTSEG,0,(unsigned int)ldt,0x7);
	printk("\nCreating Tasks...");
	makeTSS(tss);	//FGProcess
	makeTSS(tss+1);	//BGProcess
	makeTSS(tss+2);	//TaskManager
	makeTSS(tss+3);	//Timer Int
	makeTSS(tss+4);	//KBD Int
	makeTSS(tss+5);	//HD Int
	makeTSS(tss+6);	//Exception
	makeTSS(tss+7);	//Paging Exception
	makeTSS(tss+8);	//Time Request
	makeTSS(tss+9);	//Key Request
	makeTSS(tss+10);	//Paging System
	makeTSS(tss+11);	//HD Int for Paging System
	makeTSS(tss+12);	//File Manager
	makeTSS(tss+13);	//File System
	makeTSS(tss+14);	//System Control Module
	makeGDTEntry(gdt+5,AVLTSS,0,(unsigned int)tss,0x67);
	makeGDTEntry(gdt+6,AVLTSS,0,(unsigned int)(tss+1),0x67);
	makeGDTEntry(gdt+7,AVLTSS,0,(unsigned int)(tss+2),0x67);
	makeGDTEntry(gdt+8,AVLTSS,0,(unsigned int)(tss+3),0x67);
	makeGDTEntry(gdt+9,AVLTSS,0,(unsigned int)(tss+4),0x67);
	makeGDTEntry(gdt+10,AVLTSS,0,(unsigned int)(tss+5),0x67);
	makeGDTEntry(gdt+11,AVLTSS,0,(unsigned int)(tss+6),0x67);
	makeGDTEntry(gdt+12,AVLTSS,0,(unsigned int)(tss+7),0x67);
	makeGDTEntry(gdt+13,AVLTSS,0,(unsigned int)(tss+8),0x67);
	makeGDTEntry(gdt+14,AVLTSS,0,(unsigned int)(tss+9),0x67);
	makeGDTEntry(gdt+15,AVLTSS,0,(unsigned int)(tss+10),0x67);
	makeGDTEntry(gdt+16,AVLTSS,0,(unsigned int)(tss+11),0x67);
	makeGDTEntry(gdt+17,AVLTSS,0,(unsigned int)(tss+12),0x67);
	makeGDTEntry(gdt+18,AVLTSS,0,(unsigned int)(tss+13),0x67);
	makeGDTEntry(gdt+19,AVLTSS,0,(unsigned int)(tss+14),0x67);
	LGDTR((20*8)-1,gdt);			// on error,check whether gdt is loaded
	LLDTR(4*8);
	LTR(5*8);
	NGdt=11;

	(tss)->ProcessId=1;

	// (tss+1)	// BGProcess

	// (tss+2)	// Task Manager
	printk("\nCreating Support Driver Task");
	printk("\nCreating Support Driver thread for Timer");
	(tss+3)->Eip=0x00000510;
	(tss+3)->Ebp=0x0001FFFF;
	(tss+3)->Esp=0x0001FFFF;
	(tss+3)->Esp0=0x0001FFFF;
	(tss+3)->Esp1=0x0001FFFF;
	(tss+3)->Esp2=0x0001FFFF;
	(tss+3)->ProcessId=2;

	printk("\nCreating Support Driver thread for Keyboard");
	(tss+4)->Eip=0x00000520;
	(tss+4)->Ebp=0x0001BFFF;
	(tss+4)->Esp=0x0001BFFF;
	(tss+4)->Esp0=0x0001BFFF;
	(tss+4)->Esp1=0x0001BFFF;
	(tss+4)->Esp2=0x0001BFFF;
	(tss+4)->ProcessId=2;

	printk("\nCreating Support Driver thread for Hard Disk");
	(tss+5)->Eip=0x00000530;
	(tss+5)->Ebp=0x00017FFF;
	(tss+5)->Esp=0x00017FFF;
	(tss+5)->Esp0=0x00017FFF;
	(tss+5)->Esp1=0x00017FFF;
	(tss+5)->Esp2=0x00017FFF;
	(tss+5)->ProcessId=2;

	printk("\nCreating Support Driver thread for Hard Disk, used by Paging System");
	(tss+11)->Eip=0x00000530;		// HD thread for PS...
	(tss+11)->Ebp=0x00067FFF;		// Divides the stack of PS...
	(tss+11)->Esp=0x00067FFF;
	(tss+11)->Esp0=0x00067FFF;
	(tss+11)->Esp1=0x00067FFF;
	(tss+11)->Esp2=0x00067FFF;
	(tss+11)->ProcessId=2;

	printk("\nCreating Support Driver thread for Exception Handler");
	(tss+6)->Eip=0x00000540;
	(tss+6)->Ebp=0x00013FFF;
	(tss+6)->Esp=0x00013FFF;
	(tss+6)->Esp0=0x00013FFF;
	(tss+6)->Esp1=0x00013FFF;
	(tss+6)->Esp2=0x00013FFF;
	(tss+6)->ProcessId=2;

	printk("\nInitialising PICs");
	disableallirqs();
	init_pics(0x20, 0x28);
	printk("\nInitialising Timer");
	(tss+3)->Eax=1;
	CallTask(8*8);
	printk("\nInitialising KeyBoard");
	(tss+4)->Eax=1;
	CallTask(9*8);

	for(i=0;i<32;i++)
	{
		makeIDTEntry(idt+i,0,1,11*8);
	}
	makeIDTEntry(idt+32,0,1,8*8);
	makeIDTEntry(idt+33,0,1,9*8);

	LIDTR((34*8)-1,idt);

	setInt();
	EFlags=GetEFlags();
	(tss+4)->Eflags=EFlags;
				// interrupts should not be allowed while calling HD because it uses TaskId...

	printk("\nInitialising HardDisk");
	DM(1,0,0,0,0);
//	printk("\nHarddisk initialised");
/*	(tss+5)->Eax=1;
	CallTask(10*8);*/
	//DiskManager(READ,0,1,(unsigned int) -1,(unsigned int)sect,0);
	//printk("\n%s",(char *)sect);
	bpb=(struct bpbstruct *) &GData;
	DiskManager(READ,0,1,(unsigned int)-1,(unsigned int) bpb,0);
	bpbSectorsPerCluster=bpb->bpbSectorsPerCluster;
	bpbBytesPerSector=bpb->bpbBytesPerSector;
	bpbRootClus=bpb->bpbRootClus;
	fatstartlba=bpb->bpbReservedSectors;
	fatsects=((bpb->bpbTotalSectorsBig/bpb->bpbSectorsPerCluster)*4)/(bpb->bpbBytesPerSector);
	if(((bpb->bpbTotalSectorsBig/bpb->bpbSectorsPerCluster)*4)%(bpb->bpbBytesPerSector)) fatsects++;
	if(fatsects<(bpb->bpbSectsPerFAT32))fatsects=bpb->bpbSectsPerFAT32;
	datastartlba=fatstartlba+fatsects*bpb->bpbNumberOfFATs;

	initSectBuffer();
	if(bpbRootClus==0) { printk("\nRoot Directory not available"); hangon(); }

	dirclust=bpbRootClus;
	sysdir=0;
	struct fatdirstruct dir[16*bpbSectorsPerCluster];
	dirclust=dirclust&0x0FFFFFFF;
	while((dirclust)&&(dirclust<0x0FFFFFF7))
	{
		DiskManager(READ,datastartlba+(dirclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,(unsigned int)-1,(unsigned int)&dir,0);
		for(i=0;i<16*bpbSectorsPerCluster;i++)
		{
			if((!strncmp(dir[i].name,"REDSKY     ",11))&&(dir[i].attrib&0x10))
				{
					sysdir=(((unsigned int)dir[i].ClustHigh)<<16)|((unsigned int)dir[i].ClustLow);
					break;
				}
		}
		if(sysdir) break;
		dirclust=getNextClust(dirclust);
		dirclust=dirclust&0x0FFFFFFF;
	}

	if(sysdir==0)
	{
		printk("\nCannot find system directory, REDSKY");
		hangon();
	}
	//printk("\nTss : %d",(unsigned int) tss);
	dirclust=sysdir;
	
	pagefile=0;
	filesys=0;
	filemngr=0;
	pagingsys=0;
	taskmngr=0;
	scm=0;
	temp=1;
	
	//printk("\nTss : %d",(unsigned int) tss);
	dirclust=dirclust&0x0FFFFFFF;
	DiskManager(READ,datastartlba+(dirclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,(unsigned int)-1,(unsigned int)&dir,0);
	while((dirclust)&&(dirclust<0x0FFFFFF7))
	{
		DiskManager(READ,datastartlba+(dirclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,(unsigned int)-1,(unsigned int)&dir,0);
		for(i=0;i<16*bpbSectorsPerCluster;i++)
		{
				if(dir[i].name[0]==0) break;
				if((!pagefile)&&(!strncmp(dir[i].name,filenames[0],11))&&(!(dir[i].attrib&0x18)))
				{
					pagefile=(((unsigned int)dir[i].ClustHigh)<<16)|((unsigned int)dir[i].ClustLow);
					pagefile=pagefile&0x0FFFFFFF;
					if((pagefile==0)||(pagefile>=0x0FFFFFF7))
					{
						printk("\nPaging file, REDSKY/REDPAGE.SYS is empty");
						temp=0;
						pagefile=1;
					}
				}
				if((!pagingsys)&&(!strncmp(dir[i].name,filenames[1],11))&&(!(dir[i].attrib&0x18)))
				{
					pagingsys=(((unsigned int)dir[i].ClustHigh)<<16)|((unsigned int)dir[i].ClustLow);
					pagingsys=pagingsys&0x0FFFFFFF;
					if((pagingsys==0)||(pagingsys>=0x0FFFFFF7))
					{
						printk("\nPaging system, REDSKY/PAGNGSYS.BIN is empty");
						temp=0;
						pagingsys=1;
					}
				}
				if((!filemngr)&&(!strncmp(dir[i].name,filenames[2],11))&&(!(dir[i].attrib&0x18)))
				{
					filemngr=(((unsigned int)dir[i].ClustHigh)<<16)|((unsigned int)dir[i].ClustLow);
					filemngr=filemngr&0x0FFFFFFF;
					if((filemngr==0)||(filemngr>=0x0FFFFFF7))
					{
						printk("\nFile manager, REDSKY/FILEMNGR.BIN is empty");
						temp=0;
						filemngr=1;
					}
				}
				if((!filesys)&&(!strncmp(dir[i].name,filenames[3],11))&&(!(dir[i].attrib&0x18)))
				{
					filesys=(((unsigned int)dir[i].ClustHigh)<<16)|((unsigned int)dir[i].ClustLow);
					filesys=filesys&0x0FFFFFFF;
					if((filesys==0)||(filesys>=0x0FFFFFF7))
					{
						printk("\nDefault file system, REDSKY/DFILESYS.BIN is empty");
						temp=0;
						filesys=1;
					}
				}
				if((!taskmngr)&&(!strncmp(dir[i].name,filenames[4],11))&&(!(dir[i].attrib&0x18)))
				{
					taskmngr=(((unsigned int)dir[i].ClustHigh)<<16)|((unsigned int)dir[i].ClustLow);
					taskmngr=taskmngr&0x0FFFFFFF;
					if((taskmngr==0)||(taskmngr>=0x0FFFFFF7))
					{
						printk("\nTask manager, REDSKY/TASKMNGR.BIN is empty");
						temp=0;
						taskmngr=1;
					}
				}
				if((!scm)&&(!strncmp(dir[i].name,filenames[5],11))&&(!(dir[i].attrib&0x18)))
				{
					scm=(((unsigned int)dir[i].ClustHigh)<<16)|((unsigned int)dir[i].ClustLow);
					scm=scm&0x0FFFFFFF;
					if((scm==0)||(scm>=0x0FFFFFF7))
					{
						printk("\nSystem control module, REDSKY/SYSCTMOD.BIN is empty");
						temp=0;
						scm=1;
					}
				}
		}
		if((pagefile)&&(pagingsys)&&(filemngr)&&(filesys)&&(taskmngr)&&(scm)) break;
		dirclust=getNextClust(dirclust);
	}
	
	if(!pagefile) { printk("\nCannot find paging file, REDSKY/REDPAGE.SYS");temp=0;}
	if(!pagingsys){ printk("\nCannot find paging system, REDSKY/PAGNGSYS.BIN"); temp=0; }
	if(!filemngr){ printk("\nCannot find file manager, REDSKY/FILEMNGR.BIN"); temp=0; }
	if(!filesys){ printk("\nCannot find default file system, REDSKY/DFILESYS.BIN"); temp=0; }
	if(!taskmngr){ printk("\nCannot find task manager, REDSKY/TASKMNGR.BIN"); temp=0; }
	if(!scm){ printk("\nCannot find system control module, REDSKY/SYSCTMOD.BIN"); temp=0; }
	if(!temp) hangon();
	printk("\nFormalising Paging File, REDSKY/REDPAGE.SYS");
	formalisePageFile();
	
	printk("\nCreating Paging System Task");
	LoadFile(0x00300000,pagingsys);	// Paging System should be created as separate task 
	(tss+10)->Eip=0x00300000;
	(tss+10)->Ebp=0x0006FFFF;
	(tss+10)->Esp=0x0006FFFF;
	(tss+10)->Esp0=0x0006FFFF;
	(tss+10)->Esp1=0x0006FFFF;
	(tss+10)->Esp2=0x0006FFFF;
	(tss+10)->ProcessId=0;

	printk("\nCreating Paging Exception Handler thread");
	(tss+7)->Eip=0x00300010;		// Paging Exception... Paging System and Paging Exception will not occur at same... So they share the stack...
	(tss+7)->Ebp=0x0006FFFF;
	(tss+7)->Esp=0x0006FFFF;
	(tss+7)->Esp0=0x0006FFFF;
	(tss+7)->Esp1=0x0006FFFF;
	(tss+7)->Esp2=0x0006FFFF;
	(tss+7)->ProcessId=0;

	printk("\nInitialising Paging");

	printk("\nInitialising VM for PagingSystem");
	pdbr=PS(1,pageclust,n_pageclust,PhysicalMemory,0);
	if(pdbr<4096) { printk("\nError in initialising VM for PagingSystem...Error : %d",pdbr); hangon(); }
	(tss+10)->Cr3=pdbr;
	(tss+7)->Cr3=pdbr;
	//printk("\nPdbr for PagingSystem : 0x%x",pdbr);

	//makeIDTEntry(idt+14,0,1,12*8);
	//clrInt();
	//setInt();
	//EFlags=GetEFlags();

	printk("\nInitialising paging for Support Driver");
	pdbr=PS(2,2,0,0,0);
	if(pdbr<4096) { printk("\nError in initialising VM for SupportDriver. Pdbr : 0x%x",pdbr); hangon(); }
	(tss+3)->Cr3=pdbr;
	(tss+4)->Cr3=pdbr;
	(tss+5)->Cr3=pdbr;
	(tss+6)->Cr3=pdbr;
	//printk("\nPdbr for SupportDriver : 0x%x",pdbr);

	printk("\nInitialising paging for SystemAssembler(Myself)");
	pdbr=PS(2,1,0,0,0);
	if(pdbr<4096) { printk("\nError in initialising VM for SystemAssembler"); hangon(); }
	(tss)->Cr3=pdbr;
	//printk("\nPdbr for SystemAssembler : 0x%x",pdbr);

	pdbr=(tss+10)->Cr3;
	printk("\nEnabling Paging");
	EnablePaging(pdbr);
	printk("\nCreating VM for File Manager");
	pdbr=PS(3,20,300,0,0);
	//printk("\nFile Manager VM Creation : 0x%x",pdbr);
	(tss+12)->Eip=0x00000000;
	(tss+12)->Ebp=0x0;
	(tss+12)->Esp=0x0;
	(tss+12)->Esp0=0x0;
	(tss+12)->Esp1=0x0;
	(tss+12)->Esp2=0x0;
	(tss+12)->ProcessId=20;
	(tss+12)->Cr3=pdbr;
	LoadFileIntoVM(filemngr,20,0);
	printk("\nInitialising File Manager");
	//PS(9,256,20,256,0);
	(tss+12)->Eax=1;
	CallTask(17*8);
	temp=(tss+12)->Eax;
	//printk("File Manager returned : %d",temp);
	temp=formalisePath(fname,(char *)&GData);
	//printk("\nFormalise filename retd : %d\n",temp);
	//printk("%s",&GData);
	printk("\nCreating VM for Task Manager");
	pdbr=PS(3,21,300,0,0);
	//printk("\nTask Manager VM Creation : 0x%x",pdbr);
	(tss+2)->Eip=0x00000000;
	(tss+2)->Ebp=0xFFFF;
	(tss+2)->Esp=0xFFFF;
	(tss+2)->Esp0=0xFFFF;
	(tss+2)->Esp1=0xFFFF;
	(tss+2)->Esp2=0xFFFF;
	(tss+2)->ProcessId=21;
	(tss+2)->Cr3=pdbr;
	LoadFileIntoVM(taskmngr,21,0);
	printk("\nInitialising Task Manager");
	TM(1);
	printk("\nCreating VM for File System");
	pdbr=PS(3,25,300,0,0);
	//printk("\nFile System VM Creation : 0x%x",pdbr);
	(tss+13)->Eip=0x00000000;
	(tss+13)->Ebp=255*4096;
	(tss+13)->Esp=255*4096;
	(tss+13)->Esp0=255*4096;
	(tss+13)->Esp1=255*4096;
	(tss+13)->Esp2=255*4096;
	(tss+13)->ProcessId=25;
	(tss+13)->Cr3=pdbr;
	LoadFileIntoVM(filesys,25,0);
	printk("\nInitialising File System");
	temp=FS(1,(unsigned int)'C',0,0,0,0);
	//printk("\nFile System returned : %d",temp);
	printk("\nCreating VM for System Control Module");
	pdbr=PS(3,22,300,0,0);
	//printk("\nSystem Control Module VM Creation : 0x%x",pdbr);
	(tss+14)->Eip=0x00000000;
	(tss+14)->Ebp=0xFFFF;
	(tss+14)->Esp=0xFFFF;
	(tss+14)->Esp0=0xFFFF;
	(tss+14)->Esp1=0xFFFF;
	(tss+14)->Esp2=0xFFFF;
	(tss+14)->ProcessId=22;
	(tss+14)->Cr3=pdbr;
	LoadFileIntoVM(scm,22,0);
}	

void formalisePageFile()
{
	int i,j;
	unsigned int count;
	unsigned int page[1024];
	unsigned int fileclust=pagefile;
	unsigned int lastclust=fileclust,temp;
	unsigned int prevclust=fileclust;
	count=(bpbBytesPerSector*bpbSectorsPerCluster)/4096;
	if(count==0) { printk("\nPaging file clusters must have atleast 8 sectors per cluster..."); hangon(); }
	if(count>8) count=8;
	//printk("\npagefile : 0x%x",pagefile);
	n_pageclust=1;
	while((fileclust)&&(fileclust<0x0FFFFFF7))
	{
		for(i=0;i<1024;)
		{
			for(j=0;j<count;j++,i++)
			{
				page[i]=(lastclust<<4) | j ;
			}
			lastclust=getNextClust(prevclust);
			prevclust=lastclust;
			if((!lastclust)||(lastclust>=0x0FFFFFF7)) break;
			n_pageclust++;
		}
		for(;i<1024;i++)
		{
			page[i]=0;
		}
		pageclust=fileclust;
		DiskManager(WRITE,datastartlba+(fileclust-2)*bpbSectorsPerCluster,8,(unsigned int)-1,(unsigned int)page,0);
		lastclust=fileclust;
		fileclust=prevclust;
	}
	pageclust=(pageclust&0x0FFFFFFF)|(((count-1)&0x0F)<<28);
	n_pageclust=n_pageclust*count;
	//printk("pageclust : 0x%x, count : 0x%x, n_pageclust : 0x%x",pageclust,count,n_pageclust);
	return;
}

void kmain()
{
	static char fname[23]="REDSKY     ";
	static char fname2[20]="INNEW      ";
	static char fname3[20]="TEST    TXT";
	static char fname4[20]="TEST3   TXT";
	char str[20];
	char key,hour,minute,second,tenmstick,day,month;
	short year;
	unsigned int date,time,num;
	int (*supdrv)(),a,fid,fid2,fid3,fid4,fid5;
	unsigned int *temp,totpages,mempages,diskpages;
	int (*fs)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
	unsigned int *p_aPage=(unsigned int *)(0x00100000+8192+8192);
	printk("\t\t\t    R E D S K Y\n\n");
	printk("\nSystem Assembler\tVer. 1.0");
	supdrv=(int (*)()) 0x00000500;
	a=(*supdrv)();
	printk("\nSupport Driver\tVer. %d.%d",a>>16,a&0x0FFFF);
	PhysicalMemory=getPhyMem();
	printk("\nPhysical Memory  :  %d MB",PhysicalMemory);
	initMe();

/*	LoadFile(0x00200000,filesys);
	fs=(int(*)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int))0x00200000;
	printk("\nInitialising File System");
	a=(*fs)(1,(unsigned int)'C',0,0,0);
	printk("\nFile System returned : %d",a);
	fid=(*fs)(8,(unsigned int)fname,0,0,0);	//6-readonly,7-writeonly...
	if(fid<5) 
	{
		printk("\nThe file //REDSKY/TEST.TXT does not exist, FS Ret : %d",fid);
		setInt();
		return;
	}
	printk("\nReading File");
	a=(*fs)(3,fid,0,1,(unsigned int)&GData);	// 3-read,4-write...
	printk("\nFile System returned : %d",a);
	printk("\nFile Content :\n%s",(char *)&GData);
	printk("\nCreating Dir");
	fid3=(*fs)(16,fid,(unsigned int)fname2,0,0);	//6-readonly,7-writeonly...
	printk("\nFile System returned %d",fid3);
	printk("\nCreating File");
	fid4=(*fs)(15,fid,(unsigned int)fname4,0,0);	//6-readonly,7-writeonly...
	printk("\nFile System returned %d",fid4);
	printk("\nShutting down File System");
	(*fs)(10,0,0,0,0);	//10-shutdown...*/
	//printk("\nFinished Setting up the system...");
	key=0;
	/*while(1)
	{
		key=getCh();
		if(key) printk("%d",key);
	}*/
	time=getTime();
	hour=*(((char *)&time)+3);
	minute=*(((char *)&time)+2);
	second=*(((char *)&time)+1);
	tenmstick=*((char *)&time);
	printk("\nTime : %d:%d:%d.%d",hour,minute,second,tenmstick);
	date=getDate();
	year=*(((short *)&date)+1)+1980;
	month=*(((char *)&date)+1);
	day=*((char *)&date);
	printk("\nDate : %d/%d/%d",day,month,year);
	/*gets(str);
	printk("\n%s",str);
	num=getNumber();
	printk("\n%d",num);*/
	(tss+13)->Es=0x10;
	//fid=FS(3,(unsigned int)fname,0,0,0,0);
	//printk("\nFile System returned : %d",fid);
	printk("\nCalling System Control Module");
	CallTask((14+5)*8);
	return;	
}
