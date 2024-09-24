#include "../Agave/Language/api_language.h"
#include "main.h"
#include <math.h>
#include "resource.h"
#include "../Winamp/in2.h"
#include "../Winamp/wa_ipc.h"

extern In_Module mod;
void get_temp_file(char* fn)
{
	static char tmp_path[MAX_PATH];
	if (!tmp_path[0]) GetTempPathA(MAX_PATH,tmp_path);
	static DWORD num;
	if (num==0) num=GetTickCount();
	wsprintfA(fn,"%sasdf%x.tmp",tmp_path,num++);
}

void file2title(const char* f,string& t)
{
	const char* p1=strrchr(f,'\\'),*p2=strrchr(f,':'),*p3=strrchr(f,'/');
	if (p2>p1) p1=p2;
	if (p3>p1) p1=p3;
	if (p1) p1++;
	else p1=(char*)f;
	t=p1;
	p1=strrchr(t,'.');
	if (p1) t.truncate(p1-(const char*)t);
}

static char* exts[]={"MID","MIDI","RMI","KAR","HMP","HMI","XMI","MSS","MUS","CMF","GMD","MIDS","MIZ","HMZ"};
#define N_EXTS tabsize(exts)
static char is_def[N_EXTS]={1,1,1,1,0,0,0,0,0,0,0,0,1,0};

static int get_def_exts()
{
	int ret=0;
	int n;
	for(n=0;n<N_EXTS;n++)
	{
		if (is_def[n]) ret|=1<<n;
	}
	return ret;
}

cfg_int cfg_ext_mask("ext_mask",get_def_exts());

static char d_smf[128];
static char d_clo[128];
static char d_cmp[128];

int ext_descs[N_EXTS]={STRING_FILES_SMF,STRING_FILES_SMF,STRING_FILES_SMF,STRING_FILES_SMF,STRING_FILES_CLONE,STRING_FILES_CLONE,STRING_FILES_CLONE,STRING_FILES_CLONE,STRING_FILES_CLONE,STRING_FILES_CLONE,STRING_FILES_CLONE,STRING_FILES_CLONE,STRING_FILES_COMPRESSED,STRING_FILES_COMPRESSED};

int MIDI_core::FileTypes_GetNum() {return N_EXTS;}
const char * MIDI_core::FileTypes_GetExtension(int n) {return exts[n];}
char * MIDI_core::FileTypes_GetDescription(int n) {
	char* s = d_smf;
	if(ext_descs[n] == STRING_FILES_SMF) {
		if(!d_smf[0]) {
			WASABI_API_LNGSTRING_BUF(ext_descs[n],d_smf,128);
		}
		s = d_smf;
	}
	else if(ext_descs[n] == STRING_FILES_CLONE) {
		if(!d_clo[0]) {
			WASABI_API_LNGSTRING_BUF(ext_descs[n],d_clo,128);
		}
		s = d_clo;
	}
	else if(ext_descs[n] == STRING_FILES_COMPRESSED) {
		if(!d_cmp[0]) {
			WASABI_API_LNGSTRING_BUF(ext_descs[n],d_cmp,128);
		}
		s = d_cmp;
	}
	return s;
}

static int isourext(const char* ext)
{
	UINT n;
	for(n=0;n<N_EXTS;n++)
	{
		if ((cfg_ext_mask&(1<<n)) && !_stricmp(ext,exts[n])) return 1;
	}
	return 0;
}

int MIDI_core::IsOurFile(const char *fn) 
{
	const char* p=strrchr(fn,'.');
	if (p)
	{
		if (isourext(p+1)) return 1;
	}

	return 0;
}

extern UINT volmode_detect();

static BOOL CALLBACK KarProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);

