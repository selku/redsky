# define	READ	3
# define	WRITE	4
# define 	INVALIDPAGELIMIT	3
# define 	LESSMEMORY		2
# define	NOPITSLOT		4
# define	SUCCESS		0
# define	word	unsigned short
# define	dword	unsigned int

unsigned int getNewMemPage();
void freeMemPage(unsigned int);
unsigned int makeNewNPageTableEntries(unsigned int,unsigned int);
unsigned int makeNewPageTableEntries(unsigned int);
unsigned int makeNewKernelPageTableEntries(unsigned int);
unsigned int makeNewNKernelPageTableEntries(unsigned int,unsigned int);
void makePageDirEntry(unsigned int *,unsigned int,unsigned int);
void fillPageWithZero(unsigned int *);

struct PageTableOrDirEntryStruct
{
	unsigned int Present:1;
	unsigned int Writable:1;
	unsigned int UserDefined:1;
	unsigned int WriteThrough:1;
	unsigned int CacheDisable:1;
	unsigned int Accessed:1;
	unsigned int Dirty:1;
	unsigned int Res:5;
	unsigned int Address:20;		
};	

struct PageDirStruct
{
	struct PageTableOrDirEntryStruct PageDirEntry[1024];
};

struct PageTableStruct
{
	struct PageTableOrDirEntryStruct PageTableEntry[1024];
};

struct PagingInfoSlotStruct
{
	unsigned int ProcessID;
	unsigned int VideoPage;
	unsigned int RequestedPages;
	unsigned int AllocatedPages;
	unsigned int CurrentIP;
	unsigned int PagesInMem;
	unsigned short PageHits;
	unsigned short PageFaults;
	unsigned int PageDir;
};

struct PagingInfoTableStruct
{
	struct PagingInfoSlotStruct PITSlot[128];
};

struct PageListStruct
{
	unsigned int PageAddress[1024];
};

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

struct TSSStruct *tss;

extern int CallTask(unsigned int);

int DiskManager(unsigned int cmd,unsigned int lba,unsigned int nsects,unsigned int drive,unsigned int sect)
{
	tss=(struct TSSStruct *)(0x00100000+128);
	nsects=nsects&0xFFFF;
	drive=(drive<<16)&0xFFFF0000;
	nsects=nsects|drive;
	/*(tss+5)->Eax=cmd;
	(tss+5)->Ebx=lba;
	(tss+5)->Ecx=nsects;
	(tss+5)->Edx=sect;
	CallTask(10*8);
	return((tss+5)->Eax);*/
	return(DMP(cmd,lba,nsects,sect,0));
}


extern unsigned int GData;

static unsigned int PhysicalMemory;

static unsigned int *P_GPages;
static unsigned int N_GPages;
static unsigned int N_LastGPage;

static struct PageListStruct *P_DiskPagesList;
static unsigned int N_DiskPagesList;

static unsigned int N_FreeMemPages;
static unsigned int N_FreeDiskPages;

static unsigned int *P_PagingInfoDirectory;
static struct PagingInfoTableStruct *P_KernelPagingInfoTable;

void initDiskPageList(unsigned int pageClust,unsigned int n_pages)
{
	int i;
	unsigned int *p_pageAddress;
	P_DiskPagesList=(struct PageListStruct *)getNewMemPage();
	//printk("\nDisk Page List : 0x%x",(unsigned int) P_DiskPagesList);
	if((pageClust&0x70000000)>>28)
	{
		for(i=(pageClust&0x70000000)>>28;i>=0;i--)
		{
			P_DiskPagesList->PageAddress[i]=(pageClust&0x0FFFFFFF)|((i&0x7)<<28);
		}
		N_DiskPagesList=((pageClust&0x70000000)>>28)+1;
		N_FreeDiskPages=n_pages+N_DiskPagesList;
		return;
	}
	DiskManager(READ,pageClust,8,-1,(unsigned int)P_DiskPagesList);
	p_pageAddress=(unsigned int *)P_DiskPagesList;
	for(i=1023;i>=0;i--)
	{
		if(*(p_pageAddress+i)) break;
	}
	N_DiskPagesList=i+1;
	N_FreeDiskPages=n_pages;
	return;
}

unsigned int writeToADiskPage(unsigned int pageAddress)
{
	unsigned int pageClust;
	if(N_DiskPagesList==1)
	{
		pageClust=(P_DiskPagesList->PageAddress[0]&0x0FFFFFFF)+(((P_DiskPagesList->PageAddress[0]&0x70000000)>>28)*8);
		DiskManager(READ,pageClust,8,-1,(unsigned int)P_DiskPagesList);
		N_DiskPagesList=1024;
	}
	N_DiskPagesList--;
	pageClust=(P_DiskPagesList->PageAddress[N_DiskPagesList]&0x0FFFFFFF)+(((P_DiskPagesList->PageAddress[N_DiskPagesList]&0x70000000)>>28)*8);
	DiskManager(WRITE,pageClust,8,-1,pageAddress);
	freeMemPage(pageAddress);
	return(P_DiskPagesList->PageAddress[N_DiskPagesList]);
}

static unsigned int CurFGProcessId;
static unsigned int CurBGProcessId;

