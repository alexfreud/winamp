#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <mmreg.h>
#include <msacm.h>
#include "out_disk.h"
#include "../winamp/wa_ipc.h"
#include <shlwapi.h>
#include <strsafe.h>

// wasabi based services for localisation support
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

class CriticalSection : public CRITICAL_SECTION
{
public:
	inline void Enter() {EnterCriticalSection(this);}
	inline void Leave() {LeaveCriticalSection(this);}
	CriticalSection() {InitializeCriticalSection(this);}
	~CriticalSection() {DeleteCriticalSection(this);}
	//BOOL TryEnter() {return TryEnterCriticalSection(this);}
};

class __T_SYNC
{
private:
	CriticalSection *p;
public:
	inline __T_SYNC(CriticalSection& s) {p=&s;p->Enter();}
	inline void Leave() {if (p) p->Leave();}
	inline void Enter() {if (p) p->Enter();}
	inline void Abort() {p=0;}
	inline ~__T_SYNC() {Leave();}
};

static CriticalSection g_sync;

#define SYNCFUNC __T_SYNC __sync(g_sync);

#define tabsize(X) (sizeof(X)/sizeof(*X))

enum
{
	MODE_AUTO=0,
	MODE_WAV=1,
	MODE_RAW=2
};

int mode_names_idx[] = {IDS_AUTO_RECOMMENDED,IDS_FORCE_WAV_FILE,IDS_FORCE_RAW_DATA};
static const wchar_t* format_names[]={L"%title%",L"%filename%",L"%title%_%extension%",L"%filename%_%extension%"};
static wchar_t szDescription[256];
int index_name_idx[] = {IDS_DISABLED,IDS_1_DIGIT,IDS_2_DIGITS,IDS_3_DIGITS,IDS_4_DIGITS};

#define rev32(X) ((((DWORD)(X)&0xFF)<<24)|(((DWORD)(X)&0xFF00)<<8)|(((DWORD)(X)&0xFF0000)>>8)|(((DWORD)(X)&0xFF000000)>>24))

static char cfg_output_dir[MAX_PATH]="c:\\";
static char cfg_singlefile_output[MAX_PATH]="c:\\output.wav";
static bool cfg_singlefile_enabled = 0;
static bool cfg_convert_enabled=0;
static bool cfg_thread_override = 0;
static bool cfg_output_source_dir = 0;
static int cfg_output_mode=0;
static bool cfg_show_saveas=0;
static int cfg_format_mode=0;
static int cfg_format_index=2;

static bool use_convert;


static bool GetCheck(HWND wnd,int id) {return !!SendDlgItemMessage(wnd,id,BM_GETCHECK,0,0);}

static void SetCheck(HWND wnd,int id,bool b) {SendDlgItemMessage(wnd,id,BM_SETCHECK,b ? BST_CHECKED : BST_UNCHECKED,0);}

void SetPathChoiceButtonText(HWND hwndDlg, char* path, UINT id)
{
	HWND control = GetDlgItem(hwndDlg, id);
	HDC hdc = GetDC(control);
	RECT r = {0};
	char temp[MAX_PATH] = {0};

	lstrcpynA(temp, path, MAX_PATH);
	SelectObject(hdc, (HFONT)SendMessage(control, WM_GETFONT, 0, 0));
	GetClientRect(control, &r);
	r.left += 5;
	r.right -= 5;
	DrawTextA(hdc, temp, -1, &r, DT_PATH_ELLIPSIS|DT_WORD_ELLIPSIS|DT_MODIFYSTRING);
	SetWindowTextA(control, temp);
	ReleaseDC(control, hdc);
}

BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam)
{
	char cl[32] = {0};
	GetClassNameA(hwnd, cl, ARRAYSIZE(cl));
	if (!lstrcmpiA(cl, WC_TREEVIEW))
	{
		PostMessage(hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection(hwnd));
		return FALSE;
	}

	return TRUE;
}

static int _stdcall browzaproc(HWND hwnd, UINT msg, LPARAM lp, LPARAM dat)
{
	if (msg == BFFM_INITIALIZED)
	{
		SendMessage(hwnd, BFFM_SETSELECTION, 1, dat);

		// this is not nice but it fixes the selection not working correctly on all OSes
		EnumChildWindows(hwnd, browseEnumProc, 0);
	}
	return 0;
}

char g_tmp[MAX_PATH] = {0}, g_tmp_sf[MAX_PATH] = {0};
static void d_browza(HWND wnd,HWND bt,wchar_t* tx)
{
	IMalloc* pMalloc=0;
	SHGetMalloc(&pMalloc);
	if (!pMalloc) return;

	BROWSEINFOW bi=
	{
		wnd,
		0,
		0,
		tx,
		BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE,
		browzaproc,
#ifdef WIN64
		(long long)g_tmp,
#else
		(long)g_tmp,
#endif
		0
	};
	LPITEMIDLIST li=SHBrowseForFolderW(&bi);
	if (li)
	{
		SHGetPathFromIDListA(li,g_tmp);
		SetPathChoiceButtonText(wnd, g_tmp, IDC_OUTPUT_DIRECTORY);
		pMalloc->Free(li);
	}
	pMalloc->Release();
}