int MIDI_core::Init()
{
	theFile=0;
	data_src=0;
	plr=0;
	eof=0;
	mix_dev=0;mix_idx=0;
	kwnd=0;
	kmap=0;
	kmap_size=0;kmap_ptr=0;
	kmap_data=0;
	format_srate=0;format_nch=0;format_bps=0;

	device=MIDI_driver::find_device(cfg_driver,cfg_device);
	if (!device) return 0;
	use_out=device->has_output() || (cfg_smp && cfg_sampout);
	use_smp=cfg_smp && !device->has_output();

	if (cfg_volmode>2) volmod=cfg_volmode-1;
	else if (cfg_volmode==2) volmod = device->volctrl_happy() ? 1 : volmode_detect()+2;
	else volmod=cfg_volmode;

	if (volmod>1)
	{
		UINT idx=volmod-2;
		UINT id=0;
		UINT n_devz=mixerGetNumDevs();
		UINT dev=0;
		BOOL found=0;
		MIXERLINE ml;
		while(dev<n_devz)
		{
			mixerGetID((HMIXEROBJ)dev,&id,MIXER_OBJECTF_MIXER);

			ZeroMemory(&ml,sizeof(ml));
			ml.cbStruct=sizeof(ml);
			ml.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

			mixerGetLineInfo((HMIXEROBJ)id,&ml,MIXER_GETLINEINFOF_COMPONENTTYPE|MIXER_OBJECTF_MIXER);

			if (idx<ml.cConnections)
			{
				found=1;
				break;
			}
			idx-=ml.cConnections;
			dev++;
		}
		if (found)
		{
			mix_dev=id;
			mix_idx=idx;
		}
		else
		{
			volmod=0;
		}
	}
	return 1;
}

CStream * sampling_create(int srate,int nch,int bps);

cfg_int cfg_lyrics("lyrics",1);

int MIDI_core::OpenFile(MIDI_file * file)
{

#ifdef USE_LOG
	log_write("MIDI_core::Open()");
#endif

	if (!file) return 0;

	format_srate=device->has_freq() ? cfg_freq : 44100;
	format_bps=16;
	format_nch=2;



	theFile=file->AddRef();
#ifdef USE_LOG
	log_write("file loaded");
#endif
	plr=0;


	if (use_smp)
	{
#ifdef USE_LOG
		log_write("starting sampling");
#endif
		format_srate=cfg_wavein_sr;
		format_bps=cfg_wavein_bps;
		format_nch=cfg_wavein_ch;
		data_src=sampling_create(format_srate,format_nch,format_bps);
	}

	plr=device->create();

	if (plr)
	{
#ifdef USE_LOG
		if (data_src) log_write("got PCM data source");

		log_write("playback started");
#endif

    if (cfg_lyrics)
		{
			kmap=kmap_create(theFile,1,&kmap_size,&kmap_data);
			if (kmap)
			{
				kwnd=WASABI_API_CREATEDIALOGPARAMW(IDD_LYRICS,MIDI_callback::GetMainWindow(),KarProc,0);
				free(kmap_data); kmap_data=0;//not needed anymore, used only on initdialog to setdlgitemtext
			}
		}

		return 1;
	}
	else
  {
		if (data_src) {delete data_src;data_src=0;}

		theFile->Free();
		theFile=0;

		return 0;
	}
}

int MIDI_core::GetSamples(void *sample_buffer, int bytes, char *killswitch)
{
#ifdef USE_LOG
	log_write("GetSamples");
#endif
	if (data_src)
	{
		return data_src->ReadData(sample_buffer,bytes,(bool*)killswitch);
	}
	else return 0;
}

