#include "main.h"
#include "seq.h"
#include <math.h>
#include "resource.h"

// {76B6A32D-99A9-4d51-B1F5-DAD3F47FE75D}
static const GUID midiout_guid = 
{ 0x76b6a32d, 0x99a9, 0x4d51, { 0xb1, 0xf5, 0xda, 0xd3, 0xf4, 0x7f, 0xe7, 0x5d } };

// {7F00BC9C-AEA3-472a-BBB9-D74ABD4FCA58}
static const GUID midiout_driver_guid = 
{ 0x7f00bc9c, 0xaea3, 0x472a, { 0xbb, 0xb9, 0xd7, 0x4a, 0xbd, 0x4f, 0xca, 0x58 } };

static MMRESULT midiOutOpen_wrap(HMIDIOUT * hMo,int id)
{
	try {
		return midiOutOpen(hMo,(UINT)id,0,0,CALLBACK_NULL);
	} 
	catch(...) 
	{
		return -1;
	}
}

class MIDI_device_midiout : public MIDI_device
{
private:
	GUID guid;
	UINT id;
	DWORD flags;
	UINT type;

	virtual player_base * create();
	virtual GUID get_guid() {return guid;}
	virtual bool is_default() {return id==(UINT)(-1);}
	virtual bool volctrl_happy() {return (type==7) && (flags & MIDICAPS_VOLUME);}
public:
	MIDI_device_midiout(UINT p_id,DWORD p_flags,UINT p_type,const wchar_t * p_name,const wchar_t * p_info)
	{
		id=p_id;
		guid = midiout_guid;
		*(DWORD*)&guid+=id;
		flags = p_flags;
		type = p_type;
		set_name(p_name);
		set_info(p_info);
	}
	inline DWORD get_flags() {return flags;}
	inline DWORD get_id() {return id;}
};


static void midiout_sysex(HMIDIOUT hMo,BYTE* p,UINT len)
{
	MIDIHDR h;
	ZeroMemory(&h,sizeof(h));
	h.dwBytesRecorded=h.dwBufferLength=len;
	h.lpData=(char*)p;
	if (FAILED(midiOutPrepareHeader(hMo,&h,sizeof(h)))) {
#ifdef USE_LOG
		log_write("unable to send sysex");
#endif
		return;
	}
	if (SUCCEEDED(midiOutLongMsg(hMo,&h,sizeof(h))))
	{
		while(!(h.dwFlags&MHDR_DONE)) MIDI_callback::Idle();
	}
	midiOutUnprepareHeader(hMo,&h,sizeof(h));

	//log_write("sysex sent OK");
}

void midiout_sysex(HMIDIOUT hMo,BYTE* p,UINT len);

static void sysex_startup_midiout(UINT m_id)
{
	if (need_sysex_start())
	{
//		MessageBox(GetActiveWindow(),"blah",0,0);
		HMIDIOUT hMo;
		MMRESULT r=midiOutOpen_wrap(&hMo,m_id);
		if (!r)
		{
			sysex_startup((SYSEXFUNC)midiout_sysex,hMo);
			midiOutClose(hMo);
		}
	}
}

class midiout_volctrl
{
private:
	HMIDIOUT hMo;
	int vol,pan;
	void _setvol();
	static UINT map_vol(UINT volume,UINT scale);

public:
	void volctrl_init(HMIDIOUT,MIDI_device_midiout*);
	int volctrl_setvol(int);
	int volctrl_setpan(int);
};

class player_midiout : public seq_base, private midiout_volctrl
{
public:
	virtual ~player_midiout();
	virtual int setvol(int i) {return volctrl_setvol(i);};
	virtual int setpan(int i) {return volctrl_setpan(i);};

	player_midiout(MIDI_device_midiout * p_dev)
	{
		dev=p_dev;
		hMo=0;
	}
	
	int play();
private:
	MIDI_device_midiout * dev;

	HMIDIOUT hMo;

	virtual void seq_shortmsg(DWORD msg) {midiOutShortMsg(hMo,msg);}
	virtual void seq_sysex(BYTE* ptr,UINT len) {midiout_sysex(hMo,ptr,len);}
	virtual int seq_play_start();
	virtual void seq_play_stop();
};

int player_midiout::seq_play_start()
{
	return 1;
}

