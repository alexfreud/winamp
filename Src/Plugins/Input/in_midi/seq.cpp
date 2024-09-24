#include "main.h"
#include "seq.h"
#include <commctrl.h>
#include <math.h>
#include "resource.h"

#ifdef SEQ_HAVE_PANEL

cfg_int cfg_seq_showpanel("seq_showpanel",0);

enum
{
	ID_BASE   = 0x6543,
	MUTE_ID   = ID_BASE,
	VOL_ID    = MUTE_ID+16,
	INS_ID_P  = VOL_ID+16,
	INS_ID_B1 = INS_ID_P+16,
	INS_ID_B2 = INS_ID_B1+16,
	SPIN_ID   = INS_ID_B2

};

static cfg_int cfg_ctrl_min("ctrl_min",0);


static float g_tempo=1;
static BOOL g_novol,g_noins;
static char sysex1[256],sysex2[256];

extern BYTE d_GMReset[6];
extern BYTE d_XGReset[9];
extern BYTE d_GSReset[11];
#endif

#define SEND_MSG(X) seq_shortmsg(preprocess(X))

#define _sysex(A,B) seq_sysex(A,B)
#define rsysex(A) seq_sysex(A,sizeof(A))

#ifdef SEQ_HAVE_PANEL
void seq_base::set_mute(UINT ch,BOOL st)
{
	if (st)
	{
		mute_mask|=1<<ch;
		seq_shortmsg(0x07B0|ch);
	}
	else
	{
		mute_mask&=~(1<<ch);
		SEND_MSG(((DWORD)ctrl_tab[ch][7]<<16)|0x07B0|ch);
	}
}
#endif

//debug hack
#if 0
#define timeGetTime timehack
static DWORD timehack()
{
	static DWORD t;
	return t++;
}
#endif

DWORD seq_base::get_time()
{
#ifndef SEQ_HAVE_PANEL
	return timeGetTime()<<3;
#else
	if (!hCtrl) return timeGetTime()<<3;//*8;
	EnterCriticalSection(&tm_sec);
	DWORD cur_t=timeGetTime();
	if (!last_time_ms) last_time_ms=cur_t;
	int d=cur_t-last_time_ms;
	if (d<0) d=0;
	last_time_ret+=(double)(d*8.0)*tempo;

	last_time_ms=cur_t;
	DWORD r=(DWORD)last_time_ret;
	LeaveCriticalSection(&tm_sec);
	return r;
#endif
}

#ifdef SEQ_HAVE_PANEL
BOOL CALLBACK seq_base::CtrlProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	seq_base* s;
	if (msg==WM_INITDIALOG)
	{
		SetWindowLongPtr(wnd,DWLP_USER,lp);
		s=(seq_base*)lp;
		if (s) s->hCtrl=wnd;
	}
	else
	{
#if defined(_WIN64)
		s = (seq_base*)GetWindowLong(wnd, DWLP_USER);
#else
		s = (seq_base*)GetWindowLong(wnd, DWL_USER);
#endif
	}
	if (s)
	{
		s->do_msg(msg,wp,lp);
	}
	return 0;
}

static float ui2tempo(int x)
{
	return (float)pow(4.0,0.02*(float)(x-50));
}

static int tempo2ui(float x)
{
	return 50+(int) ((50.0 / log(4.0)) * log(x)  );
}

static void do_ttext(HWND w,float t)
{
	char tx[32] = {0};
	_itoa((UINT)(t*100.0),tx,10);
	char* p=tx;
	while(p && *p) p++;
	*(p++)='%';
	*p=0;
	SetDlgItemTextA(w,IDC_TDISP,tx);
}

BYTE* read_sysex_edit(HWND w,UINT *siz);

void CreateControl(DWORD ex,HWND hCtrl,const char * cls,const char * name,DWORD style,UINT x,UINT y,UINT dx,UINT dy,HINSTANCE hDll,UINT id)
{
	RECT r={(LONG)x,(LONG)y,(LONG)(x+dx),(LONG)(y+dy)};
	MapDialogRect(hCtrl,&r);
	HWND w = CreateWindowExA( ex, cls, name, WS_CHILD | WS_VISIBLE | style, r.left, r.top, r.right - r.left, r.bottom - r.top, hCtrl, 0, hDll, 0 );  // Must stay in ANSI
	if (w)
	{
		if (id) SetWindowLong(w,GWL_ID,id);
		SendMessage(w,WM_SETFONT,SendMessage(hCtrl,WM_GETFONT,0,0),MAKELONG(0,0));
	}
}

