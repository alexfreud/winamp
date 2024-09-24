#include "main.h"
#include "../Agave/language/api_language.h"
#include <commdlg.h>
#include "resource.h"

DWORD _fastcall rev32(DWORD d) {return _rv(d);}

void CPipe::WriteData(void* b,UINT s)
{
	if (closed) return;
	sec.enter();
	if (buf_n+s>buf_s)
	{
#ifdef USE_LOG
		log_write("buffer overflow");
#endif
		s=buf_s-buf_n;
		s-=s%align;
	}
	if (s)
	{
		if (buf_wp+s<buf_s)
		{
			memcpy(buf+buf_wp,b,s);
			buf_wp+=s;
		}
		else
		{
			UINT d=buf_s-buf_wp;
			memcpy(buf+buf_wp,b,d);
			memcpy(buf,(BYTE*)b+d,s-d);
			buf_wp=s-d;
		}
		buf_n+=s;
	}
	sec.leave();
}

UINT CPipe::ReadData(void* _b,UINT s,bool* ks)
{
	UINT rv=0;
	BYTE * b=(BYTE*)_b;
	sec.enter();
	while(1)
	{
		UINT d=s;
		if (d>buf_n) d=buf_n;
		if (d)
		{
			if (buf_rp+d<buf_s)
			{
				memcpy(b,buf+buf_rp,d);
				buf_rp+=d;
			}
			else
			{
				UINT d1=buf_s-buf_rp;
				memcpy(b,buf+buf_rp,d1);
				memcpy(b+d1,buf,d-d1);
				buf_rp=d-d1;
			}
			buf_n-=d;
			s-=d;
			rv+=d;
			b+=d;
		}
		if (closed || !s || *ks) break;
		sec.leave();
		MIDI_callback::Idle();
		sec.enter();
	}
	sec.leave();
	return rv;
}

#ifdef USE_LOG
static HANDLE hLog;
void log_start()
{
	hLog=CreateFile("c:\\in_midi.log",GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_ALWAYS,0,0);
	SetFilePointer(hLog,0,0,FILE_END);
	log_write("opening log");
}

void log_quit() {log_write("closing log");log_write("");log_write("");CloseHandle(hLog);}

void log_write(char* t)
{
	DWORD bw;
	WriteFile(hLog,t,strlen(t),&bw,0);
	char _t[2]={13,10};
	WriteFile(hLog,_t,2,&bw,0);
	FlushFileBuffers(hLog);
}
#endif










//tempo map object

CTempoMap* tmap_create()
{
	CTempoMap* m=new CTempoMap;
	if (m)
	{
		m->pos=0;
		m->size=0x100;
		m->data=(TMAP_ENTRY*)malloc(m->size*sizeof(TMAP_ENTRY));
	}
	return m;
}

void CTempoMap::AddEntry(int _p,int tm)
{
	if (!data) {pos=size=0;return;}
	if (pos && _p<=data[pos-1].pos) {data[pos-1].tm=tm;return;}
	if (pos==size)
	{
		size*=2;
		data=(TMAP_ENTRY*)realloc(data,size*sizeof(TMAP_ENTRY));
		if (!data) {pos=0;return;}
	}
	data[pos].pos=_p;
	data[pos].tm=tm;
	pos++;
}

int ReadSysex(const BYTE* src,int ml)
{
	int r=1;
	while(r<ml)
	{
		r++;
		if (src[r]==0xF7) return r+1;
	}
	unsigned int d;
	r=1+DecodeDelta(src+1,&d);
	r+=d;
	return r;
}

unsigned int DecodeDelta(const BYTE* src,unsigned int* _d, unsigned int limit)
{
	unsigned int l=0;
	unsigned int d=0;
	BYTE b;
	do 
	{
		if (l >= limit)
		{
			*_d=0;
			return l;
		}
		b=src[l++];
		d=(d<<7)|(b&0x7F);
	} while(b&0x80);
	*_d=d;
	return l;
}

int EncodeDelta(BYTE* dst,int d)
{
	if (d==0)
	{
		dst[0]=0;
		return 1;
	}
	else
	{
		int r=0;
		int n=1;
		unsigned int temp=d;
		while (temp >>= 7)
		{
			n++;
		}

		do {
			n--;
			BYTE b=(BYTE)((d>>(7*n))&0x7F);
			if (n) b|=0x80;
			dst[r++]=b;
		} while(n);
		return r;
	}
}