void player_midiout::seq_play_stop()
{
	if (hMo)
	{
#ifdef USE_LOG
		log_write("midiOutClose");
#endif
		DWORD r=midiOutClose(hMo);
		if (r==MIDIERR_STILLPLAYING)
		{
			log_write("still playing (?), calling midiOutReset");
			midiOutReset(hMo);
			r=midiOutClose(hMo);
		}
#ifdef USE_LOG
		if (r) log_write("warning: unable to close midiOut");
		else log_write("midiOut closed OK");
#endif
	}
	hMo=0;
}

int player_midiout::play()
{
	DWORD r=midiOutOpen_wrap(&hMo,dev->get_id());
	if (r)
	{
		if (r!=-1) MIDI_core::MM_error(r);
		return 0;
	}


	volctrl_init(hMo,dev);
	
	if (!seq_cmd_start(0))
	{
		midiOutClose(hMo);
		hMo=0;
		return 0;
	}
	return 1;
}

player_midiout::~player_midiout()
{
#ifdef USE_LOG
	log_write("shutting down midiOut");
#endif
	seq_cmd_stop();
}


class MIDI_driver_midiout : MIDI_driver
{
	virtual void do_init()
	{
		MIDIOUTCAPSW caps;
		UINT n_mo_dev=midiOutGetNumDevs()+1;
		UINT n;
		for(n=0;n<n_mo_dev;n++)
		{
			midiOutGetDevCapsW(n-1,&caps,sizeof(MIDIOUTCAPSW));
			//d.id = TYPE_MIDIOUT | n;
			//d.name=(char*)_strdup(caps.szPname);
			string_w info;
			{
				wchar_t moo[128], *t=0;
				switch(caps.wTechnology)
				{
				case MOD_FMSYNTH:
					t=WASABI_API_LNGSTRINGW_BUF(STRING_MOCAPS_FM,moo,128);
					break;
				case MOD_MAPPER:
					t=WASABI_API_LNGSTRINGW_BUF(STRING_MOCAPS_MAPPER,moo,128);
					break;
				case MOD_MIDIPORT:
					t=WASABI_API_LNGSTRINGW_BUF(STRING_MOCAPS_HWPORT,moo,128);
					break;
				case MOD_SQSYNTH:
					t=WASABI_API_LNGSTRINGW_BUF(STRING_MOCAPS_SQUARE,moo,128);
					break;
				case MOD_SYNTH:
					t=WASABI_API_LNGSTRINGW_BUF(STRING_MOCAPS_SYNTH,moo,128);
					break;
				case 6:
					t=WASABI_API_LNGSTRINGW_BUF(STRING_MOCAPS_WAVETABLE,moo,128);
					break;
				case 7:
					t=WASABI_API_LNGSTRINGW_BUF(STRING_DMCAPS_SOFTSYNTH,moo,128);
					break;
				default:
					wsprintfW(moo,WASABI_API_LNGSTRINGW(STRING_UNKNOWN),caps.wTechnology);
					t=moo;
					break;
				}
				if (t)
				{
					info+=WASABI_API_LNGSTRINGW(STRING_DEVICE_TYPE);
					info+=t;
					info+=L"\x0d\x0a";
				}
				if (caps.dwSupport & MIDICAPS_STREAM)
				{
					info+=WASABI_API_LNGSTRINGW(STRING_DIRECT_MIDISTREAM);
					info+=L"\x0d\x0a";
				}
			}

			add_device(new MIDI_device_midiout(n-1,caps.dwSupport,caps.wTechnology,caps.szPname,info));
		}
	}
	virtual const wchar_t * get_name() {return L"midiOut";}
	virtual bool is_default() {return 1;}
	virtual GUID get_guid() {return midiout_driver_guid;}
};

static MIDI_driver_midiout midi_driver_midiout;

#define WM_SEEK (WM_USER+4)

#define GET_TIME timeGetTime()

int IS_SPEC_C(int x);

#define BUF_MAX 0x1000//0x3C00
#define BUF_MAX_F (BUF_MAX+0x40)
#define N_BUFS 4
#define BUF_MASK (N_BUFS-1)

typedef struct
{
	MIDIHDR h;
	DWORD data[BUF_MAX_F];
} MIDIBUF;