static cfg_int cfg_ctrl_x("ctrl_x",0x80000000),cfg_ctrl_y("ctrl_y",0x80000000);

void seq_base::do_msg(UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_CLOSE:
		ShowWindow(hCtrl,SW_SHOWMINIMIZED);
		break;
	case WM_INITDIALOG:
		{
			HINSTANCE hCCdll=GetModuleHandle(TEXT("comctl32.dll"));
			UINT n;
			HWND w;
			for(n=0;n<16;n++)
			{
				char tmp[16] = {0};
				itoa(n,tmp,10);
				CreateControl(0,hCtrl,TRACKBAR_CLASSA,0,TBS_VERT | TBS_BOTH | TBS_NOTICKS | WS_TABSTOP,40+n*28,36,18,80,hCCdll,VOL_ID+n);
				CreateControl(0,hCtrl,"STATIC",tmp,0,46+n*28,25,8,8,0,0);
				CreateControl(0,hCtrl,"Button",0,BS_AUTOCHECKBOX | WS_TABSTOP,43+28*n,120,9,8,0,MUTE_ID+n);
				CreateControl(WS_EX_CLIENTEDGE,hCtrl,"EDIT",0,ES_AUTOHSCROLL | ES_NUMBER,36+28*n,138,26,12,0,INS_ID_P+n);
				CreateControl(0,hCtrl,"msctls_updown32",0,UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS,0,0,0,0,0,SPIN_ID+n);
				CreateControl(WS_EX_CLIENTEDGE,hCtrl,"EDIT",0,ES_AUTOHSCROLL | ES_NUMBER,36+28*n,150,26,12,0,INS_ID_B1+n);
				CreateControl(0,hCtrl,"msctls_updown32",0,UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS,0,0,0,0,0,SPIN_ID+n+16);
				CreateControl(WS_EX_CLIENTEDGE,hCtrl,"EDIT",0,ES_AUTOHSCROLL | ES_NUMBER,36+28*n,162,26,12,0,INS_ID_B2+n);
				CreateControl(0,hCtrl,"msctls_updown32",0,UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS,0,0,0,0,0,SPIN_ID+n+32);
			}

			w=GetDlgItem(hCtrl,IDC_TEMPO);
			SendMessage(w,TBM_SETRANGE,0,MAKELONG(0,100));
			SendMessage(w,TBM_SETPOS,1,tempo2ui(tempo));
			do_ttext(hCtrl,tempo);
			if (cfg_ctrl_x!=0x80000000 && cfg_ctrl_y!=0x80000000)
			{
				int max_x=GetSystemMetrics(SM_CXSCREEN)-10,max_y=GetSystemMetrics(SM_CYSCREEN)-10;
				if (cfg_ctrl_x>max_x) cfg_ctrl_x=max_x;
				if (cfg_ctrl_y>max_y) cfg_ctrl_y=max_y;
				SetWindowPos(hCtrl,0,cfg_ctrl_x,cfg_ctrl_y,0,0,SWP_NOZORDER|SWP_NOSIZE);
			}
			for(n=0;n<16;n++)
			{
				w=GetDlgItem(hCtrl,VOL_ID+n);
				SendMessage(w,TBM_SETRANGE,1,MAKELONG(0,0x7f));
				SendMessage(w,TBM_SETPOS,1,0x7f-90);
			}
			SendDlgItemMessage(hCtrl,IDC_NOVOL,BM_SETCHECK,novol,0);
			SetDlgItemTextA(hCtrl,IDC_SYSEX1,sysex1);
			SetDlgItemTextA(hCtrl,IDC_SYSEX2,sysex2);
			for(n=0;n<48;n++)
			{
				SendDlgItemMessage(hCtrl,SPIN_ID+n,UDM_SETRANGE,0,MAKELONG(127,0));
			}
			for(n=0;n<16;n++)
			{
				SendDlgItemMessage(hCtrl,INS_ID_P+n,EM_LIMITTEXT,3,0);
				SendDlgItemMessage(hCtrl,INS_ID_B1+n,EM_LIMITTEXT,3,0);
				SendDlgItemMessage(hCtrl,INS_ID_B2+n,EM_LIMITTEXT,3,0);
			}
			initialized=1;
		}		
		break;
	case WM_COMMAND:
		{
			UINT n;
			if (HIWORD(wp)==0)
			{
				if (wp==IDC_SYSEX1_SEND || wp==IDC_SYSEX2_SEND)
				{
					UINT sl;
					BYTE* s=read_sysex_edit(GetDlgItem(hCtrl,(wp==IDC_SYSEX1_SEND)?IDC_SYSEX1:IDC_SYSEX2) , &sl);
					if (s)
					{
						_sysex(s,sl);
						free(s);
					}
				}
				else if (wp==IDC_NOVOL)
				{
					novol=SendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				else if (wp==IDC_NOINS)
				{
					noins=SendMessage((HWND)lp,BM_GETCHECK,0,0);
				}
				else if (wp==IDC_ALL_ON)
				{
					UINT n;
					for(n=0;n<16;n++)
					{
						if (mute_mask&(1<<n))
						{
							SendDlgItemMessage(hCtrl,MUTE_ID+n,BM_SETCHECK,0,0);
							set_mute(n,0);
						}
					}
				}
				else if (wp==IDC_ALL_OFF)
				{
					UINT n;
					for(n=0;n<16;n++)
					{
						if (!(mute_mask&(1<<n)))
						{
							SendDlgItemMessage(hCtrl,MUTE_ID+n,BM_SETCHECK,1,0);
							set_mute(n,1);
						}
					}
				}
				else if (wp==IDC_GMRESET) 
				{
					rsysex(d_GMReset);
				}
				else if (wp==IDC_GSRESET)
				{
					rsysex(d_GSReset);
				}
				else if (wp==IDC_XGRESET)
				{
					rsysex(d_XGReset);
				}
				else for(n=0;n<16;n++)
				{
					if (wp==MUTE_ID+n)
					{
						set_mute(n,SendMessage((HWND)lp,BM_GETCHECK,0,0));
						break;
					}
				}
			}
			else if (HIWORD(wp)==EN_CHANGE)
			{
				if (initialized)
				{
					wp&=0xFFFF;
					UINT n;
					for(n=0;n<16;n++)
					{
						if (wp==INS_ID_P+n)
						{
							UINT p=GetDlgItemInt(hCtrl,wp,0,0)&0x7F;
							if (p!=ins_tab[n])
							{
								ins_tab[n]=p;
								SEND_MSG(0xC0|n|(p<<8));
							}
							break;
						}
						else if (wp==INS_ID_B1+n)
						{
							UINT p=GetDlgItemInt(hCtrl,wp,0,0)&0x7F;
							if (p!=ctrl_tab[n][0])
							{
								ctrl_tab[n][0]=p;
								SEND_MSG(0xB0|n|(p<<16));
								SEND_MSG(0xC0|n|(ins_tab[n]<<8));
							}
							break; 
						}
						else if (wp==INS_ID_B2+n)
						{
							UINT p=GetDlgItemInt(hCtrl,wp,0,0)&0x7F;
							if (p!=ctrl_tab[n][0x20])
							{
								ctrl_tab[n][0x20]=p;
								SEND_MSG(0x20B0|n|(p<<16));
								SEND_MSG(0xC0|n|(ins_tab[n]<<8));
							}
							break;
						}
					}
				}
			}

		}
		break;
	case WM_VSCROLL:
		{
			HWND sb=(HWND)lp;
			if (sb)
			{
				UINT id=GetWindowLong(sb,GWL_ID);
				UINT n;
				for(n=0;n<16;n++)
				{
					if (id==VOL_ID+n)
					{
						UINT val=0x7f-SendMessage(sb,TBM_GETPOS,0,0);
						ctrl_tab[n][7]=val;
						SEND_MSG(0x7B0|n|(val<<16));
						break;
					}

				}
			}
		}
		break;
	case WM_HSCROLL:
		tempo=ui2tempo(SendDlgItemMessage(hCtrl,IDC_TEMPO,TBM_GETPOS,0,0));
		do_ttext(hCtrl,tempo);
		break;
	}
}
#endif

