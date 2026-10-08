# define dword unsigned int
# define word unsigned short
# define byte unsigned char

# define	READONLY	6
# define	WRITEONLY	7

int CloseFile(unsigned int);
void ShutDown();
void BreakClustList(unsigned int);
int MakeNewFileOrDir(unsigned int,char *,char);
void * getnewpage();

extern inline int strncmp(const char * cs,const char * cd,int count)
{
	int i,r;
	for(i=0;i<count;i++)
	{
		if(r=*(cs+i)-*(cd+i)) break;
	}
	return(r);
}


extern inline char * strncpy(char * dest,const char *src,int count)
{
	int i;
	for(i=0;i<count;i++)
	{
		*(dest+i)=*(src+i);
	}
	return(dest);
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
};

struct TSSStruct *tss;

extern int CallTask(unsigned int);

extern int printk(const char *, ...);

int delay2(int k)
{
	int i;
	for(i=0;i<k;i++)
	{
		i=2*i+k;
	}
}

void delay()
{
	delay2(10000);
}

# define	PREAD	5
# define	PWRITE	6

int DiskManager(unsigned int cmd,unsigned int lba,unsigned int nsects,unsigned int drive,unsigned int sect,unsigned int processId)
{
	int retvalue,i;
	//tss=(struct TSSStruct *)0x00070000;
	nsects=nsects&0xFFFF;
	drive=(drive<<16)&0xFFFF0000;
	nsects=nsects|drive;
	delay();
	for(i=0;i<3;i++)
	{
		retvalue=DM(cmd,lba,nsects,sect,processId);
		if(!retvalue) break;
		delay();
	}
	return(retvalue);
}

# define	INIT	1

# define	FOPEN	2
# define	DOPEN	3
# define	NFILE	4
# define	NDIR	5
# define	DFILE	6
# define	DDIR	7

# define	FOFH	8
# define	DOFH	9
# define	FOIH	10
# define	DOIH	11
# define	NFFH	12
# define	NDFH	13
# define	NFIH	14
# define	NDIH	15
# define	DFFH	16
# define	DDFH	17

# define	DFWH	18
# define	DDWH	19
# define	FDREAD	20
# define	FWRITE	21
# define	FCLOSE	22
# define	DCLOSE	23
# define	DBACK	24
# define	FSIZE	25

# define	END	26

static char *GFilePath;

# define	CPYTOME	8
# define	CPYFRMME	9

static int MyProcessId;
static char *p_aPage;

CopyFilePath(unsigned int path,unsigned int processId)
{
	int i;
	//printk("\nIn copying path");
	if(path%4096)
	{
		PS(CPYTOME,((unsigned int)p_aPage)/4096,processId,path/4096);
		for(i=0;i<4096-(path%4096);i++)
		{
			*(GFilePath+i)=*(p_aPage+(path%4096)+i);
		}
		PS(CPYTOME,((unsigned int)p_aPage)/4096,processId,(path/4096)+1);
		for(i=0;i<path%4096;i++)
		{
			*(GFilePath+(4096-(path%4096))+i)=*(p_aPage+i);
		}
	}
	else
	{
		PS(CPYTOME,((unsigned int)GFilePath)/4096,processId,path/4096);
	}
	//printk("\nFinished copying path");
}