static WAVEFORMATEX singlefile_wfx,singlefile_wfx_temp;

#define WFSIZ 0x800
typedef struct
{
	WAVEFORMATEX wfx;
	BYTE crap[WFSIZ];
} EXT_WFX;

EXT_WFX convert_wfx,convert_wfx_temp;
static const WAVEFORMATEX wfx_default =
{
	WAVE_FORMAT_PCM,
		2,
		44100,
		44100*4,
		4,
		16,
		0
};

void _inline ACM_gettext1(char* tx, int txCch)
{
	ACMFORMATDETAILS afd;
	ZeroMemory(&afd, sizeof(afd));
	afd.cbStruct = sizeof(afd);
	afd.dwFormatTag = WAVE_FORMAT_PCM;
	afd.pwfx = &singlefile_wfx_temp;
	afd.cbwfx = sizeof(singlefile_wfx_temp);
	if (!acmFormatDetails(0, &afd, ACM_FORMATDETAILSF_FORMAT))
	{
		lstrcpyn(tx, afd.szFormat, txCch);
	}
}

void _inline ACM_gettext(char* tx)
{
	ACMFORMATTAGDETAILS aftd;
	ZeroMemory(&aftd,sizeof(aftd));
	aftd.cbStruct=sizeof(aftd);
	aftd.dwFormatTag=convert_wfx_temp.wfx.wFormatTag;
	if (!acmFormatTagDetails(0,&aftd,ACM_FORMATTAGDETAILSF_FORMATTAG))
	{
		char* p=aftd.szFormatTag;
		while(p && *p) *(tx++)=*(p++);
		*(tx++)=13;
		*(tx++)=10;
	}	
	ACMFORMATDETAILS afd;
	ZeroMemory(&afd,sizeof(afd));
	afd.cbStruct=sizeof(afd);
	afd.dwFormatTag=convert_wfx_temp.wfx.wFormatTag;
	afd.pwfx=&convert_wfx_temp.wfx;
	afd.cbwfx=sizeof(convert_wfx_temp);
	if (!acmFormatDetails(0,&afd,ACM_FORMATDETAILSF_FORMAT))
	{
		char* p=afd.szFormat;
		while(p && *p) *(tx++)=*(p++);
	}
	*tx=0;
}

void ACM_choose(HWND w,bool pcm)
{
	ACMFORMATCHOOSE afc;
	memset(&afc,0,sizeof(afc));
	afc.cbStruct=sizeof(afc);
	afc.fdwStyle=ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT;
	if (pcm)
	{
		singlefile_wfx_temp.wFormatTag=WAVE_FORMAT_PCM;
		afc.pwfxEnum=&singlefile_wfx_temp;
		afc.fdwEnum=ACM_FORMATENUMF_WFORMATTAG;
		afc.pwfx = &singlefile_wfx_temp;
		afc.cbwfx = sizeof(singlefile_wfx_temp);
	}
	else
	{
		afc.pwfx = &convert_wfx_temp.wfx;
		afc.cbwfx = sizeof(convert_wfx_temp);
	}

	afc.hwndOwner=w;

	if (!acmFormatChoose(&afc))
	{
		if (pcm)
		{
			SetDlgItemText(w,IDC_SINGLEFILE_FORMAT_BUTTON,afc.szFormat);
		}
		else
		{
			char tmp[512] = {0};
			StringCchPrintf(tmp, 512, "%s\x0d\x0a%s",afc.szFormatTag,afc.szFormat);
			SetDlgItemText(w,IDC_CONVERT_BUTTON,tmp);
		}
	}
}

void _inline do_acm_text(HWND wnd)
{
	char tmp[256] = {0};
	ACM_gettext(tmp);
	SetDlgItemText(wnd,IDC_CONVERT_BUTTON,tmp);
}

void _inline do_acm_text1(HWND wnd)
{
	char tmp[256] = {0};
	ACM_gettext1(tmp, 256);
	SetDlgItemText(wnd,IDC_SINGLEFILE_FORMAT_BUTTON,tmp);
}

void wav1_set(HWND w,bool b)
{
	static struct
	{
		WORD id;
		bool t;
	} wav1_w_c[]=
	{
		{IDC_OUTPUT_STATIC,1},
		{IDC_OUTPUT_DIRECTORY_STATIC,1},
		{IDC_OUTPUT_DIRECTORY,1},
		{IDC_OUTPUT_SRCDIR,1},
		{IDC_FILENAME_SAVEAS,1},
		{IDC_FILENAME_INDEX_STATIC,1},
		{IDC_FILENAME_INDEX,1},
		{IDC_OUTMODE_STATIC,1},
		{IDC_OUTMODE,1},
		{IDC_CONVERT_STATIC,1},
		{IDC_CONVERT_CHECK,1},
		{IDC_CONVERT_BUTTON,1},
		{IDC_CONVERT_NOTE,1},
		{IDC_FILENAME_FORMAT,1},
		{IDC_FILENAME_FORMAT_STATIC,1},
		{IDC_SINGLEFILE_FILE_STATIC,0},
		{IDC_SINGLEFILE_FILE_BUTTON,0},
		{IDC_SINGLEFILE_FORMAT_STATIC,0},
		{IDC_SINGLEFILE_FORMAT_BUTTON,0},
	};

	UINT n;
	for(n=0;n<tabsize(wav1_w_c);n++)
	{
		EnableWindow(GetDlgItem(w,wav1_w_c[n].id),wav1_w_c[n].t^b);
	}
}

