#include "main.h"
#include <shlobj.h>
#include <commctrl.h>
#include "resource.h"
#include "../winamp/wa_ipc.h"
#include "../pfc/string_unicode.h"

static HWND g_theConfig;

static struct
{
	HWND wnd;
	int id;
	UINT l_id;
} child_dlgs[]={
	{0,IDD_CONFIG1,IDS_PREFS_DEVICE},
	{0,IDD_CONFIG8,IDS_PREFS_DISPLAY},
	{0,IDD_CONFIG2,IDS_PREFS_SAMPLING},
	{0,IDD_CONFIG3,IDS_PREFS_DIRECTMUSIC},
	{0,IDD_CONFIG4,IDS_PREFS_MISC},
	{0,IDD_CONFIG7,IDS_PREFS_FILE_TYPES},
	{0,IDD_CONFIG5,IDS_PREFS_FILES},
	{0,IDD_CONFIG6,IDS_PREFS_HARDWARE_SETUP},	
};

enum
{
	IDC_CONFIG1,IDC_CONFIG8,IDC_CONFIG2,IDC_CONFIG3,IDC_CONFIG4,IDC_CONFIG7,IDC_CONFIG5,IDC_CONFIG6
};

static const char CFG_NAME[]="IN_MIDI";

#define FILE_BLAH MessageBoxA(wnd,WASABI_API_LNGSTRING(IDS_UNABLE_TO_LOAD_FILE),0,MB_ICONERROR)

int loop_txt[3]={STRING_LOOP1,STRING_LOOP2,STRING_LOOP3};

#define MAKEID(X,Y) ( ( (DWORD)(X)<<16) | (DWORD)(Y))	//MAKEID(DLG_ID,CTRL_ID)

extern cfg_int cfg_seq_showpanel;
extern cfg_int cfg_lyrics;


cfg_int cfg_smp("smp",0),cfg_reverb("reverb",0),cfg_chorus("chorus",0),cfg_dls_active("dls_active",0);

cfg_int cfg_sampout("sampout",0);
cfg_int cfg_hardware_reset("hardware_reset",0);
cfg_int cfg_gm_reset("gm_reset",0),cfg_gm_reset1("gm_reset1",0);
cfg_int cfg_gs_reset("gs_reset",0),cfg_gs_reset1("gs_reset1",0);
cfg_int cfg_nosysex("nosysex",0);
cfg_int cfg_dm_keep_port("dm_keep_port",0);
cfg_int cfg_recover_tracks("recover_tracks",1);
cfg_int cfg_rmi_def("rmi_def",0);
cfg_int cfg_logvol("logvol",1);

cfg_int cfg_ff7loopz("ff7loopz",1);
cfg_int cfg_samp_revert("samp_revert",1);

cfg_int cfg_hack_xg_drums("hack_xg_drums",0),cfg_hack_dls_instruments("hack_dls_instruments",1),cfg_hack_dls_drums("hack_dls_drums",1);

static const WORD sr_tab[]={8000,11025,16000,22050,24000,32000,44100,48000};

cfg_int cfg_cur_tab("cur_tab",0);

cfg_int cfg_volmode("volmode",2);
//0 - no volume, 1 - default, 2,3,4 ... - mixers
cfg_string cfg_extra_exts("extra_exts","");

cfg_int cfg_freq("freq",22050);
cfg_int cfg_eof_delay("eof_delay",0);

cfg_string cfg_dls_file("dls_file","");
cfg_int cfg_bugged("bugged",0);

cfg_int cfg_loop_type("loop_type",0),cfg_loop_count("loop_count",1),cfg_loop_infinite("loop_infinite",0);

cfg_int cfg_wavein_dev("wavein_dev",-1),cfg_wavein_sr("wavein_sr",44100),cfg_wavein_ch("wavein_ch",2),cfg_wavein_bps("wavein_bps",16),cfg_wavein_src("wavein_src",0);
cfg_int cfg_playback_mode("playback_mode",0);


static UINT vm_retval=-666;

cfg_struct_t<GUID> cfg_driver("driver",0),cfg_device("device",0);

UINT volmode_detect()
{
	if (vm_retval!=-666) return vm_retval;
	UINT dev;
	UINT id;
	UINT n_devz=mixerGetNumDevs();
	UINT pos=0;
	for(dev=0;dev<n_devz;dev++)
	{
		mixerGetID((HMIXEROBJ)dev,&id,MIXER_OBJECTF_MIXER);


		MIXERLINE ml;
		memset(&ml,0,sizeof(ml));
		ml.cbStruct=sizeof(ml);
		ml.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

		mixerGetLineInfo((HMIXEROBJ)id,&ml,MIXER_GETLINEINFOF_COMPONENTTYPE|MIXER_OBJECTF_MIXER);

		UINT con;
		for(con=0;con<ml.cConnections;con++)
		{
			MIXERLINE ml1;
			memset(&ml1,0,sizeof(ml1));
			ml1.cbStruct=sizeof(ml);
			ml1.dwSource=con;
			mixerGetLineInfo((HMIXEROBJ)id,&ml1,MIXER_GETLINEINFOF_SOURCE|MIXER_OBJECTF_MIXER);
			if (ml1.dwComponentType==MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER)
			{
				vm_retval=pos;
				return vm_retval;
			}
			pos++;
		}
	}
	vm_retval=-1;
	return vm_retval;
}