void MIDI_core::update_vol()
{
	MIXERLINE ml;
	ZeroMemory(&ml,sizeof(ml));
	ml.cbStruct=sizeof(ml);
	ml.dwSource=mix_idx;
	mixerGetLineInfo((HMIXEROBJ)mix_dev,&ml,MIXER_GETLINEINFOF_SOURCE|MIXER_OBJECTF_MIXER);

	MIXERLINECONTROLS cs;
	MIXERCONTROL c;
	ZeroMemory(&cs,sizeof(cs));
	cs.cbStruct=sizeof(cs);
	cs.cControls=1;
	cs.dwLineID=ml.dwLineID;
	cs.dwControlType=MIXERCONTROL_CONTROLTYPE_VOLUME;
	cs.cbmxctrl=sizeof(c);
	cs.pamxctrl=&c;
	ZeroMemory(&c,sizeof(c));
	c.cbStruct=sizeof(c);

	if (!mixerGetLineControls((HMIXEROBJ)mix_dev,&cs,MIXER_OBJECTF_MIXER|MIXER_GETLINECONTROLSF_ONEBYTYPE))
	{
		DWORD val;
		if (cfg_logvol)
		{
			double _vol=volume>0 ? 20*log10((double)volume/255.0) : -60.0;//in negative db
			_vol=_vol/60.0+1;
			if (_vol<0) _vol=0;
			val=c.Bounds.dwMinimum + (int)( _vol * (double)(c.Bounds.dwMaximum-c.Bounds.dwMinimum) );
		}
		else val=c.Bounds.dwMinimum + volume * (c.Bounds.dwMaximum-c.Bounds.dwMinimum) / 255;
		if (ml.cChannels==1)
		{
			MIXERCONTROLDETAILS_UNSIGNED ds={val};
			MIXERCONTROLDETAILS d;
			d.cbStruct=sizeof(d);
			d.dwControlID=c.dwControlID;
			d.cChannels=1;
			d.cMultipleItems=0;
			d.cbDetails=sizeof(ds);
			d.paDetails=&ds;
			mixerSetControlDetails((HMIXEROBJ)mix_dev,&d,MIXER_SETCONTROLDETAILSF_VALUE|MIXER_OBJECTF_MIXER);
		}
		else if (ml.cChannels<16)
		{
			MIXERCONTROLDETAILS_UNSIGNED ds[16];
			UINT n;
			for(n=0;n<16;n++) ds[n].dwValue=val;
			if (pan<0)
			{
				ds[1].dwValue=ds[1].dwValue*(128+pan)>>7;
			}
			else
			{
				ds[0].dwValue=ds[0].dwValue*(128-pan)>>7;
			}
			MIXERCONTROLDETAILS d;
			d.cbStruct=sizeof(d);
			d.dwControlID=c.dwControlID;
			d.cChannels=ml.cChannels;
			d.cMultipleItems=0;
			d.cbDetails=sizeof(ds[0]);
			d.paDetails=&ds;
			mixerSetControlDetails((HMIXEROBJ)mix_dev,&d,MIXER_SETCONTROLDETAILSF_VALUE|MIXER_OBJECTF_MIXER);
		}
	}
/*
	ZeroMemory(&cs,sizeof(cs));
	cs.cbStruct=sizeof(cs);
	cs.cControls=1;
	cs.dwLineID=ml.dwLineID;
	cs.dwControlType=MIXERCONTROL_CONTROLTYPE_PAN;
	cs.cbmxctrl=sizeof(c);
	cs.pamxctrl=&c;
	ZeroMemory(&c,sizeof(c));
	c.cbStruct=sizeof(c);

	if (!mixerGetLineControls((HMIXEROBJ)mix_dev,&cs,MIXER_OBJECTF_MIXER|MIXER_GETLINECONTROLSF_ONEBYTYPE))
	{
		MIXERCONTROLDETAILS_SIGNED ds={c.Bounds.lMinimum + (pan+128) * (c.Bounds.lMaximum-c.Bounds.lMinimum) / 255};
		MIXERCONTROLDETAILS d;
		d.cbStruct=sizeof(d);
		d.dwControlID=c.dwControlID;
		d.cbDetails=sizeof(ds);
		d.cChannels=ml.cChannels;
		d.cMultipleItems=c.cMultipleItems;
		d.paDetails=&ds;
		mixerSetControlDetails((HMIXEROBJ)mix_dev,&d,MIXER_SETCONTROLDETAILSF_VALUE|MIXER_OBJECTF_MIXER);
	}*/
}