seq_base::~seq_base()
{
#ifdef SEQ_HAVE_PANEL
	if (hCtrl)
	{
		cfg_ctrl_min=!!IsIconic(hCtrl);
		RECT r;
		GetWindowRect(hCtrl,&r);
		cfg_ctrl_x=r.left;
		cfg_ctrl_y=r.top;
		GetDlgItemTextA(hCtrl,IDC_SYSEX1,sysex1,256);
		GetDlgItemTextA(hCtrl,IDC_SYSEX2,sysex2,256);
		DestroyWindow(hCtrl);
		DeleteCriticalSection(&tm_sec);
	}
	g_tempo=tempo;
	g_novol=novol;
	g_noins=noins;
#endif
	if (events) free(events);
}

seq_base::seq_base()
{
	mf=0;

	kill=0;paused=0;
	smap=0;
	
	pan=0;vol=0;

	seek_to=0;
	n_events=0;
	events=0;

	c_loop=0;
	loop_start=0;
	memset(&notes,0,sizeof(notes));
	memset(&ctrl_tab,0,sizeof(ctrl_tab));
	memset(&ins_tab,0,sizeof(ins_tab));
	
	tm_ofs=0;
	p_time=0;
	hTrd=0;

	ins_set=0;

#ifdef SEQ_HAVE_PANEL
	hCtrl=0;

	tempo=g_tempo;
	novol=g_novol;
	noins=g_noins;

	last_time_ms=0;
	last_time_ret=0;

	mute_mask=0;
	initialized=0;
#endif
}