class player_midistream : public player_base, private midiout_volctrl
{
public:
	int gettime();
	int settime(int);
	void pause();
	void unpause();
	virtual int setvol(int i) {return volctrl_setvol(i);};
	virtual int setpan(int i) {return volctrl_setpan(i);};
	int play();
	player_midistream(MIDI_device_midiout *);
	~player_midistream();
private:
	UINT renderbuf(MIDIHDR* buf,DWORD ts,UINT start,UINT end);
	UINT renderbuf_seek(UINT start,UINT end);
	void renderbuf_wait(MIDIHDR* buf,UINT len);
	DWORD pos4time(DWORD t);

	MIDI_device_midiout * dev;
	UINT m_id;
	UINT c_loop;
	CSysexMap* smap;
	HMIDISTRM hMo;
	UINT n_events;
	MIDI_EVENT* events;
	DWORD tm_ofs,p_time;
	UINT loop_start,total_len;

	MIDIBUF hdrs[N_BUFS];
	MIDIBUF seekbuf;
	UINT cur_buf;
	UINT in_mm,n_free;
	DWORD ct,cp;
	bool got_eof,paused,quitting;
	UINT seek_to;
	
	HWND wnd;
	DWORD trd_id;

	void do_bufs();
	void buf_done(MIDIHDR*);
	static LRESULT WINAPI midiOutProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
};
/*
static void _sysex(player_midistream* pl,BYTE* p,UINT len)
{//assumes thread to be OK
	HMIDISTRM hMo=pl->hMo;
	MIDIHDR h;
	ZeroMemory(&h,sizeof(h));
	h.dwUser=1;
	DWORD l=12+len;
	if (l&3) l=(l+4)&~3;
	DWORD* ev=(DWORD*)alloca(l);
	ev[0]=ev[1]=0;
	ev[2]=MEVT_F_LONG|len;
	memcpy(ev+3,p,len);
	h.dwBytesRecorded=h.dwBufferLength=l;
	h.lpData=(char*)ev;
	if (FAILED(midiOutPrepareHeader((HMIDIOUT)hMo,&h,sizeof(h)))) return;
	if (FAILED(midiStreamOut(hMo,&h,sizeof(h))))
	{
		midiOutUnprepareHeader((HMIDIOUT)hMo,&h,sizeof(h));
		return;
	}
	pl->in_mm++;
	do_messages(pl->wnd,(bool*)&h.dwUser);
	log_write("sysex sent OK");
}*/

DWORD player_midistream::pos4time(DWORD t)
{
	DWORD r=0;
	while(r<n_events && events[r].tm<t) r++;
	return r;
}

static cfg_int cfg_midistream_quick_seek("midistream_quick_seek",0);

void player_midistream::do_bufs()
{
	if (seek_to!=-1)
	{
		UINT sp=pos4time(ct=seek_to);
		if (!cfg_midistream_quick_seek)
		{
			UINT st=cp;
			if (sp<cp) st=0;
			if (renderbuf_seek(st,sp)!=-1)
			{
				if (!midiOutPrepareHeader((HMIDIOUT)hMo,&seekbuf.h,sizeof(MIDIHDR)))
				{
					if (!midiStreamOut(hMo,&seekbuf.h,sizeof(MIDIHDR)))
					{
						in_mm++;
					}
					else midiOutUnprepareHeader((HMIDIOUT)hMo,&seekbuf.h,sizeof(MIDIHDR));
				}
			}
		}
		cp=sp;
		seek_to=-1;
	}
	while(n_free && !got_eof)
	{
		MIDIHDR* hdr=&hdrs[cur_buf].h;
		cp=renderbuf(hdr,ct,cp,-1);
		if (cp==-1)
		{
			if (loop_start!=-1 && c_loop>1)
			{
				c_loop--;
				cp=pos4time(ct=loop_start);
				continue;
			}
			else
			{
				got_eof=1;
				if (cfg_eof_delay)
				{
					renderbuf_wait(hdr,cfg_eof_delay);
				}
				else break;
				
			}
		}
		if (midiOutPrepareHeader((HMIDIOUT)hMo,hdr,sizeof(MIDIHDR)))
		{
			got_eof=1;
			break;
		}
		if (midiStreamOut(hMo,hdr,sizeof(MIDIHDR)))
		{
			got_eof=1;
			break;
		}
		cur_buf=(cur_buf+1)&BUF_MASK;
		in_mm++;
		n_free--;
		if (!got_eof)
			ct=cp ? events[cp-1].tm : 0;
	}
}