BOOL CALLBACK DlgProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
	{
		wchar_t title[128] = {0}, temp[128] = {0};
		StringCchPrintfW(title,128,WASABI_API_LNGSTRINGW(IDS_SETTINGS_TITLE),WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_DISK_WRITER_OLD, temp, 128));
		SetWindowTextW(wnd,title);
		SetDlgItemText(wnd,IDC_OUTPUT_DIRECTORY,cfg_output_dir);
		SetPathChoiceButtonText(wnd,cfg_output_dir,IDC_OUTPUT_DIRECTORY);
		lstrcpyn(g_tmp,cfg_output_dir,MAX_PATH);
		SetCheck(wnd,IDC_CONVERT,cfg_convert_enabled);
		memcpy(&convert_wfx_temp,&convert_wfx,sizeof(convert_wfx));
		memcpy(&singlefile_wfx_temp,&singlefile_wfx,sizeof(WAVEFORMATEX));
		SetCheck(wnd,IDC_SINGLEFILE_CHECK,cfg_singlefile_enabled);
		SetCheck(wnd,IDC_THREAD_HACK,cfg_thread_override);
		SetPathChoiceButtonText(wnd,cfg_singlefile_output,IDC_SINGLEFILE_FILE_BUTTON);
		lstrcpyn(g_tmp_sf,cfg_singlefile_output,MAX_PATH);
		do_acm_text(wnd);
		do_acm_text1(wnd);

		{
			HWND w=GetDlgItem(wnd,IDC_MODE);
			int n;
			for(n=0;n<tabsize(mode_names_idx);n++)
				SendMessageW(w,CB_ADDSTRING,0,
#ifdef WIN64
					(long long)WASABI_API_LNGSTRINGW(mode_names_idx[n])
#else
					(long)WASABI_API_LNGSTRINGW(mode_names_idx[n])
#endif
				);
			SendMessage(w,CB_SETCURSEL,cfg_output_mode,0);

			w=GetDlgItem(wnd,IDC_FILENAME_FORMAT);
			for(n=0;n<tabsize(format_names);n++)
				SendMessageW(w,CB_ADDSTRING,0,
#ifdef WIN64
				(long long)format_names[n]
#else
				(long)format_names[n]
#endif			
				);
			SendMessage(w,CB_SETCURSEL,cfg_format_mode,0);

			w=GetDlgItem(wnd,IDC_FILENAME_INDEX);
			for(n=0;n<tabsize(index_name_idx);n++)
				SendMessageW(w,CB_ADDSTRING,0,
#ifdef WIN64
				(long long)WASABI_API_LNGSTRINGW(index_name_idx[n])
#else
				(long)WASABI_API_LNGSTRINGW(index_name_idx[n])
#endif
				);
			SendMessage(w,CB_SETCURSEL,cfg_format_index,0);
		}

		wav1_set(wnd,cfg_singlefile_enabled);

		SetCheck(wnd,IDC_OUTPUT_SRCDIR,cfg_output_source_dir);
		SetCheck(wnd,IDC_FILENAME_SAVEAS,cfg_show_saveas);

		return 1;
	}
	case WM_COMMAND:
		if (wp==IDC_CONVERT_BUTTON)
		{
			ACM_choose(wnd,0);
		}
		else if (wp==IDC_OUTPUT_DIRECTORY)
		{
			d_browza(wnd,(HWND)lp,WASABI_API_LNGSTRINGW(IDS_SELECT_OUTPUT_DIRECTORY));
		}
		else if (wp==IDCANCEL) EndDialog(wnd,0);
		else if (wp==IDOK)
		{
			cfg_convert_enabled=GetCheck(wnd,IDC_CONVERT);
			lstrcpyn(cfg_output_dir,g_tmp,MAX_PATH);
			memcpy(&convert_wfx,&convert_wfx_temp,sizeof(convert_wfx));
			memcpy(&singlefile_wfx,&singlefile_wfx_temp,sizeof(WAVEFORMATEX));
			cfg_singlefile_enabled=GetCheck(wnd,IDC_SINGLEFILE_CHECK);
			cfg_thread_override=GetCheck(wnd,IDC_THREAD_HACK);
			cfg_output_mode=(int)SendDlgItemMessage(wnd,IDC_MODE,CB_GETCURSEL,0,0);
			lstrcpyn(cfg_singlefile_output,g_tmp_sf,MAX_PATH);
			cfg_output_source_dir=GetCheck(wnd,IDC_OUTPUT_SRCDIR);
			cfg_show_saveas=GetCheck(wnd,IDC_FILENAME_SAVEAS);

			cfg_format_mode=(int)SendDlgItemMessage(wnd,IDC_FILENAME_FORMAT,CB_GETCURSEL,0,0);
			cfg_format_index=(int)SendDlgItemMessage(wnd,IDC_FILENAME_INDEX,CB_GETCURSEL,0,0);

			EndDialog(wnd,0);
		}
		else if (wp==IDC_SINGLEFILE_FILE_BUTTON)
		{
			char tmp[MAX_PATH] = {0}, filter[64] = {0};

			StringCchPrintf(filter,64, WASABI_API_LNGSTRING(IDS_X_FILES_DOT_X),"WAV",".wav");
			char * ptr=filter;
			while(ptr && *ptr)
			{
				if (*ptr=='|') *ptr=0;
				ptr++;
			}

			GetDlgItemText(wnd,IDC_SINGLEFILE_FILE_BUTTON,tmp,MAX_PATH);
			lstrcpyn(tmp,g_tmp_sf,MAX_PATH);
			OPENFILENAME ofn = {0};
			ofn.lStructSize=sizeof(ofn);
			ofn.hwndOwner=wnd;
			ofn.lpstrFilter=filter;
			ofn.lpstrFile=tmp;
			ofn.nMaxFile=MAX_PATH;
			ofn.Flags=OFN_HIDEREADONLY|OFN_PATHMUSTEXIST;
			ofn.lpstrDefExt="wav";
			if (GetSaveFileName(&ofn))
			{
				SetPathChoiceButtonText(wnd,tmp,IDC_SINGLEFILE_FILE_BUTTON);
				lstrcpyn(g_tmp_sf, tmp, MAX_PATH);
			}
		}
		else if (wp==IDC_SINGLEFILE_FORMAT_BUTTON) ACM_choose(wnd,1);
		else if (wp==IDC_SINGLEFILE_CHECK) wav1_set(wnd,GetCheck(wnd,IDC_SINGLEFILE_CHECK));

		break;
	}
	return 0;
}