//avoid double id shit
#define _IDC_PORT MAKEID(IDC_CONFIG1,IDC_PORT)
#define _IDC_DEV_INFO MAKEID(IDC_CONFIG1,IDC_DEV_INFO)
#define _IDC_HARDWARE_RESET MAKEID(IDC_CONFIG1,IDC_HARDWARE_RESET)
#define _IDC_VOLMODE MAKEID(IDC_CONFIG1,IDC_VOLMODE)
#define _IDC_LOGVOL MAKEID(IDC_CONFIG1,IDC_LOGVOL)
#define _IDC_SAMPLING_ENABLED MAKEID(IDC_CONFIG2,IDC_SAMPLING_ENABLED)
#define _IDC_WAVEIN MAKEID(IDC_CONFIG2,IDC_WAVEIN)
#define _IDC_WAVEIN_SRC MAKEID(IDC_CONFIG2,IDC_WAVEIN_SRC)
#define _IDC_WAVEIN_SR MAKEID(IDC_CONFIG2,IDC_WAVEIN_SR)
#define _IDC_WAVEIN_S2 MAKEID(IDC_CONFIG2,IDC_WAVEIN_S2)
#define _IDC_WAVEIN_CH MAKEID(IDC_CONFIG2,IDC_WAVEIN_CH)
#define _IDC_WAVEIN_BPS MAKEID(IDC_CONFIG2,IDC_WAVEIN_BPS)
#define _IDC_SAMPLING_DSP MAKEID(IDC_CONFIG2,IDC_SAMPLING_DSP)
#define _IDC_SAMPLING_OUTPUT MAKEID(IDC_CONFIG2,IDC_SAMPLING_OUTPUT)
#define _IDC_SAMP_REVERT MAKEID(IDC_CONFIG2,IDC_SAMP_REVERT)
#define _IDC_REVERB MAKEID(IDC_CONFIG3,IDC_REVERB)
#define _IDC_CHORUS MAKEID(IDC_CONFIG3,IDC_CHORUS)
#define _IDC_DM_KEEP_PORT MAKEID(IDC_CONFIG3,IDC_DM_KEEP_PORT)
#define _IDC_FREQ MAKEID(IDC_CONFIG3,IDC_FREQ)
#define _IDC_DLS_CB MAKEID(IDC_CONFIG3,IDC_DLS_CB)
#define _IDC_DLS MAKEID(IDC_CONFIG3,IDC_DLS)
#define _IDC_DLS_B MAKEID(IDC_CONFIG3,IDC_DLS_B)
#define _IDC_HACK_DM_RESETS MAKEID(IDC_CONFIG3,IDC_HACK_DM_RESETS)
#define _IDC_SHOW_PANEL MAKEID(IDC_CONFIG4,IDC_SHOW_PANEL)
#define _IDC_LOOP_S MAKEID(IDC_CONFIG4,IDC_LOOP_S)
#define _IDC_LOOP MAKEID(IDC_CONFIG4,IDC_LOOP)
#define _IDC_LOOP_S1 MAKEID(IDC_CONFIG4,IDC_LOOP_S1)
#define _IDC_LOOP_S2 MAKEID(IDC_CONFIG4,IDC_LOOP_S2)
#define _IDC_LOOP_T MAKEID(IDC_CONFIG4,IDC_LOOP_T)
#define _IDC_LOOP_SP MAKEID(IDC_CONFIG4,IDC_LOOP_SP)
#define _IDC_LOOP_S3 MAKEID(IDC_CONFIG4,IDC_LOOP_S3)
#define _IDC_LOOP_S4 MAKEID(IDC_CONFIG4,IDC_LOOP_S4)
#define _IDC_INFINITE MAKEID(IDC_CONFIG4,IDC_INFINITE)
#define _IDC_PLAYBACK_METHOD MAKEID(IDC_CONFIG4,IDC_PLAYBACK_METHOD)
#define _IDC_RESET MAKEID(IDC_CONFIG4,IDC_RESET)
#define _IDC_RESET1 MAKEID(IDC_CONFIG4,IDC_RESET1)
#define _IDC_STEREO MAKEID(IDC_CONFIG5,IDC_STEREO)
#define _IDC_HACKTRACK MAKEID(IDC_CONFIG5,IDC_HACKTRACK)

#define _IDC_HACK_NO_SYSEX MAKEID(IDC_CONFIG5,IDC_HACK_NO_SYSEX)
#define _IDC_HACK_XG_DRUMS MAKEID(IDC_CONFIG5,IDC_HACK_XG_DRUMS)
#define _IDC_HACK_DLS_DRUMS MAKEID(IDC_CONFIG5,IDC_HACK_DLS_DRUMS)
#define _IDC_HACK_DLS_INSTRUMENTS MAKEID(IDC_CONFIG5,IDC_HACK_DLS_INSTRUMENTS)

#define _IDC_LYRICS_ENABLED MAKEID(IDC_CONFIG8,IDC_LYRICS_ENABLED)
#define _IDC_EOF_DELAY MAKEID(IDC_CONFIG5,IDC_EOF_DELAY)
#define _IDC_EOF_DELAY_SPIN MAKEID(IDC_CONFIG5,IDC_EOF_DELAY_SPIN)

sysex_table cfg_sysex_table;
static sysex_table edit_sysex_table;


#define _IDC_SYSEX_LIST MAKEID(IDC_CONFIG6,IDC_SYSEX_LIST)
#define _IDC_IMP_F MAKEID(IDC_CONFIG6,IDC_IMP_F)
#define _IDC_EXP_F MAKEID(IDC_CONFIG6,IDC_EXP_F)
#define _IDC_IMP_PR MAKEID(IDC_CONFIG6,IDC_IMP_PR)
#define _IDC_EXP_PR MAKEID(IDC_CONFIG6,IDC_EXP_PR)
#define _IDC_SYSEX_ADD MAKEID(IDC_CONFIG6,IDC_SYSEX_ADD)
#define _IDC_SYSEX_DELETE MAKEID(IDC_CONFIG6,IDC_SYSEX_DELETE)
#define _IDC_SYSEX_EDIT MAKEID(IDC_CONFIG6,IDC_SYSEX_EDIT)
#define _IDC_SYSEX_UP MAKEID(IDC_CONFIG6,IDC_SYSEX_UP)
#define _IDC_SYSEX_DOWN MAKEID(IDC_CONFIG6,IDC_SYSEX_DOWN)