UINT player_midistream::renderbuf(MIDIHDR* buf,DWORD ts,UINT start,UINT end)
{
	UINT n=start;
	UINT p=0;
	DWORD* pEv=(DWORD*)buf->lpData;
	UINT c_t=ts;
	while(n<end && n<n_events && p<(BUF_MAX-3))
	{
		int dt=events[n].tm-c_t;
		if (dt<0) dt=0;
		pEv[p++]=dt;
		c_t+=dt;
		pEv[p++]=0;
		if (events[n].ev&0x80000000)
		{
			SYSEX_ENTRY* se=&smap->events[events[n].ev&0x7FFFFFFF];
			if (p+(se->len>>2)>=BUF_MAX) 
			{
				p-=2;
				break;
			}
			pEv[p++]=MEVT_F_LONG|se->len;
			DWORD d=se->len>>2;
			if (se->len&3)
			{
				pEv[p+(d++)]=0;
			}
			memcpy(pEv+p,smap->data+se->ofs,se->len);
			p+=d;
		}
		else
		{
			pEv[p++]=events[n].ev;
		}
		n++;
	}
	if (p==0) 
		return -1;
	buf->dwBufferLength=p<<2;
	buf->dwBytesRecorded=p<<2;
	buf->dwFlags=0;
	return n;
}

void player_midistream::renderbuf_wait(MIDIHDR* buf,UINT len)
{
	UINT p=0;
	DWORD* pEv=(DWORD*)buf->lpData;
	pEv[p++]=len<<3;
	pEv[p++]=0;
	pEv[p++]=MEVT_NOP<<24;
	buf->dwBufferLength=p<<2;
	buf->dwBytesRecorded=p<<2;
	buf->dwFlags=0;
}

UINT player_midistream::renderbuf_seek(UINT start,UINT end)
{
	BYTE ins_tab[16] = {0};
	memset(ins_tab,-1,sizeof(ins_tab));
	BYTE ctrl_tab[16][128] = {0};
	memset(ctrl_tab,-1,sizeof(ctrl_tab));
	UINT n=start;
	DWORD* pEv=(DWORD*)seekbuf.h.lpData;
	while(n<end)
	{
		DWORD ec=events[n].ev;
		if (ec&0x80000000) {n++;continue;}
		UINT ch,cd;
		
		ch=ec&0xF;
		cd=ec&0xF0;
		if (cd==0xB0) ctrl_tab[ch][(ec>>8)&0x7F]=(BYTE)(ec>>16);
		else if (cd==0xC0) ins_tab[ch]=(BYTE)(ec>>8);
		n++;
	}
	UINT c;
	UINT p=0;
	for(c=0;c<16;c++)
	{
		for(n=0;n<128;n++)
		{
			if (!(ctrl_tab[c][n]&0x80))
			{
				pEv[p++]=0;
				pEv[p++]=0;
				pEv[p++]=0xB0|c|(n<<8)|(ctrl_tab[c][n]<<16);
			}
			if (p>=BUF_MAX) 
				goto q;
		}
		if (!(ins_tab[c]&0x80))
		{
			pEv[p++]=0;
			pEv[p++]=0;
			pEv[p++]=0xC0|c|(ins_tab[c]<<8);
		}
		if (p>=BUF_MAX) 
			goto q;
	}
q:

	if (p==0) return -1;
	
	seekbuf.h.dwBufferLength=p<<2;
	seekbuf.h.dwBytesRecorded=p<<2;
	seekbuf.h.dwFlags=0;
	return n;
}

void player_midistream::buf_done(MIDIHDR* h)
{
	in_mm--;
	midiOutUnprepareHeader((HMIDIOUT)hMo,h,sizeof(MIDIHDR));
	if (h->dwUser)
	{
		h->dwUser=0;
		return;
	}
	if (h==&seekbuf.h) return;
	n_free++;
	if (quitting) return;

	if (!in_mm && got_eof) 
		MIDI_core::Eof();
	else if (!got_eof)
	{
		do_bufs();
	}
}

LRESULT WINAPI player_midistream::midiOutProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	if (msg==MM_MOM_DONE)
	{
		player_midistream* p=(player_midistream*)GetWindowLong(wnd,0);
		if (p) p->buf_done((MIDIHDR*)lp);
	}
	else if (msg==WM_SEEK)
	{
		player_midistream* p=(player_midistream*)GetWindowLong(wnd,0);
		if (p) p->settime(lp);
	}
	return DefWindowProc(wnd,msg,wp,lp);
}