void Config(HWND hWnd)
{
	WASABI_API_DIALOGBOXW(IDD_DIALOG1,hWnd,DlgProc);
}

static int nsam;
static int g_freq,g_nch,g_bps;
static int paused;

const static char badchars[]={'.','\\','/',':','*','?','\"','<','>','|'};

char* GetDefaultSaveToFolder(char* path_to_store)
{
	if(FAILED(SHGetFolderPathA(NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, path_to_store)))
	{
		if(FAILED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path_to_store)))
		{
			lstrcpynA(path_to_store, "C:\\", MAX_PATH);
		}
	}
	return path_to_store;
}

static void GetName(char* out, int outCch, const char* format_ext)
{
	int index = (int)SendMessage(mod.hMainWindow,WM_USER,0,IPC_GETLISTPOS);

	if (cfg_output_source_dir)
	{
		const char * dir=(const char *)SendMessage(mod.hMainWindow,WM_USER,index,211);
		const char * last=strrchr(dir,'\\');
		while(dir<=last) {*(out++)=*(dir++);}
	}
	else
	{
		char * dir=cfg_output_dir;
		if (!*dir)
		{
			char tmp[MAX_PATH] = {0};
			dir=GetDefaultSaveToFolder(tmp);
		}
		while(dir && *dir) {*(out++)=*(dir++);}
		if (out[-1]!='\\') *(out++)='\\';
	}

	char * badchar_test = out;

	if (cfg_format_index>0)
	{
		char temp[16] = {0};
		StringCchPrintf(temp, 16, "%09u_",index+1);
		lstrcpyn(out, temp+9-cfg_format_index, outCch);
		out+=cfg_format_index+1;
	}

	if (cfg_format_mode&1)//filename
	{
		const char * source_full = (const char*)SendMessage(mod.hMainWindow,WM_USER,index,211);
		const char * source = strrchr(source_full,'\\');
		if (!source) source = source_full;
		else source++;

		const char * dot = strrchr(source,'.');
		if (dot) while(source<dot) *(out++)=*(source++);
		else while(source && *source) *(out++)=*(source++);		
	}
	else	//title
	{
		const char * source = (const char*)SendMessage(mod.hMainWindow,WM_USER,index,212);
		if (!source) source = "(unknown title)";
		while(source && *source) *(out++)=*(source++);
	}

	if (cfg_format_mode&2)
	{//add extension
		const char * extension = strrchr((const char*)SendMessage(mod.hMainWindow,WM_USER,index,211),'.');
		while(extension && *extension) *(out++)=*(extension++);
	}


	while(badchar_test<out)
	{
		BYTE c = *badchar_test;
		int n;
		for(n=0;n<tabsize(badchars);n++)
		{
			if (c==badchars[n]) {*badchar_test='_';break;}
		}

		badchar_test++;
	}

	while(format_ext && *format_ext) *(out++)=*(format_ext++);
	*out=0;
}

static HANDLE hOut=INVALID_HANDLE_VALUE;

static HACMSTREAM hStream;

static DWORD fact_ofs,data_ofs;
static bool riff;

static DWORD FileTell(HANDLE hFile) {return SetFilePointer(hFile,0,0,FILE_CURRENT);}
static void FileAlign(HANDLE hFile) {if (FileTell(hFile)&1) SetFilePointer(hFile,1,0,FILE_CURRENT);}

#define BUFSIZE 0x20000