int MIDI_core::SetVolume(int _volume)
{
	volume=_volume;
	if (volmod==0) return 0;
	else
	{
		if (volmod==1)
		{
			if ((use_out && !use_smp) || !plr)
			{
				return 0;
			}
			else
			{
				return plr->setvol(player_getVol());
			}
		}
		update_vol();
		return 1;
	}
}

int MIDI_core::SetPan(int _pan)
{
	pan=_pan;
	if (volmod==0) return 0;
	else
	{
		if (volmod==1)
		{
			if (plr) return plr->setpan(player_getPan());
			else return 0;
		}
		else
		{
			update_vol();
			return 1;
		}
	}
}

int MIDI_core::SetPosition(int pos)
{
	if (!plr) return 0;
	if (!plr->settime(pos)) return 0;
	sync.enter();
	kmap_ptr=0;
	LeaveCriticalSection(&sync);

	if (data_src) data_src->Flush();
	return 1;
}

void MIDI_core::Pause(int pause)
{
	if (plr)
	{
		if (pause) plr->pause();
		else plr->unpause();
	}
	if (data_src) data_src->Pause(pause);
}

int MIDI_core::GetPosition(void)
{
	int i=0;
	if (plr)
	{
		i=plr->gettime();
		if (i<0) i=0;
	}
	return i;
}

int MIDI_core::GetLength(void)
{
	if (theFile) return theFile->len;
	else return -1;
}

void MIDI_core::Close()
{
#ifdef USE_LOG
	log_write("shutting down MIDI_core");
#endif
	if (plr) {delete plr;plr=0;}
	if (data_src) {delete data_src;data_src=0;}
	if (kwnd) {DestroyWindow(kwnd);kwnd=0;}
	if (kmap) {free(kmap);kmap=0;}
	if (theFile) {theFile->Free();theFile=0;}
}

void MIDI_core::Eof()
{
	eof=1;
	if (data_src) 
		data_src->Eof();
	else
		MIDI_callback::NotifyEOF();
}


static char INI_FILE[MAX_PATH];

void MIDI_core::GlobalInit()
{
#ifdef USE_LOG
	log_start();
	log_write("initializing");
	log_write(NAME);
#endif

	char *p;
	if (mod.hMainWindow &&
		(p = (char *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE))
		&&  p!= (char *)1)
	{
		strcpy(INI_FILE, p);
	}
	else
	{
		GetModuleFileNameA(NULL,INI_FILE,sizeof(INI_FILE));
		p = INI_FILE + strlen(INI_FILE);
		while (p >= INI_FILE && *p != '.') p--;
		strcpy(++p,"ini");
	}
	cfg_var::config_read(INI_FILE,"in_midi");
}

void MIDI_core::GlobalQuit()
{
	MIDI_driver::shutdown();
	log_quit();
}

void MIDI_core::WriteConfig()
{
	cfg_var::config_write(INI_FILE,"in_midi");
}

void MIDI_core::MM_error(DWORD code)
{
	string temp;
	if (!mciGetErrorStringA(code,string_buffer_a(temp,256),256))
	{
		temp=WASABI_API_LNGSTRING(STRING_UNKNOWN_MMSYSTEM);
	}
	MIDI_callback::Error(temp);

}


static void fix_size(HWND wnd)
{
	RECT r;
	GetClientRect(wnd,&r);
	SetWindowPos(GetDlgItem(wnd,IDC_BLAH),0,0,0,r.right,r.bottom,SWP_NOZORDER|SWP_NOACTIVATE);
}

static cfg_struct_t<RECT> cfg_lyrics_pos("lyrics_pos",-1);

static void SetWindowRect(HWND w,RECT* r)
{
	SetWindowPos(w,0,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER);
}


static cfg_int cfg_lyrics_min("lyrics_min",0),cfg_lyrics_max("lyrics_max",0);