unsigned int writeMemPagesToDisk(unsigned int n_pages)		//before loading a pointer,check whether the data structure is in mem or disk.
{
	struct PagingInfoTableStruct *p_pagingInfoTable;
	struct PageDirStruct *p_pageDir;
	struct PageTableStruct *p_pageTable;
	int i,j,k,m;
	unsigned int n_freedPages=0;
	if(N_FreeDiskPages<3) return(0);
	if(n_pages==0) return(0);
	if(N_FreeDiskPages<(n_pages+3))
		n_pages=N_FreeDiskPages-2;
	for(i=1023;i>=1;i--)	// i=0 refers to kernel paging info table address...
	{
		if(*(P_PagingInfoDirectory+i))
		{
			p_pagingInfoTable= (struct PagingInfoTableStruct *) *(P_PagingInfoDirectory+i);
			for(j=0;j<128;j++)
			{
				if(((p_pagingInfoTable->PITSlot[j].PagesInMem)>16)&&(p_pagingInfoTable->PITSlot[j].ProcessID!=CurFGProcessId)&&(p_pagingInfoTable->PITSlot[j].ProcessID!=CurBGProcessId))
				{
					p_pageDir=(struct PageDirStruct *) p_pagingInfoTable->PITSlot[j].PageDir;
					for(k=0;k<1024;k++)
					{
						if((*((unsigned int *)p_pageDir+k))&&(p_pageDir->PageDirEntry[k].Present))
						{
							p_pageTable=(struct PageTableStruct *) (*((unsigned int *) p_pageDir + k)&0xFFFFF000);
							for(m=0;m<1024;m++)
							{
								if((*((unsigned int *) p_pageTable+m))&&(p_pageTable->PageTableEntry[m].Present))
								{
								 	if(!(p_pageTable->PageTableEntry[m].Accessed))
								 	{
								 		*((unsigned int*) p_pageTable + m)=(writeToADiskPage(*((unsigned int *)p_pageTable + m)&0xFFFFF000)&0x7FFFFFFF)<<1;
										n_freedPages++;
								 		p_pagingInfoTable->PITSlot[j].PagesInMem--;
										if(p_pagingInfoTable->PITSlot[j].PagesInMem<=16) break;
								 	}
								}
							}
							if(p_pagingInfoTable->PITSlot[j].PagesInMem<=16) break;
						}
					}
					if(n_freedPages>=n_pages) return(n_freedPages);
					for(k=0;k<1024;k++)
					{
						if((*((unsigned int *)p_pageDir+k))&&(p_pageDir->PageDirEntry[k].Present))
						{
							p_pageTable=(struct PageTableStruct *) (*((unsigned int *)p_pageDir + k)&0xFFFFF000);
							for(m=0;m<1024;m++)
							{
								if((*((unsigned int *)p_pageTable+m))&&(p_pageTable->PageTableEntry[m].Present))
								{
								 	if(((*((unsigned int *)p_pageTable + m)&0xFFFFF000)!=p_pagingInfoTable->PITSlot[j].VideoPage)&&((*((unsigned int *)p_pageTable + m)&0xFFFFF000)!=p_pagingInfoTable->PITSlot[j].CurrentIP&0xFFFFF000))
								 	{
								 		*((unsigned int*) p_pageTable + m)=(writeToADiskPage(*((unsigned int *)p_pageTable + m)&0xFFFFF000)&0x7FFFFFFF)<<1;
										n_freedPages++;
								 		p_pagingInfoTable->PITSlot[j].PagesInMem--;
										if(p_pagingInfoTable->PITSlot[j].PagesInMem<=16) break;
								 	}
								}
							}
							if(p_pagingInfoTable->PITSlot[j].PagesInMem<=16) break;
						}
					}
					if(n_freedPages>=n_pages) return(n_freedPages);
				}
			}
		}
	}
	p_pagingInfoTable= (struct PagingInfoTableStruct *) *(P_PagingInfoDirectory);
		for(j=16;j<128;j++)		//j=1 to j=15 - kernel paging info... j=0 refers to directly mapped page information...
		{
			if((p_pagingInfoTable->PITSlot[j].PagesInMem)>32)
			{
				p_pageDir=(struct PageDirStruct *) p_pagingInfoTable->PITSlot[j].PageDir;
				for(k=0;k<1024;k++)
				{
					if((*((unsigned int *)p_pageDir+k))&&(p_pageDir->PageDirEntry[k].Present))
					{
						p_pageTable=(struct PageTableStruct *) (*((unsigned int *)p_pageDir + k)&0xFFFFF000);
						for(m=0;m<1024;m++)
						{
							if((*((unsigned int *)p_pageTable+m))&&(p_pageTable->PageTableEntry[m].Present))
							{
							 	if(!(p_pageTable->PageTableEntry[m].Accessed))
							 	{
							 		*((unsigned int*) p_pageTable + m)=(writeToADiskPage(*((unsigned int *)p_pageTable + m)&0xFFFFF000)&0x7FFFFFFF)<<1;
									n_freedPages++;
							 		p_pagingInfoTable->PITSlot[j].PagesInMem--;
									if(p_pagingInfoTable->PITSlot[j].PagesInMem<=32) break;
							 	}
							}
						}
						if(p_pagingInfoTable->PITSlot[j].PagesInMem<=32) break;
					}
				}
				if(n_freedPages>=n_pages) return(n_freedPages);
				for(k=0;k<1024;k++)
				{
					if((*((unsigned int *)p_pageDir+k))&&(p_pageDir->PageDirEntry[k].Present))
					{
						p_pageTable=(struct PageTableStruct *) (*((unsigned int *)p_pageDir + k)&0xFFFFF000);
						for(m=0;m<1024;m++)
						{
							if((*((unsigned int *)p_pageTable+m))&&(p_pageTable->PageTableEntry[m].Present))
							{
							 	if(((*((unsigned int *)p_pageTable + m)&0xFFFFF000)!=p_pagingInfoTable->PITSlot[j].VideoPage)&&((*((unsigned int *)p_pageTable + m)&0xFFFFF000)!=p_pagingInfoTable->PITSlot[j].CurrentIP&0xFFFFF000))
							 	{
							 		*((unsigned int*) p_pageTable + m)=(writeToADiskPage(*((unsigned int *)p_pageTable + m)&0xFFFFF000)&0x7FFFFFFF)<<1;
									n_freedPages++;
							 		p_pagingInfoTable->PITSlot[j].PagesInMem--;
									if(p_pagingInfoTable->PITSlot[j].PagesInMem<=32) break;
							 	}
							}
						}
						if(p_pagingInfoTable->PITSlot[j].PagesInMem<=32) break;
					}
				}
				if(n_freedPages>=n_pages) return(n_freedPages);
			}
		}
	for(i=1023;i>=1;i--)	// i=0 refers to kernel paging info table address;
	{
		if(*(P_PagingInfoDirectory+i))
		{
			p_pagingInfoTable= (struct PagingInfoTableStruct *) *(P_PagingInfoDirectory+i);
			for(j=0;j<128;j++)
			{
				if((p_pagingInfoTable->PITSlot[j].PagesInMem)&&(p_pagingInfoTable->PITSlot[j].ProcessID!=CurFGProcessId)&&(p_pagingInfoTable->PITSlot[j].ProcessID!=CurBGProcessId))
				{
				p_pageDir=(struct PageDirStruct *) p_pagingInfoTable->PITSlot[j].PageDir;
				for(k=0;k<1024;k++)
				{
					if((*((unsigned int *)p_pageDir+k))&&(p_pageDir->PageDirEntry[k].Present))
					{
						p_pageTable=(struct PageTableStruct *) (*((unsigned int *)p_pageDir + k)&0xFFFFF000);
						for(m=0;m<1024;m++)
						{
							if((*((unsigned int *)p_pageTable+m))&&(p_pageTable->PageTableEntry[m].Present))
							{
							 	*((unsigned int*) p_pageTable + m)=(writeToADiskPage(*((unsigned int *)p_pageTable + m)&0xFFFFF000)&0x7FFFFFFF)<<1;
								n_freedPages++;
							 	p_pagingInfoTable->PITSlot[j].PagesInMem--;
								if(n_freedPages>=n_pages) return(n_freedPages);
							}
						}
						*((unsigned int *)p_pageDir+k)=(writeToADiskPage(*((unsigned int *)p_pageDir + k)&0xFFFFF000)&0x7FFFFFFF)<<1;
						n_freedPages++;
						if(n_freedPages>=n_pages) return(n_freedPages);
					}
				}
				}
			}
		}
		
	}
	return(n_freedPages);
}