static BYTE *acm_outbuf;
static UINT acm_outbufsize;
static BYTE *acm_buf;
static BYTE *acm_buf1;
static UINT inbuf,inbuf1;

static ACMSTREAMHEADER ahd,ahd1;
static bool active;

static void file_err(char* f, wchar_t* t)
{
	wchar_t tmp[512+128] = {0};
	StringCchPrintfW(tmp, 512+128, L"%s: \"%hs\"", t, f);
	MessageBoxW(mod.hMainWindow, tmp, szDescription, MB_ICONERROR);
}

void acm_close();

static int pos_delta;

int Open(int sr,int nch,int bps,int xx,int xxx)
{
	// if someone didn't call Close(), close the file for them
	if (hOut!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hOut);
		hOut=INVALID_HANDLE_VALUE;
	}
	int failFlags = 0;
	SYNCFUNC;
	char fn[512] = {0};
	use_convert=cfg_convert_enabled;
	DWORD bw = 0, t = 0;
	WAVEFORMATEX wfx=
	{
		WAVE_FORMAT_PCM,
			(WORD)nch,
			(DWORD)sr,
			(DWORD)(sr*nch*(bps>>3)),
			(WORD)(nch*(bps>>3)),
			(WORD)bps,
			0
	};
	bool append=0;

	WAVEFORMATEX *pDst=&convert_wfx.wfx;
	if (!cfg_convert_enabled) pDst=&wfx;
	if (cfg_singlefile_enabled)
	{
		pDst=&singlefile_wfx;
		use_convert=1;
		lstrcpyn(fn, cfg_singlefile_output,512);
		riff=1;
	}
	else
	{
		const char* ext=".wav";
		riff=1;
		if (cfg_output_mode==MODE_RAW)
		{
			riff=0;
			ext=".raw";
		}
		else if (cfg_output_mode==MODE_AUTO)
		{
			if (pDst->wFormatTag==WAVE_FORMAT_MPEGLAYER3)
			{
				riff=0;
				ext=".mp3";
			}
		}
		GetName(fn, 512, ext);
		if (cfg_show_saveas)
		{
			char filter[64] = {0}, title[128] = {0}, title2[128] = {0};
			StringCchPrintf(filter,64, WASABI_API_LNGSTRING(IDS_X_FILES_DOT_X),ext,ext);
			char * ptr=filter;
			while(ptr && *ptr)
			{
				if (*ptr=='|') *ptr=0;
				ptr++;
			}

			OPENFILENAME ofn= {0};
			ofn.lStructSize=sizeof(ofn);
			ofn.hwndOwner=mod.hMainWindow;
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = fn;
			StringCchPrintf(title,128,WASABI_API_LNGSTRING(IDS_CHOOSE_FILE_NAME),
							WASABI_API_LNGSTRING_BUF(IDS_NULLSOFT_DISK_WRITER,title2,128));
			ofn.lpstrTitle = title;
			ofn.nMaxFile = tabsize(fn);
			ofn.Flags = OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
			ofn.lpstrDefExt = ext;			
			if (!GetOpenFileName(&ofn)) return -1;
		}
	}

	if (memcmp(&wfx,pDst,sizeof(wfx))==0) use_convert=0;

	nsam=0;
	g_freq=sr;
	g_nch=nch;
	g_bps=bps;
	paused=0;

	SetLastError(0);
	hOut=CreateFile(fn,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0);

	if (hOut==INVALID_HANDLE_VALUE)
	{
		DWORD e=GetLastError();

		if (e==ERROR_ALREADY_EXISTS || e==0x50)
		{
			if (cfg_singlefile_enabled)
			{
				append=1;
				goto _ap;
			}
			wchar_t tmp[512+128] = {0};
			StringCchPrintfW(tmp, 512+128, WASABI_API_LNGSTRINGW(IDS_FILE_ALREADY_EXISTS_OVERWRITE),fn);
			if (MessageBoxW(mod.hMainWindow,tmp,szDescription,MB_ICONQUESTION|MB_OKCANCEL)==IDOK)
			{
				hOut=CreateFile(fn,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
			}
			else return -1;
		}
		if (hOut==INVALID_HANDLE_VALUE)
		{
			file_err(fn, WASABI_API_LNGSTRINGW(IDS_CANNOT_CREATE_OUTPUT_FILE));
			return -1;
		}
	}
_ap:
	fact_ofs=data_ofs=0;
	if (append)
	{

		hOut=CreateFile(fn,GENERIC_WRITE|GENERIC_READ,0,0,OPEN_EXISTING,0,0);
		if (hOut==INVALID_HANDLE_VALUE)
		{
			file_err(fn, WASABI_API_LNGSTRINGW(IDS_CANNOT_OPEN_FILE));
			return -1;
		}
		{
			DWORD br=0;
			DWORD rf=0,wfs=0;
			ReadFile(hOut,&rf,4,&br,0);
			if (rf!='FFIR') goto ap_f;
			SetFilePointer(hOut,4,0,FILE_CURRENT);
			br=0; ReadFile(hOut,&rf,4,&br,0);
			if (rf!='EVAW') goto ap_f;
			br=0; ReadFile(hOut,&rf,4,&br,0);
			if (rf!=' tmf') goto ap_f;
			static WAVEFORMATEX ap_wfx;
			br=0; ReadFile(hOut,&wfs,4,&br,0);
			if (wfs<sizeof(ap_wfx)-2 || wfs>sizeof(ap_wfx)) goto ap_f;
			br=0; ReadFile(hOut,&ap_wfx,wfs,&br,0);
			if (ap_wfx.wFormatTag!=WAVE_FORMAT_PCM) goto ap_f;
			br=0; ReadFile(hOut,&rf,4,&br,0);
			pDst=&ap_wfx;
			if (rf!='atad') goto ap_f;
			data_ofs=8+4+8+wfs+4;
			DWORD data_size=0;
			br=0; ReadFile(hOut,&data_size,4,&br,0);
			SetFilePointer(hOut,data_size,0,FILE_CURRENT);
			SetEndOfFile(hOut);
			use_convert = !!memcmp(&wfx,&ap_wfx,sizeof(wfx));
		}
		goto ap_ok;
ap_f:
		file_err(fn, WASABI_API_LNGSTRINGW(IDS_NOT_A_PCM_WAV_FILE));
		if (hOut && hOut!=INVALID_HANDLE_VALUE) {CloseHandle(hOut);hOut=INVALID_HANDLE_VALUE;}
		return -1;
ap_ok:;
	}

	if (riff && !append)
	{
		t=rev32('RIFF');
		bw = 0; WriteFile(hOut,&t,4,&bw,0);
		SetFilePointer(hOut,4,0,FILE_CURRENT);
		t=rev32('WAVE');
		bw = 0; WriteFile(hOut,&t,4,&bw,0);
		t=rev32('fmt ');
		bw = 0; WriteFile(hOut,&t,4,&bw,0);
	}

	if (use_convert)
	{
#ifdef SSRC
		if (pDst->wFormatTag==WAVE_FORMAT_PCM)
		{
			if ((wfx.nChannels!=1 && wfx.nChannels!=2) || (pDst->nChannels!=1 && pDst->nChannels!=2)) goto fail;

			res = SSRC_create(wfx.nSamplesPerSec,pDst->nSamplesPerSec,wfx.wBitsPerSample,pDst->wBitsPerSample,wfx.nChannels,2,1,0,0,wfx.nChannels!=pDst->nChannels);

			acm_outbuf=0;
			acm_outbufsize=0;
		}
		else
#endif
		{
			MMRESULT rs=acmStreamOpen(&hStream,0,&wfx,pDst,0,0,0,ACM_STREAMOPENF_NONREALTIME);
			if (rs)
			{
				WAVEFORMATEX wfx1;
				ZeroMemory(&wfx1,sizeof(wfx1));
				wfx1.wFormatTag=WAVE_FORMAT_PCM;
				if (acmFormatSuggest(0,pDst,&wfx1,sizeof(WAVEFORMATEX),ACM_FORMATSUGGESTF_WFORMATTAG)) goto fail;
				if (acmStreamOpen(&hStream,0,&wfx1,pDst,0,0,0,ACM_STREAMOPENF_NONREALTIME)) goto fail;

				if ((wfx.nChannels!=1 && wfx.nChannels!=2) || (wfx1.nChannels!=1 && wfx1.nChannels!=2)) goto fail;
#ifdef SSRC
				res = SSRC_create(wfx.nSamplesPerSec,wfx1.nSamplesPerSec,wfx.wBitsPerSample,wfx1.wBitsPerSample,wfx.nChannels,2,1,0,0,wfx.nChannels!=wfx1.nChannels);
				//TODO fix different channel setups
#endif				
				acm_buf1=(BYTE*)malloc(BUFSIZE);

				if (!acm_buf1 
#ifdef SSRC
					|| !res
#endif				
					)
					goto fail;
			}
			acm_buf=(BYTE*)malloc(BUFSIZE);
			acm_outbuf=(BYTE*)malloc(BUFSIZE);
			if (!acm_buf || !acm_outbuf) goto fail;
			ZeroMemory(&ahd,sizeof(ahd));
			ahd.cbStruct=sizeof(ahd);
			ahd.pbSrc=acm_buf;
			ahd.cbSrcLength=BUFSIZE;
			ahd.pbDst=acm_outbuf;
			ahd.cbDstLength=BUFSIZE;
			if (acmStreamPrepareHeader(hStream,&ahd,0)) goto fail;
		}

		if (riff && !append)
		{
			if (pDst->wFormatTag==WAVE_FORMAT_PCM) t=0x10;
			else t=sizeof(WAVEFORMATEX)+pDst->cbSize;
			bw = 0; WriteFile(hOut,&t,4,&bw,0);
			bw = 0; WriteFile(hOut,pDst,t,&bw,0);
			FileAlign(hOut);

			if (pDst->wFormatTag!=WAVE_FORMAT_PCM)
			{
				t=rev32('fact');
				bw = 0; WriteFile(hOut,&t,4,&bw,0);
				t=4;
				bw = 0; WriteFile(hOut,&t,4,&bw,0);
				fact_ofs=FileTell(hOut);
				SetFilePointer(hOut,4,0,FILE_CURRENT);
			}

			t=rev32('data');
			bw = 0; WriteFile(hOut,&t,4,&bw,0);
			data_ofs=FileTell(hOut);
			SetFilePointer(hOut,4,0,FILE_CURRENT);
		}
	}
	else if (riff && !append)
	{
		t=0x10;
		//t=sizeof(WAVEFORMATEX)+pDst->cbSize;
		bw = 0; WriteFile(hOut,&t,4,&bw,0);
		bw = 0; WriteFile(hOut,&wfx,t,&bw,0);
		t=rev32('data');
		bw = 0; WriteFile(hOut,&t,4,&bw,0);
		data_ofs=FileTell(hOut);
		SetFilePointer(hOut,4,0,FILE_CURRENT);		
	}
	inbuf1=inbuf=0;

	active=1;

	pos_delta=0;

	return 0;

fail:
	if (hOut && hOut!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hOut);
		hOut=INVALID_HANDLE_VALUE;
		DeleteFile(fn);
	}
	hOut=0;
	acm_close();
	MessageBoxW(mod.hMainWindow,WASABI_API_LNGSTRINGW(IDS_ERROR_INITIALIZING_OUTPUT),
				szDescription,MB_ICONERROR);
	return -1;
}