player_midistream::player_midistream(MIDI_device_midiout * p_dev)
{
	dev=p_dev;
	m_id=0;
	c_loop=0;
	smap=0;
	hMo=0;
	n_events=0;
	events=0;
	tm_ofs=0;p_time=0;
	loop_start=0;
	total_len=0;
	memset(&hdrs,0,sizeof(hdrs));
	memset(&seekbuf,0,sizeof(seekbuf));
	cur_buf=0;
	in_mm=0;n_free=0;
	ct=0;cp=0;
	got_eof=0;
	paused=0;
	quitting=0;
	seek_to=0;
	
	wnd=0;
	trd_id=0;


	static ATOM cb_class;
	if (!cb_class) cb_class=do_callback_class(midiOutProc);

	wnd=create_callback_wnd(cb_class,this);
	c_loop=cfg_loop_infinite ? -1 : cfg_loop_count;
}

player_base* MIDI_device_midiout::create()
{
	if (cfg_playback_mode)
	{
		player_midiout *p=new player_midiout(this);
		if (p)
		{
			if (!p->play()) {delete p;p=0;}
		}
		return p;
	}
	else
	{
		player_midistream *p=new player_midistream(this);
		if (p)
		{
			if (!p->play()) {delete p;p=0;}
		}
		return p;
	}
}

//extern bool cfg_alt_sysex;

int player_midistream::play()
{
	trd_id=GetCurrentThreadId();
	UINT n;
	for(n=0;n<N_BUFS;n++)
	{
		hdrs[n].h.lpData=(char*)hdrs[n].data;
	}
	seekbuf.h.lpData=(char*)seekbuf.data;
	
	//bool alt_sysex=cfg_alt_sysex;

	//if (alt_sysex)
	{
		sysex_startup_midiout(dev->get_id());
	}
#ifdef USE_LOG
	log_write("starting midiOut / streamed");
#endif
	{
		UINT id=dev->get_id();
		DWORD err=midiStreamOpen(&hMo,&id,1,(DWORD)wnd,0,CALLBACK_WINDOW);
		if (err)
		{
			MIDI_core::MM_error(err);
			return 0;
		}
		MIDIPROPTIMEDIV td;
		td.cbStruct=sizeof(td);
		td.dwTimeDiv=1*8;//tix / q
		err=midiStreamProperty(hMo,(BYTE*)&td,MIDIPROP_SET|MIDIPROP_TIMEDIV);
		if (err)
		{
			midiStreamClose(hMo);
			MIDI_core::MM_error(err);
			return 0;
		}

		MIDIPROPTEMPO tempo;
		tempo.cbStruct=sizeof(tempo);
		tempo.dwTempo=1000;//ns / q
		err=midiStreamProperty(hMo,(BYTE*)&tempo,MIDIPROP_SET|MIDIPROP_TEMPO);
		if (err)
		{
			midiStreamClose(hMo);
			MIDI_core::MM_error(err);
			return 0;
		}
	}
	
	events=do_table(MIDI_core::getFile(),8,&n_events,&loop_start,0);
	if (!events)
	{
		midiStreamClose(hMo);
		hMo=0;
		return 0;
	}
	total_len=events[n_events-1].tm>>3;

	if (!cfg_nosysex && MIDI_core::getFile()->smap && MIDI_core::getFile()->smap->pos)
	{
		smap=MIDI_core::getFile()->smap;
	}
	else smap=0;

	paused=0;

	volctrl_init((HMIDIOUT)hMo,dev);

	midiStreamPause(hMo);

	//sysex_startup((SYSEXFUNC)midiout_sysex,hMo);

	seek_to=-1;

	tm_ofs=GET_TIME;

	cur_buf=0;
	in_mm=0;
	n_free=N_BUFS;

	ct=cp=0;

	do_bufs();

#ifdef USE_LOG
	log_write("started OK");
#endif
	midiStreamRestart(hMo);

	return 1;
}

