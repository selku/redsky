# define	TASKMNGR	2
# define	CPYTOME	8
# define	CPYFRMME	9



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


# define PIC1 0x20

extern inline int strncmp(const char * cs,const char * cd,int count)
{
	int i,r;
	for(i=0;i<count;i++)
	{
		if(r=*(cs+i)-*(cd+i)) break;
	}
	return(r);
}

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

void inline prtDelay()
{
	inb(PIC1+1);
}

# define CTCCMD 0x43
# define CTCCH0 0x40
void setCTC2TMS()	// sets timer to give interrupt every 10 ms.
{
	outb(PIC1 + 1, 0xFF);
	outb(CTCCMD,54);	// 54-channel 0,access lowbyte then high byte,op mode=3,binary
	prtDelay();
	outb(CTCCH0,0x9B);	//0x2E9B = 11931-divisor which gives approximately 10 ms
	prtDelay();
	outb(CTCCH0,0x2E);
}

extern int getRTCTime();
extern int getRTCDate();

struct FTSslot
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

static struct FTSslot *fts;

extern int printk(const char *, ...);

int initTimer()
{
	unsigned char state,*tt;
	unsigned int time,date;
	fts=(struct FTSslot *) 0x00100000;
	fts->schstd=0;
	fts->tickcount=0;
	for(;;)
	{
		outb(0x70,0xA);
		state=inb(0x71);
		if(state&0x80) break;
	}
	for(;;)
	{
		outb(0x70,0xA);
		state=inb(0x71);
		if(!(state&0x80)) break;
	}
	time=getRTCTime();
	date=getRTCDate();
	tt=(unsigned char *) &time;
	(fts->second)=tt[0];
	(fts->second)=((fts->second)&0x0F)+(10*((fts->second)>>4));
	(fts->minute)=tt[1];
	(fts->minute)=((fts->minute)&0x0F)+(10*((fts->minute)>>4));
	(fts->hour)=tt[2];
	state=tt[3];
	if(state&0x02)
		(fts->hour)=((fts->hour)&0x0F)+(10*((fts->hour)>>4));
	else if((fts->hour)&0x80)
		(fts->hour)=((fts->hour)&0x0F)+(10*(((fts->hour)&0x70)>>4))+12;
	else
		(fts->hour)=((fts->hour)&0x0F)+(10*(((fts->hour)&0x70)>>4));
	tt=(unsigned char *) &date;
	(fts->day)=tt[0];
	(fts->day)=((fts->day)&0x0F)+(10*((fts->day)>>4));
	(fts->month)=tt[1];
	(fts->month)=((fts->month)&0x0F)+(10*((fts->month)>>4));
	(fts->year)=tt[2];
	(fts->year)=((fts->year)&0x0F)+(10*((fts->year)>>4));	
	setCTC2TMS();
	(fts->tick)=0;
	// code for initialising counters
	//printk("\nFrom initTimer");
	printk("\nDate : %d/%d/%d",(fts->day),(fts->month),2000+(fts->year));
	printk("\tTime : %d:%d:%d",(fts->hour),(fts->minute),(fts->second));
	return(0);
}