void acm_close()
{
#ifdef SSRC
	if (res)
	{
		if (acm_buf1)
		{
			free(acm_buf1);
			acm_buf1=0;
		}
		delete res;
		res=0;
	}
#endif
	if (hStream)
	{
		if (ahd.fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED) acmStreamUnprepareHeader(hStream,&ahd,0);
		ZeroMemory(&ahd,sizeof(ahd));
		acmStreamClose(hStream,0);
		hStream=0;
		if (acm_buf)
		{
			free(acm_buf);
			acm_buf=0;
		}
		if (acm_outbuf)
		{
			free(acm_outbuf);
			acm_outbuf=0;
		}
	}
}

void do_cvt(BYTE* data,UINT size);

void Close()
{
	SYNCFUNC;
	active=0;
	if (use_convert)
	{
		do_cvt(0,0);
		acm_close();
	}
	if (hOut!=INVALID_HANDLE_VALUE)
	{
		if (riff)
		{
			FileAlign(hOut);

			DWORD t,bw = 0;
			SetFilePointer(hOut,4,0,FILE_BEGIN);
			t=GetFileSize(hOut,0)-8;
			WriteFile(hOut,&t,4,&bw,0);
			if (data_ofs)
			{
				DWORD data_size=GetFileSize(hOut,0)-(data_ofs+4);
				SetFilePointer(hOut,data_ofs,0,FILE_BEGIN);
				bw = 0; WriteFile(hOut,&data_size,4,&bw,0);
			}
			if (fact_ofs)
			{
				SetFilePointer(hOut,fact_ofs,0,FILE_BEGIN);
				t=nsam;
				bw = 0; WriteFile(hOut,&t,4,&bw,0);
			}
		}
		CloseHandle(hOut);
		hOut=INVALID_HANDLE_VALUE;
	}
}

