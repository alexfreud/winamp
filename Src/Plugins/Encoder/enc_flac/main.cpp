#include "api__enc_flac.h"
#include "../Winamp/wa_ipc.h"
#include "../nsv/enc_if.h"
#include "../nu/AutoWideFn.h"
#include "AudioCoderFlac.h"
#include "resource.h"
#include <commctrl.h>
#include <windows.h>
#include <Uxtheme.h>
#include <api/service/waservicefactory.h>
#include <api/application/api_application.h>
#include <mmsystem.h>
#include <strsafe.h>

#define ENC_VERSION "2.46"

HWND winampwnd = 0;
int isthemethere = 0;
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
api_application *WASABI_API_APP = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

typedef struct
{
	configtype cfg;
	char configfile[MAX_PATH];
}
configwndrec;

void readconfig(const char *configfile, configtype *cfg)
{
	if (configfile) 
	{
		cfg->compression = GetPrivateProfileIntA("audio_flac", "compression", 5, configfile);
	}
	else
	{
		cfg->compression = 5;
	}
}

void writeconfig(const char *configfile, configtype *cfg)
{
	if (configfile) 
	{
		char str[64] = {0};
		StringCchPrintfA(str, 64, "%u", cfg->compression);
		WritePrivateProfileStringA("audio_flac", "compression", str, configfile);
	}
}

static HINSTANCE GetMyInstance()
{
	MEMORY_BASIC_INFORMATION mbi = {0};
	if(VirtualQuery(GetMyInstance, &mbi, sizeof(mbi)))
		return (HINSTANCE)mbi.AllocationBase;
	return NULL;
}

void GetLocalisationApiService(void)
{
	if(!WASABI_API_LNG)
	{
		// loader so that we can get the localisation service api for use
		if(!WASABI_API_SVC)
		{
			WASABI_API_SVC = (api_service*)SendMessage(winampwnd, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
			if (WASABI_API_SVC == (api_service*)1)
			{
				WASABI_API_SVC = NULL;
				return;
			}
		}

		if(!WASABI_API_APP)
		{
			waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
			if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());
		}

		if(!WASABI_API_LNG)
		{
			waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
			if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());
		}

		// need to have this initialised before we try to do anything with localisation features
		WASABI_API_START_LANG(GetMyInstance(),EncFlacLangGUID); 
	}
}

static const char *GetFLACVersion()
{
	return "1.4.2";
}

static HCURSOR link_hand_cursor;
LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"link_proc"), hwndDlg, uMsg, wParam, lParam);
	// override the normal cursor behaviour so we have a hand to show it is a link
	if(uMsg == WM_SETCURSOR)
	{
		if((HWND)wParam == hwndDlg)
		{
			if(!link_hand_cursor)
			{
				link_hand_cursor = LoadCursor(NULL, IDC_HAND);
			}
			SetCursor(link_hand_cursor);
			return TRUE;
		}
	}
	return ret;
}

void link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_DRAWITEM)
	{
		DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
		if (di->CtlType == ODT_BUTTON)
		{
			wchar_t wt[123] = {0};
			int y;
			RECT r;
			HPEN hPen, hOldPen;
			GetDlgItemTextW(hwndDlg, (int)wParam, wt, sizeof(wt)/sizeof(wt[0])); 

			// due to the fun of theming and owner drawing we have to get the background colour
			if(isthemethere){
				HTHEME hTheme = OpenThemeData(hwndDlg, L"Tab");
				if (hTheme) {
					DrawThemeParentBackground(di->hwndItem, di->hDC, &di->rcItem);
					CloseThemeData(hTheme);
				}
			}

			// draw text
			SetTextColor(di->hDC, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			r = di->rcItem;
			r.left += 2;
			DrawTextW(di->hDC, wt, -1, &r, DT_VCENTER | DT_SINGLELINE);

			memset(&r, 0, sizeof(r));
			DrawTextW(di->hDC, wt, -1, &r, DT_SINGLELINE | DT_CALCRECT);

			// draw underline
			y = di->rcItem.bottom - ((di->rcItem.bottom - di->rcItem.top) - (r.bottom - r.top)) / 2 - 1;
			hPen = CreatePen(PS_SOLID, 0, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			hOldPen = (HPEN) SelectObject(di->hDC, hPen);
			MoveToEx(di->hDC, di->rcItem.left + 2, y, NULL);
			LineTo(di->hDC, di->rcItem.right + 2 - ((di->rcItem.right - di->rcItem.left) - (r.right - r.left)), y);
			SelectObject(di->hDC, hOldPen);
			DeleteObject(hPen);
		}
	}
}

void link_startsubclass(HWND hwndDlg, UINT id)
{
HWND ctrl = GetDlgItem(hwndDlg, id);
	if(!GetPropW(ctrl, L"link_proc"))
	{
		SetPropW(ctrl, L"link_proc",
				(HANDLE)SetWindowLongPtrW(ctrl, GWLP_WNDPROC, (LONG_PTR)link_handlecursor));
	}
}

BOOL CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static configwndrec *wr;
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			wr = (configwndrec *)lParam;
			SendMessage(GetDlgItem(hwndDlg,IDC_COMPRESSIONSLIDER),TBM_SETRANGE,TRUE,MAKELONG(0,8));
			SendMessage(GetDlgItem(hwndDlg,IDC_COMPRESSIONSLIDER),TBM_SETPOS,TRUE,wr->cfg.compression);
			const char *libFlacVer = GetFLACVersion();
			if (libFlacVer && *libFlacVer)
			{
				char flac_string[1024] = {0};
				StringCchPrintfA(flac_string, 1024, "libFLAC v%s",libFlacVer);
				SetDlgItemTextA(hwndDlg, IDC_STATIC_FLAC_VER, flac_string);
			}
			link_startsubclass(hwndDlg, IDC_URL);
		}
		break;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_URL)
			{
				SendMessage(winampwnd, WM_WA_IPC, (WPARAM)"http://flac.sf.net/", IPC_OPEN_URL);
			}
		break;

		case WM_NOTIFY:
			if(wParam == IDC_COMPRESSIONSLIDER)
			{
				LPNMHDR l = (LPNMHDR)lParam;
				if(l->idFrom == IDC_COMPRESSIONSLIDER)
				wr->cfg.compression = (unsigned int)SendMessage(GetDlgItem(hwndDlg,IDC_COMPRESSIONSLIDER),TBM_GETPOS,0,0);
			}
		break;

		case WM_DESTROY:
			writeconfig(wr->configfile,&wr->cfg);
			free(wr); wr=NULL;
			break;
	}

	const int controls[] = 
	{
		IDC_COMPRESSIONSLIDER,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
	{
		return TRUE;
	}

	link_handledraw(hwndDlg,uMsg,wParam,lParam);
	return 0;
}