#define _IDC_EXTS_LIST MAKEID(IDC_CONFIG7,IDC_EXTS_LIST)
#define _IDC_EXTS_ED MAKEID(IDC_CONFIG7,IDC_EXTS_ED)
#define _IDC_RMI_FMT MAKEID(IDC_CONFIG8,IDC_RMI_FMT)
#define _IDC_RMI_DEF MAKEID(IDC_CONFIG8,IDC_RMI_DEF)


#define EnableDlgItem(X,Y,Z) EnableWindow(_GetDlgItem(X,Y),Z)

static HWND cfgGetTab(UINT id)
{
	id-=IDC_CONFIG1;
	if (id>=tabsize(child_dlgs)) return 0;
	return child_dlgs[id].wnd;
}

static HWND __getdlgitem(UINT id)
{

	HWND w=cfgGetTab(id>>16);
	if (!w) return 0;
	return GetDlgItem(w,id&0xFFFF);
}

#define _GetDlgItem(w,id) __getdlgitem(id)

#define _GetDlgItemText(p,id,tx,l) GetWindowText(__getdlgitem(id),tx,l)
#define _SetDlgItemText(p,id,tx) SetWindowText(__getdlgitem(id),tx)
#define _SetDlgItemTextW(p,id,tx) SetWindowTextW(__getdlgitem(id),tx)
#define _GetDlgItemInt(a,b,c,d) __getdlgitemint(b)
#define _SetDlgItemInt(a,b,c,d) __setdlgitemint(b,c)
#define _SendDlgItemMessage(a,b,c,d,e) SendMessage(_GetDlgItem(a,b),c,d,e)

static void __setdlgitemint(UINT id,UINT val)
{
	char buf[32] = {0};
	_itoa(val,buf,10);
	SetWindowTextA(__getdlgitem(id),buf);
}

static int __getdlgitemint(UINT id)
{
	char buf[32] = {0};
	GetWindowTextA(__getdlgitem(id),buf,32);
	return atoi(buf);
}


static BOOL CALLBACK cfgVisStatusProc(HWND wnd,LPARAM param)
{
	if (GetWindowLong(wnd,GWL_ID)==IDC_SAMPLING_ENABLED)
	{
		EnableWindow(wnd,(param & 2) >> 1);
	}
	else
	{
		EnableWindow(wnd,param & 1);
	}
	return 1;
}

static MIDI_device * get_device(HWND wnd)
{
	int idx = _SendDlgItemMessage(wnd,_IDC_PORT,CB_GETCURSEL,0,0);
	if (idx<0) return 0;
	return (MIDI_device*)_SendDlgItemMessage(wnd,_IDC_PORT,CB_GETITEMDATA,idx,0);
}


static void cfgVisStatus(HWND wnd)
{
	int samp =_SendDlgItemMessage(wnd,_IDC_SAMPLING_ENABLED,BM_GETCHECK,0,0);
	int output = 0;
	int flags = 0;
	MIDI_device * dev = get_device(wnd);
	if (dev && dev->has_output()) output = 1;
	
	if (output && samp) {samp = 0;_SendDlgItemMessage(wnd,_IDC_SAMPLING_ENABLED,BM_SETCHECK,0,0);}
	if (!output)
	{
		flags|=2;
		if (samp) flags|=1;
		else {
			_SendDlgItemMessage(wnd,_IDC_SAMPLING_DSP,BM_SETCHECK,0,0);
			_SendDlgItemMessage(wnd,_IDC_SAMPLING_OUTPUT,BM_SETCHECK,0,0);
		}
	}
	
	EnumChildWindows(cfgGetTab(IDC_CONFIG2),cfgVisStatusProc,flags);
}


static void cfgVisMix(HWND wnd,int dev)
{
	HWND w=_GetDlgItem(wnd,_IDC_WAVEIN_SRC);
	SendMessage(w,CB_RESETCONTENT,0,0);
	SendMessageW(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRINGW(STRING_SAMP_SRC_DEFAULT));
	SendMessage(w,CB_SETCURSEL,0,0);
	UINT id=0;
	
	if (mixerGetID((HMIXEROBJ)dev,&id,MIXER_OBJECTF_WAVEOUT)) return;

	MIXERLINE ml;
	memset(&ml,0,sizeof(ml));
	ml.cbStruct=sizeof(ml);
	ml.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

	if (mixerGetLineInfo((HMIXEROBJ)id,&ml,MIXER_GETLINEINFOF_COMPONENTTYPE|MIXER_OBJECTF_MIXER)) return;

	MIXERLINECONTROLS cs;
	MIXERCONTROL c;
	memset(&cs,0,sizeof(cs));
	cs.cbStruct=sizeof(cs);
	cs.cControls=1;
	cs.dwLineID=ml.dwLineID;
	cs.dwControlType=MIXERCONTROL_CONTROLTYPE_MUX;
	cs.cbmxctrl=sizeof(c);
	cs.pamxctrl=&c;
	memset(&c,0,sizeof(c));
	c.cbStruct=sizeof(c);

	if (!mixerGetLineControls((HMIXEROBJ)id,&cs,MIXER_OBJECTF_MIXER|MIXER_GETLINECONTROLSF_ONEBYTYPE))
	{
		MIXERCONTROLDETAILS_LISTTEXT *t = (MIXERCONTROLDETAILS_LISTTEXT*)malloc(sizeof(MIXERCONTROLDETAILS_LISTTEXT)*c.cMultipleItems);
		MIXERCONTROLDETAILS d;
		d.cbStruct=sizeof(d);
		d.dwControlID=c.dwControlID;
		d.cbDetails=sizeof(*t);
		d.cChannels=ml.cChannels;
		d.cMultipleItems=c.cMultipleItems;
		d.paDetails=t;
		UINT n;
		for(n=0;n<c.cMultipleItems;n++)
		{
			t[n].dwParam1=ml.dwLineID;
		}
		mixerGetControlDetails((HMIXEROBJ)id,&d,MIXER_GETCONTROLDETAILSF_LISTTEXT|MIXER_OBJECTF_MIXER);
		for(n=0;n<c.cMultipleItems;n++)
		{
			SendMessage(w,CB_ADDSTRING,0,(long)t[n].szName);
		}
		free(t);
	}
}

