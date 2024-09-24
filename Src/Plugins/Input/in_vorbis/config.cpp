#include "main.h"
#include "api__in_vorbis.h"
#include "../nu/ns_wc.h"
#include <commctrl.h>
#include <shlobj.h>
#include "../winamp/wa_ipc.h"
#include "../nu/AutoChar.h"
#include <strsafe.h>

int mc6_dm_names_ids[]={IDS_LEAVE_AS_IS,IDS_REMAP_6_CHANNELS,IDS_DOWNMIX_TO_4_CHANNELS,IDS_DOWNMIX_TO_2_CHANNELS_DS,IDS_DOWNMIX_TO_2_CHANNELS_DS2,IDS_DOWNMIX_TO_MONO};
int mc6_map_names_id[]={IDS_CORRECT_FL_FC_FR_BL_BR_LFE,IDS_BROKEN_FL_FR_FC_BL_BR_LFE};
int32_t priority_tab[7]={THREAD_PRIORITY_IDLE,THREAD_PRIORITY_LOWEST,THREAD_PRIORITY_BELOW_NORMAL,THREAD_PRIORITY_NORMAL,THREAD_PRIORITY_ABOVE_NORMAL,THREAD_PRIORITY_HIGHEST,THREAD_PRIORITY_TIME_CRITICAL};

char* defaultDumpDir()
{
	static char dumpdir[MAX_PATH] = {0};
	if(FAILED(SHGetFolderPathA(NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, dumpdir)))
	{
		if(FAILED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, dumpdir)))
		{
			lstrcpynA(dumpdir, "C:\\", MAX_PATH);
		}
	}
	return dumpdir;
}

CfgString 
	cfg_ssave_format("ssave_format","%filename%"),
	cfg_dumpdir("dumpdir",defaultDumpDir());

CfgInt
	cfg_http_bsize("http_bsize",0x10000),
	cfg_fsave("fsave",0),
	cfg_abr("abr",0),
	cfg_proxy_mode("proxy_mode",2),
	cfg_prebuf1("prebuf1",50),
	cfg_prebuf2("prebuf2",75),
	cfg_httpseek2("httpseek2",0),
	cfg_fix0r("fix0r",1),
	cfg_mc6_dm("mc6_dm",0),
	cfg_mc6_map("_mc6_map",0),
	cfg_remember_infosize("remember_infosize",1),
	cfg_fullbuf("fullbuf",0),
	cfg_cur_tab("cur_tab",0);

static int old_preamp;
CfgFont cfg_font("font");
static LOGFONT cfg_font_edit;

BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam)
{
	char cl[32] = {0};
	GetClassNameA(hwnd, cl, ARRAYSIZE(cl));
	if (!lstrcmpiA(cl, WC_TREEVIEWA))
	{
		PostMessage(hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection(hwnd));
		return FALSE;
	}

	return TRUE;
}

static int CALLBACK browzaproc(HWND hwnd, UINT msg, LPARAM lp, LPARAM dat)
{
	if (msg == BFFM_INITIALIZED)
	{
		SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)dat);

		// this is not nice but it fixes the selection not working correctly on all OSes
		EnumChildWindows(hwnd, browseEnumProc, 0);
	}
	return 0;
}

static void d_browza(HWND wnd,HWND bt,wchar_t* tx)
{
	IMalloc* pMalloc=0;
	
	SHGetMalloc(&pMalloc);
	if (!pMalloc) return;

	wchar_t dir[MAX_PATH] = {0};
	GetWindowTextW(bt,dir,MAX_PATH);
	BROWSEINFOW bi=
	{
		wnd,
		0,
		0,
		tx,
		BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE,
		browzaproc,
		(LPARAM)dir,
		0
	};
	ITEMIDLIST* li=SHBrowseForFolderW(&bi);
	if (li)
	{
		SHGetPathFromIDListW(li,dir);
		SetWindowTextW(bt,dir);
		pMalloc->Free(li);
	}

	pMalloc->Release();
}