unsigned int getNewMemPage()
{
	unsigned int freePage;
	unsigned int *p_aPage;
	if(N_FreeMemPages<16)
		writeMemPagesToDisk(16);
	if(N_FreeMemPages==0) return(0);
	if(N_LastGPage>0)
	{
		p_aPage=(unsigned int *) *(P_GPages+N_GPages-1);
		freePage=*(p_aPage+N_LastGPage-1);
		N_LastGPage--;
		N_FreeMemPages--;
	}
	else if(N_LastGPage==0)
	{
		if(N_GPages>0)
		{	freePage=*(P_GPages+N_GPages-1);
			N_GPages--;
			N_LastGPage=1024;
			N_FreeMemPages--;
		}
		else
		{
			freePage=(unsigned int) P_GPages;
			N_FreeMemPages--;
		}
	}
	return(freePage);
}

void freeMemPage(unsigned int pageAddress)
{
	unsigned int *p_aPage;
	if(N_FreeMemPages==0)
	{
		P_GPages=(unsigned int *) pageAddress;
		N_FreeMemPages++;
	}
	else if(N_GPages==0)
	{
		*(P_GPages+N_GPages)=pageAddress;
		N_GPages++;
		N_LastGPage=0;
	}
	else if(N_LastGPage==1024)
	{
		*(P_GPages+N_GPages)=pageAddress;
		N_GPages++;
		N_LastGPage=0;
	}
	else
	{
		p_aPage=(unsigned int *) *(P_GPages+N_GPages-1);
		*(p_aPage+N_LastGPage)=pageAddress;
		N_LastGPage++;
	}
}

unsigned int getNewDiskPage()
{
	unsigned int pageClust;
	N_FreeDiskPages--;
	N_DiskPagesList--;
	pageClust=P_DiskPagesList->PageAddress[N_DiskPagesList];
	P_DiskPagesList->PageAddress[N_DiskPagesList]=0;	
	if((!N_DiskPagesList)&&(N_FreeDiskPages))
	{
		DiskManager(READ,(pageClust&0x0FFFFFFF)+(((pageClust&0xF0000000)>>28)*8),8,-1,(unsigned int)P_DiskPagesList);
		N_DiskPagesList=1024;
	}
	return(pageClust);
}

void freeDiskPage(unsigned int pageClust)
{
	N_FreeDiskPages++;
	if(N_DiskPagesList>1023)	// >1023 means currently N_DiskPages is 1024...
	{
		DiskManager(WRITE,(pageClust&0x0FFFFFFF)+(((pageClust&0xF0000000)>>28)*8),8,-1,(unsigned int)P_DiskPagesList);
		N_DiskPagesList=0;
	}
	P_DiskPagesList->PageAddress[N_DiskPagesList]=pageClust;
	N_DiskPagesList++;
}