BOOL CALLBACK MIDI_core::KarProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		SetDlgItemTextA(wnd,IDC_BLAH,MIDI_core::kmap_data);

		if (cfg_lyrics_pos.get_val().left!=-1)
		{
			int sx=GetSystemMetrics(SM_CXSCREEN),sy=GetSystemMetrics(SM_CYSCREEN);
			if (cfg_lyrics_pos.get_val().right>sx)
			{
				cfg_lyrics_pos.get_val().left-=cfg_lyrics_pos.get_val().right-sx;
				cfg_lyrics_pos.get_val().right=sx;
			}
			if (cfg_lyrics_pos.get_val().bottom>sy)
			{
				cfg_lyrics_pos.get_val().top-=cfg_lyrics_pos.get_val().bottom-sy;
				cfg_lyrics_pos.get_val().bottom=sy;
			}
			if (cfg_lyrics_pos.get_val().left<0)
			{
				cfg_lyrics_pos.get_val().right-=cfg_lyrics_pos.get_val().left;
				cfg_lyrics_pos.get_val().left=0;
			}
			if (cfg_lyrics_pos.get_val().top<0)
			{
				cfg_lyrics_pos.get_val().bottom-=cfg_lyrics_pos.get_val().top;
				cfg_lyrics_pos.get_val().top=0;
			}
			SetWindowRect(wnd,&cfg_lyrics_pos.get_val());
		}
		if (cfg_lyrics_min)
		{
			ShowWindow(wnd,SW_MINIMIZE);
		}
		else if (cfg_lyrics_max)
		{
			ShowWindow(wnd,SW_MAXIMIZE);
		}
		fix_size(wnd);
		SetTimer(wnd,1,100,0);
		return 1;
	case WM_TIMER:
		{
			sync.enter();
			UINT time=GetPosition();
			KAR_ENTRY * set=0;
			UINT ptr=kmap_ptr;
			while(ptr<kmap_size && kmap[ptr].time<time)
			{
				if (!kmap[ptr].foo) set=&kmap[ptr];
				ptr++;
			}
			kmap_ptr=ptr;
			sync.leave();
			if (set)
			{
				SendDlgItemMessage(wnd,IDC_BLAH,EM_SETSEL,set->start,set->end);
			}

		}
		break;
	case WM_DESTROY:
		KillTimer(wnd,1);
		kwnd=0;
		GetWindowRect(wnd,&cfg_lyrics_pos.get_val());
		cfg_lyrics_max=!!IsZoomed(wnd);
		cfg_lyrics_min=!!IsIconic(wnd);
		break;
	case WM_CLOSE:

		cfg_lyrics=0;

		if (!((int)cfg_bugged & BUGGED_BLAH))
		{
			char title[32] = {0};
			cfg_bugged = (int)cfg_bugged | BUGGED_BLAH;
			MessageBoxA(wnd,WASABI_API_LNGSTRING(IDS_TO_ENABLE_LYRICS_DISPLAY),
					   WASABI_API_LNGSTRING_BUF(IDS_INFORMATION,title,32),MB_ICONINFORMATION);
		}
		DestroyWindow(wnd);
		break;
	case WM_SIZE:
		fix_size(wnd);
		break;
	}
	return 0;
}

//MIDI_core static crap
bool MIDI_core::use_out;
MIDI_file* MIDI_core::theFile;
CStream* MIDI_core::data_src;
player_base* MIDI_core::plr;
int MIDI_core::format_srate,MIDI_core::format_nch,MIDI_core::format_bps;
int MIDI_core::volume=255,MIDI_core::pan=0;
bool MIDI_core::eof;
UINT MIDI_core::volmod;
UINT MIDI_core::mix_dev,MIDI_core::mix_idx;
MIDI_device * MIDI_core::device;
bool MIDI_core::use_smp;
HWND MIDI_core::kwnd;
KAR_ENTRY* MIDI_core::kmap;
UINT MIDI_core::kmap_size,MIDI_core::kmap_ptr;
char * MIDI_core::kmap_data;
critical_section MIDI_core::sync;