extern unsigned char inline curmaxdays(unsigned char m)
{
	static unsigned char maxdays[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	m=maxdays[m-1];
	if((m==28)&&((fts->year)%4==0))
		m++;
	return m;
}

int TimerInt()
{
	struct TSSStruct *tss=(struct TSSStruct *)(0x00100000+128);
	unsigned short myTaskId=3,prevTaskId,currentTaskId;
	int i;
	//printk("\nStarted TimerInt");
	fts->tick++;
	fts->tickcount++;
	if((fts->tick)==100)
	{
		(fts->tick)=0;
		(fts->second)++;
		if((fts->second)==60) 
		{
			(fts->second)=0; 
			(fts->minute)++;  
			if((fts->minute)==60)
			{
				(fts->minute)=0;
				(fts->hour)++;
				if((fts->hour)==24)
				{
					(fts->hour)=0;
					(fts->day)++;
					if((fts->day)==(curmaxdays((fts->month))+1))
					{
						(fts->day)=1;
						(fts->month)++;
						if((fts->month)==13)
						{
							(fts->month)=1;
							(fts->year)++;
						}
					}
				}
			}
		}
	//printk("\n Date: %d/%d/%d, Time: %d:%d:%d ",(fts->day),(fts->month),(fts->year),(fts->hour),(fts->minute),(fts->second));
	}
	outb(0x20,0x20);
	if((fts->schstd)&&(fts->schdtime==fts->tickcount))
	{
		currentTaskId=myTaskId;
		prevTaskId=(tss+currentTaskId)->Back_Link/8-5;
		for(i=0;i<16;i++)
		{
			if(prevTaskId<3) break;
			currentTaskId=prevTaskId;
			prevTaskId=(tss+currentTaskId)->Back_Link/8-5;
		}
		if(prevTaskId!=TASKMNGR)
		{
			(tss+TASKMNGR)->Eax=5;	// Do schedule
			(tss+currentTaskId)->Back_Link=(TASKMNGR+5)*8;
		}
	}	
	return(0);
}





# define SHIFT	1
# define ALT	2
# define CTRL	3

# define LSHIFT	1
# define RSHIFT	2
# define LCTRL	4
# define RCTRL	8
# define LALT	16
# define RALT	32

# define TABCODE	0xF

# define	SCRL	1
# define	NUM	2
# define	CAPS	4

# define LSHIFTCODE	0x2A
# define RSHIFTCODE	0x36
# define ULSHIFTCODE	0x80+0x2A
# define URSHIFTCODE	0x80+0x36
# define LCTRLCODE	0x1D
# define RCTRLCODE	0xE01D
# define ULCTRLCODE	0x80+0x1D
# define URCTRLCODE	0xE0*256+0x80+0x1D
# define LALTCODE	0x38
# define RALTCODE	0xE038
# define ULALTCODE	0x80+0x38
# define URALTCODE	0xE0*256+0x80+0x38
# define CAPSCODE	0x3A
# define NUMCODE		0x45
# define SCRLCODE	0x46


static unsigned char SKey;
static char unw;
static char LED;
static unsigned short prevcode;
static unsigned int CurSKeyList;
static unsigned char skeypos[3];
static unsigned short key[0x58][4]=
{	//normal		//shift+		//alt+		//ctrl+
	0x01*256+0x1B,	0x01*256+0,	0x01*256+0,	0x01*256+0,
	0x02*256+'1',	0x02*256+'!',	0x78*256+0,	0x2*256+0,
	0x03*256+'2',	0x03*256+'@',	0x79*256+0,	0x03*256+0,
	0x04*256+'3',	0x04*256+'#',	0x7A*256+0,	0x04*256+0,
	0x05*256+'4',	0x05*256+'$',	0x7B*256+0,	0x05*256+0,
	0x06*256+'5',	0x06*256+'%',	0x7C*256+0,	0x06*256+0,
	0x07*256+'6',	0x07*256+'^',	0x7D*256+0,	0x07*256+0,
	0x08*256+'7',	0x08*256+'&',	0x7E*256+0,	0x08*256+0,
	0x09*256+'8',	0x09*256+'*',	0x7F*256+0,	0x09*256+0,
	0x0A*256+'9',	0x0A*256+'(',	0x80*256+0,	0x0A*256+0,
	0x0B*256+'0',	0x0B*256+')',	0x81*256+0,	0x0B*256+0,
	0x0C*256+'-',	0x0C*256+'_',	0x82*256+0,	0x0C*256+0,
	0x0D*256+'=',	0x0D*256+'+',	0x83*256+0,	0x0D*256+0,
	0x0E*256+'\b',	0x0E*256+0,	0x0E*256+0,	0x0E*256+0,
	0x0F*256+'\t',	0x0F*256+0,	0x0F*256+0,	0x0F*256+0,
	0x10*256+'q',	0x10*256+'Q',	0x10*256+0,	0x10*256+0,
	0x11*256+'w',	0x11*256+'W',	0x11*256+0,	0x11*256+0,
	0x12*256+'e',	0x12*256+'E',	0x12*256+0,	0x12*256+0,
	0x13*256+'r',	0x13*256+'R',	0x13*256+0,	0x13*256+0,
	0x14*256+'t',	0x14*256+'T',	0x14*256+0,	0x14*256+0,
	0x15*256+'y',	0x15*256+'Y',	0x15*256+0,	0x15*256+0,
	0x16*256+'u',	0x16*256+'U',	0x16*256+0,	0x16*256+0,
	0x17*256+'i',	0x17*256+'I',	0x17*256+0,	0x17*256+0,
	0x18*256+'o',	0x18*256+'O',	0x18*256+0,	0x18*256+0,
	0x19*256+'p',	0x19*256+'P',	0x19*256+0,	0x19*256+0,
	0x1A*256+'[',	0x1A*256+'{',	0x1A*256+0,	0x1A*256+0,
	0x1B*256+']',	0x1B*256+'}',	0x1B*256+0,	0x1B*256+0,
	0x1C*256+'\n',	0x1C*256+'\n',	0x1C*256+0,	0x1C*256+0,
	0x1D*256+0,	0x1D*256+0,	0x1D*256+0,	0x1D*256+0,
	0x1E*256+'a',	0x1E*256+'A',	0x1E*256+0,	0x1E*256+0,
	0x1F*256+'s',	0x1F*256+'S',	0x1F*256+0,	0x1F*256+0,
	0x20*256+'d',	0x20*256+'D',	0x20*256+0,	0x20*256+0,
	0x21*256+'f',	0x21*256+'F',	0x21*256+0,	0x21*256+0,
	0x22*256+'g',	0x22*256+'G',	0x22*256+0,	0x22*256+0,
	0x23*256+'h',	0x23*256+'H',	0x23*256+0,	0x23*256+0,
	0x24*256+'j',	0x24*256+'J',	0x24*256+0,	0x24*256+0,
	0x25*256+'k',	0x25*256+'K',	0x25*256+0,	0x25*256+0,
	0x26*256+'l',	0x26*256+'L',	0x26*256+0,	0x26*256+0,
	0x27*256+';',	0x27*256+':',	0x27*256+0,	0x27*256+0,
	0x28*256+'\'',	0x28*256+'"',	0x28*256+0,	0x28*256+0,
	0x29*256+'`',	0x29*256+'~',	0x29*256+0,	0x29*256+0,
	0x2A*256+0,	0x2A*256+0,	0x2A*256+0,	0x2A*256+0,
	0x2B*256+'\\',	0x2B*256+'|',	0x2B*256+0,	0x2B*256+0,
	0x2C*256+'z',	0x2C*256+'Z',	0x2C*256+0,	0x2C*256+0,
	0x2D*256+'x',	0x2D*256+'X',	0x2D*256+0,	0x2D*256+0,
	0x2E*256+'c',	0x2E*256+'C',	0x2E*256+0,	0x2E*256+0,
	0x2F*256+'v',	0x2F*256+'V',	0x2F*256+0,	0x2F*256+0,
	0x30*256+'b',	0x30*256+'B',	0x30*256+0,	0x30*256+0,
	0x31*256+'n',	0x31*256+'N',	0x31*256+0,	0x31*256+0,
	0x32*256+'m',	0x32*256+'M',	0x32*256+0,	0x32*256+0,
	0x33*256+',',	0x33*256+'<',	0x33*256+0,	0x33*256+0,
	0x34*256+'.',	0x34*256+'>',	0x34*256+0,	0x34*256+0,
	0x35*256+'/',	0x35*256+'?',	0x35*256+0,	0x35*256+0,
	0x36*256+0,	0x36*256+0,	0x36*256+0,	0x36*256+0,
	0x37*256+'*',	0x37*256+0,	0x37*256+0,	0x96*256+0,
	0x38*256+0,	0x38*256+0,	0x38*256+0,	0x38*256+0,
	0x39*256+' ',	0x39*256+' ',	0x39*256+0,	0x39*256+0,
	0x3A*256+0,	0x3A*256+0,	0x3A*256+0,	0x3A*256+0,
	0x3B00,		0x5400,		0x6800,		0x5E00,
	0x3C00,		0x5500,		0x6900,		0x5F00,
	0x3D00,		0x5600,		0x6A00,		0x6000,
	0x3E00,		0x5700,		0x6B00,		0x6100,
	0x3F00,		0x5800,		0x6C00,		0x6200,
	0x4000,		0x5900,		0x6D00,		0x6300,
	0x4100,		0x5A00,		0x6E00,		0x6400,
	0x4200,		0x5B00,		0x6F00,		0x6500,
	0x4300,		0x5C00,		0x7000,		0x6600,
	0x4400,		0x5D00,		0x7100,		0x6700,
	0x4500,		0x4500,		0x4500,		0x4500,
	0x4600,		0x4600,		0x4600,		0x4600,
	0x47*256+'7',	0x47*256+0,	0x47*256+'7',	0x47*256+'7',
	0x48*256+'8',	0x48*256+0,	0x48*256+'8',	0x48*256+'8',
	0x49*256+'9',	0x49*256+0,	0x49*256+'9',	0x49*256+'9',
	0x4A*256+'-',	0x4A*256+0,	0x4A*256+0,	0x8E*256+0,
	0x4B*256+'4',	0x4B*256+0,	0x4B*256+'4',	0x4B*256+'4',
	0x4C*256+'5',	0x4C*256+0,	0x4C*256+'5',	0x4C*256+'5',
	0x4D*256+'6',	0x4D*256+0,	0x4D*256+'6',	0x4D*256+'6',
	0x4E*256+'+',	0x4E*256+0,	0x4E*256+0,	0x90*256+0,
	0x4F*256+'1',	0x4F*256+0,	0x4F*256+'1',	0x4F*256+'1',
	0x50*256+'2',	0x50*256+0,	0x50*256+'2',	0x50*256+'2',
	0x51*256+'3',	0x51*256+0,	0x51*256+'3',	0x51*256+'3',
	0x52*256+'0',	0x52*256+0,	0x52*256+'0',	0x52*256+'0',
	0x53*256+'.',	0x53*256+0,	0x53*256+'.',	0x53*256+'.',
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0x85*256+0,	0x87*256+0,	0x8B*256+0,	0x89*256+0,
	0x86*256+0,	0x88*256+0,	0x8C*256+0,	0x8A*256+0,
};

static unsigned short skey[18][4]=
{	//normal		//shift+		//alt+		//ctrl+
	0x1C*256+'\n',	0x1C*256+0,	0xA6*256+0,	0x1C*256+0,
	0x35*256+'/',	0x35*256+0,	0xA4*256+0,	0x95*256+0,
	0x1D00,		0x1D00,		0x1D00,		0x1D00,
	0x00,		0x00,		0x00,		0x00,
	0x3800,		0x3800,		0x3800,		0x3800,
	0x47*256+0,	0x47*256+0,	0x97*256+0,	0x77*256+0,
	0x48*256+0,	0x48*256+0,	0x98*256+0,	0x8D*256+0,
	0x49*256+0,	0x49*256+0,	0x99*256+0,	0x84*256+0,
	0,		0,		0,		0,
	0x4B*256+0,	0x4B*256+0,	0x9B*256+0,	0x73*256+0,
	0x4C*256+0,	0x4C*256+0,	0x9C*256+0,	0x74*256+0,
	0x4D*256+0,	0x4D*256+0,	0x9D*256+0,	0x8F*256+0,
	0,		0,		0,		0,
	0x4F*256+0,	0x4F*256+0,	0x9F*256+0,	0x75*256+0,
	0x50*256+0,	0x50*256+0,	0xA0*256+0,	0x91*256+0,
	0x51*256+0,	0x51*256+0,	0xA1*256+0,	0x76*256+0,
	0x52*256+0,	0x52*256+0,	0xA2*256+0,	0x92*256+0,
	0x53*256+0,	0x53*256+0,	0xA3*256+0,	0x93*256+0,
};

static unsigned short numkey[13]=
{
0xE047,
0xE048,
0xE049,
0x4A,
0xE04B,
0x4C,
0xE04D,
0x4E,
0xE04F,
0xE050,
0xE051,
0xE052,
0xE053
};

extern int printk(const char *, ...);


int Kbd_Wait (void)
{
    	unsigned long timeout;
    	for (timeout= 0; timeout< 0x500000L; timeout++)
    	 	if ((inb(0x64) & 0x02) == 0)
		    	return 0;

    	printk("Kernel: Keyboard timed out!!\r\n");
	return 1;
}

int Kbd_SendCmd (unsigned char cmd)
{
    	if(Kbd_Wait ()) return 1;
    	outb(0x64, cmd);
	return 0;
}

int Kbd_SendData (unsigned char data)
{
    	if(Kbd_Wait ()) return 1;
	outb(0x60, data);
    	if(Kbd_Wait ()) return 1;
	return 0;
}

void SetKeyboardLEDs (void)
{
      	if(Kbd_SendData(0xed)) return;
         	Kbd_SendData(LED);
}

MCurSKey(unsigned char dkey)
{
	int i;
	if(skeypos[dkey-1]) return;
	CurSKeyList=(CurSKeyList<<8)|dkey;
	for(i=0;i<3;i++)
	{
		if(skeypos[i]) skeypos[i]++;
	}
	skeypos[dkey-1]=1;
	//printk("\nCurSKeyList=0x%x",CurSKeyList);
	//printk("\nSKey=0x%x",SKey);
	//printk("\nPos: Shift=%d,Alt=%d,Ctrl=%d",skeypos[0],skeypos[1],skeypos[2]);
}

RCurSKey(unsigned char dkey)
{
	unsigned int temp,i;
	unsigned char *CurSKey;
	CurSKey=(unsigned char *) &CurSKeyList;
	temp=CurSKeyList;
	CurSKeyList=CurSKeyList>>(8*skeypos[dkey-1]);
	CurSKeyList=CurSKeyList<<(8*(skeypos[dkey-1]-1));
	//printk("\nRCS Mid: CurSKeyList=0x%x",CurSKeyList);
	//printk("\nRCS Mid: Temp(1)=0x%x",temp);
	if(skeypos[dkey-1]==1) temp=0;
	else if(skeypos[dkey-1]==2) temp=temp&0xFF;
	else if(skeypos[dkey-1]==3) temp=temp&0xFFFF;
	//printk("\nRCS Mid: Temp(2)=0x%x",temp);
	CurSKeyList=CurSKeyList|temp;
	temp=3-skeypos[dkey-1];
	for(i=skeypos[dkey-1]-1;i<temp;i++)
	{
		if(skeypos[CurSKey[i]-1])	
			skeypos[CurSKey[i]-1]--;
	}
	skeypos[dkey-1]=0;
	//printk("\nCurSKeyList=0x%x",CurSKeyList);
	//printk("\nSKey=0x%x",SKey);
	//printk("\nPos: Shift=%d,Alt=%d,Ctrl=%d",skeypos[0],skeypos[1],skeypos[2]);
}	

int initKeyInt()
{
	fts=(struct FTSslot *) 0x00100000;
	fts->fgpKeyStart=0;
	fts->fgpKeyEnd=0;
	fts->bgpKeyStart=0;
	fts->bgpKeyEnd=0;
	Kbd_Wait();
	//Kbd_SendData (0xF0);
	//Kbd_SendData (0x1);
	SKey=0;
	unw=0;
	LED=0;
	prevcode=0;
	CurSKeyList=0;
	skeypos[0]=0;skeypos[1]=0;skeypos[2]=0;
	SetKeyboardLEDs(); setLED();
	//printk("\nFrom initKeyInt");
	return(0);
};

setLED()
{
	//printk("LED=%d",LED);
}

addToQueue(unsigned short dkey)
{
	//printk("\nChar=%d , Spec=%d",(char)dkey,(dkey>>8));
	//if((char)dkey>0) printk("%c",(char)dkey);
	if((fts->fgpKeyStart==0)&&(fts->fgpKeyEnd==15)) return;
	if(fts->fgpKeyStart==fts->fgpKeyEnd+1) return;
	fts->fgpKey[fts->fgpKeyEnd]=dkey;
	fts->fgpKeyEnd++;
	if(fts->fgpKeyEnd==16) fts->fgpKeyEnd=0;
}
	
void ConvKey(unsigned short code)
{	
	unsigned char *CurSKey;
	CurSKey=(unsigned char *) &CurSKeyList;
	if(unw) { unw--; return; }
	code=(prevcode<<8)|code;

	if(code==0xE1) { unw=5; prevcode=0; return; } // Add any procedure for PAUSE handler

	if((code==TABCODE)&&(*CurSKey==1))		// ALT is ON HOLD
	{
		//printk("SYSCTMOD");
		prevcode=0;
		return;
	}
	else if(code==0xE0)
	{
		prevcode=0xE0;
		return;
	}

	if(code==LSHIFTCODE)		{SKey=SKey|LSHIFT;MCurSKey(SHIFT);}
	else if(code==RSHIFTCODE)		{SKey=SKey|RSHIFT;MCurSKey(SHIFT);}
	else if(code==ULSHIFTCODE)	{SKey=SKey&~LSHIFT;RCurSKey(SHIFT);}
	else if(code==URSHIFTCODE)	{SKey=SKey&~RSHIFT;RCurSKey(SHIFT);}
	else if(code==LCTRLCODE)		{SKey=SKey|LCTRL;MCurSKey(CTRL);}
	else if(code==RCTRLCODE)		{SKey=SKey|RCTRL;MCurSKey(CTRL);}
	else if(code==ULCTRLCODE)		{SKey=SKey&~LCTRL;RCurSKey(CTRL);}
	else if(code==URCTRLCODE)	{SKey=SKey&~RCTRL;RCurSKey(CTRL);}
	else if(code==LALTCODE)		{SKey=SKey|LALT;MCurSKey(ALT);}
	else if(code==RALTCODE)		{SKey=SKey|RALT;MCurSKey(ALT);}
	else if(code==ULALTCODE)		{SKey=SKey&~LALT;RCurSKey(ALT);}
	else if(code==URALTCODE)		{SKey=SKey&~RALT;RCurSKey(ALT);}
	else if(code==CAPSCODE)		{ LED=LED^CAPS;SetKeyboardLEDs(); setLED(); return;}
	else if(code==NUMCODE)		{LED=LED^NUM;SetKeyboardLEDs(); setLED(); return;}
	else if(code==SCRLCODE)		{LED=LED^SCRL;SetKeyboardLEDs(); setLED(); return;}

	if(code==0xE037) { prevcode=0; return;} //Add Print Screen Procedure

	if((!(LED&NUM)) && (code>=0x47) && (code<=0x53))
		code=numkey[code-0x47];

	if(code==0xE01C) code=code+0x18;
	else if(code==0xE01D) code=code+0x19;

	if((code>0xE033)&&(code<0xE039)) code=code+0xE;

	if((code>0x0)&&(code<0x59))
	{
		code=key[code-1][*CurSKey];
		if(LED&CAPS)
		{
			if(((code&0xFF)>='a')  &&((code&0xFF)<='z'))
				code=(code&0xFF00)|((code&0xFF)+'A'-'a');
			else if(((code&0xFF)>='A')  &&((code&0xFF)<='Z'))
				code=(code&0xFF00)|((code&0xFF)+'a'-'A');
		}
		addToQueue(code);
		prevcode=0;
		return;
	}
	else if((code>0xE041)&&(code<0xE054))
	{
		addToQueue(skey[code-0xE042][*CurSKey]);
		prevcode=0;
		return;
	}
	prevcode=0;
	return;
}

int KeyInt()
{
	unsigned short code, status;
   	unsigned char i;
	//printk("\nStarting KeyInt");
	Kbd_SendCmd (0xad);
	if(Kbd_Wait ())return;
	status = inb(0x64);
	if ((status & 1) == 1)
   	{
   		code = inb(0x60);
		ConvKey(code);
	}
	Kbd_SendCmd (0xae);
	i = inb(0x61);
	outb(0x61, i | 0x80);
	outb(0x61, i);
	outb(0x20,0x20);	// send ACK to PIC1
	return(0);
}


# define ACTIVEPART	0x80
# define INACTIVEPART	0x00
# define DOSPART		0x0B
# define EXTENDEDPART	0x05

struct partitionentrystruct
{
	unsigned char BootIndicator;
	unsigned char StartingHead;
	unsigned char StartingSectAndCylHigh;
	unsigned char StartingCylLow;
	unsigned char FileSystem;
	unsigned char EndingHead;
	unsigned char EndingSectAndCylHigh;
	unsigned char EndingCylLow;
	unsigned int PrevSectors;
	unsigned int NoOfSectors; 
};

struct partitionstruct
{
	unsigned char code[446];
	struct partitionentrystruct entry[4];
	unsigned short bootsign;
};


struct syspartitionstruct
{
	unsigned int MinLBA;
	unsigned int MaxLBA;
	unsigned int processid;
	unsigned char drive;
	unsigned char FileSystem;
	unsigned char bootable;
	unsigned char present;
};

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
	unsigned char bootcode[420];
	unsigned short bootsign;
};