unsigned int InitPagingSystem(unsigned int pageClust,unsigned int n_pages,unsigned int memory)
{
	unsigned int i,j;
	unsigned int *p_aFreePage;
	unsigned int *p_aPage;
	struct PagingInfoSlotStruct *p_pagingInfoSlot;
	struct PageDirStruct *p_pageDir;
	struct PageTableStruct *p_pageTable;
	unsigned int totalPageTables;
	unsigned int n_lastPageTable;
	PhysicalMemory=memory;	//in MB
	p_aFreePage=&GData;
	if(((unsigned int)p_aFreePage)%4096) p_aFreePage=(unsigned int *) ((((unsigned int)p_aFreePage)/4096)*4096+4096);
	N_FreeMemPages=(memory*256)-((unsigned int) p_aFreePage)/4096;
	N_GPages=0;
	P_GPages=p_aFreePage;
	//printk("\nGPages : 0x%x",(unsigned int) P_GPages);
	for(i=1;i<N_FreeMemPages;)
	{
		p_aPage=p_aFreePage+(i*1024);
		*(P_GPages+N_GPages)=(unsigned int) p_aPage;
		N_GPages++;
		i++;
		N_LastGPage=0;
		for(j=0;j<1024;j++)
		{
			if(i>=N_FreeMemPages) break;
			*(p_aPage+j)=(unsigned int)(p_aFreePage+(i*1024));
			N_LastGPage++;
			i++;
		}
	}
	N_FreeDiskPages=0;
	initDiskPageList(pageClust,n_pages);
	P_PagingInfoDirectory=(unsigned int *)getNewMemPage();
	//printk("PagingInfoDirectory : 0x%x",(unsigned int) P_PagingInfoDirectory);
	fillPageWithZero(P_PagingInfoDirectory);
	P_KernelPagingInfoTable=(struct PagingInfoTableStruct *)getNewMemPage();
	//printk("Kernel PIT : 0x%x",(unsigned int) P_KernelPagingInfoTable);
	*(P_PagingInfoDirectory+0)=((unsigned int) P_KernelPagingInfoTable)+1023;	// 1023 slots will remain after we take first slot
	for(j=1;j<128;j++)		// first slot is to be used now
	{
		P_KernelPagingInfoTable->PITSlot[j].ProcessID=0;
	}
	p_pagingInfoSlot=(struct PagingInfoSlotStruct *)P_KernelPagingInfoTable;

	p_pagingInfoSlot->ProcessID=0;
	p_pagingInfoSlot->VideoPage=0;
	p_pagingInfoSlot->RequestedPages=0x1000000;
	p_pagingInfoSlot->AllocatedPages=0x1000000;
	p_pagingInfoSlot->CurrentIP=0;
	p_pagingInfoSlot->PagesInMem=0;
	p_pagingInfoSlot->PageHits=0;
	p_pagingInfoSlot->PageFaults=0;
	p_pagingInfoSlot->PageDir=0;

	totalPageTables=(memory*256)/1024;
	n_lastPageTable=(memory*256)%1024;
	if(n_lastPageTable>0) totalPageTables++;
	//printk("\nTotalPageTables : %d ,Last PT Entry : %d ",totalPageTables,n_lastPageTable);
	p_pageDir=(struct PageDirStruct *)getNewMemPage();
	fillPageWithZero((unsigned int *)p_pageDir);

	for(i=0;i<totalPageTables-1;i++)
	{
		p_pageTable=(struct PageTableStruct *)makeNewKernelPageTableEntries(i);
		makePageDirEntry((unsigned int *)p_pageDir,i,(unsigned int)p_pageTable);
	}
	if(!n_lastPageTable) n_lastPageTable=1024;
	p_pageTable=(struct PageTableStruct *)makeNewNKernelPageTableEntries(i,n_lastPageTable);
	makePageDirEntry((unsigned int *)p_pageDir,i,(unsigned int) p_pageTable);
	p_pagingInfoSlot->PageDir=(unsigned int)p_pageDir;
	CurFGProcessId=0;
	CurBGProcessId=0;
	return((unsigned int) p_pageDir);
}


unsigned int makeNewPageTableEntries(unsigned int i_pageTable)
{
	int i;
	unsigned int entry;
	unsigned int *p_pageTable;
	p_pageTable=(unsigned int *)getNewMemPage();
	fillPageWithZero(p_pageTable);
	for(i=0;i<1024;i++)
	{
		if(N_FreeMemPages>64)
		{
			entry=getNewMemPage();
			entry=entry|0x1F;
			*(p_pageTable+i)=entry;
		}
		else
		{
			entry=getNewDiskPage();
			entry=entry<<1;
			*(p_pageTable+i)=entry;	
		}
	}
	if(i_pageTable==0)
	{
		if(*(p_pageTable+256))
		{
			if((*(p_pageTable+256))&1) freeMemPage((*(p_pageTable+256))&0xFFFFF000);
			else freeDiskPage((*(p_pageTable+256))>>1);
			*(p_pageTable+256)=(256*4096)|0x1F;
		}
	}
	return((unsigned int) p_pageTable);
}