#define GET_TIME get_time()//timeGetTime()

int IS_SPEC_C(int x) {return (x>=0x60 && x<=0x65) || x==6 || x==26 || x>=120;}

#define n_sysex smap->pos


DWORD seq_base::preprocess(DWORD e)
{
	BYTE t=(BYTE)(e&0xF0);
	if (t==0xB0)
	{
		UINT v=(e>>16)&0xFF;
		BYTE c=(BYTE)(e>>8);
#ifdef SEQ_HAVE_PANEL
		if (c==7)
		{
			if (mute_mask&(1<<(e&0xF))) v=0;
		}
#endif
		e=(e&0xFFFF)|((v&0xFF)<<16);
	}
	else if (t==0xC0)
	{
		ins_set|=1<<(e&0xF);
	}
	return e;
}

void seq_base::send_sysex(int n)
{
#ifdef USE_LOG
	log_write("send_sysex()");
#endif
	if (!smap || n>=n_sysex) return;
	_sysex(smap->data+smap->events[n].ofs,smap->events[n].len);
}

/*
void seq_base::reset_ins()
{
	UINT n;
	for(n=0;n<16;n++)
	{
		cb->shortmsg(0xC0|n);
	}
}
*/

BOOL seq_base::do_ctrl(DWORD e)
{
	BYTE tp=(BYTE)(e&0xF0);
	BYTE ch=(BYTE)(e&0x0F);
	if (tp==0xC0)
	{
#ifdef SEQ_HAVE_PANEL
		if (noins) return 0;
#endif
		//if (!cfg_fctrl && (e>>8)==ins_tab[e&0xF]) return 0;
		UINT val=e>>8;
		ins_tab[ch]=val;
#ifdef SEQ_HAVE_PANEL
		if (hCtrl) SetDlgItemInt(hCtrl,INS_ID_P+ch,val,0);
#endif
	} else if (tp==0xB0)
	{
		UINT cn = (e>>8)&0x7F;
		UINT val= (e>>16)&0x7F;
#ifdef SEQ_HAVE_PANEL
		if (cn==0)
		{
			if (noins) return 0;
			if (hCtrl) SetDlgItemInt(hCtrl,INS_ID_B1+ch,val,0);
		}
		else if (cn==0x20)
		{
			if (noins) return 0;
			if (hCtrl) SetDlgItemInt(hCtrl,INS_ID_B2+ch,val,0);
		}
		else if (cn==7)
		{
			if (novol) return 0;
			if (hCtrl) PostMessage(GetDlgItem(hCtrl,VOL_ID+(e&0xF)),TBM_SETPOS,1,0x7F-val);
		}
		else if (cn==0x27)
		{
			if (novol) return 0;
		}
#endif
		if (!IS_SPEC_C(cn)) ctrl_tab[e&0xF][cn]=val;
	}
	else if (tp==0x90)
	{
		if (!(ins_set&(1<<ch)))
		{
			SEND_MSG(0xC0|ch);
		}
	}
	return 1;
}

void seq_base::reset()
{
	int not,ch;
	for(ch=0;ch<16;ch++)
	{
		if (ctrl_tab[ch][0x40])
		{
			seq_shortmsg(0x40B0|ch);
			ctrl_tab[ch][0x40]=0;
		}
		if (ch==9) continue;
		for(not=0;not<128;not++)
		{
			if (note_state(ch,not))
			{
				seq_shortmsg((not<<8)|0x80|ch);
				note_off(ch,not);
			}
		}
	}
}


