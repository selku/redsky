# define	PSCREATEVM	3
# define	PSSETCFGVM	4
# define	PSSETCBGVM	5
# define	PSCLOSEVM	6
# define	PSSETMYVIDPG	7
# define	PSCPYTOMYVM	8
# define	PSCPYFRMMYVM	9
# define	PSFREEPAGES	10
# define	PSFREEMEMPGS	11
# define	PSFREEDISKPGS	12

struct ProcessStruct
{
	struct ProcessStruct *NextProcess;
	dword	Esp0;
	dword	Esp1;
	dword	Esp2;
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
	dword	ProcessId;
	byte	datasegsmask;
	byte	PrevWait;
	byte	KeyStart;
	byte	KeyEnd;
	word	Key[30];
};

static unsigned int NextProcessId;
static unsigned int EFlags;

static struct ProcessStruct *NewProcess;

static struct ProcessStruct *ProcessList;
static struct ProcessStruct *LastProcessEntry;
static unsigned int NoOfProcesses;

static unsigned int FGProcessId;
static unsigned int BGProcessId;

static unsigned char cprocess;

initTM()
{
	NextProcessId=200;
	EFlags=GetEFlags();
	NewProcess=0;
	ProcessList=0;
	LastProcessEntry=0;
	FGProcessId=0;
	BGProcessId=0;
	cprocess=0;
	return(0);
}

int TMService(unsigned int code)
{
	if(code==100)	// the currently executing process ends...
	{
	}
	else if(code==200)	// the currently executing process waits for GetCh()...
	{
	}
	else if(code==500)	// time for schedule...it checks sysinfoslot->SCM and brings SCM to FG...
	{
	}
	else if(code==2)	// create new process...
	{
	}
	else if(code==3)	// add the new process to queue...
	{
	}
	else if(code==4)	// delete a process...
	{
	}
	else if(code==5)	// bring a process to foreground...
	{
	}
	else if(code==10)	// register SCM process...
}

static unsigned int SCMProcessId;

int RegisterSCMProcess(unsigned int ProcessId);
{
	struct ProcessStruct *Process;
	Process=getProcessAddress(ProcessId);
	if((unsigned int)Process==0) return(2);	// process not created...
	SCMProcessId=ProcessId;
	return(0);
}

int WaitForGetCh()
{
	unsigned int temp;
	struct ProcessStruct *Process;
	if(cprocess==2)		// FGProcess running currently...
	{
		Process=getProcessAddress(FGProcessId);
		Process->PrevWait=Process->PrevWait | 0x01;
		Process=getActiveProcess();
		if(Process)
		{
			CopyProcessToTask(Process,bgtss);
					// copy keys...
			return(LoadBGTask());
		}
				// check whether to update state ?



		Process=getProcessAddress(BGProcessId);
		if(Process)
		{
			if(!(Process->PrevWait&0x01))
				return(LoadBGTask());
		}
		while(!fgkeyAvailable()) { temp=temp+2; }
		Process=getProcessAddress(FGProcessId);
		Process->PrevWait=Process->PrevWait & 0xFE;
		return(LoadFGTask());
	}
	else
	{
		Process=getProcessAddress(FGProcessId);
		if(!(Process->PrevWait&0x01)) return(LoadFGTask());
		while(!fgkeyAvailable()) { temp=temp+2; }
		Process=getProcessAddress(FGProcessId);
		Process->PrevWait=Process->PrevWait & 0xFE;
		return(LoadFGTask());
	}
}