int CTempoMap::BuildTrack(grow_buf & out)
{
	if (!pos) return 0;
	int start=out.get_size();
	//BYTE* trk=(BYTE*)malloc(8+4+pos*10);
	//if (!trk) return 0;
	out.write_dword(_rv('MTrk'));
	out.write_dword(0);//track size
	DWORD ct=0;
	int n;
	BYTE t_event[6]={0xFF,0x51,0x03,0,0,0};
	for(n=0;n<pos;n++)
	{
		DWORD t=data[n].pos;
		gb_write_delta(out,t-ct);
		ct=t;
		t=data[n].tm;
		t_event[3]=(BYTE)(t>>16);
		t_event[4]=(BYTE)(t>>8);
		t_event[5]=(BYTE)(t);
		out.write(t_event,6);
	}
	out.write_dword(0x002FFF00);
	out.write_dword_ptr(rev32(out.get_size()-(start+8)),start+4);
	return 1;
}

//sysex map management

void CSysexMap::AddEvent(const BYTE* e,DWORD s,DWORD t)
{
	if (!data || !events) return;
	DWORD np=pos+1;
	if (np>=e_size)
	{
		do {
			e_size<<=1;
		} while(np>=e_size);
		events=(SYSEX_ENTRY*)realloc(events,e_size*sizeof(SYSEX_ENTRY));
		if (!events) return;
	}
	DWORD nd=d_pos+s;
	if (nd>=d_size)
	{
		do {
			d_size<<=1;
		} while(nd>=d_size);
		data=(BYTE*)realloc(data,d_size);
		if (!data) return;
	}
	data[d_pos]=0xF0;
	unsigned int x;
	unsigned int sp=DecodeDelta(e+1,&x);
	if (sp >= s)
		return;
	memcpy(data+d_pos+1,e+1+sp,s-1-sp);
	events[pos].pos=t;
	events[pos].ofs=d_pos;
	events[pos].len=s-sp;
	d_pos=nd-sp;
	pos++;
}

CSysexMap* smap_create()
{
	CSysexMap* s=new CSysexMap;
	if (s)
	{
		s->e_size=0x10;
		s->d_size=0x40;
		s->events=(SYSEX_ENTRY*)malloc(sizeof(SYSEX_ENTRY)*s->e_size);
		s->data=(BYTE*)malloc(s->d_size);
		s->d_pos=s->pos=0;
	}
	return s;
}


CSysexMap::~CSysexMap()
{
	if (data) free(data);
	if (events) free(events);
}

BYTE d_GMReset[6]={0xF0,0x7E,0x7F,0x09,0x01,0xF7};
BYTE d_XGReset[9]={0xf0,0x43,0x10,0x4c,0x00,0x00,0x7e,0x00,0xf7};
BYTE d_GSReset[11]={0xF0,0x41,0x10,0x42,0x12,0x40,0x00,0x7F,0x00,0x41,0xF7};

CSysexMap* CSysexMap::Translate(MIDI_file * mf)
{
	CTempoMap* tmap=mf->tmap;
	if (!events || !data || !tmap) return 0;
	CSysexMap* nm=smap_create();
	if (!nm) return 0;
	nm->d_size=d_size;
	nm->d_pos=d_pos;
	nm->data=(BYTE*)realloc(nm->data,nm->d_size);
	if (!nm->data) {delete nm;return 0;}
	memcpy(nm->data,data,d_pos);
	nm->e_size=e_size;
	nm->pos=pos;
	nm->events=(SYSEX_ENTRY*)realloc(nm->events,sizeof(SYSEX_ENTRY)*nm->e_size);
	if (!nm->events) {delete nm;return 0;}
	
	int pos_ms=0;
	int n=0;
	int cur_temp=0;
	int ntm=tmap->pos,t_pos=0;
	int p_t=0;
	int dtx = rev16(*(WORD*)(mf->data+12))*1000;
	int pos_tx=0;

	while(n<pos)
	{
		pos_tx=events[n].pos;
		int d=pos_tx-p_t;
		p_t=pos_tx;
		while(t_pos<ntm && pos_tx+d>=tmap->data[t_pos].pos)
		{
			DWORD d1=tmap->data[t_pos].pos-pos_tx;
			pos_ms+=MulDiv(cur_temp,d1<<8,dtx);
			cur_temp=tmap->data[t_pos].tm;
			t_pos++;
			pos_tx+=d1;
			d-=d1;
		}
		pos_ms+=MulDiv(cur_temp,d<<8,dtx);
		pos_tx+=d;

		nm->events[n].pos=pos_ms>>8;
		nm->events[n].ofs=events[n].ofs;
		nm->events[n].len=events[n].len;
		n++;
	}
	return nm;
}