unsigned int makeNewKernelPageTableEntries(unsigned int i_pageTable)
{
	int i;
	unsigned int entry;
	unsigned int *p_pageTable;
	p_pageTable=(unsigned int *)getNewMemPage();
	fillPageWithZero(p_pageTable);
	for(i=0;i<1024;i++)
	{
		entry=(i_pageTable*0x00400000)+(i*4096);		// 0x00400000 - 4MB
		entry=entry|0x1F;
		*(p_pageTable+i)=entry;
	}
	return((unsigned int) p_pageTable);
}

unsigned int makeNewNPageTableEntries(unsigned int i_pageTable,unsigned int n_lastPageTable)
{
	int i;
	unsigned int entry;
	unsigned int *p_pageTable;
	p_pageTable=(unsigned int *)getNewMemPage();
	fillPageWithZero(p_pageTable);
	for(i=0;i<n_lastPageTable;i++)
	{
		if(N_FreeMemPages>64)
		{
			entry=getNewMemPage();
			entry=entry|0x1F;
			*(p_pageTable+i)=entry;
		}
		else
		{
			entry=getNewDiskPage();
			entry=entry<<1;
			*(p_pageTable+i)=entry;	
		}
	}
	if(i_pageTable==0)
	{
		if(*(p_pageTable+256))
		{
			if((*(p_pageTable+256))&1) freeMemPage((*(p_pageTable+256))&0xFFFFF000);
			else freeDiskPage((*(p_pageTable+256))>>1);
			*(p_pageTable+256)=(256*4096)|0x1F;
		}
	}
	return((unsigned int) p_pageTable);
}

unsigned int makeNewNKernelPageTableEntries(unsigned int i_pageTable,unsigned int n_lastPageTable)
{
	int i;
	unsigned int entry;
	unsigned int *p_pageTable;
	p_pageTable=(unsigned int *)getNewMemPage();
	fillPageWithZero(p_pageTable);
	for(i=0;i<n_lastPageTable;i++)
	{
		entry=(i_pageTable*0x00400000)+(i*4096);
		entry=entry|0x1F;
		*(p_pageTable+i)=entry;
	}
	return((unsigned int) p_pageTable);
}

void makePageDirEntry(unsigned int *p_pageDir,unsigned int i,unsigned int pageTable)
{
	if(N_FreeMemPages<64)
	{
		pageTable=writeToADiskPage(pageTable)<<1;
		*(p_pageDir+i)=pageTable;
	}
	else
	{
		*(p_pageDir+i)=pageTable|0x1F;
	}
}

void fillPageWithZero(unsigned int *p_page)
{
	int i;
	for(i=0;i<1024;i++)
	{
		*(p_page+i)=0;
	}
}

struct PagingInfoSlotStruct * getAFreePagingInfoSlot()
{
	int i,j;
	struct PagingInfoTableStruct *p_pagingInfoTable;
	for(i=1;i<1024;i++)	// i=0 refers to KernelPageTable
	{
		if((*(P_PagingInfoDirectory+i))&&((*(P_PagingInfoDirectory+i))&0x00000FFF))
		{
			*(P_PagingInfoDirectory+i)=*(P_PagingInfoDirectory+i)-1;
			p_pagingInfoTable=(struct PagingInfoTableStruct *)(*(P_PagingInfoDirectory+i)&0xFFFFF000);
			for(j=0;j<128;j++)
			{
				if(!(p_pagingInfoTable->PITSlot[j].ProcessID))
				{
					return((struct PagingInfoSlotStruct *)p_pagingInfoTable + j);
				}
			}
		}
	}
	for(i=1;i<1024;i++)
	{
		if(!(*(P_PagingInfoDirectory+i)))
		{
			p_pagingInfoTable=(struct PagingInfoTableStruct *)getNewMemPage();
			*(P_PagingInfoDirectory+i)=((unsigned int) p_pagingInfoTable)+127;	// 127 slots will remain after we take first slot
			for(j=1;j<128;j++)		// first slot is to be used now
			{
				p_pagingInfoTable->PITSlot[j].ProcessID=0;
			}
			return((struct PagingInfoSlotStruct *)p_pagingInfoTable);
		}
	}
}

