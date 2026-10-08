extern int printk(const char *, ...);
extern unsigned int GData;
static unsigned int *nextaddr;

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
};

struct TSSStruct *tss;

extern int CallTask(unsigned int);

struct filemanagerfileentry
{
	struct filemanagerfileentry *NextEntry;
	unsigned int FileSystemHandle;
	unsigned int processID;
	unsigned short handle;
	unsigned char attrib;
	unsigned char drive;
};

struct drivestruct
{
	unsigned int processId;
	unsigned int attrib;	// the drive may be read only, virtual, restricted...
}

static struct drivestruct drive[26];

static unsigned int *nextaddr;

static char *NewFilePath;
static char *FilePath;

static struct filemanagerfileentry *FileList;
static struct filemanagerfileentry *LastFileEntry;
static unsigned int NoOfFiles;




struct freepage
{
	int numofpages;
	struct freepage *next;
	int data[1022];
};

static struct freepage *fdatapage;

struct feslot
{
	int numofslots;
	struct feslot *next;
	char data[8];
};
	 
static struct feslot *curfeslot;

void initfmmem()
{
	if( ((unsigned int)nextaddr) %4096 ) nextaddr=(unsigned int *)( ( (((unsigned int)nextaddr) /4096 )*4096)+4096);
	fdatapage=(struct freepage *) nextaddr;
	fdatapage->numofpages=(0xFFFFF000-(unsigned int)fdatapage)/4096;
	fdatapage->next=(struct freepage*) 0;
	curfeslot=(struct feslot *) 0;
}

void * getnewpage()
{
	struct freepage temppage;
	struct freepage *retpage;
	retpage=fdatapage;
	if((fdatapage->numofpages)>1)
	{
	 	temppage.next=fdatapage->next;
	  	temppage.numofpages=fdatapage->numofpages-1;  
	  	fdatapage=fdatapage+1;
	  	fdatapage->next=temppage.next;
	  	fdatapage->numofpages=temppage.numofpages;
	}
	else
	{
	 	fdatapage=fdatapage->next;
	}
	return((void *) retpage);
}

void freeapage(struct freepage *usedpage)
{
	usedpage->numofpages=1;
	usedpage->next=fdatapage;
	fdatapage=usedpage;
}



void * getnewfeslot()
{
	struct feslot *retslot,tempslot;
	if( curfeslot==(struct feslot *) 0 )
	{
		curfeslot=(struct feslot *) getnewpage();
	  	curfeslot->numofslots=256;
	  	curfeslot->next=(struct feslot *) 0;
	}
	retslot=curfeslot;
	if(curfeslot->numofslots>1)
	{
	  	tempslot.next=curfeslot->next;
	  	tempslot.numofslots=curfeslot->numofslots-1;
	  	curfeslot=curfeslot+1;
	  	curfeslot->next=tempslot.next;
	  	curfeslot->numofslots=tempslot.numofslots;
	}
	else
	{
	  	curfeslot=curfeslot->next;
	}
	return((void *)retslot);
}
	  
void freeafeslot(void *used1slot)
{
	struct feslot *usedslot=(struct feslot *) used1slot;
	usedslot->numofslots=1;
	usedslot->next=curfeslot;
	curfeslot=usedslot;
}

static unsigned short nexthandleid;

int initFM()
{
	nextaddr=(unsigned int *)&GData;
	if(((unsigned int)nextaddr)%4096) nextaddr=(unsigned int *)((((unsigned int)nextaddr)/4096)*4096+4096);
	printk("\nGData : 0x%x, NextAddr : 0x%x",(unsigned int)&GData,(unsigned int)nextaddr);
	initFileRefCache();
	FileList=0;
	LastFileEntry=FileList;
	NoOfFiles=0;
	initfmmem();
	NewFilePath=(char *)getnewpage();
	FilePath=(char *)getnewpage();
	initialiseDriveList();
	nexthandleid=10;
	return(0);
}

static char maxdrive;	//maximum drive letter...

initialiseDriveList()
{
	DM(getMaxDrive);
}

int registerDrive(unsigned char drive,unsigned int processId)	// system drive is identified with a call to DM...
{
	drive[drive-'A'].processId=processId;
	drive[drive-'A'].attrib=0;
	CallAFS(processId,INIT,drive);
}

char getDrive(char *path)
{
	if((path[1]==':')&&((path[2]=='/')||(path[2]=='\\')))
		return(path[0]);
	return(0);
}

unsigned short getNewHandle(unsigned int processId)
{
			// search for unique id for file for each process...
	nexthandleid++;
	return(nexthandleid-1);
}

int OpenFile(unsigned int path,unsigned int mode,unsigned int processId)
{
	unsigned int fsfilehandle;
	unsigned short handle;
	char drive;
	struct filemanagerfileentry *FileEntry;
	PS(CPYTOME,(unsigned int)FilePath/4096,processId,path/4096);
	drive=getDrive(FilePath);
	if((drive<'A') || (drive>'Z')) return 1;	// drive not valid...
	if(formalisePath((char *)FilePath,NewFilePath)<0) return 2;		// path not valid...
	fsfilehandle=CallAFS(drive[drive-'A'].processId,OPENFILE,(unsigned int)NewFilePath,mode);		//FS(OPENFILE,(unsigned int)NewFilePath,mode);
	if(fsfilehandle<5) return(fsfilehandle+3);	// because error upto 2 is used by FM and fs may also return 0...
					// fm starting filehandle is 10...
	handle=getNewHandle(processId);
	FileEntry=getnewfeslot();
	if(!FileList) FileList=FileEntry;
	if(LastFileEntry) LastFileEntry->NextEntry=FileEntry;
	LastFileEntry=FileEntry;
	FileEntry->NextEntry=0;
	FileEntry->handle=handle;
	FileEntry->FileSystemHande=fsfilehandle;
	FileEntry->processId=processId;
	FileEntry->drive=drive;
	FileEntry->attrib=mode;
	NoOfFiles++;
	return(handle);
}


int ReadFile(

int FileManager(unsigned int code,unsigned int param1,unsigned int param2,unsigned int param3,unsigned int param4)
{
	if(code==1) return(initFM());
	else return(FMService(code,param1,param2,param3,param4));
}