int CSysexMap::BuildTrack(grow_buf & out)
{
	if (!pos) return 0;
		
	int start=out.get_size();
	out.write_dword(_rv('MTrk'));
	out.write_dword(0);
	
	int ct=0;
	int n;
	for(n=0;n<pos;n++)
	{
		DWORD t=events[n].pos;
		gb_write_delta(out,t-ct);
		ct=t;
		out.write_byte(0xF0);
		gb_write_delta(out,events[n].len-1);
		out.write(data+events[n].ofs+1,events[n].len-1);
	}
	out.write_dword(0x002FFF00);
	out.write_dword_ptr(rev32(out.get_size()-(start+8)),start+4);
	return 1;
}

const char* CSysexMap::GetType()
{
	int ret=0;
	int n;
	for(n=0;n<pos;n++)
	{
		ret=data[events[n].ofs+1];
		if (ret!=0x7E) break;
	}

	switch(ret)
	{
	case 0x7E:
		return "GM";
	case 0x43:
		return "XG";
	case 0x42:
		return "X5";
	case 0x41:
		return "GS";
	}
	return 0;
}

void CSysexMap::CleanUp()
{
	if (!pos) return;
	int n,m;
	for(n=0;n<pos-1;n++)
	{
		for(m=n+1;m<pos;m++)
		{
			if (events[n].pos>events[m].pos)
			{
				SYSEX_ENTRY t=events[n];
				events[n]=events[m];
				events[m]=t;
			}
		}
	}
}

char* BuildFilterString(UINT res_id, char* ext, int* len)
{
	static char filterStr[256];
	char *f = filterStr;
	ZeroMemory(filterStr,256);
	*len = 0;
	WASABI_API_LNGSTRING_BUF(res_id,filterStr,256);
	f += (*len = lstrlenA(filterStr) + 1);
	lstrcatA(f,"*.");
	f += 2;
	lstrcatA(f,ext);
	*(f + lstrlenA(ext)+1) = 0;
	*len += lstrlenA(ext)+3;
	return filterStr;
}

BOOL DoOpenFile(HWND w,char* fn,UINT res_id, char* ext,BOOL save)
{
	int len = 0;
	OPENFILENAMEA ofn = {sizeof(ofn),0};
	ofn.hwndOwner=w;
	ofn.lpstrFilter=BuildFilterString(res_id,ext,&len);
	ofn.lpstrFile=fn;
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrDefExt=ext;
	if (save)
	{
		ofn.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
		return GetSaveFileNameA(&ofn);		
	}
	else
	{
		ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
		return GetOpenFileNameA(&ofn);
	}
}