unsigned int CreateVM(unsigned int processID,unsigned int n_reqtdPages,int privilege)
{
	int i;
	unsigned int totalReqdPages;
	unsigned int totalPageTables;
	unsigned int n_lastPageTable;
	struct PagingInfoSlotStruct *p_pagingInfoSlot;
	struct PageDirStruct *p_pageDir;
	struct PageTableStruct *p_pageTable;
	//printk("\nIn Create VM");
	if(n_reqtdPages>0xFFFFF+1) { return(INVALIDPAGELIMIT); }
	totalPageTables=n_reqtdPages/1024;
	n_lastPageTable=n_reqtdPages%1024;
	if(n_lastPageTable>0) totalPageTables++;
	totalReqdPages=n_reqtdPages+totalPageTables+1;	// for page tables and page directory
	totalReqdPages=totalReqdPages+10;	//+10 for TSS, File Management, etc... Only 255 files can be opened for a task.
	if((N_FreeMemPages+N_FreeDiskPages)<(totalReqdPages+10)) // +10 for safety purpose
	{
		return(LESSMEMORY);
	}
	if(N_FreeMemPages<128)
		writeMemPagesToDisk(128);		// Ensure atleast 128 memory pages are free for creating VM
	if(privilege==0)
	{
		p_pagingInfoSlot=0;
		for(i=16;i<128;i++)	// i=0 is global page information
		{
			if(P_KernelPagingInfoTable->PITSlot[i].ProcessID==0)
			p_pagingInfoSlot=(struct PagingInfoSlotStruct *)P_KernelPagingInfoTable + i;
		}
		if(!p_pagingInfoSlot) return(NOPITSLOT);
	}
	else
	{
		p_pagingInfoSlot=getAFreePagingInfoSlot();
	}
	
	p_pagingInfoSlot->ProcessID=processID;
	p_pagingInfoSlot->VideoPage=0x000B8000;
	p_pagingInfoSlot->RequestedPages=n_reqtdPages;
	p_pagingInfoSlot->AllocatedPages=totalReqdPages;
	p_pagingInfoSlot->CurrentIP=0;
	p_pagingInfoSlot->PagesInMem=0;
	p_pagingInfoSlot->PageHits=0;
	p_pagingInfoSlot->PageFaults=0;
	p_pagingInfoSlot->PageDir=0;

	p_pageDir=(struct PageDirStruct *)getNewMemPage();
	fillPageWithZero((unsigned int *)p_pageDir);

	for(i=0;i<totalPageTables-1;i++)
	{
		p_pageTable=(struct PageTableStruct *)makeNewPageTableEntries(i);
		makePageDirEntry((unsigned int *)p_pageDir,i,(unsigned int)p_pageTable);
	}
	if(!n_lastPageTable) n_lastPageTable=1024;
	p_pageTable=(struct PageTableStruct *)makeNewNPageTableEntries(i,n_lastPageTable);
	makePageDirEntry((unsigned int *)p_pageDir,i,(unsigned int) p_pageTable);
	p_pagingInfoSlot->PageDir=(unsigned int) p_pageDir;
	return((unsigned int) p_pageDir);
}

unsigned int CreateKernelVM(unsigned int processId)
{
	int i;
	unsigned int totalPageTables,n_lastPageTable;
	struct PagingInfoSlotStruct *p_pagingInfoSlot;
	struct PageDirStruct *p_pageDir;
	struct PageTableStruct *p_pageTable;
	p_pagingInfoSlot=0;
		for(i=1;i<16;i++)	// i=0 is global page information
		{
			if(P_KernelPagingInfoTable->PITSlot[i].ProcessID==0)
			{
				p_pagingInfoSlot=(struct PagingInfoSlotStruct *)P_KernelPagingInfoTable + i;
				break;
			}
		}
	if(i==16) return(0);

	p_pagingInfoSlot->ProcessID=processId;
	p_pagingInfoSlot->VideoPage=0x000B8000;
	p_pagingInfoSlot->RequestedPages=0x1000000;
	p_pagingInfoSlot->AllocatedPages=0x1000000;
	p_pagingInfoSlot->CurrentIP=0;
	p_pagingInfoSlot->PagesInMem=0;
	p_pagingInfoSlot->PageHits=0;
	p_pagingInfoSlot->PageFaults=0;
	p_pagingInfoSlot->PageDir=0;

	totalPageTables=(PhysicalMemory*256)/1024;
	n_lastPageTable=(PhysicalMemory*256)%1024;
	if(n_lastPageTable>0) totalPageTables++;

	p_pageDir=(struct PageDirStruct *)getNewMemPage();
	fillPageWithZero((unsigned int *)p_pageDir);

	for(i=0;i<totalPageTables-1;i++)
	{
		p_pageTable=(struct PageTableStruct *)makeNewKernelPageTableEntries(i);
		makePageDirEntry((unsigned int *)p_pageDir,i,(unsigned int)p_pageTable);
	}
	if(!n_lastPageTable) n_lastPageTable=1024;
	p_pageTable=(struct PageTableStruct *)makeNewNKernelPageTableEntries(i,n_lastPageTable);
	makePageDirEntry((unsigned int *)p_pageDir,i,(unsigned int) p_pageTable);
	p_pagingInfoSlot->PageDir=(unsigned int)p_pageDir;
	return((unsigned int) p_pageDir);
}

unsigned int getPagingInfoSlot(unsigned int processId)
{
	int i,j;
	struct PagingInfoTableStruct *p_pagingInfoTable;
	if(processId<128)
	{
		for(i=1;i<128;i++)
		{
			if(P_KernelPagingInfoTable->PITSlot[i].ProcessID==processId)
			{
				return( (unsigned int) (((struct PagingInfoSlotStruct *)P_KernelPagingInfoTable) + i) );
			}
		}
		if(i==128) { return(0); }
	}
	else
	{
		for(i=1;i<1024;i++)
		{
			if(*(P_PagingInfoDirectory+i))
			{
				p_pagingInfoTable= (struct PagingInfoTableStruct *) *(P_PagingInfoDirectory+i);
				for(j=0;j<128;j++)
				{
					if(p_pagingInfoTable->PITSlot[j].ProcessID==processId)
					{
						return( (unsigned int) (((struct PagingInfoSlotStruct *) p_pagingInfoTable) + j) );
					}
				}
			}
		}
	}
	//printk("\nGPIT: Process not found");
	return(0);
}