static void ChangePort(HWND wnd,MIDI_device * dev)
{
	cfgVisStatus(wnd);

	{
		string_w temp;
		temp+=WASABI_API_LNGSTRINGW(IDS_TYPE);
		temp+=dev->get_driver()->get_name();
		temp+=L"\x0d\x0a";
		temp+=dev->get_info();
		if (!wcscmp((const wchar_t*)temp + temp.length()-2,L"\x0d\x0a")) temp.truncate(temp.length()-2);
		_SetDlgItemTextW(wnd,_IDC_DEV_INFO,temp);
	}
}

#undef DEV

const static struct
{
	UINT id;
	cfg_int * var;
} bWnds[]=
{
	{_IDC_SAMP_REVERT,&cfg_samp_revert},
	{_IDC_LOGVOL,&cfg_logvol},
	{_IDC_RMI_DEF,&cfg_rmi_def},
	{_IDC_HACK_DLS_DRUMS,&cfg_hack_dls_drums},
	{_IDC_HACK_DLS_INSTRUMENTS,&cfg_hack_dls_instruments},
	{_IDC_HACK_XG_DRUMS,&cfg_hack_xg_drums},
	{_IDC_HACKTRACK,&cfg_recover_tracks},
	{_IDC_DM_KEEP_PORT,&cfg_dm_keep_port},
	{_IDC_SAMPLING_ENABLED,&cfg_smp},
	{_IDC_REVERB,&cfg_reverb},
	{_IDC_CHORUS,&cfg_chorus},
	{_IDC_SAMPLING_OUTPUT,&cfg_sampout},
	{_IDC_HACK_NO_SYSEX,&cfg_nosysex},
  {_IDC_LYRICS_ENABLED,&cfg_lyrics},
	{_IDC_SHOW_PANEL,&cfg_seq_showpanel},
};

#define NUM_BWNDS (sizeof(bWnds)/sizeof(*bWnds))

#define WM_CMDNOTIFY (WM_USER+6)

static BOOL WINAPI CfgChildProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	if (msg==WM_COMMAND)
	{
#if defined(_WIN64)
		SendMessage(GetParent(wnd),WM_CMDNOTIFY,wp,MAKEID(GetWindowLong(wnd,DWLP_USER),LOWORD(wp)));
#else
		SendMessage(GetParent(wnd), WM_CMDNOTIFY, wp, MAKEID(GetWindowLong(wnd, DWL_USER), LOWORD(wp)));
#endif
	}
	return 0;
}


static bool is_hex(char c) {return (c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F');}

static UINT read_hex(char* t)
{
	UINT r=0;
	while(is_hex(*t))
	{
		r<<=4;
		if (*t>='0' && *t<='9')
			r|=*t-'0';
		else if (*t>='a' && *t<='f')
			r|=*t-'a'+10;
		else r|=*t-'A'+10;
		t++;
	}
	return r;
}

BYTE* read_sysex_edit(HWND w,UINT *siz)//used in seq.cpp
{
	UINT tl=GetWindowTextLength(w)+2;
	char* tx=(char*)malloc(tl);
	if (!tx) return 0;
	GetWindowTextA(w,tx,tl+2);
	UINT sl=0;
	BYTE* tmp=(BYTE*)malloc(tl/2+2);
	char* t=tx;
	if (!tmp)
	{
		free(tx);
		return 0;
	}
	tmp[sl++]=0xF0;
	while(t && *t)
	{
		while(!is_hex(*t) && *t) t++;
		if (!*t) break;
		tmp[sl++]=read_hex(t);
		while(is_hex(*t)) t++;
	}

	tmp[sl++]=0xF7;
	free(tx);
	*siz=sl;
	return tmp;
}

static BOOL WINAPI SysexProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
#if defined(_WIN64)
		SetWindowLong(wnd,DWLP_USER,lp);
#else
		SetWindowLong(wnd, DWL_USER, lp);
#endif
		SendDlgItemMessage(wnd,IDC_SPIN1,UDM_SETRANGE,0,MAKELONG(9999,0));
		SetDlgItemInt(wnd,IDC_DELAY,edit_sysex_table.get_time(lp),0);
		edit_sysex_table.print_edit(lp,GetDlgItem(wnd,IDC_EDIT1));
		return 1;
	case WM_COMMAND:
		switch(wp)
		{
		case IDOK:
			{
				UINT size;
				BYTE* data=read_sysex_edit(GetDlgItem(wnd,IDC_EDIT1),&size);
				if (!data)
				{
					EndDialog(wnd,0);
					break;
				}
#if defined(_WIN64)
				edit_sysex_table.modify_entry(GetWindowLong(wnd, DWLP_USER), data, size, GetDlgItemInt(wnd, IDC_DELAY, 0, 0));
#else
				edit_sysex_table.modify_entry(GetWindowLong(wnd, DWL_USER), data, size, GetDlgItemInt(wnd, IDC_DELAY, 0, 0));
#endif
				free(data);
			}
			
			EndDialog(wnd,1);
			break;
		case IDCANCEL:
			EndDialog(wnd,0);
			break;
		}
	}
	return 0;
}

extern int initDefaultDeviceShit();