int CopyTaskToProcess(struct TSSStruct *tss,struct ProcessStruct *Process)
{
	unsigned char segstate;
	segstate=0;
	Process->Eax=tss->Eax;
	Process->Ebx=tss->Ebx;
	Process->Ecx=tss->Ecx;
	Process->Edx=tss->Edx;
	Process->Esi=tss->Esi
	Process->Edi=tss->Edi;
	Process->Ebp=tss->Ebp;
	Process->Esp=tss->Esp;
	Process->Esp0=tss->Esp0;
	Process->Esp1=tss->Esp1;
	Process->Esp2=tss->Esp2;
	Process->Eip=tss->Eip;
	Process->Eflags=tss->Eflags;
	if(tss->Ds) segstate=segstate | 0x1;
	if(tss->Es) segstate=segstate | 0x2;
	if(tss->Fs) segstate=segstate | 0x4;
	if(tss->Gs) segstate=segstate | 0x8;
	Process->datasegsmask=segstate;
	return(0);
}

int CopyProcessToTask(struct ProcessStruct *Process,struct TSSStruct *tss)
{
	unsigned char segstate;
	tss->Eax=Process->Eax;
	tss->Ebx=Process->Ebx;
	tss->Ecx=Process->Ecx;
	tss->Edx=Process->Edx;
	tss->Esi=Process->Esi;
	tss->Edi=Process->Edi;
	tss->Ebp=Process->Ebp;
	tss->Esp=Process->Esp;
	tss->Esp0=Process->Esp0;
	tss->Esp1=Process->Esp1;
	tss->Esp2=Process->Esp2;
	tss->Eip=Process->Eip;
	tss->Eflags=Process->Eflags;
	tss->Cs=1*8;
	tss->Ss=3*8;
	tss->Ldt=4*8;
	segstate=Process->datasegsmask;
	if(segstate&0x1) tss->Ds=2*8;
	if(segstate&0x2) tss->Es=2*8;
	if(segstate&0x4) tss->Fs=2*8;
	if(segstate&0x8) tss->Gs=2*8;
	tss->ProcessId=Process->ProcessId;
	return(0);
}

struct ProcessStruct * getActiveProcess()
{
}

int LoadFGTask()
{
	// clear busy flag of TSS of FG...
	return(20);
}

int LoadBGTask()
{
	// clear busy flag of TSS of BG...
	return(30);
}

int Schedule()
{
	struct ProcessStruct *Process;
	struct TSSStruct *tss,*fgtss,*bgtss;
	tss=(struct TSSStruct *) (0x00100000+128);
	fgtss=tss;
	bgtss=tss+1;
	if(!cprocess) return(LoadFGTask());
	else if(cprocess==2)		// FGProcess running currently...
	{
		if(BGProcessId)
		{
			Process=getProcessAddress(BGProcessId);
			CopyTaskToProcess(bgtss,Process);
					// copy keys...
		}
		Process=getActiveProcess();		// getActive Process Id and store it ...
		if((unsigned int)Process==0) 
		{
			if(BGProcessId) 
			{
				if(getProcessAddress(BGProcessId)) return(LoadBGTask()); // check whether BGProcess is not waiting for key ?
			}
			else
			{
				return(LoadFGTask());
			}
		}
		CopyProcessToTask(Process,bgtss);
				// copy keys...
		return(LoadBGTask());
	}
	else			// BGProcess running currently...
	{
		Process=getProcessAddress(FGProcessId);
		if((Fts->SCM)||(Process==0))
		{
			if((unsigned int)Process)
			{
				CopyTaskToProcess(fgtss,Process);
						// copy keys...
			}
			PutSCMToFG();
			FGProcessId=SCMProcessId;
			return(LoadFGTask());
		}
		if(Process->PrevWait&0x1)	// waiting for key
		{
			if(fgKeyAvailable())
				return(LoadFGTask());	// check whether state should be altered ?
			else
			{
				Process=getActiveProcess();
				if(Process)
				{
					CopyProcessToTask(Process,bgtss);
							// copy keys...
					return(LoadBGTask());
				}
				else
					return(LoadFGTask());
			}
		}
		return(LoadFGTask());
	}
}