player_midistream::~player_midistream()
{
//	bool alt_sysex=cfg_alt_sysex;
#ifdef USE_LOG
	log_write("shutting down midistream");
#endif
	if (hMo)
	{
		//ASSERT(trd_id!=GetCurrentThreadId());
		quitting=1;
#ifdef USE_LOG
		log_write("midiStreamStop");
#endif
		midiStreamStop(hMo);
	
//		do_messages(wnd,(bool*)&in_mm);
		if (n_free!=N_BUFS)
		{
			UINT n;
			for(n=0;n<N_BUFS;n++)
			{
				if (hdrs[n].h.dwFlags&MHDR_PREPARED)
				{
					midiOutUnprepareHeader((HMIDIOUT)hMo,&hdrs[n].h,sizeof(MIDIHDR));
					in_mm--;
					n_free++;
				}
			}
		}

#ifdef HUNT_LEAKS
		if (n_free!=N_BUFS) Warning("Not all buffers collected.");
#endif
#ifdef USE_LOG
		log_write("midiStreamClose");
#endif
		midiStreamClose(hMo);
		//if (midiStreamClose(hMo)) Warning(STRING_MIDISTREAM_WARNING);
	}
	if (events) free(events);
	if (wnd) DestroyWindow(wnd);
#ifdef USE_LOG
	log_write("midistream shut down");
#endif
}

int player_midistream::gettime()
{
	DWORD ret;
	if (paused) ret=p_time;
	else if (!tm_ofs) ret=0;
	else
	{
		ret=GET_TIME-tm_ofs;
		if (loop_start!=-1 && ret>total_len)
		{
			UINT _ls=loop_start>>3;
			ret=(ret-_ls)%(total_len-_ls)+_ls;
		}
	}
	return ret;
}

int player_midistream::settime(int tm)
{
	if (!paused)
	{
		if (trd_id==GetCurrentThreadId())
		{
			seek_to=tm<<3;
			got_eof=0;
			tm_ofs=GET_TIME-tm;
			midiStreamStop(hMo);
			midiStreamPause(hMo);
			quitting=1;
			do_messages(wnd,(bool*)&in_mm);
			quitting=0;
			do_bufs();
			midiStreamRestart(hMo);
		}
		else PostMessage(wnd,WM_SEEK,0,tm);
	}
	else
	{
		p_time=tm;
	}
	return 1;
}

void player_midistream::pause()
{
	p_time=GET_TIME-tm_ofs;
	paused=1;
	midiStreamPause(hMo);
}

void player_midistream::unpause()
{
	tm_ofs=GET_TIME-p_time;
	paused=0;
	if (seek_to!=-1)
	{
		midiStreamStop(hMo);
		midiStreamPause(hMo);
		if (trd_id==GetCurrentThreadId())
		{
			quitting=1;
			do_messages(wnd,(bool*)&in_mm);
			quitting=0;
			do_bufs();
		}
	}
	midiStreamRestart(hMo);	
}




UINT midiout_volctrl::map_vol(UINT volume,UINT scale)
{
	double _vol=volume>0 ? 20*log10((double)volume/(double)scale) : -60.0;//in negative db
	_vol=_vol/60.0+1;
	if (_vol<0) _vol=0;
	return (UINT)(_vol*(double)scale);
}


void midiout_volctrl::_setvol()
{
	DWORD _vol=257*vol;
	DWORD vol1=_vol,vol2=_vol;
	if (pan!=666)
	{
		if (pan<0)
		{
			vol2=(vol2*(128+pan))>>7;
		}
		else if (pan>0)
		{
			vol1=(vol1*(128-pan))>>7;
		}
	}
	if (cfg_logvol)
	{
		vol1=map_vol(vol1,0xFFFF);
		vol2=map_vol(vol2,0xFFFF);
	}
	midiOutSetVolume((HMIDIOUT)hMo,(vol2<<16)|vol1);
}

void midiout_volctrl::volctrl_init(HMIDIOUT _hMo,MIDI_device_midiout * dev)
{
	hMo=_hMo;
	pan=(dev->get_flags()&MIDICAPS_LRVOLUME) ? MIDI_core::player_getPan() : 666;
	vol=(dev->get_flags()&MIDICAPS_VOLUME) ? MIDI_core::player_getVol() : 666;
	_setvol();
}

int midiout_volctrl::volctrl_setvol(int _vol)
{
	if (vol!=666)
	{
		vol=_vol;
		_setvol();
		return 1;
	}
	else return 0;
}

int midiout_volctrl::volctrl_setpan(int _pan)
{
	if (pan!=666)
	{
		pan=_pan;
		_setvol();
		return 1;
	}
	else return 0;
}