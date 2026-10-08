void inline clrInt()
{
	__asm__("cli");
}
void inline setInt()
{
	__asm__("sti");
}	

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

int initTM()
{
	fts=(struct FTSslot *) 0x00100000;
	return(0);
}

unsigned int getTime()
{
	unsigned int time=0;
	clrInt();
	*(((char *)&time)+3)=fts->hour;
	*(((char *)&time)+2)=fts->minute;
	*(((char *)&time)+1)=fts->second;
	*((char *)&time)=(char) fts->tick;
	setInt();
	return(time);
}

unsigned int getDate()
{
	unsigned int date;
	clrInt();
	*(((short *)&date)+1)=(fts->year+20);
	*(((char *)&date)+1)=fts->month;
	*((char *)&date)=fts->day;
	setInt();
	return(date);
}

short getKey()
{
	short key;
	clrInt();
	if(fts->fgpKeyStart==fts->fgpKeyEnd) { setInt(); return(0); }
	key=fts->fgpKey[fts->fgpKeyStart];
	fts->fgpKeyStart++;
	if(fts->fgpKeyStart==16) fts->fgpKeyStart=0;
	setInt();
	return(key);
}

short getCh()
{
	short key;
	key=0;
	while(!key) { key=getKey(); }
	return(key);
}