struct drivestruct
{
	unsigned char present;
	unsigned short maxcylinder;
	unsigned char maxhead;
	unsigned char sectorspertrack;
};

static struct drivestruct hddrive[2];

static struct syspartitionstruct syspart[26];
static int totalpart;
static char spart;

# define	READ	3
# define	WRITE	4

extern int ReadHD(unsigned int,unsigned int,unsigned int);
extern int WriteHD(unsigned int,unsigned int,unsigned int);

int summa(int cmd,int sect)
{
	int i;
	for(i=0;i<1000;i++)
	{
		cmd=cmd+sect;
	}
	return(cmd);
}

int RWSect(unsigned int cmd,unsigned int lba,unsigned int nsects,unsigned int drive,unsigned int sect)
{
	unsigned int driveparam;
	unsigned char* dp;
	unsigned char heads_per_cylinder,sectors_per_track,head,sector;
	unsigned short cylinder,temp;

	heads_per_cylinder=hddrive[drive].maxhead;
	sectors_per_track=hddrive[drive].sectorspertrack;
        	cylinder = lba / (heads_per_cylinder * sectors_per_track);
       	temp = lba % (heads_per_cylinder * sectors_per_track);
        	head = temp / sectors_per_track;
      	sector = (temp % sectors_per_track) + 1;
	//printk("Sect=%d,Cyl=%d,Head=%d,nsects=%d",sector,cylinder,head,nsects);
	//printk("Addr = %d",(unsigned int) sect);
	dp=(unsigned char*) &driveparam;
	dp[0]=sector;
	dp[1]=(unsigned char)((((drive&0x01)|0x0A)<<4)|(head&0x0F));
	dp[2]=(unsigned char) cylinder;
	dp[3]=*(((unsigned char *) &cylinder)+1);
	//printk("\nCylinder : %d\nHead : %d\nSector %d\nNSects %d",cylinder,head,sector,nsects);
	//printk("\nSD : Cmd=%d, Address = 0x%x",cmd,sect);
	summa(cmd,sect);
	if(cmd==READ) driveparam=ReadHD(*((unsigned int *) dp),nsects,sect);
	else if(cmd==WRITE) driveparam=WriteHD(*((unsigned int *) dp),nsects,sect);
	if(driveparam>0) 
	{
		if(cmd==READ) printk("\nHD: Read Error :0x%x ",driveparam);
		else if(cmd==WRITE) printk("\nHD: Write Error :0x%x ",driveparam);
		else printk("\nHD: Unknown Command");
	}
	return(driveparam);
}