void do_cvt(BYTE* data,UINT size)
{
#ifdef SSRC
	if (res && !hStream)
	{
		if (!data || !size) res->Finish();
		else res->Write(data,size);

		UINT out_size;
		void *out = res->GetBuffer(&out_size);
		DWORD bw = 0;
		WriteFile(hOut,out,out_size,&bw,0);
		res->Read(out_size);

	}
	else
#endif
	{
		DWORD flags=0;
		if (nsam==0) flags|=ACM_STREAMCONVERTF_START;
		if (data) flags|=ACM_STREAMCONVERTF_BLOCKALIGN;
#ifdef SSRC
		if (res)
		{
			if (inbuf1+size>BUFSIZE) return;
			if (data)
			{
				memcpy(acm_buf1+inbuf1,data,size);
				inbuf1+=size;
			}
			res->Write(acm_buf1,inbuf1);
			memcpy(acm_buf1,acm_buf1+inbuf1,inbuf1);
			inbuf1=0;
			if (!data || !size) res->Finish();
			data = (BYTE*)res->GetBuffer(&size);
			if (inbuf+size>BUFSIZE) return;
			memcpy(acm_buf+inbuf,data,size);
			inbuf+=size;
			res->Read(size);
		}
		else 
#endif
			if (data)
			{
				if (inbuf+size>BUFSIZE) return;
				memcpy(acm_buf+inbuf,data,size);
				inbuf+=size;
			}

			ahd.cbSrcLength=inbuf;
			acmStreamConvert(hStream,&ahd,flags);
			inbuf-=ahd.cbSrcLengthUsed;
			memmove(acm_buf,acm_buf+ahd.cbSrcLengthUsed,inbuf);//memmove
			DWORD bw = 0;
			WriteFile(hOut,acm_outbuf,ahd.cbDstLengthUsed,&bw,0);
	}
}