unsigned int getPage(unsigned int processId,unsigned int page)
{
	struct PagingInfoSlotStruct *p_pagingInfoSlot;
	unsigned int pageClust;
	unsigned int *p_aPage;
	struct PageDirStruct *p_pageDir;
	struct PageTableStruct *p_pageTable;
	p_pagingInfoSlot=(struct PagingInfoSlotStruct *) getPagingInfoSlot(processId);
	if((unsigned int)p_pagingInfoSlot==0) return(1);	// 1-toprocess not found
	if(page>p_pagingInfoSlot->RequestedPages) return(3);	// 3-reference to an unavailable page...
	p_pageDir=(struct PageDirStruct *)p_pagingInfoSlot->PageDir;
	p_pageTable=(struct PageTableStruct *) *(((unsigned int *)p_pageDir) + (page>>10));
	if((unsigned int)p_pageTable)
	{
		if(((unsigned int) p_pageTable)&1) p_pageTable=(struct PageTableStruct *) (((unsigned int) p_pageTable) & 0xFFFFF000);
		else
		{
			pageClust=((unsigned int) p_pageTable)>>1;
			p_pageTable=(struct PageTableStruct *)getNewMemPage();
			DiskManager(READ,(pageClust&0x0FFFFFFF)+(pageClust>>28)*8,8,-1,(unsigned int) p_pageTable);
			freeDiskPage(pageClust);
			*(((unsigned int *)p_pageDir) + (page>>10))=(((unsigned int)p_pageTable)|0x0000001F);
			//printk("\nPageTable Updated");
		}
	}
	else
	{
		//printk("\nPageTable not found");
		return(4);
	}
	p_aPage=(unsigned int *) *(((unsigned int *)p_pageTable)+(page&0x000003FF));
	if((unsigned int)p_aPage)
	{
		if(((unsigned int) p_aPage)&1) p_aPage=(unsigned int *) (((unsigned int) p_aPage) & 0xFFFFF000);
		else
		{
			pageClust=((unsigned int) p_aPage)>>1;
			p_aPage=(unsigned int *) getNewMemPage();
			DiskManager(READ,(pageClust&0x0FFFFFFF)+(pageClust>>28)*8,8,-1,(unsigned int) p_aPage);
			freeDiskPage(pageClust);
			//printk("\nPageClust : 0x%x",pageClust);
			*(((unsigned int *)p_pageTable)+(page&0x000003FF))=(((unsigned int)p_aPage)|0x0000001F);
			//printk("\nPage Updated");
		}
	}
	else
	{
		//printk("\nPage 0x%x of process 0x%x not found",page,processId);
		return(5);
	}
	return((unsigned int)p_aPage);
}

unsigned int CopyPage(unsigned int toProcessId,unsigned int toProcessPage,unsigned int fromProcessId,unsigned int fromProcessPage)
{
	int i;
	unsigned int *p_toPage,*p_fromPage;
	p_toPage=(unsigned int *)getPage(toProcessId,toProcessPage);
	if(((unsigned int)p_toPage)%4096) return((unsigned int) p_toPage);
	p_fromPage=(unsigned int *)getPage(fromProcessId,fromProcessPage);
	if(((unsigned int)p_fromPage)%4096) return(((unsigned int) p_fromPage)+10);
	for(i=0;i<1024;i++)
		*(p_toPage+i)=*(p_fromPage+i);
	//printk("\nTo Page : 0x%x , From Page : 0x%x",(unsigned int)p_toPage,(unsigned int)p_fromPage);
	return(0);
}

unsigned int CopyToMyVM(unsigned int myProcessId,unsigned int myPage,unsigned int secProcessId,unsigned int secProcessPage)
{
	return(CopyPage(myProcessId,myPage,secProcessId,secProcessPage));
}

unsigned int CopyFromMyVM(unsigned int myProcessId,unsigned int myPage,unsigned int secProcessId,unsigned int secProcessPage)
{
	return(CopyPage(secProcessId,secProcessPage,myProcessId,myPage));	
}

int SetMyVideoPage(unsigned int processId,unsigned int page)
{
	struct PagingInfoSlotStruct *p_pagingInfoSlot;
	unsigned int pageClust;
	unsigned int *p_aPage;
	struct PageDirStruct *p_pageDir;
	struct PageTableStruct *p_pageTable;
	p_pagingInfoSlot=(struct PagingInfoSlotStruct *) getPagingInfoSlot(processId);
	if((unsigned int)p_pagingInfoSlot==0) return(1);	// 1-toprocess not found
	if(page>p_pagingInfoSlot->RequestedPages) return(3);	// 3-reference to an unavailable page...
	p_pageDir=(struct PageDirStruct *)p_pagingInfoSlot->PageDir;
	p_pageTable=(struct PageTableStruct *) *(((unsigned int *)p_pageDir) + (page>>10));
	if((unsigned int)p_pageTable)
	{
		if(((unsigned int) p_pageTable)&1) p_pageTable=(struct PageTableStruct *) (((unsigned int) p_pageTable) & 0xFFFFF000);
		else
		{
			pageClust=((unsigned int) p_pageTable)>>1;
			p_pageTable=(struct PageTableStruct *)getNewMemPage();
			DiskManager(READ,(pageClust&0x0FFFFFFF)+(pageClust>>28)*8,8,-1,(unsigned int) p_pageTable);
			freeDiskPage(pageClust);
			*(((unsigned int *)p_pageDir) + (page>>10))=(((unsigned int)p_pageTable)|0x0000001F);
			//printk("\nPageTable Updated");
		}
	}
	else
	{
		//printk("\nPageTable not found");
		return(4);
	}
	p_aPage=(unsigned int *) *(((unsigned int *)p_pageTable)+(page&0x000003FF));
	if((unsigned int)p_aPage)
	{
		if(((unsigned int) p_aPage)&1) 
		{
			p_aPage=(unsigned int *) (((unsigned int) p_aPage) & 0xFFFFF000);
			freeMemPage((unsigned int)p_aPage);
		}
		else
		{
			pageClust=((unsigned int) p_aPage)>>1;
			freeDiskPage(pageClust);
			//printk("\nPage Updated");
		}
		*(((unsigned int *)p_pageTable)+(page&0x000003FF))=(((unsigned int)0x000B8000)|0x0000001F);
	}
	else
	{
		//printk("\nPage 0x%x of process 0x%x not found",page,processId);
		return(5);
	}
	return(0);
}