extern int RWSectToProcess(unsigned int cmd,unsigned int lba,unsigned int nsects,unsigned int drive,unsigned int sect,unsigned int processId)
{
	unsigned int *dclustmem=(unsigned int *) 0x00010000;
	int retvalue;
	if(sect%4096)
	{
		PS(CPYTOME,((unsigned int)dclustmem)/4096,processId,sect/4096);
		PS(CPYTOME,(((unsigned int)dclustmem)/4096)+1,processId,(sect/4096)+1);
		retvalue=RWSect(cmd-2,lba,nsects,drive,((unsigned int)dclustmem)+(sect%4096));
		PS(CPYFRMME,((unsigned int)dclustmem)/4096,processId,sect/4096);
		PS(CPYFRMME,(((unsigned int)dclustmem)/4096)+1,processId,(sect/4096)+1);
	}
	else
	{
		PS(CPYTOME,((unsigned int)dclustmem)/4096,processId,sect/4096);
		retvalue=RWSect(cmd-2,lba,nsects,drive,(unsigned int) dclustmem);
		PS(CPYFRMME,((unsigned int)dclustmem)/4096,processId,sect/4096);
	}
	return(retvalue);
}

int LoadAllPartitions(unsigned int lba,unsigned int drive,int k)
{
	static int pc;
	int i;
	struct partitionstruct partition;
	pc=k;
	RWSect(READ,lba,1,drive,(unsigned int) &partition);
	if(partition.bootsign!=0xAA55) return(pc);
	for(i=0;i<4;i++)
	{
		if((partition.entry[i].FileSystem!=EXTENDEDPART)&&(partition.entry[i].FileSystem!=0x0))
		{
			syspart[pc].MinLBA=lba+partition.entry[i].PrevSectors;
			syspart[pc].MaxLBA=syspart[pc].MinLBA+partition.entry[i].NoOfSectors;
			syspart[pc].drive=drive;
			syspart[pc].FileSystem=partition.entry[i].FileSystem;
			syspart[pc].bootable=partition.entry[i].BootIndicator;
			syspart[pc].present=1;
			pc++;
			continue;
		}
		else if(partition.entry[i].FileSystem==EXTENDEDPART)
		{
			LoadAllPartitions(lba+partition.entry[i].PrevSectors,drive,pc);
		}
	}
	return(pc);
}
		