int EndMe()
{
	// close the current process...
	struct ProcessStruct *Process;
	struct TSSStruct *tss,*bgtss;
	tss=(struct TSSStruct *) (0x00100000+128);
	bgtss=tss+1;
	if(!cprocess)
	{
		Fts->SchStd=1;
		cprocess=2;	// FGProcess
		if(PutSCMToFG()) { printk("\nSCM not created or registered..."); hangon(); }
		FGProcessId=SCMProcessId;
		return(LoadFGTask());
	}
	else if(cprocess==2)		// FGProcess
	{
		FM(FMCLOSEPF,FGProcessId,0,0,0);
		PS(PSCLOSEVM,FGProcessId,0,0,0);
		RemoveProcessEntry(FGProcessId);
		PutSCMToFG();
		FGProcessId=SCMProcessId;
		return(LoadFGTask());
	}
	else
	{
		FM(FMCLOSEPF,BGProcessId,0,0,0);
		PS(PSCLOSEVM,BGProcessId,0,0,0);
		RemoveProcessEntry(FGProcessId);
		Process=getActiveProcess();
		if((unsigned int)Process==0)
		{
			PutSCMToFG();
			cprocess=2;
			FGProcessId=SCMProcessId;
			return(20);
		}
		CopyProcessToTask(Process,bgtss);
				// copy keys...
		return(LoadBGTask());
}

int PutSCMToFG()
{
	struct ProcessStruct *Process,*FGProcess;
	struct TSSStruct *tss,*fgtss;
	tss=(struct TSSStruct *) (0x00100000+128);
	fgtss=tss;
	if(!SCMProcessId)	// process not created or registered...
	{
		printk("\nSCM Process not registered...");
		return(1);
	}
	Process=getProcessAddress(SCMProcessId);
	if(FGProcessId)
	{
		FGProcess=getProcessAddress(FGProcessId);
		CopyTaskToProcess(fgtss,FGProcess);
		// copy keys...
	}
	CopyProcessToTask(Process,fgtss);
			// copy keys...
	return(0);
}	

int CreateNewProcess(unsigned int NoOfPages,unsigned int prev)
{
	struct ProcessStruct *Process;
	unsigned int pdbr;
	if((unsigned int)NewProcess) return(4);	// Already a new process is waiting...
	pdbr=PS(PSCREATEVM,NextProcessId,NoOfPages,prev,0);
	if(pdbr%4096) { printk("\nTM : Error in creating process : 0x%x",NextProcessId); return(1); }
	Process=getNewProcessSlot();
	if((unsigned int)Process==0) return(2);	// cannot create process. no memory.
	Process->Eax=0;
	Process->Ebx=0;
	Process->Ecx=0;
	Process->Edx=0;
	Process->Esi=0;
	Process->Edi=0;
	Process->Ebp=256*4096;
	Process->Esp=256*4096;
	Process->Esp0=256*4096;
	Process->Esp1=256*4096;
	Process->Esp2=256*4096;
	Process->Eip=0;
	Process->Cr3=pdbr;
	Process->Eflags=EFlags;
	Process->datasegsmask=0xF;	//ds,es,fs,gs - all are loaded...
	Process->ProcessId=NextProcessId;
	NextProcessId++;
	Process->PrevWait=((prev&0x1)<<1);	// bit1=0 - wait for schedule, bit1=1 - wait for key.
	Process->KeyStart=0;
	Process->KeyEnd=0;
	NewProcess=Process;
	NoOfProcess++;
	return(Process->ProcessId);
}

int AddNewProcessToQueue()
{
	if(!(unsigned int)NewProcess) return(2);	// no new process created...
	if(!ProcessList) ProcessList=NewProcess;
	if(LastProcessEntry) LastProcessEntry->NextProcess=NewProcess;
	LastProcessEntry=NewProcess;
	NewProcess->NextProcess=0;
	NewProcess=0;
}

int TimeService();
int KeyService();
	int GetKey();
	int GetCh();