int seq_base::note_state(int ch,int note)
{
	UINT pos=(ch<<7)+note;
	return notes[pos>>3]&(1<<(pos&0x7));
}

void seq_base::note_on(int ch,int note)
{
	UINT pos=(ch<<7)+note;
	notes[pos>>3]|=(1<<(pos&0x7));
}

void seq_base::note_off(int ch,int note)
{
	UINT pos=(ch<<7)+note;
	notes[pos>>3]&=~(1<<(pos&0x7));
}

UINT seq_base::do_seek(DWORD n,DWORD p)
{
	UINT m,c;
	BYTE _ctrl_tab[16][128] = {0};
	BYTE _ins_tab[16] = {0};
	memcpy(_ctrl_tab,ctrl_tab,sizeof(_ctrl_tab));
	memcpy(_ins_tab,ins_tab,sizeof(_ins_tab));

	if (n==0)
	{
		memset(ins_tab,0,sizeof(ins_tab));
		for(m=0;m<16;m++)
		{
			_ctrl_tab[m][0]=_ctrl_tab[m][0x20]=0;
		}
	}

	while(n<n_events && p>events[n].tm)
	{
		DWORD e=events[n].ev;
		if (!(e&0x80000000))
		{
			if (do_ctrl(e))
			{
				if (((e&0xF0)==0xB0) && IS_SPEC_C((e>>8)&0xFF))
				{
					seq_shortmsg(e);
				}
			}
		}
		n++;
	}
	for(c=0;c<16;c++)
	{
		for(m=0;m<128;m++)
		{
			if (!IS_SPEC_C(m) && _ctrl_tab[c][m]!=ctrl_tab[c][m])
			{
				SEND_MSG(((DWORD)ctrl_tab[c][m]<<16)|(m<<8)|0xB0|c);
			}
		}
		if (_ins_tab[c]!=ins_tab[c])
		{
			SEND_MSG(((DWORD)ins_tab[c]<<8)|0xC0|c);
		}
	}
	return n;
}

DWORD WINAPI seq_base::seq_trd(void* p)
{
	((seq_base*)p)->thread();
	return 0;
}

void seq_base::sysexfunc(seq_base* cb,BYTE* s,UINT sz)
{
	cb->seq_sysex(s,sz);
}

void seq_base::thread()
{
	tm_ofs=-1;
	if (seq_play_start())
	{

		sysex_startup((SYSEXFUNC)sysexfunc,this);

		tm_ofs=GET_TIME;
		DWORD pos=0;
		while(!kill)
		{
			DWORD c_t=GET_TIME-tm_ofs;
			if (paused)
			{
				reset();
				while(paused && !kill) MIDI_callback::Idle();
				if (kill) break;
				tm_ofs=GET_TIME-c_t;
			}

			if (seek_to!=-1)
			{
_seek:
				DWORD _p=seek_to > c_t ? pos : 0;
				c_t=seek_to;
				seek_to=-1;
				tm_ofs=GET_TIME-c_t;
				reset();
				pos=c_t ? do_seek(_p,c_t) : 0;
			}
			if (events[pos].tm+1600 < c_t)
			{
				reset();
				pos=do_seek(pos,c_t);
			}
			while(pos<n_events && events[pos].tm<=c_t && !kill)
			{
				DWORD e=events[pos++].ev;
				if (e)
				{
					if (e&0x80000000)
					{
						send_sysex(e&0x7FFFFFFF);
					}
					else
					{
						if ((e&0xF0)==0x90)
						{
							note_on(e&0xf,(e>>8)&0xFF);
						}
						else if ((e&0xF0)==0x80)
						{
							note_off(e&0xf,(e>>8)&0xFF);
						}
						if (do_ctrl(e))
							SEND_MSG(e);
					}
				}
			}

			if (pos>=n_events || c_t >= events[n_events-1].tm)
			{
				if (loop_start!=-1 && (--c_loop))
				{
					c_t=loop_start;
					tm_ofs=GET_TIME-c_t;
					pos=do_seek(0,c_t);
					continue;
				}
				if (cfg_eof_delay)
				{
					DWORD t=timeGetTime();
					do
					{
						MIDI_callback::Idle();
					} while(!kill && seek_to==-1 && t+cfg_eof_delay>timeGetTime());
					if (seek_to!=-1) {
						pos=0;
						goto _seek;
					}
				}
				if (!kill) MIDI_core::Eof();
				break;
			}
			if (kill) break;
			
			MIDI_callback::Idle();
		}
		reset();
	}
	seq_play_stop();
}