int Write(char *data, int len)
{
	SYNCFUNC;
	if (!active) return 0;
	if (cfg_thread_override) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
	else Sleep(0);

	len-=len%((g_bps>>3)*g_nch);

	nsam+=len/((g_bps>>3)*g_nch);

	if (use_convert)
	{
		do_cvt((BYTE*)data,len);
	}
	else
	{
		DWORD bw = 0;
		WriteFile(hOut,data,len,&bw,0);
	}


	return 0;
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
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_DISK_WRITER_OLD,text,1024);
	StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					 szDescription, __DATE__);
	DoAboutMessageBox(hwndParent,text,message);
}

static bool _cfg_store;
static const char *inifile;

#define CFG_NAME "out_disk"//fixme

static void wppi(char* nam,UINT val)
{
	char t[16] = {0};
	StringCchPrintf(t,16, "%u",val);
	WritePrivateProfileString(CFG_NAME,nam,t,inifile);
}

#define WritePrivateProfileInt(A,B,C,D) wppi(B,C)

static int _do_int(int x,char* nam)
{
	if (_cfg_store)
	{
		WritePrivateProfileInt(CFG_NAME,nam,x,inifile);
		return x;
	}
	else
	{
		return GetPrivateProfileInt(CFG_NAME,nam,x,inifile);
	}
}

#define do_int(x) x=_do_int(x,#x)
#define do_bool(x) x=!!_do_int(x,#x)

static void _do_string(char* x,char* nam)
{
	if (_cfg_store)
	{
		WritePrivateProfileString(CFG_NAME,nam,x,inifile);
	}
	else
	{
		GetPrivateProfileString(CFG_NAME,nam,x,x,MAX_PATH,inifile);
	}
}

#define do_string(x) _do_string(x,#x)

void do_cfg()
{
	if (!_cfg_store)
	{
		GetDefaultSaveToFolder(cfg_output_dir);
		PathCombine(cfg_singlefile_output, cfg_output_dir, "output.wav");
	}

	do_string(cfg_output_dir);
	do_string(cfg_singlefile_output);
	do_bool(cfg_singlefile_enabled);
	do_bool(cfg_convert_enabled);
	do_bool(cfg_thread_override);
	do_int(cfg_output_mode);
	do_bool(cfg_output_source_dir);
	do_bool(cfg_show_saveas);
	do_int(cfg_format_mode);
	do_int(cfg_format_index);


	if (_cfg_store)
	{
		UINT _s=sizeof(WAVEFORMATEX)+convert_wfx.wfx.cbSize;
		WritePrivateProfileInt(CFG_NAME,"cfg_wfx_s",_s,inifile);
		WritePrivateProfileStruct(CFG_NAME,"cfg_wfx",&convert_wfx,_s,inifile);
		WritePrivateProfileStruct(CFG_NAME,"cfg_wfx1",&singlefile_wfx,sizeof(singlefile_wfx),inifile);
	}
	else
	{
		UINT _s=GetPrivateProfileInt(CFG_NAME,"cfg_wfx_s",0,inifile);
		if (_s && _s<=sizeof(convert_wfx))
		{
			GetPrivateProfileStruct(CFG_NAME,"cfg_wfx",&convert_wfx,_s,inifile);
		}
		GetPrivateProfileStruct(CFG_NAME,"cfg_wfx1",&singlefile_wfx,sizeof(singlefile_wfx),inifile);
	}
}

void Init()
{
	SYNCFUNC;
	if (!mod.hMainWindow)
		return;
	inifile = (const char *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE);
	singlefile_wfx = wfx_default;
	convert_wfx.wfx = wfx_default;
	memset(convert_wfx.crap, 0, sizeof(convert_wfx.crap));

	// loader so that we can get the localisation service api for use
	WASABI_API_SVC = (api_service*)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (WASABI_API_SVC == (api_service*)1) WASABI_API_SVC = NULL;

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mod.hDllInstance,OutDiskLangGUID);

	StringCchPrintfW(szDescription, 256, WASABI_API_LNGSTRINGW(IDS_NULLSOFT_DISK_WRITER), PLUGIN_VERSION);
	mod.description = (char*)szDescription;

	_cfg_store=0;
	do_cfg();
}

void Quit()
{
	SYNCFUNC;
	if (active) Close();

	_cfg_store=1;
	do_cfg();
}

int CanWrite()
{
	return paused ? 0 : 666666666;
}

int IsPlaying()
{
	return 0;
}

int Pause(int p)
{
	int _p=paused;
	paused=p;
	return _p;
}

void SetVolume(int volume)
{
}

void SetPan(int pan)
{
}

void Flush(int t)
{
	nsam=0;
	pos_delta=t;
}

int GetOutputTime()
{
	return pos_delta+MulDiv(nsam,1000,g_freq);
}

Out_Module mod=
{
	OUT_VER_U,
	0,
	426119909,
	0,
	0,
	Config,
	About,
	Init,
	Quit,
	Open,
	Close,
	Write,
	CanWrite,
	IsPlaying,
	Pause,
	SetVolume,
	SetVolume,
	Flush,
	GetOutputTime,
	GetOutputTime
};

extern "C"
{
	__declspec( dllexport ) Out_Module * winampGetOutModule()
	{
		return &mod;
	}
}

BOOL APIENTRY DllMain(HANDLE hMod,DWORD r,void*)
{
	if (r==DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls((HMODULE)hMod);
	}
	return TRUE;
}