static BOOL WINAPI CfgProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		if (g_theConfig && g_theConfig!=wnd)
		{
			ShowWindow(g_theConfig,SW_SHOW);
			EndDialog(wnd,0);
			return 0;
		}
		
		initDefaultDeviceShit();

		g_theConfig=wnd;
		{
			HWND hTab=GetDlgItem(wnd,IDC_TAB);
#if defined(_WIN64)
			SetWindowLong(wnd, DWLP_USER, (long)hTab);
#else
			SetWindowLong(wnd, DWL_USER, (long)hTab);
#endif
			HWND w;
			UINT n;
			TC_ITEMW it=
			{
				TCIF_TEXT,
				0,0,
				0,	//pszText
				0,
				-1,0
			};
			for(n=0;n<tabsize(child_dlgs);n++)
			{
				it.pszText=WASABI_API_LNGSTRINGW(child_dlgs[n].l_id);
				SendMessage(hTab,TCM_INSERTITEMW,n,(long)&it);
			}
			SendMessage(hTab,TCM_SETCURFOCUS,cfg_cur_tab,0);
			RECT r;
			GetClientRect(hTab,&r);
			TabCtrl_AdjustRect(hTab,0,&r);
			MapWindowPoints(hTab,wnd,(LPPOINT)&r,2);

			for(n=0;n<tabsize(child_dlgs);n++)
			{
				child_dlgs[n].wnd=w=WASABI_API_CREATEDIALOGW(child_dlgs[n].id,wnd,CfgChildProc);
				SendMessage(MIDI_callback::GetMainWindow(),WM_WA_IPC,(WPARAM)w,IPC_USE_UXTHEME_FUNC);
				SetWindowPos(w,0,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER);
#if defined(_WIN64)
				SetWindowLong(w, DWLP_USER, IDC_CONFIG1 + n);
#else
				SetWindowLong(w, DWL_USER, IDC_CONFIG1 + n);
#endif
				ShowWindow(w,n==cfg_cur_tab ? SW_SHOW : SW_HIDE);
			}
		}

		{
			UINT n;
			for(n=0;n<NUM_BWNDS;n++)
			{
				_SendDlgItemMessage(wnd,bWnds[n].id,BM_SETCHECK,*bWnds[n].var,0);
			}
		}

		SetDlgItemTextA(wnd,_IDC_DLS,cfg_dls_file);
		_SendDlgItemMessage(wnd,_IDC_DLS_CB,BM_SETCHECK,cfg_dls_active,0);
		EnableWindow(_GetDlgItem(wnd,_IDC_DLS),cfg_dls_active);
		EnableWindow(_GetDlgItem(wnd,_IDC_DLS_B),cfg_dls_active);
		_SetDlgItemInt(wnd,_IDC_LOOP_T,cfg_loop_count,0);
		_SendDlgItemMessage(wnd,_IDC_INFINITE,BM_SETCHECK,cfg_loop_infinite,0);
		_SendDlgItemMessage(wnd,_IDC_LOOP_T,EM_LIMITTEXT,3,0);
		_SendDlgItemMessage(wnd,_IDC_LOOP_SP,UDM_SETRANGE,0,MAKELONG(999,1));
		{
			int n;
			HWND w=_GetDlgItem(wnd,_IDC_LOOP);
			for(n=0;n<3;n++)
			{
				SendMessageW(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRINGW(loop_txt[n]));
			}
			SendMessage(w,CB_SETCURSEL,cfg_loop_type,0);
			char tmp[10] = {0};
			w=_GetDlgItem(wnd,_IDC_FREQ);
			for(n=0;n<3;n++)
			{
				int freq = 11025<<n;
				_itoa(freq,tmp,10);
				int idx = SendMessageA(w,CB_ADDSTRING,0,(long)tmp);  // Must stay in ANSI
				if (cfg_freq==freq)
					SendMessageA(w,CB_SETCURSEL,idx,0);              // Must stay in ANSI
			}
			
			w=_GetDlgItem(wnd,_IDC_PORT);
			{
				int idx_driver=0;
				MIDI_driver * driver;
				while(driver = MIDI_driver::driver_enumerate(idx_driver++))
				{
					int idx_device=0;
					MIDI_device * device;
					while(device = driver->device_enumerate(idx_device++))
					{
						string_w temp;
						temp+=driver->get_name();
						temp+=L" / ";
						temp+=device->get_name();
						int idx_combo = SendMessageW(w,CB_ADDSTRING,0,(LPARAM)(const wchar_t*)temp);
						SendMessage(w,CB_SETITEMDATA,idx_combo,(long)device);

						if (driver->get_guid() == cfg_driver && device->get_guid() == cfg_device)
						{
							SendMessage(w,CB_SETCURSEL,idx_combo,0);
							ChangePort(wnd,device);
						}
					}
					
				}
			}
			
			w=_GetDlgItem(wnd,_IDC_WAVEIN);
			for(n=-1;n<(int)waveInGetNumDevs();n++)
			{
				WAVEINCAPS caps;
				if (waveInGetDevCaps((UINT)n,&caps,sizeof(caps))==MMSYSERR_NOERROR && caps.dwFormats!=0)
				{
					int idx=SendMessage(w,CB_ADDSTRING,0,(long)caps.szPname);
					SendMessage(w,CB_SETITEMDATA,idx,n);
					if (n==cfg_wavein_dev) SendMessage(w,CB_SETCURSEL,idx,0);
				}
			}
			
			cfgVisMix(wnd,cfg_wavein_dev);
			_SendDlgItemMessage(wnd,_IDC_WAVEIN_SRC,CB_SETCURSEL,cfg_wavein_src,0);
			
			w=_GetDlgItem(wnd,_IDC_PLAYBACK_METHOD);
			SendMessageW(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRINGW(IDS_STREAMED));
			SendMessageW(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRINGW(IDS_IMMEDIATE));
			SendMessage(w,CB_SETCURSEL,cfg_playback_mode,0);

			w=_GetDlgItem(wnd,_IDC_SYSEX_LIST);
			edit_sysex_table = cfg_sysex_table;
			
			{
				char temp[128] = {0};
				n=0;
				while(edit_sysex_table.print_preview(n++,temp)) SendMessage(w,LB_ADDSTRING,0,(long)temp);
			}

			w=_GetDlgItem(wnd,_IDC_EXTS_LIST);
			for(n=0;n<MIDI_core::FileTypes_GetNum();n++)
			{
				SendMessageA(w,LB_ADDSTRING,0,(long)MIDI_core::FileTypes_GetExtension(n));  // Must stay in ANSI

				if (cfg_ext_mask & (1<<n))
					SendMessage(w,LB_SETSEL,1,n);
			}
			SetDlgItemTextA(wnd,_IDC_EXTS_ED,cfg_extra_exts);


			w=_GetDlgItem(wnd,_IDC_VOLMODE);
			SendMessageW(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRINGW(STRING_VOLUME_NONE));
			SendMessageW(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRINGW(STRING_VOLUME_DRIVER_SPECIFIC));
			SendMessageW(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRINGW(STRING_VOLUME_AUTO));

			{
				UINT id=0;
				UINT n_devz=mixerGetNumDevs();
				UINT dev;
				for(dev=0;dev<n_devz;dev++)
				{
					mixerGetID((HMIXEROBJ)dev,&id,MIXER_OBJECTF_MIXER);

					MIXERCAPSW capz;
					mixerGetDevCapsW(id,&capz,sizeof(capz));

					MIXERLINEW ml;
					memset(&ml,0,sizeof(ml));
					ml.cbStruct=sizeof(ml);
					ml.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

					mixerGetLineInfoW((HMIXEROBJ)id,&ml,MIXER_GETLINEINFOF_COMPONENTTYPE|MIXER_OBJECTF_MIXER);

					UINT con;
					for(con=0;con<ml.cConnections;con++)
					{
						MIXERLINEW ml1;
						memset(&ml1,0,sizeof(ml1));
						ml1.cbStruct=sizeof(ml);
						ml1.dwSource=con;
						mixerGetLineInfoW((HMIXEROBJ)id,&ml1,MIXER_GETLINEINFOF_SOURCE|MIXER_OBJECTF_MIXER);
						if (n_devz==1) SendMessageW(w,CB_ADDSTRING,0,(long)ml1.szName);
						else
						{
							string_w temp;
							temp+=capz.szPname;
							temp+=ml1.szShortName;
							SendMessageW(w,CB_ADDSTRING,0,(long)(const wchar_t*)temp);
						}
					}
				}
			}

			SendMessage(w,CB_SETCURSEL,cfg_volmode,0);

			w=_GetDlgItem(wnd,_IDC_WAVEIN_CH);
			SendMessageW(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRINGW(STRING_MONO));
			SendMessageW(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRINGW(STRING_STEREO));
			SendMessage(w,CB_SETCURSEL,cfg_wavein_ch-1,0);
			w=_GetDlgItem(wnd,_IDC_WAVEIN_BPS);

			for(n=1;n<=4;n++)
			{
				wchar_t foo[32] = {0};
				wsprintfW(foo,WASABI_API_LNGSTRINGW(STRING_BIT_FMT),n<<3);
				SendMessageW(w,CB_ADDSTRING,0,(long)foo);
			}
			SendMessage(w,CB_SETCURSEL,(cfg_wavein_bps>>3)-1,0);

			w=_GetDlgItem(wnd,_IDC_WAVEIN_SR);
			for(n=0;n<sizeof(sr_tab)/sizeof(sr_tab[0]);n++)
			{
				char foo[8] = {0};
				_itoa(sr_tab[n],foo,10);
				SendMessageA(w,CB_ADDSTRING,0,(long)foo);   // Must stay in ANSI mode
			}

			_SetDlgItemInt(wnd,_IDC_WAVEIN_SR,cfg_wavein_sr,0);

			w=_GetDlgItem(wnd,_IDC_HARDWARE_RESET);
			SendMessageW(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRINGW(IDS_NONE));
			SendMessageW(w,CB_ADDSTRING,0,(long)L"GM (General MIDI)");
			SendMessageW(w,CB_ADDSTRING,0,(long)L"GS (Roland)");
			SendMessageW(w,CB_ADDSTRING,0,(long)L"XG (Yamaha)");
			SendMessageW(w,CB_SETCURSEL,cfg_hardware_reset,0);
		}