BOOL DoSaveFile(HWND w, char* fn, char* filt, char* ext)
{
	OPENFILENAMEA ofn;
	ZeroMemory(&ofn,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=w;
	ofn.lpstrFilter=filt;
	ofn.lpstrFile=fn;
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrDefExt=ext;
	ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
	return GetOpenFileNameA(&ofn);
}

typedef void (*SYSEXFUNC)(void*,BYTE*,UINT);

#define rsysex(X) f(i,X,sizeof(X))
#define _sysex(X,Y) f(i,X,Y)

bool need_sysex_start()
{
	return cfg_hardware_reset>0 
		|| cfg_sysex_table.num_entries()>0
		;
}

void sysex_startup(SYSEXFUNC f,void* i)
{
	if (cfg_hardware_reset>0)
	{
		switch(cfg_hardware_reset)
		{
		case 1:rsysex(d_GMReset);break;
		case 2:rsysex(d_GSReset);break;
		case 3:rsysex(d_XGReset);break;
		}
		MIDI_callback::Idle(200);
	}
	if (cfg_sysex_table.num_entries()>0)
	{
		int idx=0;
		BYTE * data;
		int size,time;
		while(cfg_sysex_table.get_entry(idx++,&data,&size,&time))
		{
			_sysex(data,size);
			MIDI_callback::Idle(time);
		}
	}
}


MIDI_EVENT* do_table(MIDI_file * mf,UINT prec,UINT * size,UINT* _lstart,DWORD cflags)
{
	BYTE * data_ptr = 0;
	int data_size = 0;
	if (!DoCleanUp(mf,CLEAN_1TRACK|CLEAN_NOSYSEX|CLEAN_NOTEMPO|cflags,(void**)&data_ptr,&data_size)) return 0;
	if (data_size<=0x0e) {free(data_ptr);return 0;}

	UINT ts;
	BYTE* track;
	UINT ntm;
	track=data_ptr+8+6+8;
	ts=rev32(*(DWORD*)(track-4));
	CTempoMap* tmap=mf->tmap;
	UINT n=0;
	UINT pt=0;
	ntm=tmap->pos;
	CSysexMap* smap;

	if (!cfg_nosysex && mf->smap && mf->smap->pos)
	{
		smap=mf->smap;
	}
	else smap=0;

	n=0;
	DWORD pos=0;
	DWORD pos_ms=0;
	DWORD t_pos=0;
	DWORD cur_temp=0;
	UINT dtx=(UINT)rev16(*(WORD*)(data_ptr+8+4))*1000/prec;
	grow_buf boo;

	int ns=0;
	UINT track_pos=0,smap_pos=0;
	UINT loop_start=-1;

	{
		unsigned int _d;
		n+=DecodeDelta(track+n,&_d);
		track_pos+=_d;
	}

	if (smap)
	{
		smap_pos=smap->events[0].pos;
	}
	else smap_pos=-1;

	while(1)
	{
		DWORD ev=0;
		DWORD d=0;
		{
			if (n >= (data_size-26))
				break;
			if (track_pos<smap_pos)
			{
				d=track_pos-pos;
				ev=(*(DWORD*)(track+n))&0xFFFFFF;
				if ((ev&0xF0)==0xF0)
				{
					track_pos=-1;
					continue;
				}
				if ((ev&0xF0)==0xC0 || (ev&0xF0)==0xD0)
				{
					ev&=0xFFFF;n+=2;
				}
				else
				{
					n+=3;
				}
				if ((ev&0xFF00F0)==0x90)
				{
					ev=(ev&0xFF0F)|0x7F0080;
				}
				unsigned int _d;
				n+=DecodeDelta(track+n,&_d);
				track_pos+=_d;
				if (n >= (data_size-26))
					break;
			}
			else if (smap_pos!=-1)
			{
				d=smap_pos-pos;
				ev=0x80000000|ns;
				ns++;
				if (ns==smap->pos) 
					smap_pos=-1;
				else
					smap_pos=smap->events[ns].pos;
			}
		}
		if (!ev) break;
		while(t_pos<ntm && pos+d>=(UINT)tmap->data[t_pos].pos)
		{
			DWORD d1=tmap->data[t_pos].pos-pos;
			if (loop_start==-1 && (UINT)mf->loopstart_t<=pos+d1) loop_start=pos_ms+MulDiv(cur_temp,pos+d1-mf->loopstart_t,dtx);
			pos_ms+=MulDiv(cur_temp,d1,dtx);
			cur_temp=tmap->data[t_pos].tm;
			t_pos++;
			pos+=d1;
			d-=d1;
		}
		if (loop_start==-1 && (UINT)mf->loopstart_t<=pos+d) loop_start=pos_ms+MulDiv(cur_temp,d,dtx);
		pos_ms+=MulDiv(cur_temp,d,dtx);
		pos+=d;
		{
			MIDI_EVENT me={pos_ms,ev};
			boo.write(&me,sizeof(me));
		}
	}
	
	free(data_ptr);

	UINT sz=boo.get_size();
	MIDI_EVENT* ret=(MIDI_EVENT*)boo.finish();
	if (ret)
	{
		*size=sz>>3;//sz/sizeof(MIDI_EVENT);
		if (cfg_loop_type==2 && loop_start==-1) loop_start=0;
		else if (cfg_loop_type==0) loop_start=-1;
		if (_lstart) *_lstart=loop_start;
	}
	return ret;
}


void gb_write_delta(grow_buf & gb,DWORD d)
{
	BYTE tmp[8] = {0};
	gb.write(tmp,EncodeDelta(tmp,d));
}

void do_messages(HWND w,bool* b)
{
	MSG msg;
	while(b && *b)
	{
		BOOL b=GetMessage(&msg,w,0,0);
		if (b==-1 || !b) break;
		DispatchMessage(&msg);
	}
}

static wchar_t cb_class[]=TEXT("CallbackWndClass0");

ATOM do_callback_class(WNDPROC p)
{
	cb_class[sizeof(cb_class)-2]++;
	WNDCLASS wc=
	{
		0,p,0,4,MIDI_callback::GetInstance(),0,0,0,0,cb_class
	};
	return RegisterClassW(&wc);
}

HWND create_callback_wnd(ATOM cl,void* p)
{
	HWND w=CreateWindowA((char*)cl,0,0,0,0,0,0,MIDI_callback::GetMainWindow(),0,MIDI_callback::GetInstance(),0);
	if (w) SetWindowLong(w,0,(long)p);
	return w;
}

CTempoMap* tmap_merge(CTempoMap* m1,CTempoMap* m2)
{
	int p1=0,p2=0;
	CTempoMap * ret=0;
	if (m1 && m2 && m1->data && m2->data)
	{
		ret=tmap_create();
		if (ret)
		{
			while(p1<m1->pos && p2<m2->pos)
			{
				if (m1->data[p1].pos<=m2->data[p2].pos)
				{
					ret->AddEntry(m1->data[p1].pos,m1->data[p1].tm);
					p1++;
				}
				else
				{
					ret->AddEntry(m2->data[p2].pos,m2->data[p2].tm);
					p2++;
				}
			}
			while(p1<m1->pos)
			{
				ret->AddEntry(m1->data[p1].pos,m1->data[p1].tm);
				p1++;
			}
			while(p2<m2->pos)
			{
				ret->AddEntry(m2->data[p2].pos,m2->data[p2].tm);
				p2++;
			}
		}
	}
	if (m1) delete m1;
	if (m2) delete m2;
	return ret;

}

KAR_ENTRY * kmap_create(MIDI_file* mf,UINT prec,UINT * num,char** text)
{
	if (!mf->kar_track) return 0;
	grow_buf b_data,b_map;
	KAR_ENTRY te;
	BYTE *track=(BYTE*)mf->data+mf->kar_track+8;
	BYTE *track_end = track+rev32(*(DWORD*)(mf->data+mf->kar_track+4));
	int time=0;
	int ptr=0;
	BYTE lc=0;
	while(track<track_end)
	{
		unsigned int d;
		track+=DecodeDelta(track,&d);
		time+=d;
		if (*track==0xFF)	//meta
		{
			BYTE type=track[1];
			track+=2;
			track+=DecodeDelta(track,&d);
			char * ptr=(char*)track;
			track+=d;
			if ((type==0x5 || type==0x1) && d && *ptr!='@')	//lyrics
			{
				te.time=time;
				te.foo=1;
				unsigned int n;
				te.start=b_data.get_size();
				for(n=0;n<d;n++)
				{
					switch(ptr[n])
					{
//					case '@':
					case '\\':
					case '/':
					case 0x0D:
						b_data.write("\x0d\x0a",2);
						break;
					case 0x0A:
						break;
					default:
						te.foo=0;
						b_data.write_byte(ptr[n]);
						break;
					}
				}
				te.end=b_data.get_size();
				if (te.start<te.end) b_map.write(&te,sizeof(te));
			}
		}
		else if (*track==0xF0)
		{
			track++;
			track+=DecodeDelta(track,&d);
			track+=d;
		}
		else if ((*track&0xF0)==0xF0)
		{
			track++;//hack
		}
		else
		{
			if (*track&0x80) lc=*(track++)&0xF0;
			if (lc==0 || lc==0xC0 || lc==0xD0) track++;
			else track+=2;
		}
	}
	int map_siz = b_map.get_size();
	KAR_ENTRY * map=(KAR_ENTRY*)b_map.finish();
	map_siz/=sizeof(KAR_ENTRY);

	if (num) *num=map_siz;
	
	if (text)
	{
		b_data.write_byte(0);
		*text=(char*)b_data.finish();
	}
	else b_data.reset();

	if (map)
	{
		int n;

		time=0;
		
		CTempoMap* tmap=mf->tmap;

		int pos_ms=0;
		int t_pos=0;
		int cur_temp=0;
		int dtx=(UINT)rev16(*(WORD*)(mf->data+8+4))*1000/prec;

		for(n=0;n<map_siz;n++)
		{
			int d=0;
			d=map[n].time-time;
			
			while(t_pos<tmap->pos && time+d>=tmap->data[t_pos].pos)
			{
				DWORD d1=tmap->data[t_pos].pos-time;
				pos_ms+=MulDiv(cur_temp,d1,dtx);
				cur_temp=tmap->data[t_pos].tm;
				t_pos++;
				time+=d1;
				d-=d1;
			}
			pos_ms+=MulDiv(cur_temp,d,dtx);
			time+=d;
			map[n].time=pos_ms;
		}
	}

	return map;
}

int sysex_table::num_entries() const
{
	int num=0;
	entry * ptr=entries;
	while(ptr) {ptr=ptr->next;num++;}
	return num;
}

int sysex_table::get_entry(int idx,BYTE ** p_data,int * p_size,int * p_time) const
{
	entry * ptr=entries;
	while(ptr && idx>0) {ptr=ptr->next;idx--;}
	if (!ptr) return 0;
	if (p_data) *p_data = ptr->data;
	if (p_size) *p_size = ptr->size;
	if (p_time) *p_time = ptr->time;
	return 1;
}

void sysex_table::insert_entry(int idx,BYTE * data,int size,int time)
{
	entry ** ptr = &entries;
	while(idx>0 && *ptr)
	{
		ptr = &(*ptr)->next;
		idx--;
	}
	entry * insert = new entry;
	insert->data = (BYTE*)malloc(size);
	memcpy(insert->data,data,size);
	insert->size = size;
	insert->time = time;
	insert->next = *ptr;
	*ptr = insert;
}

int sysex_table::remove_entry(int idx)
{
	entry ** ptr = &entries;
	while(idx>0 && *ptr)
	{
		ptr = &(*ptr)->next;
		idx--;
	}
	if (!*ptr) return 0;
	entry * remove = *ptr;
	*ptr=remove->next;
	free(remove->data);
	delete remove;
	return 1;
}


int sysex_table::file_write(const char* file) const
{
	HANDLE f=CreateFileA(file,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
	if (f==INVALID_HANDLE_VALUE) return 0;
	
	int size;
	void * ptr = memblock_write(&size);
	DWORD bw = 0;
	WriteFile(f,ptr,size,&bw,0);
	free(ptr);
	CloseHandle(f);
	return 1;
}

void * sysex_table::memblock_write(int * size) const
{
	grow_buf wb;

	entry * ptr;
	//MAGIC:DWORD , NUM: DWORD,DATA_SIZE:DWORD, offsets, sleep,data
	DWORD temp;
	temp=MHP_MAGIC;
	wb.write(&temp,4);
	temp=num_entries();
	wb.write(&temp,4);

	temp=0;
	for(ptr=entries;ptr;ptr=ptr->next) temp+=ptr->size;
	wb.write(&temp,4);
	temp=0;
	for(ptr=entries;ptr;ptr=ptr->next)
	{
		wb.write(&temp,4);
		temp+=ptr->size;
	}
	for(ptr=entries;ptr;ptr=ptr->next)
	{
		temp = ptr->time;
		wb.write(&temp,4);
	}

	for(ptr=entries;ptr;ptr=ptr->next)
	{
		wb.write(ptr->data,ptr->size);
	}

	if (size) *size = wb.get_size();

	return wb.finish();
}

int sysex_table::memblock_read(const void * block,int size)
{
	entry * ptr;
	const BYTE * src = (const BYTE*)block;
	DWORD temp,total_size,total_num;
	

	if (*(DWORD*)src!=MHP_MAGIC) return 0;
	src+=4;

	temp=total_num=*(DWORD*)src;
	src+=4;
	if (total_num>0xFFFF) return 0;	

	reset();
	while(temp>0)
	{
		ptr=new entry;
		ptr->next=entries;
		entries = ptr;
		temp--;
	}

	total_size=*(DWORD*)src;

	UINT n;

	for(n=0,ptr=entries;ptr;ptr=ptr->next,n++)
	{
//offset : 12 + 4 * n;
//time : 12 + 4 * total_num + 4 * n;
//data : 12 + 8 * total_num + offset
		DWORD offset,time,offset2;
		src = (const BYTE*)block + 12 + 4*n;
		offset=*(DWORD*)src;

		if (n!=total_num-1) offset2=*(DWORD*)(src+4);
		else offset2=total_size;
		ptr->size = offset2-offset;
		src = (const BYTE*)block + 12 + 4*total_num + 4*n;
		time = *(DWORD*)src;

		ptr->data = (BYTE*)malloc(offset2);
		src = (const BYTE*)block + 12 + 8*total_num + offset;
		memcpy(ptr->data,src,ptr->size);

		ptr->time = time;
	}
	
	return 1;
}

int sysex_table::file_read(const char* file)
{
	
	HANDLE f=CreateFileA(file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	if (f==INVALID_HANDLE_VALUE) return 0;
	int size = GetFileSize(f,0);
	void * temp = malloc(size);
	DWORD br = 0;
	ReadFile(f,temp,size,&br,0);
	CloseHandle(f);
	int rv = memblock_read(temp,size);
	free(temp);
	return rv;	
}

int sysex_table::print_preview(int idx,char * out) const
{
	BYTE* data;
	int size,time;
	if (!get_entry(idx,&data,&size,&time)) return 0;
	int size2=size;
	if (size2>10) size2=10;
	wsprintfA(out,WASABI_API_LNGSTRING(STRING_MS_FMT),time);
	while(out && *out) out++;
	int n;
	for(n=0;n<size2;n++)
	{
		wsprintfA(out," %02X",data[n]);
		out+=3;
	}

	if (size!=size2)
	{
		strcpy(out,"...");
	}
	return 1;
}

void sysex_table::print_edit(int idx,HWND wnd) const
{
	BYTE* data;
	int size,time;
	if (!get_entry(idx,&data,&size,&time)) {SetWindowTextA(wnd,"");return;}
	if (size<=2) {SetWindowTextA(wnd,"");return;}
	char *temp = (char*)malloc(3*size);
	char *ptr = temp;
	int n;
	for(n=1;n<size-1;n++)
	{
		wsprintfA(ptr,"%02X ",data[n]);
		ptr+=3;
	}
	ptr[-1]=0;
	SetWindowTextA(wnd,temp);
	free(temp);
}

void sysex_table::copy(const sysex_table & src)
{
	reset();
	int idx=0;
	BYTE * data;
	int size,time;
	while(src.get_entry(idx++,&data,&size,&time))//ASS SLOW
		insert_entry(idx,data,size,time);
}

//special sysex table cfg_var hack
class cfg_var_sysex : private cfg_var
{
private:
	sysex_table * tab;

	virtual void read(HKEY hk)
	{
		int size=reg_get_struct_size(hk);
		if (size>0)
		{
			void * temp = malloc(size);
			if (temp)
			{
				reg_read_struct(hk,temp,size);
				tab->memblock_read(temp,size);
				free(temp);
			}
		}
	}
	virtual void write(HKEY hk)
	{
		void * data;
		int size;
		data = tab->memblock_write(&size);
		if (data) reg_write_struct(hk,data,size);
		
	}
	virtual void reset() {tab->reset();}

public:
	cfg_var_sysex(const char * name,sysex_table * p_tab) : cfg_var(name) {tab=p_tab;}
};

static cfg_var_sysex thevar("sysex_table",&cfg_sysex_table);
