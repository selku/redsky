extern unsigned int GData;
static char systemdrive;
static char *wd;	// working directory...
static char *cmd;

# define	ESC	27
# define	BSP	8

struct freepage
{
	int numofpages;
	struct freepage *next;
	int data[1022];
};

static struct freepage *fdatapage;
static unsigned int *nextaddr;

void initMem()
{
	struct freepage *ndatapage;
	nextaddr=&GData;
	if( ((unsigned int)nextaddr) %4096 ) nextaddr=(unsigned int *)( ( (((unsigned int)nextaddr) /4096 )*4096)+4096);
	fdatapage=(struct freepage *) nextaddr;
	fdatapage->numofpages=(0x000B8000-(unsigned int)fdatapage)/4096;
	ndatapage=(struct freepage *) 0x000B8000;
	ndatapage=ndatapage+1;
	fdatapage->next=ndatapage;
	ndatapage->numofpages=(0xFFFFF000-(unsigned int)ndatapage)/4096;
	ndatapage->next=(struct freepage*) 0;
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

initVid()
{
	PS(7,0x000B8);
}

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
	//printk("%c , %d",ch,ch);
	return(ch);
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

char * gets(char *str)
{
	char ch=0;
	int i=0,offset;
	while(ch!='\n')
	{
		ch=getChar();
		if(validChar(ch)||ch==' '||ch=='.'||ch=='/'||ch=='\\'||ch==','||ch=='?'||ch==':'||ch==';')
		{
			printk("%c",ch);
			str[i]=ch; i++;
		}
		if(ch==BSP)
		{
			if(i!=0)
			{
				out(0x3D4, 14);
  				offset = in(0x3D5) << 8;
  				out(0x3D4, 15);
  				offset |= in(0x3D5);
				offset=offset-1;
 				out(0x3D5, (unsigned char)(offset));
 				out(0x3D4, 14);
  				out(0x3D5, (unsigned char)(offset >> 8));
				printk("%c",' ');
				out(0x3D4, 14);
  				offset = in(0x3D5) << 8;
  				out(0x3D4, 15);
  				offset |= in(0x3D5);
				offset=offset-1;
 				out(0x3D5, (unsigned char)(offset));
 				out(0x3D4, 14);
  				out(0x3D5, (unsigned char)(offset >> 8));
				i--;
				str[i]=0;
			}
		}
	}
	str[i]=0;
	return(str);
}

static char *cmdbuf;

getCmdParam(char *cmd,char **param)
{
	int i,j;
	*cmd=0;
	*(cmdbuf+4095)=0;
	//printk("\nCmdbuf : %s\n\n",cmdbuf);
	for(i=0;i<4095;i++)
	{
		//printk("%c",*(cmdbuf+i));
		if(*(cmdbuf+i)==0) { *param=cmdbuf+i; return; }
		if(*(cmdbuf+i)!=' ') break;
	}
	//printk("break\n");
	for(j=0;i<4095;i++,j++)
	{
		//printk("%c",*(cmdbuf+i));
		*(cmd+j)=*(cmdbuf+i);
		if((*(cmdbuf+i)==0)||(*(cmdbuf+i)==' ')) { *(cmd+j)=0;  break; }
	}
	//printk("break\n");
	for(;i<4095;i++)
	{
		//printk("%c",*(cmdbuf+i));
		if((*(cmdbuf+i)==0)||(*(cmdbuf+i)!=' ')) break;
	}
	//printk("break\n");
	*param=cmdbuf+i;
	return;
}

int stridcmp(const char * cs,const char * cd)
{
	int i,r;char ch;
	for(i=0;i<1000;i++)
	{
		ch=*(cd+i);
		if((ch>='a')&&(ch<='z')) ch=ch+'A'-'a';
		if(r=*(cs+i)-ch) break;
		if(*(cs+i)==0) return(0);
	}
	return(r);
}



formalisePath(char *Path,char *NewPath)
{
	int i,j,k,l,x;
	
	   // totally there can be 372*11 byte entries = 4092 bytes
	
	for(i=0,k=0;i<4092;i++)
	{
		if(k==4092) return -1;
		for(x=0;x<12;x++)
		{
			*(NewPath+k+x)=' ';
		}
		*(NewPath+k+x-1)=0;
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

int strlen(char *str)
{
	int i=0;
	while(*(str+i)) i++;
	return(i);
}

extern inline char * strcpy(char * dest,const char *src)
{
	int i=-1;
	do
	{
		i++;
		*(dest+i)=*(src+i);
	}
	while(*(src+i));
	return(dest);
}

extern inline int striscpy(char * dest,const char *src)
{
	int i=-1;char ch;
	do
	{
		i++;
		ch=*(src+i);
		if((ch>='a')&&(ch<='z')) ch=ch+'A'-'a';
		if(ch=='\\') ch='/';
		*(dest+i)=ch;
	}
	while(*(src+i));
	return(i);
}

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

int kmain()
{
	int i;
	char cmd[5],*param,*filepath,*pathdisp;
	static char fname[23]="USER       ";
	char filename[13],ch;
	struct fatdirstruct *dir;
	unsigned int live,time,date,fid,pdl;
	int k,n,files,dirs,h,j,t,l;
	unsigned char day,month,second,minute,hour,rootdir;
	unsigned short year;
	unsigned short dirhandle,filehandle;
	live=1;
	rootdir=0;
	initMem();
	initVid();
	clrscr();
	printk("System Control Module. Ver. 1.0");
	printk("\nWelcome To REDSKY...");
	fid=FS(3,(unsigned int)fname,0,0,0,0);
	//printk("\nFile System returned : %d",fid);
	if(fid<5)
		fid=FS(5,(unsigned int)fname,0,0,0,0);
	cmdbuf=(char *)getnewpage();
	filepath=getnewpage();
	pathdisp=getnewpage();
	dir=(struct fatdirstruct *)getnewpage();
	strcpy(pathdisp,"C:/USER");
	pdl=7;
	while(live)
	{
		printk("\n%s>",pathdisp);
		gets(cmdbuf);
		getCmdParam(cmd,&param);
		if(*cmd==0) continue;
		if(!stridcmp("CLS",cmd)) clrscr();
		else if(!stridcmp("DATE",cmd))
		{
			date=getDate();
			year=*(((short *)&date)+1)+1980;
			month=*(((char *)&date)+1);
			day=*((char *)&date);
			printk("\nDate : %d/%d/%d",day,month,year);
		}
		else if(!stridcmp("TIME",cmd))
		{
			time=getTime();
			hour=*(((char *)&time)+3);
			minute=*(((char *)&time)+2);
			second=*(((char *)&time)+1);
			printk("\nTime : %d:%d:%d",hour,minute,second);
		}
		else if(!stridcmp("EXIT",cmd))
		{
			live=0;
		}
		else if(!stridcmp("CD..",cmd))
		{
			if(*param) printk("\nNo Parameters Allowed");
			else if(rootdir) printk("\nThis command cannot be executed here");
			else
			{
				k=FS(24,fid,0,0,0,0);
				if(k) printk("\nUnknown Error : %d",k);
				else
				{
					for(i=pdl;i>3;i--)
					{
						if(pathdisp[i]=='/') break;
					}
					pathdisp[i]=0;
					if(i==3) rootdir=1;
					pdl=i;
				}
			}
		}
		else if(!stridcmp("MD",cmd))
		{
			//printk("\nParam : %s",param);
			k=formalisePath(param,filepath);
			//printk("\n%s",filepath);
			if((k)||(*param==0)) printk("\nBad Parameter");
			else
			{
				k=FS(13,fid,(unsigned int)filepath,0,0,0);
				//printk("\nPath : %s%d",filepath,filepath[11]);
				if(k<5){ printk("\nA Directory Or File In The Same Name Is Already Present");
				//printk("\n : %d",k);	
					}
				else
				{
					FS(23,k,0,0,0,0);
				}
			}

		}
		else if(!stridcmp("RD",cmd))
		{
			//printk("\nParam : %s",param);
			k=formalisePath(param,filepath);
			//printk("\n%s",filepath);
			if((k)||(*param==0)) printk("\nBad Parameter");
			else
			{
				k=FS(17,fid,(unsigned int)filepath,0,0,0);
				//printk("\nPath : %s%d",filepath,filepath[11]);
				if((k)&&(k<5)){ printk("\nDirectory Not Present");
				//printk("\n : %d",k);	 
				}
			}

		}
		else if(!stridcmp("RF",cmd))
		{
			//printk("\nParam : %s",param);
			k=formalisePath(param,filepath);
			//printk("\n%s",filepath);
			if((k)||(*param==0)) printk("\nBad Parameter");
			else
			{
				k=FS(16,fid,(unsigned int)filepath,0,0,0);
				//printk("\nPath : %s%d",filepath,filepath[11]);
				if((k)&&(k<5)){ printk("\nFile Not Present");
				//printk("\n : %d",k);	
				}
			}

		}
		else if(!stridcmp("MF",cmd))
		{
			//printk("\nParam : %s",param);
			k=formalisePath(param,filepath);
			//printk("\n%s",filepath);
			if((k)||(*param==0)) printk("\nBad Parameter");
			else
			{
				k=FS(12,fid,(unsigned int)filepath,0,0,0);
				//printk("\nPath : %s%d",filepath,filepath[11]);
				if(k<5){ printk("\nA Directory Or File In The Same Name Is Already Present");
				//printk("\n : %d",k);	
					}
				else
				{
					FS(22,k,0,0,0,0);
				}
			}

		}
		else if(!stridcmp("CD",cmd))
		{
			if(!stridcmp(param,".."))
			{
				if(rootdir) printk("\nThis command cannot be executed here");
				else
				{
					k=FS(24,fid,0,0,0,0);
					if(k) printk("\nUnknown Error : %d",k);
					else
					{
						for(i=pdl;i>3;i--)
						{
							if(pathdisp[i]=='/') break;
						}
						pathdisp[i]=0;
						if(i==3) rootdir=1;
						pdl=i;
					}
				}
			}
			else
			{
				//printk("\nParam : %s",param);
				k=formalisePath(param,filepath);
				//printk("\n%s",filepath);
				if((k)||(*param==0)) printk("\nBad Parameter");
				else
				{
					k=FS(11,fid,(unsigned int)filepath,0,0,0);
					//printk("\nPath : %s%d",filepath,filepath[11]);
					if(k<5){ printk("\nDirectory Not Present");
						//printk("\n : %d",k);
							}
					else
					{
						if(!rootdir)
						{
							pathdisp[pdl]='/';
							pdl++;
						}
						pdl=pdl+striscpy(pathdisp+pdl,param);
						rootdir=0;
					}
				}
			}
		}
		else if(!stridcmp("DIR",cmd))
		{
			k=0;n=0;files=0;dirs=0;h=0;
			if(*param) printk("\nNo Parameters Allowed");
			else
			while((k&0x0000FFFF)!=0x0000FFFF)
			{
				k=FS(20,fid,n,1,(unsigned int)dir,0);
				if(k==0xFFFF) h=1;
				if((k)&&(k<5)) { printk("\nError : %d",k); k=0xFFFF; }
				else
				{
					t=0;
					for(i=0;i<128;i++)
					{
						if(dir[i].name[0]==0) { h=1; k=0xFFFF; break; }
						if(dir[i].attrib&0x8) continue;
						if(!strncmp(dir[i].name,"..",2)) continue;
						if(!strncmp(dir[i].name,".",1)) continue;
						if(dir[i].name[0]==(char)0xE5) continue;
						l=0;
						for(j=0,k=0;j<12,k<11;j++,k++)
						{
							filename[j+1]=0;
							if((j+l)==8)
							{
								if(dir[i].name[8]!=' ') filename[j]='.';
								j++;
								filename[j+1]=0;
							}
							ch=dir[i].name[k];
							if(ch==0x05) ch=(char)0xE5;
							if(ch==' ') { l++; j--; continue; }
							filename[j]=ch;
						}
						filename[12]=0;
						printk("\n%s",filename);
						if(strlen(filename)<10) printk("\t");
						if(dir[i].attrib&0x10) { printk("\t<DIR>\t"); dirs++; }
						else { printk("\t \t"); files++;}
						date=(dir[i].UDate)&0xFFFF;
						day=date&0x1F;
						month=(date&0x1FF)>>5;
						year=date>>9;
						time=(dir[i].UTime)&0xFFFF;
						second=(time&0x1F)*2;
						minute=(time&0x7FF)>>5;
						hour=time>>11;
						if(!dir[i].attrib&0x10) printk("%d",dir[i].Size);
						printk("\t %02d/%02d/%04d\t%02d:%02d:%02d",day,month,year+1980,hour,minute,second);
						t++;
						if((t==16)&&(dir[i+1].name[0]))
						{
							printk("\nPress any key to continue...");
							printk("\nPress Esc to stop...");
							ch=getch();
							if(ch==ESC) k=0xFFFF;
						}
						
					}
					if(h==1) printk("\n\t%d Dir(s)\n\t%d File(s)",dirs,files);
				}
			}
		}
		else printk("Bad Command");
	}
	FS(26,0,0,0,0,0);
	clrscr();
	printk("\nBye...");
	printk("\n\n\n\n\n\n\n\n\n\n\n\t\tIT'S NOW SAFE TURN OFF YOUR COMPUTER");
	hangon();



/*	systemdrive=DM(GETSYSTEMDRIVE);	// it should actually be enquired from FM...
	wd=(char *)getNewMemPage();
	cmd=(char *)getNewMemPage();
	*wd=systemdrive;
	strcpy(wd+1,":/USER");
	dirhandle=FM(OPENDIR,wd);
	if(dirhandle<10) dirhandle=FM(CREATEDIR,wd);
	printk("\n\t\t\tREDSKY COMMAND PROMPT");
	while(live)
	{
		printk("%s>",wd);
		scank("%s",cmd);
		live=executecmd();
	}
	FM(SHUTDOWN);
	TM(SHUTDOWN);
	PS(SHUTDOWN);
	DM(SHUTDOWN);
	clrscr();
	printk("\nBye...");
	printk("\n\n\n\n\n\n\tIT'S NOW SAFE TURN OFF YOUR COMPUTER");
	hangon();*/
}