//		f_num=0;

		_SendDlgItemMessage(wnd,_IDC_EOF_DELAY_SPIN,UDM_SETRANGE,0,MAKELONG(0x7FFF,0));
		_SetDlgItemInt(wnd,_IDC_EOF_DELAY,cfg_eof_delay,0);

		
		return 1;
	case WM_NOTIFY:
		switch(wp)
		{
		case IDC_TAB:
			if (((NMHDR*)lp)->code==TCN_SELCHANGE)
			{
				UINT n;
				HWND hTab=((NMHDR*)lp)->hwndFrom;
				cfg_cur_tab=SendMessage(hTab,TCM_GETCURSEL,0,0);
				for(n=0;n<tabsize(child_dlgs);n++)
				{
					HWND w=cfgGetTab(IDC_CONFIG1+n);
					ShowWindow(w,n==cfg_cur_tab ? SW_SHOW : SW_HIDE);
				}
			}
			break;
		}
		break;
	case WM_CMDNOTIFY://WM_COMMAND from one of child dialogs
		if (wp>>16)
		{
			switch(lp)
			{
			case _IDC_SYSEX_LIST:
				if (wp>>16==LBN_DBLCLK)
				{
					CfgProc(wnd,WM_CMDNOTIFY,0,_IDC_SYSEX_EDIT);
				}
				break;
			case _IDC_WAVEIN:
				if (wp>>16==CBN_SELCHANGE)
				{
					int d=_SendDlgItemMessage(wnd,_IDC_WAVEIN,CB_GETCURSEL,0,0);
					if (d>=0) cfgVisMix(wnd,_SendDlgItemMessage(wnd,_IDC_WAVEIN,CB_GETITEMDATA,d,0));
					
				}
				break;
			case _IDC_PORT:
				if (wp>>16==CBN_SELCHANGE)
				{
					ChangePort(wnd,get_device(wnd));
				}
				break;
			}
		}
		else
		{
			switch(lp)
			{
			case _IDC_DLS_CB:
			{
				int checked = _SendDlgItemMessage(wnd,_IDC_DLS_CB,BM_GETCHECK,0,0);
				EnableWindow(_GetDlgItem(wnd,_IDC_DLS),checked);
				EnableWindow(_GetDlgItem(wnd,_IDC_DLS_B),checked);
			}
				break;
			case _IDC_IMP_F:
				if (edit_sysex_table.num_entries()>0)
				{
					char fn[MAX_PATH] = {0};
					if (DoOpenFile(wnd,fn,IDS_SYSEX_DATA,"SYX",0))
					{
						HANDLE f=CreateFileA(fn,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
						if (f!=INVALID_HANDLE_VALUE)
						{
							DWORD sz=GetFileSize(f,0);
							BYTE * temp=(BYTE*)malloc(sz);
							if (temp)
							{
								DWORD br = 0;
								ReadFile(f,temp,sz,&br,0);
								edit_sysex_table.add_entry(temp,sz,0);
								free(temp);
							}
							char tmp[128] = {0};
							edit_sysex_table.print_preview(edit_sysex_table.num_entries()-1,tmp);
							_SendDlgItemMessage(wnd,_IDC_SYSEX_LIST,LB_ADDSTRING,0,(long)tmp);
							CloseHandle(f);
						}
						else FILE_BLAH;
					}
				}
				break;
			case _IDC_IMP_PR:
	//			if (!edit_sysex_table.is_empty())
				{
					char fn[MAX_PATH] = {0};
					fn[0]=0;
					if (DoOpenFile(wnd,fn,IDS_MIDI_HARDWARE_PRESETS,"MHP",0))
					{
						if (!edit_sysex_table.file_read(fn))
						{
							FILE_BLAH;
						}
					}
				}
				break;
			case _IDC_EXP_PR:
				if (!edit_sysex_table.is_empty())
				{
					char fn[MAX_PATH] = {0};
					fn[0]=0;
					if (DoOpenFile(wnd,fn,IDS_MIDI_HARDWARE_PRESETS,"MHP",1))
					{
						if (!edit_sysex_table.file_write(fn))
						{
							FILE_BLAH;
						}
					}
				}
				break;
			case _IDC_EXP_F:
				if (!edit_sysex_table.is_empty())
				{
					UINT ns=_SendDlgItemMessage(wnd,_IDC_SYSEX_LIST,LB_GETCURSEL,0,0);
					if (ns!=-1)
					{
						char fn[MAX_PATH] = {0};
						fn[0]=0;
						if (DoOpenFile(wnd,fn,IDS_SYSEX_DATA,"SYX",1))
						{
							HANDLE f=CreateFileA(fn,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
							if (f!=INVALID_HANDLE_VALUE)
							{
								BYTE * data;
								int size = 0;
								edit_sysex_table.get_entry(ns,&data,&size,0);
								DWORD bw = 0;
								WriteFile(f,data,size,&bw,0);
								CloseHandle(f);
							}
							else FILE_BLAH;
						}
					}
				}

				break;
			case _IDC_SYSEX_EDIT:
				if (!edit_sysex_table.is_empty())
				{
					HWND w=_GetDlgItem(wnd,_IDC_SYSEX_LIST);
					int n=SendMessage(w,LB_GETCURSEL,0,0);
					if (n!=-1 && n<edit_sysex_table.num_entries())
					{
						if (WASABI_API_DIALOGBOXPARAM(IDD_SYSEX,wnd,SysexProc,n))
						{
							SendMessage(w,LB_DELETESTRING,n,0);
							char tmp[128] = {0};
							edit_sysex_table.print_preview(n,tmp);
							SendMessage(w,LB_INSERTSTRING,n,(long)tmp);
						}
					}
				}
				break;
			case _IDC_SYSEX_ADD:
//				if (!edit_sysex_table.is_empty())
				{
					BYTE data[2]={0xF0,0xF7};
					edit_sysex_table.add_entry(data,2,0);
					char tmp[128] = {0};
					edit_sysex_table.print_preview(edit_sysex_table.num_entries()-1,tmp);
					_SendDlgItemMessage(wnd,_IDC_SYSEX_LIST,LB_ADDSTRING,0,(long)tmp);
				}
				break;
			case _IDC_SYSEX_DELETE:
				if (!edit_sysex_table.is_empty())
				{
					HWND w=_GetDlgItem(wnd,_IDC_SYSEX_LIST);
					int n=SendMessage(w,LB_GETCURSEL,0,0);
					if (n != LB_ERR)
					{
						SendMessage(w,LB_DELETESTRING,n,0);
						edit_sysex_table.remove_entry(n);
					}
				}
				break;
			case _IDC_SYSEX_UP:
				{
					HWND w=_GetDlgItem(wnd,_IDC_SYSEX_LIST);
					int ns=SendMessage(w,LB_GETCURSEL,0,0);
					if (ns==-1 || ns==0) break;
					int ns1=ns-1;
					BYTE * data;
					int size,time;
					edit_sysex_table.get_entry(ns,&data,&size,&time);
					edit_sysex_table.insert_entry(ns1,data,size,time);
					edit_sysex_table.remove_entry(ns+1);

					char tmp[128] = {0};
					SendMessage(w,LB_DELETESTRING,ns1,0);
					edit_sysex_table.print_preview(ns,tmp);
					SendMessage(w,LB_INSERTSTRING,ns,(long)tmp);
				}
				break;
			case _IDC_SYSEX_DOWN:
				{
					HWND w=_GetDlgItem(wnd,_IDC_SYSEX_LIST);
					int ns1=SendMessage(w,LB_GETCURSEL,0,0);
					if (ns1==-1 || ns1==edit_sysex_table.num_entries()-1) break;
					int ns=ns1+1;
					BYTE * data;
					int size,time;
					edit_sysex_table.get_entry(ns,&data,&size,&time);
					edit_sysex_table.insert_entry(ns1,data,size,time);
					edit_sysex_table.remove_entry(ns+1);

					char tmp[128] = {0};
					SendMessage(w,LB_DELETESTRING,ns,0);
					edit_sysex_table.print_preview(ns1,tmp);
					SendMessage(w,LB_INSERTSTRING,ns1,(long)tmp);
				}
				break;
			case _IDC_SAMPLING_ENABLED:
				cfgVisStatus(wnd);
				break;
			case _IDC_DLS_B:
				{
					char tmp[MAX_PATH] = {0};
					GetDlgItemTextA(wnd,_IDC_DLS,tmp,MAX_PATH);
					if (DoOpenFile(wnd,tmp,IDS_DLS_FILES,"DLS",0)) SetDlgItemTextA(wnd,_IDC_DLS,tmp);
				}
				break;
			}
			break;

		}
		break;
	case WM_COMMAND:
		switch(wp)
		{
		case IDRESET:
			if (MessageBoxW(wnd,WASABI_API_LNGSTRINGW(STRING_CONFIG_RESET),(wchar_t*)mod.description,
							MB_ICONWARNING|MB_YESNO)==IDYES)
			{
				edit_sysex_table.reset();
				if (g_theConfig==wnd) g_theConfig=0;
				EndDialog(wnd,666);
			}
			break;
		case IDCANCEL:
			edit_sysex_table.reset();
			if (g_theConfig==wnd) g_theConfig=0;
			EndDialog(wnd,0);			
			break;
		case IDOK:
			{
				UINT n;
				for(n=0;n<NUM_BWNDS;n++)
				{
					*bWnds[n].var=!!_SendDlgItemMessage(wnd,bWnds[n].id,BM_GETCHECK,0,0);
				}
			}
			cfg_wavein_src=_SendDlgItemMessage(wnd,_IDC_WAVEIN_SRC,CB_GETCURSEL,0,0);
			cfg_playback_mode = _SendDlgItemMessage(wnd,_IDC_PLAYBACK_METHOD,CB_GETCURSEL,0,0);
			{
				MIDI_device * dev = get_device(wnd);
				if (dev)
				{
					cfg_driver = dev->get_driver()->get_guid();
					cfg_device = dev->get_guid();
				}
			}
			
			{
				int t=_SendDlgItemMessage(wnd,_IDC_WAVEIN,CB_GETCURSEL,0,0);
				if (t<0) cfg_wavein_dev=-1;
				else cfg_wavein_dev = _SendDlgItemMessage(wnd,_IDC_WAVEIN,CB_GETITEMDATA,t,0);
			}

			cfg_wavein_sr=_GetDlgItemInt(wnd,_IDC_WAVEIN_SR,0,0);
			cfg_wavein_ch=_SendDlgItemMessage(wnd,_IDC_WAVEIN_CH,CB_GETCURSEL,0,0)+1;
			cfg_wavein_bps=(_SendDlgItemMessage(wnd,_IDC_WAVEIN_BPS,CB_GETCURSEL,0,0)+1)<<3;

			cfg_loop_type=_SendDlgItemMessage(wnd,_IDC_LOOP,CB_GETCURSEL,0,0);
			cfg_loop_count=_GetDlgItemInt(wnd,_IDC_LOOP_T,0,0);
			cfg_loop_infinite=_SendDlgItemMessage(wnd,_IDC_INFINITE,BM_GETCHECK,0,0);

			{
				int t=_SendDlgItemMessage(wnd,_IDC_FREQ,CB_GETCURSEL,0,0);
				if (t<0) cfg_freq=22050;
				else cfg_freq=11025<<t;
			}

			cfg_dls_file.get_string().from_window(_GetDlgItem(wnd,_IDC_DLS));
			cfg_dls_active = _SendDlgItemMessage(wnd,_IDC_DLS_CB,BM_GETCHECK,0,0);

			cfg_sysex_table=edit_sysex_table;
			edit_sysex_table.reset();

			cfg_extra_exts.get_string().from_window(_GetDlgItem(wnd,_IDC_EXTS_ED));

			{
				int n;
				HWND w=_GetDlgItem(wnd,_IDC_EXTS_LIST);
				cfg_ext_mask=0;
				for(n=0;n<MIDI_core::FileTypes_GetNum();n++)
					if (SendMessage(w,LB_GETSEL,n,0)) cfg_ext_mask = (int)cfg_ext_mask | (1<<n);
			}
			
			cfg_volmode=_SendDlgItemMessage(wnd,_IDC_VOLMODE,CB_GETCURSEL,0,0);

			cfg_eof_delay=_GetDlgItemInt(wnd,_IDC_EOF_DELAY,0,0);

			cfg_hardware_reset=_SendDlgItemMessage(wnd,_IDC_HARDWARE_RESET,CB_GETCURSEL,0,0);

			if (g_theConfig==wnd) g_theConfig=0;
			EndDialog(wnd,1);
			break;
		}
		break;
	}
	return 0;
}


int MIDI_core::Config(HWND p)
{
	int r;
db:
	r=WASABI_API_DIALOGBOXW(IDD_CONFIG,p,CfgProc);
	if (r==666)
	{
		cfg_var::config_reset();
		goto db;
	}
	return r;
}

int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
{
	MSGBOXPARAMSW msgbx = {sizeof(MSGBOXPARAMSW),0};
	msgbx.lpszText = message;
	msgbx.lpszCaption = title;
	msgbx.lpszIcon = MAKEINTRESOURCEW(102);
	msgbx.hInstance = GetModuleHandle(0);
	msgbx.dwStyle = MB_USERICON;
	msgbx.hwndOwner = parent;
	return MessageBoxIndirectW(&msgbx);
}

void About(HWND hwndParent)
{
	wchar_t message[1024] = {0}, text[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_MIDI_PLAYER_OLD,text,1024);
	wsprintfW(message, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
			  mod.description, __DATE__);
	DoAboutMessageBox(hwndParent,text,message);
}