static BOOL CALLBACK CfgProc1(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			{
				wchar_t temp[128] = {0}, cfg_dialog_name[128] = {0};
				StringCchPrintfW(cfg_dialog_name,128,WASABI_API_LNGSTRINGW(IDS_TITLE_PREFERENCES),
								 WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_VORBIS_DECODER_OLD, temp, 128));
				SetWindowTextW(wnd,cfg_dialog_name);

				SendDlgItemMessage(wnd,IDC_FULLBUF,BM_SETCHECK,cfg_fullbuf,0);

				UINT n;
				HWND w=GetDlgItem(wnd,IDC_MC6_DM);
				for(n=0;n<sizeof(mc6_dm_names_ids)/sizeof(mc6_dm_names_ids[0]);n++)
				{
					SendMessageW(w,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(mc6_dm_names_ids[n]));
				}
				SendMessage(w,CB_SETCURSEL,cfg_mc6_dm,0);

				w=GetDlgItem(wnd,IDC_MC6_MAP);
				for(n=0;n<sizeof(mc6_map_names_id)/sizeof(mc6_map_names_id[0]);n++)
				{
					SendMessageW(w,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(mc6_map_names_id[n]));
				}
				SendMessage(w,CB_SETCURSEL,cfg_mc6_map,0);

				SendDlgItemMessage(wnd,IDC_AVG_BR,BM_SETCHECK,cfg_abr,0);

				SetDlgItemInt(wnd,IDC_HTTP_BSIZE,cfg_http_bsize>>10,0);
				if (cfg_fsave) SendDlgItemMessage(wnd,IDC_FSAVE,BM_SETCHECK,1,0);
				if (cfg_fix0r) SendDlgItemMessage(wnd,IDC_FIX0R,BM_SETCHECK,1,0);
				cfg_dumpdir.s_SetDlgItemText(wnd,IDC_STREAM_SAVE);
				w=GetDlgItem(wnd,IDC_PROXY);
				SendMessageW(w,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_NEVER));
				SendMessageW(w,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_PORT_80_ONLY));
				SendMessageW(w,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_ALWAYS));
				SendMessage(w,CB_SETCURSEL,cfg_proxy_mode,0);

				w=GetDlgItem(wnd,IDC_SLIDER1);
				SendMessage(w,TBM_SETRANGE,0,MAKELONG(1,100));
				SendMessage(w,TBM_SETPOS,1,cfg_prebuf1);
				w=GetDlgItem(wnd,IDC_SLIDER2);
				SendMessage(w,TBM_SETRANGE,0,MAKELONG(1,100));
				SendMessage(w,TBM_SETPOS,1,cfg_prebuf2);

				cfg_ssave_format.s_SetDlgItemText(wnd,IDC_SSAVE_FMT);
				SendMessage(wnd,WM_COMMAND,MAKEWPARAM(IDC_FSAVE,BN_CLICKED),(LPARAM)GetDlgItem(wnd,IDC_FSAVE));
			}
			return 1;

		case WM_COMMAND:
			switch(LOWORD(wp))
			{
				case IDC_STREAM_SAVE:
					d_browza(wnd,(HWND)lp,WASABI_API_LNGSTRINGW(IDS_SELECT_OUTPUT_DIRECTORY));
					break;

				case IDC_SSAVE_FMT_DEF:
					SetDlgItemText(wnd,IDC_SSAVE_FMT,L"%filename%");
					break;

				case IDC_FSAVE:
				{
					int checked = IsDlgButtonChecked(wnd,IDC_FSAVE);
					EnableWindow(GetDlgItem(wnd,IDC_STREAM_SAVE),checked);
					EnableWindow(GetDlgItem(wnd,IDC_SSAVE_FMT),checked);
					EnableWindow(GetDlgItem(wnd,IDC_SSAVE_FMT_DEF),checked);
				}
					break;

				case IDOK:
				case IDCANCEL:
				{
					if (LOWORD(wp) == IDOK)
					{
						cfg_fullbuf=(int)SendDlgItemMessage(wnd,IDC_FULLBUF,BM_GETCHECK,0,0);
						
						cfg_mc6_dm=(int)SendDlgItemMessage(wnd,IDC_MC6_DM,CB_GETCURSEL,0,0);
						cfg_mc6_map=(int)SendDlgItemMessage(wnd,IDC_MC6_MAP,CB_GETCURSEL,0,0);
	
						cfg_abr=(int)SendDlgItemMessage(wnd,IDC_AVG_BR,BM_GETCHECK,0,0);
	
						cfg_dumpdir.s_GetDlgItemText(wnd,IDC_STREAM_SAVE);
						cfg_http_bsize=GetDlgItemInt(wnd,IDC_HTTP_BSIZE,0,0)<<10;
						cfg_fsave=(int)SendDlgItemMessage(wnd,IDC_FSAVE,BM_GETCHECK,0,0);
						cfg_fix0r=(int)SendDlgItemMessage(wnd,IDC_FIX0R,BM_GETCHECK,0,0);
						cfg_proxy_mode=(int)SendDlgItemMessage(wnd,IDC_PROXY,CB_GETCURSEL,0,0);
						cfg_prebuf1=(int)SendDlgItemMessage(wnd,IDC_SLIDER1,TBM_GETPOS,0,0);
						cfg_prebuf2=(int)SendDlgItemMessage(wnd,IDC_SLIDER2,TBM_GETPOS,0,0);
						cfg_ssave_format.s_GetDlgItemText(wnd,IDC_SSAVE_FMT);
					}
					do_cfg(1);
					EndDialog(wnd,(LOWORD(wp) == IDOK));
				}
					break;
			}
			break;
	}

	const int controls[] = 
	{
		IDC_SLIDER1,
		IDC_SLIDER2,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(wnd, msg, wp, lp, controls, ARRAYSIZE(controls)))
	{
		return TRUE;
	}

	return 0;
}