extern "C"
{
	unsigned int __declspec(dllexport) GetAudioTypes3(int idx, char *desc)
	{
		if (idx == 0)
		{
			GetLocalisationApiService();
			const char *libFlacVer = GetFLACVersion();
			StringCchPrintfA(desc, 1024, WASABI_API_LNGSTRING(IDS_ENC_FLAC_DESC), ENC_VERSION, libFlacVer);
			return mmioFOURCC('F', 'L', 'A', 'C');
		}
		return 0;
	}

	AudioCoder __declspec(dllexport) *CreateAudio3(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile)
	{
		if (srct == mmioFOURCC('P', 'C', 'M', ' ') && *outt == mmioFOURCC('F', 'L', 'A', 'C'))
		{
			configtype cfg;
			readconfig(configfile, &cfg);
			*outt = mmioFOURCC('F', 'L', 'A', 'C');
			AudioCoderFlac *t=new AudioCoderFlac(nch, bps, srate, cfg.compression);
			if (!t->OK())
			{
				delete t;
				return NULL;
			}
			return t;
		}
		return NULL;
	}

	void __declspec(dllexport) FinishAudio3(const char *filename, AudioCoder *coder)
	{
		((AudioCoderFlac*)coder)->Finish(AutoWideFn(filename));
	}

	void __declspec(dllexport) FinishAudio3W(const wchar_t *filename, AudioCoder *coder)
	{
			((AudioCoderFlac*)coder)->Finish(filename);
	}

	void __declspec(dllexport) PrepareToFinish(const char *filename, AudioCoder *coder)
	{
		((AudioCoderFlac*)coder)->PrepareToFinish();
	}

	void __declspec(dllexport) PrepareToFinishW(const wchar_t *filename, AudioCoder *coder)
	{
		((AudioCoderFlac*)coder)->PrepareToFinish();
	}


	HWND __declspec(dllexport) ConfigAudio3(HWND hwndParent, HINSTANCE hinst, unsigned int outt, char *configfile)
	{
		if (outt == mmioFOURCC('F', 'L', 'A', 'C'))
		{
			configwndrec *wr = (configwndrec*)malloc(sizeof(configwndrec));
			if (configfile) StringCchCopyA(wr->configfile, MAX_PATH, configfile);
			else wr->configfile[0] = 0;
			readconfig(configfile, &wr->cfg);
			GetLocalisationApiService();
			return WASABI_API_CREATEDIALOGPARAMW(IDD_CONFIG, hwndParent, DlgProc, (LPARAM)wr);
		}
		return NULL;
	}

	int __declspec(dllexport) SetConfigItem(unsigned int outt, char *item, char *data, char *configfile)
	{
		// nothing yet
		return 0;
	}

	int __declspec(dllexport) GetConfigItem(unsigned int outt, char *item, char *data, int len, char *configfile)
	{
		if (outt == mmioFOURCC('F', 'L', 'A', 'C'))
		{
			/*
			configtype cfg;
			readconfig(configfile, &cfg);
			*/
			if (!_stricmp(item, "bitrate"))  StringCchCopyA(data, len, "755"); // FUCKO: this is ment to be an estimate for approximations of output filesize (used by ml_pmp). Improve this.
			else if (!_stricmp(item,"extension")) StringCchCopyA(data, len, "flac");
			return 1;
		}
		return 0;
	}

	void __declspec(dllexport) SetWinampHWND(HWND hwnd)
	{
		winampwnd = hwnd;
		isthemethere = !SendMessage(hwnd,WM_WA_IPC,IPC_ISWINTHEMEPRESENT,IPC_USE_UXTHEME_FUNC);
	}
};