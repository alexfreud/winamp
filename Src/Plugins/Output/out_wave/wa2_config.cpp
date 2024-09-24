#include "out_wave.h"
#include "api.h"
#include <commctrl.h>
#include "resource.h"
#include <math.h>
#include "../winamp/wa_ipc.h"
#pragma intrinsic(log)

#ifndef _WIN64
//__inline long int lrint (double flt) 
//{
//	int intgr;
//
//	_asm
//	{
//		fld flt
//		fistp intgr
//	}
//
//	return intgr;
//} 
#else
//__inline long int lrint (double flt) 
//{
//	return (int)flt;
//} 
#endif


#pragma warning(disable:4800)

extern Out_Module mod;
extern UINT cfg_buf_ms,cfg_dev,cfg_prebuf;
extern bool cfg_volume,cfg_altvol,cfg_resetvol;
extern UINT cfg_trackhack;

bool get_waveout_state(char * z) throw();

void _init();

#define BUFFER_SCALE 4000.0

static UINT cur_buffer;

static UINT get_buffer(HWND wnd)
{
	if (cur_buffer) return cur_buffer;

	//0-BUFFER_SCALE => 200-20000	
	LRESULT z=SendDlgItemMessage(wnd,IDC_BUFFER,TBM_GETPOS,0,0);
	//return cur_buffer=lrint( 0.5 + 200.0*pow(100.0,((double)z)/BUFFER_SCALE) );

}

#define LOG100 4.6051701859880913680359829093687
static void set_buffer(HWND wnd,UINT b)
{
	cur_buffer=b;
	SendDlgItemMessage(wnd,IDC_BUFFER,TBM_SETPOS,1,lrint( 0.5 + BUFFER_SCALE * log( (double)b/200.0 ) / LOG100 /* / log( 100.0 )*/ ));
}

static void update_prebuf_1(HWND wnd)
{
	WCHAR zz[128] = { 0 };
	wsprintf(zz, WASABI_API_LNGSTRINGW(IDS_WAVE_U_MS), SendDlgItemMessage(wnd, IDC_PREBUFFER_1, TBM_GETPOS, 0, 0));
	SetDlgItemText(wnd,IDC_PREBUF_DISP_1,zz);
}

static void update_prebuf_2(HWND wnd)
{
	WCHAR zz[128] = { 0 };
	wsprintf(zz, WASABI_API_LNGSTRINGW(IDS_WAVE_U_MS), SendDlgItemMessage(wnd, IDC_PREBUFFER_2, TBM_GETPOS, 0, 0));
	SetDlgItemText(wnd,IDC_PREBUF_DISP_2,zz);
}

static void update_prebuf_range(HWND wnd)
{
	UINT max=get_buffer(wnd);
	if (max>0x7FFF) max=0x7FFF;
	SendDlgItemMessage(wnd,IDC_PREBUFFER_1,TBM_SETRANGE,1,MAKELONG(0,max));
	SendDlgItemMessage(wnd,IDC_PREBUFFER_2,TBM_SETRANGE,1,MAKELONG(0,max));
}

static void update_buf(HWND wnd)
{
	WCHAR zz[128] = { 0 };
	wsprintf(zz, WASABI_API_LNGSTRINGW(IDS_WAVE_U_MS), get_buffer(wnd));
	SetDlgItemText(wnd,IDC_BUF_DISP,zz);
}