extern HANDLE hThread;//hack

void Config(HWND p)
{
	if (WASABI_API_DIALOGBOXPARAMW(IDD_CONFIG,p,CfgProc1,0))
	{
		if (hThread) PostMessage(mod.hMainWindow,WM_USER,0,243);
	}
}

int CfgVar::read_int(const char *inifile, const char *section,const char * name,int def)
{
	return GetPrivateProfileIntA(section, name, def, inifile);
}

void CfgVar::write_int(const char *inifile, const char *section, const char * name,int val)
{
	char temp[32] = {0};
	StringCchPrintfA(temp, 32, "%d", val);
	WritePrivateProfileStringA(section, name, temp, inifile);
}

void CfgVar::write_struct(const char *inifile, const char *section, const char * name, void * ptr,UINT size)
{
	WritePrivateProfileStructA("in_vorbis", name, ptr, size, INI_FILE);
}

bool CfgVar::read_struct(const char *inifile, const char *section, const char * name,void * ptr,UINT size)
{
	return !!GetPrivateProfileStructA("in_vorbis", name, ptr, size, INI_FILE);
}

void do_cfg(int s)
{
	#define CFG_VERSION 0x10204

	if (!s)
	{
		if (CfgVar::read_int(INI_FILE, "in_vorbis", "version",0)==CFG_VERSION)
			CfgVar::ReadConfig();
	}
	else
	{
		CfgVar::WriteConfig();
		CfgVar::write_int(INI_FILE, "in_vorbis", "version",CFG_VERSION);
	}
}

CfgVar * CfgVar::list=0;

void CfgVar::ReadConfig()
{
	CfgVar * p=list;
	while(p)
	{
		p->Read(p->name);
		p=p->next;
	}
}

void CfgVar::WriteConfig()
{
	CfgVar * p=list;
	while(p)
	{
		p->Write(p->name);
		p=p->next;
	}
}

bool StringW::reg_read(const char * name)
{
	char utf8_data[2048] = {0};
	wchar_t utf16_data[2048] = {0};
	GetPrivateProfileStringA("in_vorbis", name, "@default@", utf8_data, 2048, INI_FILE);
	if (!strcmp("@default@", utf8_data))
		return false;

	MultiByteToWideCharSZ(CP_UTF8, 0, utf8_data, -1, utf16_data, 2048);
	SetString(utf16_data);

	return true;
}

void StringW::reg_write(const char * name)
{
	WritePrivateProfileStringA("in_vorbis", name, AutoChar((const WCHAR *)*this, CP_UTF8), INI_FILE);
}

void CfgString::Read(const char * name)
{
	reg_read(name);
}

void CfgString::Write(const char * name)
{
	StringW temp;
	if (temp.reg_read(name))
	{
		if (wcscmp(temp,*this)) reg_write(name);
	}
	else
	{
		if (wcscmp(def,*this)) reg_write(name);
	}
}

void CfgInt::Write(const char * name)
{
	if (read_int(INI_FILE, "in_vorbis", name,def)!=value) write_int(INI_FILE, "in_vorbis",name,value);
}

void CfgInt::Read(const char * name)
{
	value=read_int(INI_FILE, "in_vorbis", name,def);
}