void initialisePartitionList()
{
	int pc,i;
	struct bpbstruct bpb;
	pc=2;
	spart=0;
	if(hddrive[0].present) { pc=LoadAllPartitions(0,0,pc); }
	if(hddrive[1].present) { pc=LoadAllPartitions(0,1,pc); }
	totalpart=pc-2;
	if(totalpart==0) { printk("\nNo valid hard disk partition found..."); hangon(); }
	for(i=2;i<totalpart+2;i++)
	{
		if((syspart[i].FileSystem==DOSPART)&&(syspart[i].bootable=0x80))
		{
			RWSect(READ,syspart[i].MinLBA,1,syspart[i].drive,(unsigned int) &bpb);
			if(bpb.bootsign!=0xAA55) { printk("\nBootsign not found in a partition"); continue; }
			if(!strncmp(bpb.bsOemName,"REDSKY",6))
				spart=i+'A';
		}
	}
	for(i=totalpart+2;i<26;i++)
	{
		syspart[i].present=0;syspart[i].MinLBA=0;syspart[i].MaxLBA=0;
	}
	for(i=0;i<2;i++)
	{
		syspart[i].present=0;syspart[i].MinLBA=0;syspart[i].MaxLBA=0;
	}
}

unsigned char getCylinderLow_CMOS(int i)
{
	outb(0x70,0x1B+i);
	return(inb(0x71));
}