int PSService(unsigned int code,unsigned int param1,unsigned int param2,unsigned int param3,unsigned int param4,unsigned int processId)
{
	//printk("\n In Paging");
	if(code==2) return(CreateKernelVM(param1));
	else if(code==3) return(CreateVM(param1,param2,param3));
	else if(code==4) return(setCurFGVM(param1,param2));
	else if(code==5) return(setCurBGVM(param1,param2));
	else if(code==6) return(CloseVM(param1));
	else if(code==7) return(SetMyVideoPage(processId,param1));
	else if(code==8) return(CopyToMyVM(processId,param1,param2,param3));
	else if(code==9) return(CopyFromMyVM(processId,param1,param2,param3));
	else if(code==10) return(N_FreeMemPages + N_FreeDiskPages);
	else if(code==11) return(N_FreeMemPages);
	else if(code==12) return(N_FreeDiskPages);
	else return(100);
}	

extern void hangon();

void DemandPaging(unsigned int memAddress,unsigned int errorCode)
{
	int i,j,t;
	unsigned int processId;
	unsigned int currentIp;
	unsigned int pageClust;
	unsigned int *p_aPage;
	struct PagingInfoSlotStruct *p_pagingInfoSlot;
	struct PageDirStruct *p_pageDir;
	struct PageTableStruct *p_pageTable;
	if(errorCode&1) { printk("\nUnknown page fault : 0x%x",errorCode); hangon(); }
					// first 10 gdt entries are not for TSS...
	struct TSSStruct *tss=(struct TSSStruct *)(0x00100000+128);
	unsigned short myTaskId=2,prevTaskId;
	prevTaskId=(tss+myTaskId)->Back_Link/8-5;
	if(prevTaskId>15) { printk("\nUnknown Task 0x%x has demanded for Page 0x%x",prevTaskId,(memAddress>>12)); hangon(); }
	processId=(tss+prevTaskId)->ProcessId;
	if(processId<16) { printk("\nDirectly mapped Kernel Process 0x%x has demanded for Page 0x%x",processId,(memAddress>>12)); hangon(); }
	currentIp=(tss+prevTaskId)->Eip;
	p_pagingInfoSlot=(struct PagingInfoSlotStruct *) getPagingInfoSlot(processId);
	if(!((unsigned int) p_pagingInfoSlot)) { printk("\nThe process 0x%x has no VM",processId); hangon(); }
	p_pagingInfoSlot->CurrentIP=currentIp;
	if((processId>128)&&(memAddress>=(p_pagingInfoSlot->RequestedPages*4096)))
	{
		// terminate process;
		printk("\nThe process 0x%x has referenced memory that it has not requested",processId);
		hangon();
	}
	p_pageDir=(struct PageDirStruct *) p_pagingInfoSlot->PageDir;
	p_pageTable=(struct PageTableStruct *) *(((unsigned int *)p_pageDir)+(memAddress>>22));
	if((unsigned int)p_pageTable)
	{
		if(((unsigned int) p_pageTable)&1) p_pageTable=(struct PageTableStruct *) (((unsigned int) p_pageTable) & 0xFFFFF000);
		else
		{
			pageClust=((unsigned int) p_pageTable)>>1;
			p_pageTable=(struct PageTableStruct *)getNewMemPage();
			DiskManager(READ,(pageClust&0x0FFFFFFF)+(pageClust>>28)*8,8,-1,(unsigned int) p_pageTable);
			freeDiskPage(pageClust);
			*(((unsigned int *) p_pageDir)+(memAddress>>22))=((unsigned int) p_pageTable)|0x0000001F;
		}
	}
	else
	{
		p_pageTable=(struct PageTableStruct *) getNewMemPage();
		fillPageWithZero((unsigned int *) p_pageTable);
		*(((unsigned int *) p_pageDir)+(memAddress>>22))=((unsigned int) p_pageTable)|0x0000001F;
	}
	p_aPage=(unsigned int *) *(((unsigned int *)p_pageTable)+((memAddress>>12)&0x000003FF));
	if((unsigned int)p_aPage)
	{
		if(((unsigned int)p_aPage)&1) { printk("\nPresent Page 0x%x is demanded by Process 0x%x",((unsigned int)p_aPage)>>12,processId); hangon(); }
		pageClust=((unsigned int) p_aPage)>>1;
		p_aPage=(unsigned int *) getNewMemPage();
		DiskManager(READ,(pageClust&0x0FFFFFFF)+(pageClust>>28)*8,8,-1,(unsigned int) p_aPage);
		freeDiskPage(pageClust);
	}
	else
		p_aPage=(unsigned int *) getNewMemPage();
		fillPageWithZero((unsigned int *) p_aPage);

	*(((unsigned int *)p_pageTable)+((memAddress>>12)&0x000003FF))=(((unsigned int)p_aPage)|0x0000001F);
	printk("\nMemDemand : 0x%x",memAddress);
}