int FSService(unsigned int code,unsigned int param1,unsigned int param2,unsigned int param3,unsigned int param4,unsigned int param5,unsigned int process)
{
	//printk("\nIn FS Service");
	if((code>=FOPEN)&&(code<=DDIR))	 CopyFilePath(param1,process);
	else if((code>=FOFH)&&(code<=DDFH))CopyFilePath(param2,process);

	if(code==FOPEN)
		return(OpenFile((unsigned int)GFilePath,param2));
	else if(code==DOPEN)
		return(OpenDir((unsigned int)GFilePath));
	else if(code==NFILE)
		return(NewFile((unsigned int)GFilePath));
	else if(code==NDIR)
		return(NewDir((unsigned int)GFilePath));
	else if(code==DFILE)
		return(DeleteFile((unsigned int)GFilePath));
	else if(code==DDIR)
		return(DeleteDir((unsigned int)GFilePath));
	else if(code==FOFH)
		return(OpenFileFromDirHandle(param1,(unsigned int)GFilePath,param3));
	else if(code==DOFH)
		return(OpenDirFromDirHandle(param1,(unsigned int)GFilePath));
	else if(code==FOIH)
		return(OpenFileInDirHandle(param1,(unsigned int)GFilePath,param3));
	else if(code==DOIH)
		return(OpenDirInDirHandle(param1,(unsigned int)GFilePath,param3));
	else if(code==NFFH)
		return(NewFileFromDirHandle(param1,(unsigned int)GFilePath));
	else if(code==NDFH)
		return(NewDirFromDirHandle(param1,(unsigned int)GFilePath));
	else if(code==NFIH)
		return(NewFileInDirHandle(param1,(unsigned int)GFilePath));
	else if(code==NDIH)
		return(NewDirInDirHandle(param1,(unsigned int)GFilePath));
	else if(code==DFFH)
		return(DeleteFileFromDirHandle(param1,(unsigned int)GFilePath));
	else if(code==DDFH)
		return(DeleteDirFromDirHandle(param1,(unsigned int)GFilePath));
	else if(code==DFWH)
		return(DeleteFileWithHandle(param1));
	else if(code==DDWH)
		return(DeleteDirWithDirHandle(param1));
	else if(code==FDREAD)
		return(ReadFile(param1,param2,param3,param4,param5,process));
	else if(code==FWRITE)
		return(WriteFile(param1,param2,param3,param4,param5,process));
	else if(code==FCLOSE)
		return(CloseFile(param1));
	else if(code==DCLOSE)
		return(CloseDir(param1));
	else if(code==DBACK)
		return(MoveToHeadDir(param1));
	else if(code==FSIZE)
		return(GetFileSize(param1));
	else if(code==END)
		ShutDown();
	return(0);
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


struct FileRefCacheStruct
{
	unsigned int fileid;
	unsigned int refs;
	unsigned int addr;
	unsigned int res0;
};

struct FileStruct
{
	struct FileStruct *Next;
	unsigned int fileid;
	unsigned int dirlba;
	unsigned int entryno;
	unsigned int FirstClust;
	unsigned int CurrentClustNo;
	unsigned int CurrentClust;
	unsigned int Size;	// when opening a directory , Size will be used to store dir first lba, since Size of any directory is 0.
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
	char bootcode[420];
	unsigned short bootsign;
};

extern unsigned int GData;

static unsigned int drive;

static unsigned int *table;
static unsigned int *dirtable;
static struct FileRefCacheStruct *filetable;
static struct FileStruct *FileList;
static struct FileStruct *LastFileEntry;
static unsigned int NoOfFiles;

static unsigned int *nextaddr;

static unsigned int fatstartlba;
static unsigned int fatsects;
static unsigned int datastartlba;

static unsigned char bpbSectorsPerCluster;
static unsigned short bpbBytesPerSector;
static unsigned int bpbRootClus;

static unsigned int lfsect;

void initFATBuffer()
{
	int i;
	table=nextaddr;
	/*for(i=0;i<127;i++)
	{
		*(table+i)=0;		//table+((i+1)*128);
	}*/
	nextaddr=table+(128*128);
	//printk("\nTable : 0x%x, NextAddr : 0x%x",(unsigned int)table,(unsigned int)nextaddr);
	lfsect=0;
}

void ShutDownFATBuffer()
{
	int i;
	/*for(i=0;i<127;i++)
	{
		if(*(table+i)&0x80000000)
		{
			*(table+i)=*(table+i)&0x0FFFFFFF;
			DiskManager(PWRITE,*(table+i),1,drive,(unsigned int)(table+((i+1)*128)),MyProcessId);
			DiskManager(PWRITE,*(table+i)+fatsects,1,drive,(unsigned int)(table+((i+1)*128)),MyProcessId);
		}
	}*/
	if(lfsect)
	{
		DiskManager(PWRITE,lfsect,1,drive,(unsigned int)(table+128),MyProcessId);
		DiskManager(PWRITE,lfsect+fatsects,1,drive,(unsigned int)(table+128),MyProcessId);
	}
}

unsigned int * LoadFATSect2(unsigned int fatsect,unsigned int mode)
{
	fatsect=fatstartlba+fatsect;
	int i;
	static int n=0;
	mode=mode&0x80000000;
	for(i=0;i<127;i++)
	{
		if((*(table+i))&0x0FFFFFFF==fatsect) 
		{
			if(!(*(table+i)&0x80000000))
				*(table+i)=*(table+i)|mode;
			return(table+((i+1)*128));
		}
	}
	for(i=0;i<127;i++)
	{
		if(*(table+i)==0)
		{
			DiskManager(PREAD,fatsect,1,drive,(unsigned int)(table+((i+1)*128)),MyProcessId);
			*(table+i)=fatsect;
			if(!(*(table+i)&0x80000000))
				*(table+i)=*(table+i)|mode;
			return(table+((i+1)*128));
		}
	}
	if((*(table+n)&0x80000000))
		DiskManager(PWRITE,(*(table+n))&0x0FFFFFFF,1,drive,(unsigned int)(table+((n+1)*128)),MyProcessId);
		DiskManager(PWRITE,((*(table+n))&0x0FFFFFFF)+fatsects,1,drive,(unsigned int)(table+((n+1)*128)),MyProcessId);
	DiskManager(PREAD,fatsect,1,drive,(unsigned int)(table+((n+1)*128)),MyProcessId);
	*(table+n)=fatsect | mode;
	n++;
	if(n==20) n=0;
	return(table+((n+1)*128));
}

unsigned int * LoadFATSect(unsigned int fatsect,unsigned int mode)
{
	fatsect=fatstartlba+fatsect;
	if(lfsect!=fatsect)
	{
		if(lfsect)	// initially, lfsect=0.
		{
			DiskManager(PWRITE,lfsect,1,drive,(unsigned int)(table+128),MyProcessId);
			DiskManager(PWRITE,lfsect+fatsects,1,drive,(unsigned int)(table+128),MyProcessId);
		}
		DiskManager(PREAD,fatsect,1,drive,(unsigned int)(table+128),MyProcessId);	
		lfsect=fatsect;
	}
	return(table+128);
}

unsigned int getNextClust(unsigned int dirclust)
{
	unsigned int *fat;
	unsigned int fatsect,item;
	fatsect=dirclust/128;
	item=dirclust%128;
	fat=(unsigned int *)LoadFATSect(fatsect,0);
	return((*(fat+item))&0x0FFFFFFF);
}

static unsigned int dirclustsize;

static unsigned int ldclust;

void initDirBuffer()
{
	int i;
	dirclustsize=bpbSectorsPerCluster*128;		//128=512/4
	dirtable=nextaddr;
	for(i=0;i<32;i++)
	{
		*(dirtable+i)=0;		//dirtable+32+(i*dirclustsize);
	}
	nextaddr=dirtable+32+(32*dirclustsize);
	//printk("\nDirTable : 0x%x, NextAddr : 0x%x",(unsigned int)dirtable,(unsigned int)nextaddr);
	ldclust=0;
}

void ShutDownDirBuffer()
{
	int i;
	/*for(i=0;i<32;i++)
	{
		if(*(dirtable+i)&0x80000000)
		{
			*(dirtable+i)=*(dirtable+i)&0x0FFFFFFF;
			//printk("\nSDDB, clust : %d  , addr : 0x%x",*(dirtable+i),(dirtable+32+(i*dirclustsize)));
			//printk("\nSDDB content:\n%s",(dirtable+32+(i*dirclustsize)));
			DiskManager(PWRITE,datastartlba+(*(dirtable+i)-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,drive,(unsigned int)(dirtable+32+(i*dirclustsize)),MyProcessId);
			//printk("\nDir : %d written",datastartlba+(*(dirtable+i)-2)*bpbSectorsPerCluster);
		}
	}*/
	if(ldclust)
		DiskManager(PWRITE,datastartlba+(ldclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,drive,(unsigned int)(dirtable+32),MyProcessId);
}

unsigned int * LoadDirClust2(unsigned int dirclust,unsigned int mode)
{
	int i;
	static int n=0;
	static char bfull;
	bfull=0;
	mode=mode&0x80000000;
	for(i=0;i<32;i++)
	{
		if(*(dirtable+i)&0x0FFFFFFF==dirclust)
		{
			if(!(*(dirtable+i)&0x80000000))
				*(dirtable+i)=*(dirtable+i)|mode;
			//printk("\nDir Table : 0x%x",*(dirtable+i));
			return(dirtable+32+(i*dirclustsize));
		}
	}
	if(!bfull)
	{
		for(i=0;i<32;i++)
		{
			if(*(dirtable+i)==0)
			{
				//printk("\nLDC, clust : %d  , addr : 0x%x",dirclust,(dirtable+32+(i*dirclustsize)));
				DiskManager(PREAD,datastartlba+(dirclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,drive,(unsigned int)(dirtable+32+(i*dirclustsize)),MyProcessId);
				*(dirtable+i)=dirclust;
				*(dirtable+i)=*(dirtable+i)|mode;
				return(dirtable+32+(i*dirclustsize));
			}
		}
	}
	if((!bfull)&&(i==32)) bfull=1;
	if(bfull)
	{
		if(*(dirtable+n)&0x80000000)
			DiskManager(PWRITE,datastartlba+(*(dirtable+n)-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,drive,(unsigned int)(dirtable+32+(n*dirclustsize)),MyProcessId);
		DiskManager(PREAD,datastartlba+(dirclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,drive,(unsigned int)(dirtable+32+(n*dirclustsize)),MyProcessId);
		*(dirtable+n)=dirclust;
		*(dirtable+n)=*(dirtable+n)|mode;
		n++;
		if(n==20) n=0;
		return(dirtable+32+(n*dirclustsize));
	}
}

unsigned int * LoadDirClust(unsigned int dirclust,unsigned int mode)
{
	if(ldclust!=dirclust)
	{
		if(ldclust)
			DiskManager(PWRITE,datastartlba+(ldclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,drive,(unsigned int)(dirtable+32),MyProcessId);
		DiskManager(PREAD,datastartlba+(dirclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,drive,(unsigned int)(dirtable+32),MyProcessId);
		ldclust=dirclust;
	}
	return(dirtable+32);
}

void initFileRefCache()
{
	int i;
	filetable=(struct FileRefCacheStruct *)nextaddr;
	for(i=0;i<64;i++)
	{
		filetable[i].fileid=0;
	}
	nextaddr=(unsigned int *)(filetable+64);
	//printk("\nFileTable : 0x%x, NextAddr : 0x%x",(unsigned int)filetable,(unsigned int)nextaddr);
}

void removeFileCacheRef(fileid)
{
	int i;
	for(i=0;i<64;i++)
	{
		if(filetable[i].fileid==fileid) 
		{
			filetable[i].fileid=0;
		}
	}
}

unsigned int getFileAddrBySearching(unsigned int fileid)
{
	int i;
	i=NoOfFiles;
	struct FileStruct *FileEntry;
	FileEntry=FileList;
	while(i--)
	{
		//printk("\nFI: %d, FA: 0x%x",FileEntry->fileid,(unsigned int)FileEntry);
		if(FileEntry==0) return(0);
		if(FileEntry->fileid==fileid) return((unsigned int) FileEntry);
		FileEntry=FileEntry->Next;
	}
	//printk("\nCannot Find %d",fileid);
	return(0);
}

struct FileStruct * getFileAddress(unsigned int fileid)
{
	int i,n;
	static unsigned int high=0;
	unsigned int low,temp;
	for(i=0;i<64;i++)
	{
		if(filetable[i].fileid==fileid) 
		{	
			if(filetable[i].refs!=0xFFFFFFFF)
			{
				filetable[i].refs++;
				if(filetable[i].refs>high) high=filetable[i].refs;
			}
			return((struct FileStruct *)filetable[i].addr);
		}
		if(filetable[i].fileid==0) 
		{
			temp=getFileAddrBySearching(fileid);
			if(!temp) return((struct FileStruct *) 0);
			filetable[i].fileid=fileid;
			filetable[i].addr=temp;
			filetable[i].refs=1;
			return((struct FileStruct *)temp);
		}
	}
	low=filetable[0].refs;
	for(i=0;i<64;i++)
	{
		if(filetable[i].refs<low) { low=filetable[i].refs; n=i; }
	}
	if(low>0xFFFF)
	{
		for(i=0;i<64;i++)
		{
			filetable[i].refs=filetable[i].refs-0xFFFF;
		}
		low=low-0xFFFF;
		high=high-0xFFFF;
	}
	temp=getFileAddrBySearching(fileid);
	if(!temp) return((struct FileStruct *) 0);
	filetable[n].fileid=fileid;
	filetable[n].addr=temp;
	filetable[n].refs=low+1;
	return((struct FileStruct *)temp);
}



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
	char data[24];
};
	 
static struct feslot *curfeslot;

void initfsmem()
{
	if( ((unsigned int)nextaddr) %4096 ) nextaddr=(unsigned int *)( ((((unsigned int)nextaddr) /4096 )*4096)+4096);
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
	  	curfeslot->numofslots=128;
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

static unsigned int clustcnt;
static unsigned int FreeClustCount;
static unsigned int FirstFreeClust;

void SearchFreeClusters()
{
	unsigned int i,j;
	unsigned int *fat;
	FirstFreeClust=0;
	FreeClustCount=0;
	for(i=0;i<fatsects;i++)
	{
		delay();
		fat=LoadFATSect(i,0);
		delay();
		for(j=0;j<128;j++)
		{
			if(!fat[j]) 
			{
				FreeClustCount++;
				if(!FirstFreeClust) FirstFreeClust=i*128+j;
			}
		}
	}
}

unsigned int getFreeClust()
{
	unsigned int i,j;
	unsigned int *fat;
	unsigned int clust;
	clust=FirstFreeClust;
	i=clust/128;
	j=clust%128;
	fat=LoadFATSect(i,0x80000000); //0x80000000-writemode
	fat[j]=0x0FFFFFF8;
	FirstFreeClust=0;
	for(i=(FirstFreeClust/128);i<fatsects;i++)
	{
		fat=LoadFATSect(i,0);
		for(j=0;j<128;j++)
		{
			if(!fat[j]) { FirstFreeClust=i*128+j; break; }
		}
		if(FirstFreeClust) break;
	}
	FreeClustCount--;
	return(clust);
}

unsigned int getNextFreeClust(unsigned int clust)
{
	unsigned int i,j;
	unsigned int *fat;
	unsigned int nextclust;
	nextclust=FirstFreeClust;
	i=clust/128;
	j=clust%128;
	fat=LoadFATSect(i,0x80000000); //0x80000000-writemode
	fat[j]=nextclust;
	i=nextclust/128;
	j=nextclust%128;
	fat=LoadFATSect(i,0x80000000); //0x80000000-writemode
	fat[j]=0x0FFFFFF8;	
	for(i=(FirstFreeClust/128);i<fatsects;i++)
	{
		fat=LoadFATSect(i,0);
		for(j=0;j<128;j++)
		{
			if(!fat[j]) { FirstFreeClust=i*128+j; break; }
		}
	}
	FreeClustCount--;
	return(nextclust);	
}

initVid()
{
	PS(7,0x000B8);
}

int initFS(unsigned int pdrive)
{
	MyProcessId=25;
	initVid();
	if((pdrive!=(unsigned int)-1)&&((pdrive<'A')||(pdrive>'Z'))) 
	{
		printk("\nIllegal drive value : %d",pdrive);
		return(1);
	}
	drive=pdrive;
	nextaddr=(unsigned int *)&GData;
	if(((unsigned int)nextaddr)%4096) nextaddr=(unsigned int *)((((unsigned int)nextaddr)/4096)*4096+4096);
	//printk("\nGData : 0x%x, NextAddr : 0x%x",(unsigned int)&GData,(unsigned int)nextaddr);
	struct bpbstruct *bpb;
	bpb=(struct bpbstruct *) &GData;
	delay();
	delay();
	DiskManager(PREAD,0,1,drive,(unsigned int) bpb,MyProcessId);
	delay();
	bpbSectorsPerCluster=bpb->bpbSectorsPerCluster;
	clustcnt=bpbSectorsPerCluster/8;
	if(clustcnt==0) { printk("\nAtleast 4KB clusters are required..."); hangon(); }
	if(clustcnt>8) clustcnt=8;
	bpbBytesPerSector=bpb->bpbBytesPerSector;
	bpbRootClus=bpb->bpbRootClus;
	if(bpbBytesPerSector!=512) { printk("\nOnly 512 bytes per sector is allowed..."); hangon(); }
	fatstartlba=bpb->bpbReservedSectors;
	fatsects=((bpb->bpbTotalSectorsBig/bpb->bpbSectorsPerCluster)*4)/(bpb->bpbBytesPerSector);
	if(((bpb->bpbTotalSectorsBig/bpb->bpbSectorsPerCluster)*4)%(bpb->bpbBytesPerSector)) fatsects++;
	if(fatsects<(bpb->bpbSectsPerFAT32))fatsects=bpb->bpbSectsPerFAT32;
	datastartlba=fatstartlba+fatsects*bpb->bpbNumberOfFATs;

	if(bpbRootClus==0) { printk("\nRoot Directory not available"); hangon(); }

	initFATBuffer();
	initDirBuffer();
	initFileRefCache();
	FileList=0;
	LastFileEntry=FileList;
	NoOfFiles=0;
	initfsmem();
	printk("\nSearching Free Clusters");
	SearchFreeClusters();
	printk("\nFinished Searching Free Clusters");
	GFilePath=(char *) getnewpage();
	p_aPage=(char *)getnewpage();
	printk("\nGFilePath=0x%x",(unsigned int)GFilePath);
	return(0);
}

int OpenFileOrDir(unsigned int param1,unsigned int mode,unsigned int type,unsigned int dirhandle)
{
	static int fileidcount=5;
	int i,j,dirlba,filelba,entryno;
	unsigned char ff=0;
	unsigned int fsize,firstdirlba,reusehandle;
	char *FilePath;
	struct FileStruct *FileEntry;
	int dirclust;
	struct fatdirstruct *dir;
	char dirname[12];
	char filename[12];
	reusehandle=0;
	FilePath=(char *)param1;
	i=0;
	dirlba=bpbRootClus;
	//printk("\nFilePath : %s",FilePath);
	if((type==6)||(type==7))	// openfileordir in handle and reuse handle...
	{
		type=type-2;
		reusehandle=1;
	}
	if((type==4)||(type==5))	// openfileordir from handle...
	{
		type=type-2;
		dirlba=0;
		if(FilePath[11]){ /*printk("\nF Path");*/ return(3); }	//file not found...	//only single file or dir name. no path...
		FileEntry=getFileAddress(dirhandle);
		if((unsigned int)FileEntry==0) { /*printk("\nFE=0");*/ return(2); }	//dir not found...
		if(!(FileEntry->entryno&0x40000000)) return(2);	//dir not found...
		dirlba=FileEntry->FirstClust;
		//if(reusehandle) CloseFile(dirhandle);
		i=0;
		goto SearchFileOrDir;
	}
	if((type==3)&&(!FilePath[0]))
	{
		if(mode==WRITEONLY) return(4);	//read only...
		dirclust=dirlba&0x0FFFFFFF;
		filelba=dirlba&0x0FFFFFFF;
		entryno=0x80000000;
		firstdirlba=dirclust;
		goto LoadFileInfo;
	}
		
	while(FilePath[i+11])
	{
		if(FilePath[i]==(char)0xE5) FilePath[i]=0x05;
		strncpy(dirname,FilePath+i,11);
		dirname[11]=0;
		dirclust=dirlba&0x0FFFFFFF;
		dirlba=0;
		while((dirclust)&&(dirclust<0x0FFFFFF7))
		{
			//DiskManager(READ,datastartlba+(dirclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,drive,(unsigned int)&dir);
			dir=(struct fatdirstruct *) LoadDirClust(dirclust,0);	//0-readmode
			for(j=0;j<16*bpbSectorsPerCluster;j++)
			{
				//printk("\n%s",dir[j].name);
				if(!dir[j].name[0]) break;
				if((!strncmp(dir[j].name,dirname,11))&&(dir[j].attrib&0x10))
				{
					dirlba=(((unsigned int)dir[j].ClustHigh)<<16)|((unsigned int)dir[j].ClustLow);
					break;
				}
			}
			if(dirlba) break;
			dirclust=getNextClust(dirclust);
		}
		if(dirlba==0) return(2);	//Directory not found. Incorrect path.
		i=i+11;
	}

SearchFileOrDir:
	//printk("\nSearching File Or Dir");
	if(FilePath[i]==(char)0xE5) FilePath[i]=0x05;
	strncpy(filename,FilePath+i,11);
	dirclust=dirlba&0x0FFFFFFF;
	firstdirlba=dirclust;
	filelba=0;
	while((dirclust)&&(dirclust<0x0FFFFFF7))
	{
		//printk("\nDirClust 1 : %d",dirclust);
		//DiskManager(READ,datastartlba+(dirclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,drive,(unsigned int)&dir);
		dir=(struct fatdirstruct *) LoadDirClust(dirclust,0);	//0-readmode
		for(j=0;j<16*bpbSectorsPerCluster;j++)
		{
			//printk("\n%s",&dir[j].name[0]);
			if(!dir[j].name[0]) break;
			if((!strncmp(dir[j].name,filename,11))&&(!(dir[j].attrib&0x08)))
			{
			 if((mode==WRITEONLY)&&(dir[j].attrib&0x10)) return(4);	// dir as file-read only
			 else if((type==3)&&(!(dir[j].attrib&0x10))) { /*printk("\nno dir present");*/ return(2);	}	// directory not found...
				filelba=(((unsigned int)dir[j].ClustHigh)<<16)|((unsigned int)dir[j].ClustLow);
				filelba=filelba&0x0FFFFFFF;
				entryno=j;
				fsize=dir[j].Size;
				if((mode==WRITEONLY)&&(dir[j].attrib&0x01)) return(4); //file readonly.
				if((mode==READONLY)&&(!(dir[j].attrib&0x10))&&((filelba==0)||(filelba>=0x0FFFFFF7)||(dir[j].Size==0))) return(1);	//file is empty to read.
				if(filelba==0) filelba=0x0FFFFFFF;
				ff=1;
				break;
			}
		}
		if(ff) break;
		dirclust=getNextClust(dirclust);
	}
	if(!ff) return(3); 		//file not found.
	//printk("\nFilelba= %d",filelba);
LoadFileInfo:
	//printk("\nLoading File Info");
	i=NoOfFiles;
	FileEntry=FileList;
	while(i--)
	{
		if(FileEntry==0) break;
		if((FileEntry->dirlba==dirclust)&&(FileEntry->entryno==entryno)&&(FileEntry->FirstClust==filelba))
		{
			if((mode==WRITEONLY)&&(FileEntry->entryno&0x80000000))
				return(4);		//only one time file can be opened in write only...
			return(FileEntry->fileid);
		}
		FileEntry=FileEntry->Next;
	}
	if(!reusehandle)
	{
		FileEntry=getnewfeslot();
		FileEntry->Next=0;
		if(!(unsigned int)FileList) FileList=FileEntry;
		if((unsigned int)LastFileEntry) LastFileEntry->Next=FileEntry;
		LastFileEntry=FileEntry;
	}
	else 
	{
		FileEntry=getFileAddress(dirhandle);
		if(!(unsigned int)FileEntry) { /*printk("Reuse handle FE not found");*/ return(2); }
	}
	if(!reusehandle)
	{
		FileEntry->fileid=fileidcount;
		fileidcount++;
	}
	else
	{
		FileEntry->fileid=dirhandle;
	}
	FileEntry->dirlba=dirclust;
	//printk("\nDirClust 2 : %d",dirclust);
	FileEntry->entryno=entryno;
	//printk("\nFile lba=%d",filelba);
	if(mode==WRITEONLY) 
	{
		FileEntry->entryno=FileEntry->entryno | 0x80000000;
		BreakClustList(filelba);
		FileEntry->FirstClust=0;
		FileEntry->CurrentClust=0;
		FileEntry->CurrentClustNo=0;
		FileEntry->Size=0;
	}
	else
	{
		FileEntry->FirstClust=filelba;
		FileEntry->CurrentClust=filelba;
		FileEntry->CurrentClustNo=0;
		FileEntry->Size=fsize;
	}
	if(type==3)
	{
		FileEntry->entryno=FileEntry->entryno | 0x40000000;
		FileEntry->Size=firstdirlba;		// used to store first dir lba...
	}
	//printk("\nHandle=%d",FileEntry->fileid);
	if(!reusehandle) NoOfFiles++;
	//printk("\nNoOfFiles=%d, FileList=0x%x, LastFileEntry=0x%x",NoOfFiles,(unsigned int)FileList,(unsigned int)LastFileEntry);
	return(FileEntry->fileid);
}

unsigned int WriteFileClusts(unsigned int clust,unsigned int noofclusts,unsigned int addr,unsigned int processId)
{
	unsigned int i,n,k;
	for(i=0;i<noofclusts;)
	{
		n=(clustcnt-((clust&0xF0000000)>>28));
		if((i+n)>noofclusts) n=noofclusts-i;
		//printk("\nDirClust in DM : %d",clust);
		//printk("\nWriting by DM lba=%d",datastartlba+((clust&0x0FFFFFFF)-2)*bpbSectorsPerCluster+((clust&0xF0000000)>>28));
		k=DiskManager(PWRITE,datastartlba+((clust&0x0FFFFFFF)-2)*bpbSectorsPerCluster+((clust&0xF0000000)>>28),n*8,drive,addr,processId);
		//printk("\nFinished writing by DM : %d",k);
		addr=addr+(n*4096);
		i=i+n;
		if(i<noofclusts) clust=getNextFreeClust(clust&0x0FFFFFFF);
	}
	n=((clust&0xF0000000)>>28)+n;
	return(clust|(n<<28));
}

CopyUAPage(char *src,char *dest)	// Copy unaligned page...
{
	int i;
	for(i=0;i<4096;i++)
	{
		*(dest+i)=*(src+i);
	}
}

void CopyNPages(unsigned int *src,unsigned int *dest,unsigned int n,unsigned int processId)
{
	char *p_aPage,*p_srcPage;
	int i,j;			// should be implemented with process pages...

		/*for(i=0;i<n;i++)
		{
			for(j=0;j<1024;j++)
			{
				*(dest+(i*1024)+j)=*(src+(i*1024)+j);
			}
		}*/

	if(((unsigned int)dest)%4096)
	{
		unsigned int disp;
		disp=((unsigned int)dest)%4096;
		p_aPage=getnewpage();
		p_srcPage=(char *) src;
		PS(CPYTOME,((unsigned int)p_aPage)/4096,processId,((unsigned int)dest)/4096);
		for(i=0;i<4096-disp;i++)
		{
			*(p_aPage+disp+i)=*(p_srcPage+i);
		}
		PS(CPYFRMME,((unsigned int)p_aPage)/4096,processId,((unsigned int)dest)/4096);
		p_srcPage=p_srcPage+4096;
		for(i=0;i<n-1;i++)
		{
			CopyUAPage(p_srcPage,p_aPage);
			PS(CPYFRMME,((unsigned int)p_aPage)/4096,processId,(((unsigned int)dest)/4096)+1+i);
			p_srcPage=p_srcPage+4096;
		}
		PS(CPYTOME,((unsigned int)p_aPage)/4096,processId,(((unsigned int)dest)/4096)+n);
		for(i=0;i<disp;i++)
		{
			*(p_aPage+i)=*(p_srcPage+i);
		}
		PS(CPYFRMME,((unsigned int)p_aPage)/4096,processId,(((unsigned int)dest)/4096)+n);
	}
	else
	{
		for(i=0;i<n;i++)
		{
			PS(CPYFRMME,(((unsigned int)src)/4096)+i,processId,(((unsigned int)dest)/4096)+i);
		}
	}
}

unsigned int LoadFileClusts(unsigned int clust,unsigned int noofclusts,unsigned int addr,unsigned int dir,unsigned int processId)
{
	unsigned int i,n,k,j,g,cnt;
	for(i=0;i<noofclusts;)
	{
		n=(clustcnt-((clust&0xF0000000)>>28));
		if((i+n)>noofclusts) n=noofclusts-i;
		if(dir)
		{
			cnt=(clust&0xF0000000)>>28;    //CopyNPages is not used, because dirtable+32 doesnot start at page boundary...
			/*for(j=0;j<32;j++)
			{
				if(*(dirtable+j)&0x0FFFFFFF==clust)
				{
					CopyNPages(dirtable+32+(j*dirclustsize)+(cnt*1024),(unsigned int *)addr,n,processId);
					break;
				}
			}
			if(j==32)
				DiskManager(PREAD,datastartlba+((clust&0x0FFFFFFF)-2)*bpbSectorsPerCluster+(cnt*8),n*8,drive,addr,processId);
			*/
			if(ldclust==clust)
				DiskManager(PWRITE,datastartlba+(ldclust-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,drive,(unsigned int)(dirtable+32),MyProcessId);
				DiskManager(PREAD,datastartlba+((clust&0x0FFFFFFF)-2)*bpbSectorsPerCluster+(cnt*8),n*8,drive,addr,processId);
		}
		else
		{
			DiskManager(PREAD,datastartlba+((clust&0x0FFFFFFF)-2)*bpbSectorsPerCluster+((clust&0xF0000000)>>28),n*8,drive,addr,processId);
		}
		addr=addr+(n*4096);
		i=i+n;
		if(i<noofclusts) clust=getNextClust(clust&0x0FFFFFFF);
	}
	if((n=((clust&0xF0000000)>>28)+n+1)>=8) 
	{
		clust=getNextClust(clust&0x0FFFFFFF);
		n=n-8;
	}
	return(clust|(n<<28));
}

int ReadFile(unsigned int fileid,unsigned int cluststartno,unsigned int noofclusts,unsigned int addr,unsigned int tprocess,unsigned int cprocess)
{
	int fclustcnt,i;
	unsigned int j,temp;
	unsigned int clust,dir;
	struct FileStruct *FileEntry;
	if(tprocess==0) tprocess=cprocess;
	dir=0;
	if(fileid<5) return(2);	//file not opened.
	FileEntry=getFileAddress(fileid);
	if(FileEntry==0) return(2);	//file not opened.
	if(FileEntry->entryno&0x80000000) return(4);	//file opened for writeonly.
	if(FileEntry->entryno&0x40000000) dir=1;
	if(noofclusts==0) return(3);	//no data to read.
	if((FileEntry->Size==0)||(FileEntry->FirstClust==0)||(FileEntry->FirstClust>=0x0FFFFFF7)) return(5);	//file is empty.
	if(dir)
	{
		fclustcnt=0;
		clust=FileEntry->FirstClust;
		while((clust)&&(clust<0x7FFFFFFF))
		{
			fclustcnt++;
			clust=getNextClust(clust);
		}
	}
	else
	{
		fclustcnt=FileEntry->Size/4096;
		if(FileEntry->Size%4096) fclustcnt++;
	}
	if(cluststartno>fclustcnt-1) return(0x0000FFFF);	//end of cluster.
	if(cluststartno+noofclusts>fclustcnt) return(6); //less number of clusters available.	
	if(cluststartno==0) FileEntry->CurrentClust=LoadFileClusts(FileEntry->FirstClust,noofclusts,addr,dir,tprocess);
	else if(cluststartno==FileEntry->CurrentClustNo) FileEntry->CurrentClust=LoadFileClusts(FileEntry->CurrentClust,noofclusts,addr,dir,tprocess);
	else
	{	temp=0;
		clust=FileEntry->FirstClust;
		for(i=0;i<cluststartno;i++)
		{
			for(j=0;j<clustcnt;j++)
			{
				if((i*clustcnt+j)==cluststartno)
				{
					clust=clust|(j<<28);
					temp=1;
					break;
				}
			}
			if(temp)break;
			clust=getNextClust(clust);
		}
		FileEntry->CurrentClust=LoadFileClusts(clust,noofclusts,addr,dir,tprocess);
	}
	FileEntry->CurrentClustNo=cluststartno+noofclusts;
	if(cluststartno+noofclusts==fclustcnt) 
	{
		if(dir) return(0x0000FFFF);
		temp=(FileEntry->Size)%4096;
		if(!temp) temp=4096;
		return((temp<<16)|0x0000FFFF);
	}
	return(0);
}

int WriteFile(unsigned int fileid,unsigned int noofclusts,unsigned int bytesinlastclust,unsigned int addr,unsigned int tprocess,unsigned int cprocess)
{
	int fclustcnt,i;
	unsigned int j,temp;
	unsigned int clust;
	struct FileStruct *FileEntry;
	if(tprocess==0) tprocess=cprocess;
	if(fileid<5) return(2);	//file not opened.
	FileEntry=getFileAddress(fileid);
	if(FileEntry==0) return(2);	//file not opened.
	if(!(FileEntry->entryno&0x80000000)) return(4);	//file opened for readonly.
	if(noofclusts==0) return(3);	//no data to write.
	if(FileEntry->Size==0) FileEntry->Size=(noofclusts-1)*4096+bytesinlastclust;
	else 
	{
		if(FileEntry->Size%4096) FileEntry->Size=(FileEntry->Size/4096)*4096+4096;
		FileEntry->Size=FileEntry->Size+bytesinlastclust;
	}
	if(FreeClustCount<noofclusts) return(5);	//less clusters available.
	if(!FileEntry->FirstClust) 
	{
		FileEntry->FirstClust=getFreeClust();
		FileEntry->CurrentClust=FileEntry->FirstClust;
	}
	else
	{
		if((FileEntry->CurrentClust&0x70000000)>>28==clustcnt) FileEntry->CurrentClust=getNextFreeClust(FileEntry->CurrentClust&0x0FFFFFFF);
		else FileEntry->CurrentClust=FileEntry->CurrentClust+0x10000000;
	}
	FileEntry->CurrentClust=WriteFileClusts(FileEntry->CurrentClust,noofclusts,addr,tprocess);
	FileEntry->entryno=FileEntry->entryno|0x20000000;	// file modified...
	return(0);
}

void BreakClustList(unsigned int clust)
{
	unsigned int i,j;
	unsigned int *fat;
	unsigned int nextclust;
	i=clust/128;
	j=clust%128;
	fat=LoadFATSect(i,0x80000000); //0x80000000-writemode
	nextclust=clust;
	while((nextclust)&&(nextclust<0x0FFFFFF7))
	{
		if(nextclust<FirstFreeClust) FirstFreeClust=nextclust;
		nextclust=fat[j]&0x0FFFFFFF;
		fat[j]=0;
		FreeClustCount++;
		if((!nextclust)||(nextclust>=0x0FFFFFF7)) break;
		if((nextclust/128)!=i)
		{
			i=nextclust/128;
			fat=LoadFATSect(i,0x80000000); //0x80000000-writemode
		}
		j=nextclust%128;
	}
}

unsigned int getTime()
{
	return(TM(4));
}

unsigned int getDate()
{
	return(TM(5));	
}

static unsigned char day,month,second,minute,hour,tenmstick;
static unsigned short year;
static unsigned int time,date;

int CloseFile(unsigned int fileid)
{
	int i;			// do not use getFileAddress() because we need PrevFileEntry...
	i=NoOfFiles;
	struct FileStruct *FileEntry,*PrevFileEntry;
	struct fatdirstruct *dir;

	time=getTime();
	hour=*(((char *)&time)+3);
	minute=*(((char *)&time)+2);
	second=*(((char *)&time)+1);
	//printk("Time %d:%d:%d",hour,minute,second);
	tenmstick=*((char *)&time);
	if(second%2) tenmstick=tenmstick+100;
	second=second/2;

	time=(unsigned int) hour&0x1F;
	time=(time<<6)|((unsigned int)minute&0x3F);
	time=(time<<5)|((unsigned int)second&0x1F);

	date=getDate();
	year=*(((short *)&date)+1);
	month=*(((char *)&date)+1);
	day=*((char *)&date);
	//printk("Date : %d/%d/%d",day,month,year);
	date=(unsigned int) year&0x7F;
	date=(date<<4)|((unsigned int)month&0xF);
	date=(date<<5)|((unsigned int)day&0x1F);

	FileEntry=FileList;
	PrevFileEntry=0;
	while(i--)
	{
		if(FileEntry==0) return(1);	// file not opened...
		if(FileEntry->fileid==fileid)
		{
			if((FileEntry->FirstClust!=bpbRootClus)&&(FileEntry->FirstClust!=0))
			{
				//printk("\nLoading dir for fileclose :\n");
				dir=(struct fatdirstruct *) LoadDirClust(FileEntry->dirlba,0x80000000);
				//printk("%s",(char *)dir);
				if(FileEntry->entryno&0x20000000)
				{
					(dir+(FileEntry->entryno&0x0FFFFFFF))->ClustLow=(unsigned short) FileEntry->FirstClust;
					(dir+(FileEntry->entryno&0x0FFFFFFF))->ClustHigh=*(((unsigned short *) &(FileEntry->FirstClust))+1);
					(dir+(FileEntry->entryno&0x0FFFFFFF))->Size=FileEntry->Size;
				}
				// put file accessed date...
				(dir+(FileEntry->entryno&0x0FFFFFFF))->ADate=date;
				if(FileEntry->entryno&0x20000000)
				{
					(dir+(FileEntry->entryno&0x0FFFFFFF))->UDate=date;
					(dir+(FileEntry->entryno&0x0FFFFFFF))->UTime=time;
					// put file modified date and time...
				}
			}
			removeFileCacheRef(fileid);
			if((unsigned int) PrevFileEntry)
			{
				PrevFileEntry->Next=FileEntry->Next;
				if(LastFileEntry==FileEntry)
					LastFileEntry=PrevFileEntry;
			}
			else 
			{
				FileList=FileEntry->Next;
				if(FileList->Next==0) LastFileEntry=FileList;
				if(FileList==0) LastFileEntry=FileList;
			}
			freeafeslot((void *)FileEntry);
			NoOfFiles--;
			return(0);
		}
		PrevFileEntry=FileEntry;
		FileEntry=FileEntry->Next;
	}
	return(1); // file not opened...
}

void ShutDown()
{
	int i;
	i=NoOfFiles;
	struct FileStruct *FileEntry;
	struct fatdirstruct *dir;

	time=getTime();
	hour=*(((char *)&time)+3);
	minute=*(((char *)&time)+2);
	second=*(((char *)&time)+1);
	//printk("Time %d:%d:%d",hour,minute,second);
	tenmstick=*((char *)&time);
	if(second%2) tenmstick=tenmstick+100;
	second=second/2;

	time=(unsigned int) hour&0x1F;
	time=(time<<6)|((unsigned int)minute&0x3F);
	time=(time<<5)|((unsigned int)second&0x1F);

	date=getDate();
	year=*(((short *)&date)+1);
	month=*(((char *)&date)+1);
	day=*((char *)&date);
	//printk("Date : %d/%d/%d",day,month,year);
	date=(unsigned int) year&0x7F;
	date=(date<<4)|((unsigned int)month&0xF);
	date=(date<<5)|((unsigned int)day&0x1F);

	FileEntry=FileList;
	while(i--)
	{
		if(FileEntry==0) break;	// no file remains opened...
		if((FileEntry->FirstClust!=bpbRootClus)&&(FileEntry->FirstClust!=0))
		{
			//printk("\nLoading dir for shutdown :\n");
			dir=(struct fatdirstruct *) LoadDirClust(FileEntry->dirlba,0x80000000);
			if(FileEntry->entryno&0x20000000)
			{
				(dir+(FileEntry->entryno&0x0FFFFFFF))->ClustLow=(unsigned short) FileEntry->FirstClust;
				(dir+(FileEntry->entryno&0x0FFFFFFF))->ClustHigh=*(((unsigned short *) &(FileEntry->FirstClust))+1);
				(dir+(FileEntry->entryno&0x0FFFFFFF))->Size=FileEntry->Size;
			}
			// put file accessed date...
				(dir+(FileEntry->entryno&0x0FFFFFFF))->ADate=date;
			if(FileEntry->entryno&0x20000000)
			{
				(dir+(FileEntry->entryno&0x0FFFFFFF))->UDate=date;
				(dir+(FileEntry->entryno&0x0FFFFFFF))->UTime=time;
				// put file modified date and time...
			}
		}
		FileEntry=FileEntry->Next;
	}
	ShutDownDirBuffer(); // writes to bad place and creates problem. But when the dir is referenced again, the problem is cleared...
	ShutDownFATBuffer();
	return;	// no file remains opened...
}

int DeleteFileOrDirWithHandle(unsigned int handle)
{
	struct fatdirstruct *dir;
	unsigned int dirlba,entryno,firstclust;
	struct FileStruct *FileEntry;
	FileEntry=getFileAddress(handle);
	//printk("\nDFD : %d",handle);
	if((unsigned int)FileEntry==0) return(2);	//file or dir not opened...
	dirlba=FileEntry->dirlba;
	firstclust=FileEntry->FirstClust;
	entryno=FileEntry->entryno&0x0FFFFFFF;
	CloseFile(handle);
	dir=(struct fatdirstruct *) LoadDirClust(dirlba,0x80000000);
	dir[entryno].name[0]=0xE5;
	if((firstclust)&&(firstclust<0x0FFFFFF7));
		BreakClustList(firstclust);
	return(0);
}

int DeleteFileWithHandle(unsigned int fileid)
{				// first close file and remove dir entry...
	struct FileStruct *FileEntry;
	//printk("\nfileid=%d",fileid);
	FileEntry=getFileAddress(fileid);
	if((unsigned int)FileEntry==0) return(2);	// file not opened...
	if(FileEntry->entryno & 0x40000000) {/* printk("File is a dir"); */ return(2); }	// file not opened. The opened one is a directory.
	return(DeleteFileOrDirWithHandle(fileid));
}

int DeleteFile(unsigned int param1)	//param1 is the address of filepath string.
{
	unsigned int temp,fileid;
	temp=NoOfFiles;
	fileid=OpenFileOrDir(param1,READONLY,2,0);
	if((fileid<5)&&(fileid!=1)) return(fileid);
	if((fileid!=1)&&(temp==NoOfFiles)) return(4);	//file opened by someone.
	if(fileid==1) 
	{
		fileid=OpenFileOrDir(param1,WRITEONLY,2,0);
		if(fileid<5) return(fileid);
		if(temp==NoOfFiles) return(4);	//file opened by someone.
	}
	return(DeleteFileWithHandle(fileid));
}

int DeleteFileFromDirHandle(unsigned int dirhandle,unsigned int filename)
{
	unsigned int temp,fileid;
	temp=NoOfFiles;
	fileid=OpenFileFromDirHandle(dirhandle,filename,READONLY);
	if((fileid<5)&&(fileid!=1)) return(fileid);
	if((fileid!=1)&&(temp==NoOfFiles)) return(4);	//file opened by someone.
	if(fileid==1) 
	{
		fileid=OpenFileFromDirHandle(dirhandle,filename,WRITEONLY);
		if(fileid<5) return(fileid);
		if(temp==NoOfFiles) return(4);	//file opened by someone.
	}
	return(DeleteFileWithHandle(fileid));
}

int OpenFile(unsigned int path,unsigned int mode)
{
	return(OpenFileOrDir(path,mode,2,0));
}

int OpenFileFromDirHandle(unsigned int dirhandle,unsigned int filename,unsigned int mode)
{
	return(OpenFileOrDir(filename,mode,4,dirhandle));
}

int OpenFileInDirHandle(unsigned int dirhandle,unsigned int filename,unsigned int mode)
{
	return(OpenFileOrDir(filename,mode,6,dirhandle));
}

int OpenDirFromDirHandle(unsigned int dirhandle,unsigned int dirname)
{
	return(OpenFileOrDir(dirname,READONLY,5,dirhandle));
}

int OpenDirInDirHandle(unsigned int dirhandle,unsigned int dirname)
{
	return(OpenFileOrDir(dirname,READONLY,7,dirhandle));
}

int OpenDir(unsigned int path)
{
	//printk("\nIn open dir");
	return(OpenFileOrDir(path,READONLY,3,0));
}

int CloseDir(unsigned int dirid)
{
	struct FileStruct *FileEntry;
	FileEntry=getFileAddress(dirid);
	if((unsigned int)FileEntry==0) return(2);	//dir not found...
	if(!(FileEntry->entryno&0x40000000)) return(2);	//dir not found...
	return(CloseFile(dirid));
}

int DeleteDirWithDirHandle(unsigned int dirhandle)
{
	int j,t=0;
	unsigned int dirclust,dirhandle2,temp;
	struct FileStruct *FileEntry;
	struct fatdirstruct *dir;
	char filename[12];
	FileEntry=getFileAddress(dirhandle);
	if((unsigned int)FileEntry==0) { /*printk("DD FE=0");*/ return(2); }	// dir not opened...
	if(!(FileEntry->entryno&0x40000000)) { /*printk("dir not opened");*/ return(2); }	// dir not opened. The opened one is a file.
	if((FileEntry->FirstClust==bpbRootClus)||(FileEntry->FirstClust==0)) return(3);	// cannot delete root directory...
	dirclust=FileEntry->FirstClust;
	while((dirclust)&&(dirclust<0x0FFFFFF7))
	{
		dir=(struct fatdirstruct *) LoadDirClust(dirclust,0x80000000);
		for(j=0;j<16*bpbSectorsPerCluster;j++)
		{
			if(!dir[j].name[0]) { t=1; break; }
			if((dir[j].name[0]!='.')&&(dir[j].name[0]!=(char)0xE5)&&(!(dir[j].attrib&0x08)))
			{
				strncpy(&filename[0],&dir[j].name[0],11);
				//printk("\nSelected %s",filename);
				filename[12]=0;
				if(dir[j].attrib&0x10)
				{
					temp=NoOfFiles;
					dirhandle2=OpenDirFromDirHandle(dirhandle,(unsigned int)&filename[0]);
					if(dirhandle2<5) return(dirhandle2);
					if(temp==NoOfFiles) return(4);	// dir already opened...
					//printk("\nDeleting Dir %s",filename);
					dirhandle2=DeleteDirWithDirHandle(dirhandle2);
					if(dirhandle2) return(dirhandle2);
				}
				else
				{
					dirhandle2=DeleteFileFromDirHandle(dirhandle,(unsigned int)&filename[0]);
					if(dirhandle2) return(dirhandle2);
				}
			}
		}
		if(t) break;
		dirclust=getNextClust(dirclust);
	}
	return(DeleteFileOrDirWithHandle(dirhandle));
}

int DeleteDir(unsigned int path)
{
	unsigned int dirhandle,temp;
	temp=NoOfFiles;
	dirhandle=OpenDir(path);
	if(dirhandle<5) return(dirhandle);
	if(temp==NoOfFiles) return(4);	// dir opened by someone...
	dirhandle=DeleteDirWithDirHandle(dirhandle);
	return(dirhandle);
}

int DeleteDirFromDirHandle(unsigned int dirhandle,unsigned int dirname)
{
	unsigned int dirhandle2,temp;
	temp=NoOfFiles;
	dirhandle2=OpenDirFromDirHandle(dirhandle,dirname);
	if(dirhandle2<5) return(dirhandle2);
	if(temp==NoOfFiles) return(4);	// dir opened by someone...
	dirhandle2=DeleteDirWithDirHandle(dirhandle2);
	return(dirhandle2);
}

void fillPageWithZero(unsigned int *p_page)
{
	int i;
	for(i=0;i<1024;i++)
	{
		*(p_page+i)=0;
	}
}

int getFreeDirSlot(unsigned int *dirlba,struct fatdirstruct *dir)
{
	int j;
	unsigned int dirclust;
	dirclust=*dirlba;
	if((!(*dirlba))||((*dirlba)>=0x0FFFFFF7))
	{
		*dirlba=getFreeClust();
		dir=(struct fatdirstruct *) LoadDirClust((*dirlba),0x80000000);
		for(j=0;j<clustcnt;j++)
		{
			fillPageWithZero(((unsigned int*)dir)+(j*1024));
		}
		return(0);	// firstslot...
	}	
	while((*dirlba)&&((*dirlba)<0x0FFFFFF7))
	{
		dir=(struct fatdirstruct *) LoadDirClust((*dirlba),0x00000000);
		for(j=0;j<16*bpbSectorsPerCluster;j++)
		{
			if(!dir[j].name[0])
			{
				dir=(struct fatdirstruct *) LoadDirClust((*dirlba),0x80000000);
				return(j);
			}
		}
		dirclust=*dirlba;
		*dirlba=getNextClust(*dirlba);
	}
	*dirlba=dirclust;
	*dirlba=getNextFreeClust(*dirlba);
	dir=(struct fatdirstruct *) LoadDirClust((*dirlba),0x80000000);
	for(j=0;j<clustcnt;j++)
	{
		fillPageWithZero(((unsigned int*)dir)+(j*1024));
	}
	return(0);	// firstslot...
}

int NewDir(unsigned int path)
{
	int i;
	char *filepath;
	char filename[12];
	unsigned int dirhandle;
	filepath=(char *)path;
	if(*filepath==0) return(1);	// no file to create...
	for(i=0;i<4096;i++)
	{
		if(*(filepath+i)==0)
		{
			if(i%11) return(2);	// invalid path...
			strncpy(&filename[0],filepath+i,11);
			filename[12]=0;
			*(filepath+i-11)=0;
			break;
		}
	}
	if(i==4096) return(2);		// invalid path...
	dirhandle=OpenDir(path);
	if(dirhandle<5) return(dirhandle);
	return(NewDirInDirHandle(dirhandle,(unsigned int) &filename));
}

int NewDirInDirHandle(unsigned int dirhandle,unsigned int dirname)
{
	int retvalue;
	unsigned int dirhandle2;
	retvalue=MakeNewFileOrDir(dirhandle,(char *)dirname,3);	// 3-dir...Dir Attributes are not implemented now...
	if(retvalue) return(retvalue);
	dirhandle2=OpenDirInDirHandle(dirhandle,dirname);
	return(dirhandle2);
}

int NewDirFromDirHandle(unsigned int dirhandle,unsigned int dirname)
{
	int retvalue;
	unsigned int dirhandle2;
	//printk("\nIn Create New Dir");
	retvalue=MakeNewFileOrDir(dirhandle,(char *)dirname,3);	// 3-dir...Dir Attributes are not implemented now...
	if(retvalue) return(retvalue);
	dirhandle2=OpenDirFromDirHandle(dirhandle,dirname);
	return(dirhandle2);
}

int NewFile(unsigned int path)
{
	int i;
	char *filepath;
	char filename[12];
	unsigned int dirhandle;
	filepath=(char *)path;
	if(*filepath==0) return(1);	// no file to create...
	for(i=0;i<4096;i++)
	{
		if(*(filepath+i)==0)
		{
			if(i%11) return(2);	// invalid path...
			strncpy(&filename[0],filepath+i,11);
			filename[12]=0;
			*(filepath+i-11)=0;
			break;
		}
	}
	if(i==4096) return(2);		// invalid path...
	dirhandle=OpenDir(path);
	if(dirhandle<5) return(dirhandle);
	return(NewFileInDirHandle(dirhandle,(unsigned int) &filename));
}

/*struct fatdirstruct
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
};*/

int MakeNewFileOrDir(unsigned int dirhandle,char *filename,char type)	// type=2 - file, type=3 - dir
{
	int i;
	unsigned int filehandle,dirclust,filelba,dirlba,entryno;
	struct FileStruct *FileEntry;
	struct fatdirstruct *dir;

	time=getTime();
	hour=*(((char *)&time)+3);
	minute=*(((char *)&time)+2);
	second=*(((char *)&time)+1);
	//printk("Time %d:%d:%d",hour,minute,second);
	tenmstick=*((char *)&time);
	if(second%2) tenmstick=tenmstick+100;
	second=second/2;

	time=(unsigned int) hour&0x1F;
	time=(time<<6)|((unsigned int)minute&0x3F);
	time=(time<<5)|((unsigned int)second&0x1F);

	date=getDate();
	year=*(((short *)&date)+1);
	month=*(((char *)&date)+1);
	day=*((char *)&date);
	//printk("Date : %d/%d/%d",day,month,year);
	date=(unsigned int) year&0x7F;
	date=(date<<4)|((unsigned int)month&0xF);
	date=(date<<5)|((unsigned int)day&0x1F);

	if(filename[0]==(char)0xE5) filename[0]=0x05;

	FileEntry=getFileAddress(dirhandle);
	if((unsigned int)FileEntry==0) return(2);	// dir not opened...
	if(!(FileEntry->entryno&0x40000000)) return(2);	// dir not opened...
	dirclust=FileEntry->FirstClust;
	dirlba=dirclust;
	filelba=0;
	//printk("\nIn Search");
	while((dirclust)&&(dirclust<0x0FFFFFF7))
	{
		dir=(struct fatdirstruct *) LoadDirClust(dirclust,0);	//0-readmode
		for(i=0;i<16*bpbSectorsPerCluster;i++)
		{
			//printk("\n%s",dir[i].name);
			if(!dir[i].name[0]) break;
			if((!strncmp(dir[i].name,filename,11))&&(!(dir[i].attrib&0x08)))
			{
			 	filelba=1;
			 	break;
			}
		}
		if(filelba) break;
		dirlba=dirclust;
		dirclust=getNextClust(dirclust);
	}
	//printk("\nComp Search");
	if(filelba) return(4);	// file or directory already present as a file or directory.
	entryno=getFreeDirSlot(&dirlba,dir);
	//printk("\nDir Content\n%s",(char *)dir);
	//printk("\nEntryNo: %d",entryno);
	//printk("Dir lba : %d",dirlba);
	dir=(struct fatdirstruct *) LoadDirClust(dirlba,0x80000000);
	strncpy(dir[entryno].name,filename,11);
	dir[entryno].attrib=0;
	if(type==3) dir[entryno].attrib=dir[entryno].attrib | 0x10;
	dir[entryno].Size=0;
	dir[entryno].CTime10mS=tenmstick;
	dir[entryno].CTime=time;
	dir[entryno].CDate=date;	
	dir[entryno].UTime=time;
	dir[entryno].UDate=date;
	if(type==2)
	{
		dir[entryno].ClustHigh=0;
		dir[entryno].ClustLow=0;
		return(0);
	}
	dirlba=getFreeClust();
	//printk("\nFree Dir lba : %d",dirlba);
	dir[entryno].ClustHigh=(unsigned short) (dirlba>>16);
	dir[entryno].ClustLow=(unsigned short) (dirlba&0x0000FFFF);
	//delay();
	//DiskManager(WRITE,datastartlba+(dirlba-2)*bpbSectorsPerCluster,bpbSectorsPerCluster,drive,(unsigned int)dir);
	//delay();
	dir=(struct fatdirstruct *) LoadDirClust(dirlba,0x80000000);
	//printk("Dir lba : %d",dirlba);
	for(i=0;i<clustcnt;i++)
	{
		fillPageWithZero(((unsigned int*)dir)+(i*1024));
	}

	strncpy(&dir[0].name[0],".          ",11);
	dir[0].attrib=0x10;
	dir[0].ClustHigh=(unsigned short) (dirlba>>16);
	dir[0].ClustLow=(unsigned short) (dirlba&0x0000FFFF);

	strncpy(&dir[1].name[0],"..         ",11);
	dir[1].attrib=0x10;
	dir[1].ClustHigh=(unsigned short) ((FileEntry->FirstClust)>>16);
	dir[1].ClustLow=(unsigned short) ((FileEntry->FirstClust)&0x0000FFFF);
	
	return(0);
}

int NewFileInDirHandle(unsigned int dirhandle,unsigned int filename)
{
	int retvalue;
	unsigned int filehandle;
	retvalue=MakeNewFileOrDir(dirhandle,(char *)filename,2);	// 2-file...File Attributes are not implemented now...
	if(retvalue) return(retvalue);
	filehandle=OpenFileInDirHandle(dirhandle,filename,WRITEONLY);
	return(filehandle);
}

int NewFileFromDirHandle(unsigned int dirhandle,unsigned int filename)
{
	int retvalue;
	unsigned int filehandle;
	retvalue=MakeNewFileOrDir(dirhandle,(char *)filename,2);	// 2-file...File Attributes are not implemented now...
	if(retvalue) return(retvalue);
	filehandle=OpenFileFromDirHandle(dirhandle,filename,WRITEONLY);
	return(filehandle);
}

int MoveToHeadDir(unsigned int dirhandle)
{
	int j;
	unsigned int dirclust,filelba,ClustLow,ClustHigh;
	struct FileStruct *FileEntry;
	struct fatdirstruct *dir;
	FileEntry=getFileAddress(dirhandle);
	if((unsigned int)FileEntry==0) return(2);	// dir not found...
	if(!(FileEntry->entryno&0x40000000)) return(2);	//dir not found...
	if((FileEntry->FirstClust==bpbRootClus)||(FileEntry->FirstClust==0)) return(3);	// root dir. cannot step back further...
	//dir=(struct fatdirstruct *) LoadDirClust(FileEntry->dirlba,0x00000000);		// directory don't need to update accessed or modified...
	FileEntry->FirstClust=FileEntry->Size;
	FileEntry->CurrentClust=FileEntry->Size;
	FileEntry->CurrentClustNo=0;
	dir=(struct fatdirstruct *) LoadDirClust(FileEntry->Size,0x00000000);
	for(j=0;j<16*bpbSectorsPerCluster;j++)
	{
		if((!strncmp(dir[j].name,"..         ",11))&&(!(dir[j].attrib&0x08)))
		{
			FileEntry->Size=(((unsigned int)dir[j].ClustHigh)<<16)|((unsigned int)dir[j].ClustLow);
			ClustHigh=dir[j].ClustHigh;
			ClustLow=dir[j].ClustLow;
			if(FileEntry->Size==0) FileEntry->Size=bpbRootClus;
			if(ClustHigh==0) ClustHigh=bpbRootClus>>16;
			if(ClustLow==0) ClustLow=bpbRootClus&0xFFFF;
		}
	}
	dirclust=FileEntry->Size;
	filelba=0;
	while((dirclust)&&(dirclust<0x0FFFFFF7))
	{
		dir=(struct fatdirstruct *) LoadDirClust(dirclust,0);	//0-readmode
		for(j=0;j<16*bpbSectorsPerCluster;j++)
		{
			if((dir[j].name[0])&&(dir[j].name[0]!=(char)0xE5)&&(dir[j].ClustHigh==ClustHigh)&&(dir[j].ClustLow==ClustLow)&&(!(dir[j].attrib&0x08)))
			{
				filelba=dirclust;
				FileEntry->entryno=j | 0x40000000;
			}
		}
		if(filelba) break;
		dirclust=getNextClust(dirclust);
	}
	FileEntry->dirlba=filelba;
	//printk("\nDir : %d",FileEntry->FirstClust);
	//printk("\nHead Dir : %d",FileEntry->Size);
	return(0);
}

int GetFileSize(filehandle)
{
	struct FileStruct *FileEntry;
	FileEntry=getFileAddress(filehandle);
	if((unsigned int)FileEntry==0) return(0);
	if(FileEntry->entryno&0x40000000) return(0);
	return(FileEntry->Size);
}