unsigned char getCylinderHigh_CMOS(int i)
{
	outb(0x70,0x1C+i);
	return(inb(0x71));
}

unsigned short getCylinder_CMOS(int i)
{
	unsigned temp;
	i=((i&0x03)-1)*9;
	temp=256*getCylinderHigh_CMOS(i)+getCylinderLow_CMOS(i);
	return(temp);	
}

unsigned char getHead_CMOS(int i)
{
	i=((i&0x03)-1)*9;
	outb(0x70,0x1D);
	return(inb(0x71));
}

unsigned char getSectorsPerTrack_CMOS(int i)
{
	i=((i&0x03)-1)*9;
	outb(0x70,0x23);
	return(inb(0x71));
}

int HardDiskService(unsigned int code,unsigned int param1,unsigned int param2,unsigned int param3,unsigned int param4)
{
	int drive,nsects,lba;
	int retvalue;
	if(code==2) 
	{
		drive=param1;
		if(drive==0xFFFFFFFF) drive=spart;
		if((drive<'A')&&(drive>'Z')) return(10);
		drive=drive-'A';
		if(drive<2) return(10);
		if(!syspart[drive].present) return(10);
		return(syspart[drive].MaxLBA);
	}
	if((code==3)||(code==4)||(code==5)||(code==6)) 
	{
		nsects=param2;
		lba=param1;
		drive=(nsects&0xFFFF0000)>>16;
		nsects=nsects&0x0000FFFF;
		//printk("\n In Service");
		if(drive==0xFFFF) drive=spart;

		if((drive<'A')&&(drive>'Z')) return(10);
		drive=drive-'A';
		if(drive<2) return(10);
		if(!syspart[drive].present) return(10);
		lba=syspart[drive].MinLBA+lba;
		if((lba+nsects)>=syspart[drive].MaxLBA) return(11);
		drive=syspart[drive].drive;
		if((code==5)||(code==6))
		{
			retvalue=RWSectToProcess(code,lba,nsects,drive,param3,param4);
		}
		else
		{
			retvalue=RWSect(code,lba,nsects,drive,param3);
		}
		return(retvalue);
	}
	if(code==7)
	{
		retvalue=totalpart;
		retvalue=(retvalue<<8)|(unsigned int)spart;
		return(retvalue);
	}
	return(0);
}