static INT_PTR WINAPI CfgProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
	{
		wchar_t title[128] = {0}, temp[128] = {0};
		swprintf(title,128,WASABI_API_LNGSTRINGW(IDS_PREFS_TITLE),WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_WAVEOUT_OLD,temp,128));
		SetWindowTextW(wnd,title);

		SendDlgItemMessage(wnd,IDC_VOL_ENABLE,BM_SETCHECK,(long)cfg_volume,0);
		SendDlgItemMessage(wnd,IDC_ALT_VOL,BM_SETCHECK,(long)cfg_altvol,0);
		SendDlgItemMessage(wnd,IDC_VOL_RESET,BM_SETCHECK,(long)cfg_resetvol,0);

		{
			int dev;
			HWND w=GetDlgItem(wnd,IDC_DEV);
			UINT max=waveOutGetNumDevs();
			WAVEOUTCAPS caps;
			for(dev=-1;dev<(int)max;dev++)
			{
				if (waveOutGetDevCaps((UINT)dev,&caps,sizeof(caps)) == MMSYSERR_NOERROR)
					SendMessage(w,CB_ADDSTRING,0,(LPARAM)caps.szPname);
			}
			SendMessage(w,CB_SETCURSEL,cfg_dev,0);
		}

		SendDlgItemMessage(wnd,IDC_BUFFER,TBM_SETRANGE,0,MAKELONG(0,(int)BUFFER_SCALE));
		set_buffer(wnd,cfg_buf_ms);
		update_prebuf_range(wnd);
		SendDlgItemMessage(wnd,IDC_PREBUFFER_1,TBM_SETPOS,1,cfg_prebuf);
		SendDlgItemMessage(wnd,IDC_PREBUFFER_2,TBM_SETPOS,1,cfg_trackhack);
		update_prebuf_1(wnd);
		update_prebuf_2(wnd);
		update_buf(wnd);

		SetTimer(wnd,1,500,0);
		CfgProc(wnd,WM_TIMER,0,0);
	}
		return 1;
	case WM_COMMAND:
		switch(wp)
		{
		case IDC_RESET:
			SendDlgItemMessage(wnd,IDC_VOL_ENABLE,BM_SETCHECK,1,0);
			SendDlgItemMessage(wnd,IDC_ALT_VOL,BM_SETCHECK,0,0);
			SendDlgItemMessage(wnd,IDC_VOL_RESET,BM_SETCHECK,0,0);

			SendDlgItemMessage(wnd,IDC_DEV,CB_SETCURSEL,0,0);

			set_buffer(wnd,2000);
			update_prebuf_range(wnd);
			SendDlgItemMessage(wnd,IDC_PREBUFFER_1,TBM_SETPOS,1,200);
			SendDlgItemMessage(wnd,IDC_PREBUFFER_2,TBM_SETPOS,1,200);
			update_prebuf_1(wnd);
			update_prebuf_2(wnd);
			update_buf(wnd);
			break;
		case IDOK:
			KillTimer(wnd,1);
			cfg_dev=(UINT)SendDlgItemMessage(wnd,IDC_DEV,CB_GETCURSEL,0,0);
			cfg_buf_ms=get_buffer(wnd);
			cfg_prebuf= (UINT)SendDlgItemMessage(wnd,IDC_PREBUFFER_1,TBM_GETPOS,0,0);
			cfg_trackhack=(UINT)SendDlgItemMessage(wnd,IDC_PREBUFFER_2,TBM_GETPOS,0,0);
			cfg_volume=(bool)SendDlgItemMessage(wnd,IDC_VOL_ENABLE,BM_GETCHECK,0,0);
			cfg_altvol=(bool)SendDlgItemMessage(wnd,IDC_ALT_VOL,BM_GETCHECK,0,0);
			cfg_resetvol=(bool)SendDlgItemMessage(wnd,IDC_VOL_RESET,BM_GETCHECK,0,0);
			EndDialog(wnd,1);
			break;
		case IDCANCEL:
			KillTimer(wnd,1);
			EndDialog(wnd,0);
			break;
		}
		break;
	case WM_HSCROLL:
		switch(GetWindowLong((HWND)lp,GWL_ID))
		{
		case IDC_BUFFER:
			cur_buffer=0;
			update_buf(wnd);
			update_prebuf_range(wnd);
			update_prebuf_1(wnd);
			update_prebuf_2(wnd);
			break;
		case IDC_PREBUFFER_1:
			update_prebuf_1(wnd);
			break;
		case IDC_PREBUFFER_2:
			update_prebuf_2(wnd);
			break;
		}
		break;
	case WM_TIMER:
		{
			char poo[512] = {0};
			bool z=get_waveout_state(poo);
			SetDlgItemTextA(wnd,IDC_STATE,z ? poo : WASABI_API_LNGSTRING(IDS_NOT_ACTIVE));
			EnableWindow(GetDlgItem(wnd,IDC_BLAH),z);
		}
		break;
	}

	const int controls[] = 
	{
		IDC_BUFFER,
		IDC_PREBUFFER_1,
		IDC_PREBUFFER_2,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(wnd, msg, wp, lp, controls, ARRAYSIZE(controls)))
	{
		return TRUE;
	}
	return 0;
}

void Config(HWND w)
{
	_init();
	WASABI_API_DIALOGBOXW(IDD_CONFIG,w,CfgProc);
}

static char _dllfile[MAX_PATH];
static char * dllfile;
char *inifile=0;

static UINT atoui(char* s)
{
	int ret=0;
	while(s && *s>='0' && *s<='9') {ret=10*ret+(*s-'0');s++;}
	return ret;
}

static int _do_var(unsigned int v,char* n,bool s)
{
	if (s)
	{
		char tmp[2*sizeof(unsigned int) + 1] = {0}; // max 32 bit unsigned int == 4 294 967 296 == 10 digits, 11 if signed
		wsprintfA(tmp,"%u",v);
		WritePrivateProfileStringA(dllfile,n,tmp,inifile);
		return v;
	}
	else
	{
		char tmp[64],tmp_s[2*sizeof(unsigned int) + 1] = {0};
		wsprintfA(tmp_s,"%u",v);
		GetPrivateProfileStringA(dllfile,n,tmp_s,tmp,64,inifile);
		return atoui(tmp);
	}

}

#define do_var(V) {V=_do_var(V,#V,s);}

void do_cfg(bool s)
{
	if (!inifile)
		inifile  =(char *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE);

	if (!dllfile)
	{
		GetModuleFileNameA(mod.hDllInstance,_dllfile,sizeof(_dllfile));
		dllfile=strrchr(_dllfile,'\\');
		if (!dllfile) dllfile=_dllfile;
		else dllfile++;
		char * p=strchr(dllfile,'.');
		if (p) *p=0;
	}

	do_var(cfg_buf_ms);
	do_var(cfg_dev);
	do_var(cfg_volume);
	do_var(cfg_altvol);
	do_var(cfg_resetvol);
	do_var(cfg_prebuf);
	do_var(cfg_trackhack);
}