int seq_base::gettime()
{
	if (paused)
		return (seek_to==-1) ? seek_to>>3 : p_time;
	else
		 return (GET_TIME-tm_ofs)>>3;
}

int seq_base::settime(int tm)
{
	seek_to=tm<<3;
	return 1;
}


void seq_base::pause()
{
	paused=1;
	p_time=GET_TIME-tm_ofs;
}

void seq_base::unpause()
{
	paused=0;
}

int seq_base::seq_cmd_start(DWORD cflags)
{
	mf=MIDI_core::getFile();
#ifdef SEQ_HAVE_PANEL
	mute_mask=0;
#endif
	c_loop=cfg_loop_infinite ? -1 : cfg_loop_count;
	memset(notes,0,sizeof(notes));
	memset(ctrl_tab,-1,sizeof(ctrl_tab));
	memset(ins_tab,0,sizeof(ins_tab));

	UINT n;
	for(n=0;n<16;n++) ctrl_tab[n][7]=90;

	events=do_table(mf,8,&n_events,&loop_start,cflags);
	if (!events) return 0;

	if (!cfg_nosysex && mf->smap && mf->smap->pos)
	{
		smap=mf->smap;
	}
	else smap=0;

	kill=0;
	seek_to=-1;
	paused=0;

#ifdef SEQ_HAVE_PANEL
	if (cfg_seq_showpanel)
	{
		InitializeCriticalSection(&tm_sec);
		WASABI_API_CREATEDIALOGPARAMW(IDD_EXT_IMM, MIDI_callback::GetMainWindow(), CtrlProc, (LPARAM)this);
		ShowWindow(hCtrl,cfg_ctrl_min ? SW_SHOWMINIMIZED : SW_SHOW);
	}
	else
	{
		tempo=1;
		novol=0;
		noins=0;
	}
#endif
	
	DWORD id;
	hTrd=CreateThread(0,0,seq_trd,this,CREATE_SUSPENDED,&id);
#ifndef _DEBUG
	SetThreadPriority(hTrd,THREAD_PRIORITY_TIME_CRITICAL);
#endif
	ResumeThread(hTrd);
	return 1;
}

void seq_base::seq_cmd_stop()
{
#ifdef USE_LOG
	log_write("stopping sequencer");
#endif
	if (hTrd)
	{
#ifdef USE_LOG
		log_write("killing thread");
#endif
		kill=1;
		if (WaitForSingleObject(hTrd,4000)!=WAIT_OBJECT_0)
		{
#ifdef USE_LOG
			log_write("unable to kill thread");
#endif
			TerminateThread(hTrd,0);
		}
#ifdef USE_LOG
		else log_write("thread killed normally");
#endif
		CloseHandle(hTrd);
	}
}
/*
void seq_base::enum_ins()
{
	DWORD ttab[256];
	memset(ttab,-1,sizeof(ttab));
	UINT tpt=0;
	UINT n;
	DWORD c_ins[16];
	memset(c_ins,0,sizeof(c_ins));
	c_ins[9]=0x80000000;
	for(n=0;n<n_events;n++)
	{
		DWORD t=events[n].ev;
		if (t&0xFF000000) continue;
		UINT c=t&0xF0;
		UINT ch=events[n].ev&0xF;
		if ((t&0xFFF0)==0x20B0)
		{
			c_ins[ch]=(c_ins[ch]&0xFFFF00FF)|((t>>8)&0xFF00);
		}
		else if ((t&0xFFF0)==0xB0)
		{
			c_ins[ch]=(c_ins[ch]&0xFF00FFFF)|(t&0xFF0000);
		}
		else if ((t&0xF0)==0xC0)
		{
			c_ins[ch]=(c_ins[ch]&0xFFFFFF00)|((t>>8)&0xFF);
		}
		else if ((t&0xF0)==0x90)
		{
			UINT n;
			for(n=0;n<256;n++)
			{
				if (ttab[n]==c_ins[ch]) goto ok;
			}
			cb->enum_ins(ttab[tpt]=c_ins[ch]);
			tpt=(tpt+1)&0xFF;
ok:;
		}		
	}
}
*/