extern int GetDrive(unsigned int,unsigned int);

extern int initHardDisk()
{
	unsigned char sect[512],hdtype;
	unsigned int i;
	out(0x70,0x12);
	hdtype=in(0x71);
	i=0;
	if(hddrive[0].present=(hdtype&0xF0)>>4) i++;
	if(hddrive[1].present=hdtype&0x0F) i++;
	printk("\nNumber of HardDisks : %d",i);

	for(i=0;i<2;i++)
	{
		if(hddrive[i].present)
		{
			{
				GetDrive(((i&0x01)|0x0A)<<4,(unsigned int) &sect);
				hddrive[i].maxcylinder=sect[1];
				hddrive[i].maxhead=sect[3];
				hddrive[i].sectorspertrack=sect[6];
				if((hddrive[i].present)&&((!hddrive[i].maxcylinder)||(!hddrive[i].maxhead)||(!hddrive[i].sectorspertrack)))
				{
					hddrive[i].maxcylinder=getCylinder_CMOS(i+1);
					hddrive[i].maxhead=getHead_CMOS(i+1);
					hddrive[i].sectorspertrack=getSectorsPerTrack_CMOS(i+1);
				}
				printk("\nHard Disk %d Information",i+1);
				hdtype=sect[0x14];
				sect[0x14]=0;
				printk("\nSerial No : %s",(char *)&sect[0xA]);
				sect[0x14]=hdtype;
				hdtype=sect[0x14];
				sect[0x1B]=0;
				printk("\nFirmware Revision : %s",(char *)&sect[0x17]);
				sect[0x1B]=hdtype;
				hdtype=sect[0x14];
				sect[0x2F]=0;
				printk("\nModel No : %s",(char *)&sect[0x1B]);
				sect[0x2F]=hdtype;
				printk("\nMax Cylinders : %d\nHeads Per Cylinder : %d\nSectors Per Track : %d",hddrive[i].maxcylinder,hddrive[i].maxhead,hddrive[i].sectorspertrack);
			}
		}
		else
			printk("\nHard Disk %d not present",i+1);
	}
	for(i=0;i<2;i++)
	{
		if(hddrive[i].maxhead>16)
		{
			printk("\nHard Disk %d has more than 16 heads. The current driver cannot handle it. So ignoring the disk...",i+1);
			hddrive[i].present==0;
		}
	}
	if((!hddrive[0].present)&&(!hddrive[1].present))
	{
		printk("\nNo compatible hard disk available...");
		hangon();
	}
	initialisePartitionList();
	printk("\nNumber of disk partitions: %d",totalpart);
	if(spart==0) { printk("\nCannot find system partition..."); hangon(); }
	printk("\nSystem partition is drive %c:",spart);
	return 0;
}



int Timer(unsigned int code)
{
	if(code==1) return(initTimer());
	else return(TimerInt());
}

int KeyBoard(unsigned int code)
{
	if(code==1) return(initKeyInt());
	else return(KeyInt());
}

int HardDisk(unsigned int code,unsigned int param1,unsigned int param2,unsigned int param3,unsigned int param4)
{
	//printk("\n In Hard Disk : %d",code);
	if(code==1) return(initHardDisk());
	else return(HardDiskService(code,param1,param2,param3,param4));
}

int ExceptionHandler(unsigned int taskId)
{
	unsigned short myTaskId=6;
	struct TSSStruct *tss;
	(tss+TASKMNGR)->Eax=6;	// End the current Task...
	(tss+TASKMNGR)->Eax=taskId;	// Task which caused exception...
	(tss+myTaskId)->Back_Link=(TASKMNGR+5)*8;
	printk("\nSome exception occured"); hangon();
	return(0);
}
