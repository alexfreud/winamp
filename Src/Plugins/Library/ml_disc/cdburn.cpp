#include "main.h"
#include <stdio.h>
#include "../nu/ns_wc.h"
#include "resource.h"
#include "../nu/listview.h"
#include "../nu/DialogSkinner.h"
#include "../nu/ChildSizer.h"
#include "config.h"
#include "../../General/gen_ml/gaystring.h"
#include "../Winamp/burn.h"
#include "../Winamp/strutil.h"
#include <std::vector>
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include <api/service/waServiceFactory.h>
#include "../playlist/ifc_playlistloadercallback.h"
#include "../playlist/api_playlistmanager.h"
#include <imapi.h>
#include <imapierror.h>
#include <shlwapi.h>
#include <strsafe.h>

//shit to finish:
//-erase CDRWs
//-cache the veritas handle
//-recurse add folders
//-check for available space on HD before burning
//-the resampling in convert


#define WM_EX_OPCOMPLETED		(WM_USER + 0x100)

class PLCallBack : ifc_playlistloadercallback
{
public:
	PLCallBack(void) : fileList(0) {};
	~PLCallBack(void) {};
public:
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
	{
		fileList->push_back(new GayString(AutoChar(filename)));
	}
	RECVS_DISPATCH;
public:
	std::vector<GayString*> *fileList;
};

#define CBCLASS PLCallBack
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
END_DISPATCH;
#undef CBCLASS

class PLCallBackW : ifc_playlistloadercallback
{
public:
	PLCallBackW(void) : fileList(0) {};
	~PLCallBackW(void) {};
public:
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
	{
		fileList->push_back(new GayStringW(filename));
	}
	RECVS_DISPATCH;
public:
	std::vector<GayStringW*> *fileList;
};

#define CBCLASS PLCallBackW
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
END_DISPATCH;
#undef CBCLASS

static INT_PTR WINAPI DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#include "../burnlib/burnlib.h"

static W_ListView m_statuslist;

static HWND m_hwndstatus;
static char m_cdrom;
static int m_is_cdrw, m_availsecs;
static int m_max_speed;
static int m_dragging, m_drag_item;
static HWND prevWnd = NULL;
itemRecordListW itemCache[100] = {0};
static int percentCompleted = 0;
static DWORD pidBurner	= 0;

static HFONT hPLFont = NULL;

static int LETTERTOINDEX(char c)
{
	c = (char)toupper(c);
	if (c < 'A') c = 'A';
	if (c > 'Z') c = 'Z';
	return c -'A';
}

#include "../winamp/wa_ipc.h"


#define TIMER_NOTIFYINFO_ID		1985
#define TIMER_NOTIFYINFO_DELAY	200


static ChildWndResizeItem burnwnd_rlist[] =
{
	{IDC_LIST2, 0x0011},
	{IDC_CDINFO, 0x0010},
	{IDC_ADD, 0x0101},
	{IDC_BURN, 0x0101},
	{IDC_CLEAR, 0x0101},
	{IDC_BURN_OPTS, 0x0101},
	{IDC_CANCEL_BURN, 0x0101},
	{IDC_LOGO, 0x1010},
	{IDC_BTN_SHOWINFO, 0x1111},
};
static _inline void code(long* v, long* k)
{
	unsigned long y = v[0], z = v[1], sum = 0,        /* set up */
	                                        delta = 0x9e3779b9, n = 32 ;  /* key schedule constant*/

	while (n-- > 0)
	{                                       /* basic cycle start */
		sum += delta;
		y += ((z << 4) + k[0]) ^(z + sum) ^((z >> 5) + k[1]);
		z += ((y << 4) + k[2]) ^(y + sum) ^((y >> 5) + k[3]); /* end cycle */
	}
	v[0] = y; v[1] = z;

}

static void startBurn(HWND hwndDlg, char driveletter)
{
	g_config->WriteInt(L"cdburnmaxspeed", m_max_speed);

	//write the temp playlist to disk
	FILE *fp;
	char filename[MAX_PATH] = {0}, tp[MAX_PATH] = {0};
	
	pidBurner = 0;

	if (!GetTempPathA(sizeof(tp), tp)) lstrcpynA(tp, ".", MAX_PATH);
	if (GetTempFileNameA(tp, "BRN", 0, filename))
	{
		unlink(filename);
		StringCchCatA(filename, MAX_PATH, ".m3u8");
	}
	else lstrcpynA(filename, "brn_tmp.m3u8", MAX_PATH);

	fp = fopen(filename, "wt");
	if (!fp)
	{
		wchar_t title[16] = {0};
		MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_ERROR_WRITING_TEMP_BURN_LIST),
				   WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,title,16), MB_OK);
		return ;
	}
	int idx = LETTERTOINDEX(driveletter);
	fprintf(fp, "#EXTM3U\n");
	for (int i = 0;i < itemCache[idx].Size;i++)
	{
		fprintf(fp, "#EXTINF:%d,%s\n", itemCache[idx].Items[i].length, (char *)AutoChar(itemCache[idx].Items[i].title, CP_UTF8));
		fprintf(fp, "%s\n", (char *)AutoChar(itemCache[idx].Items[i].filename, CP_UTF8));
	}
	fclose(fp);

	burnCDStruct bcds =
	  {
	    m_cdrom,
	    filename,
	    hwndDlg,
	    "",
	  };
	pidBurner = (DWORD)(INT_PTR)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM) & bcds, IPC_BURN_CD);
	if (!pidBurner)
	{
		wchar_t title[16] = {0};
		unlink(filename);
		MessageBox(hwndDlg, AutoWide(bcds.error), WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,title,16), MB_OK);
	}
}

static void link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
			GetDlgItemText(hwndDlg, (int)wParam, wt, 123);

			// draw text
			SetTextColor(di->hDC, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			r = di->rcItem;
			r.left += 2;
			DrawText(di->hDC, wt, -1, &r, DT_VCENTER | DT_SINGLELINE);

			memset(&r, 0, sizeof(r));
			DrawText(di->hDC, wt, -1, &r, DT_SINGLELINE | DT_CALCRECT);

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

static void refreshList()
{
	if (!m_hwndstatus) return ;

	ListView_SetItemCount(m_statuslist.getwnd(), 0);
	int idx = LETTERTOINDEX(m_cdrom);
	ListView_SetItemCount(m_statuslist.getwnd(), itemCache[idx].Size);
	if (itemCache[idx].Size > 0) ListView_RedrawItems(m_statuslist.getwnd(), 0, itemCache[idx].Size - 1);
}
static int m_last_trackpos;


typedef struct _MEDIAINFO
{
	CHAR	cLetter;
	BOOL	bInserted;
	BOOL	bRecordable;
	BOOL	bRewritable;
	BOOL	bBlank;
	ULONG	nSectorsFree;
	ULONG	nSectorsUsed;
} MEDIAINFO;

static HRESULT GetMediaInfoFromSonic(MEDIAINFO *pmi)
{
	HRESULT hr;

	hr = S_OK;

	char name[]= "cda://X.cda";
	char buf2[64] = "";
	char buf3[64] = "";

	name[6] = pmi->cLetter;

	pmi->bInserted = FALSE;
	pmi->bRewritable = FALSE;
	pmi->nSectorsFree = 0;
	pmi->nSectorsUsed = 0;

	pmi->bRecordable = TRUE; 
	getFileInfo(name, "cdtype", buf3, sizeof(buf3));
	if (buf3[0] && 0 == lstrcmpA(buf3, "CDRW")) pmi->bRewritable = TRUE; 

	getFileInfo(name, "cdlengths", buf2, sizeof(buf2));
	if (buf2[0]) 
	{
		pmi->bInserted = TRUE;
		pmi->nSectorsFree = atoi(buf2);
	}

	return hr;
}

static void CALLBACK FreeAsyncParam(DM_NOTIFY_PARAM *phdr)
{
	switch(phdr->opCode)
	{
		case DMOP_IMAPIINFO:
			break;
	}
	free(phdr);
}
static void FinishSetStatus(HWND hwndDlg, MEDIAINFO *pmi)
{
	int freesecs;
	if(pmi->bInserted)
	{
		freesecs = (pmi->nSectorsFree * 2048) / (150 * 1024); //150kb/s as its considered DATA CD at this point in veritas
	}
	else
	{
		freesecs = 74 * 60; //Default to 74mns CDR
	}
	
	m_availsecs = freesecs;

	int idx = LETTERTOINDEX(m_cdrom);
	int usedlen = 0;
	int truncpos = 0;
	for (int i = 0;i < itemCache[idx].Size;i++)
	{
		usedlen += itemCache[idx].Items[i].length;
		if (usedlen > m_availsecs)
			truncpos++;
	}
	m_availsecs -= usedlen;

	wchar_t status[256] = {0};
	if (!pmi->bInserted)
		WASABI_API_LNGSTRINGW_BUF(IDS_NO_BLANK_CDR_IN_DRIVE,status,512);
	else
	{
		StringCchPrintf(status, 512, WASABI_API_LNGSTRINGW(IDS_X_CAPACITY_DETAILS),
						(pmi->bRewritable) ? L"CD-RW" : L"CD-R" , freesecs / 60, freesecs % 60);
	}

	wchar_t temp[16] = {0};
	StringCchPrintf(status + wcslen(status), 256,
					WASABI_API_LNGSTRINGW(IDS_USED_X_X_TRACKS),
					usedlen / 60,
					usedlen % 60,
					itemCache[idx].Size,
					WASABI_API_LNGSTRINGW_BUF(itemCache[idx].Size == 1 ? IDS_TRACK : IDS_TRACKS,temp,16));

	if (freesecs && pmi->bInserted)
	{
		if (m_availsecs >= 0) StringCchPrintf(status + wcslen(status), 256, WASABI_API_LNGSTRINGW(IDS_AVAILABLE_X_X),
											  m_availsecs / 60, m_availsecs % 60);
		else StringCchPrintf(status + wcslen(status), 256, WASABI_API_LNGSTRINGW(IDS_X_OVER_CAPACITY_REMOVE_X_TRACKS),
							 -m_availsecs / 60, -m_availsecs % 60, truncpos);
	}
	SetDlgItemText(hwndDlg, IDC_CDINFO, status);
	m_last_trackpos = -1;

	m_is_cdrw = pmi->bRewritable;
	ListView_RedrawItems(m_statuslist.getwnd(), 0, itemCache[idx].Size - 1);
}

static void SetStatus(HWND hwndDlg, CHAR cLetter)
{
	if (DM_MODE_BURNING == DriveManager_GetDriveMode(cLetter) &&
		NULL != (m_burning_other_wnd = cdburn_FindBurningHWND(cLetter)))
	{
		prevWnd = (HWND)SendMessage(m_burning_other_wnd, WM_BURNUPDATEOWNER, 0, (LPARAM)hwndDlg);
		if (prevWnd == hwndDlg) prevWnd = NULL;
		DWORD state = (DWORD)(INT_PTR)SendMessage(m_burning_other_wnd, WM_BURNGETSTATUS, BURNSTATUS_STATE, 0);
		if (state)
		{
			SetDlgItemText(hwndDlg, IDC_CDINFO, WASABI_API_LNGSTRINGW(IDS_BURNING));
			ShowWindow(GetDlgItem(hwndDlg, IDC_CLEAR), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_BURN), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_BURN_OPTS), SW_SHOWNA);
			ShowWindow(GetDlgItem(hwndDlg, IDC_CANCEL_BURN), SW_SHOWNA);
			SetDlgItemText(hwndDlg, IDC_CANCEL_BURN, WASABI_API_LNGSTRINGW(IDS_CANCEL_BURN));
			m_availsecs = 0;
			m_last_trackpos = -1;
			m_is_cdrw = 0;
			percentCompleted = 0;
			UpdateWindow(hwndDlg);
		}

		SendMessage(hwndDlg, WM_BURNNOTIFY, BURN_STATECHANGED, state);
		ShowWindow(m_burning_other_wnd, g_config->ReadInt(L"cdburnstatuswnd", 1) ? SW_SHOWNA : SW_HIDE);
		PostMessage(m_burning_other_wnd, WM_BURNCONFIGCHANGED, BURNCFG_HIDEVIEW, !g_config->ReadInt(L"cdburnstatuswnd", 1));
		PostMessage(m_burning_other_wnd, WM_BURNCONFIGCHANGED, BURNCFG_AUTOEJECT, g_config->ReadInt(L"cdburnautoeject", 1));
		PostMessage(m_burning_other_wnd, WM_BURNCONFIGCHANGED, BURNCFG_ADDTODB, g_config->ReadInt(L"cdburnautoadddb", 0));
		PostMessage(m_burning_other_wnd, WM_BURNCONFIGCHANGED, BURNCFG_AUTOCLOSE, g_config->ReadInt(L"cdburnautoclose", 0));
	}
	else
	{
		BOOL br;
		ShowWindow(GetDlgItem(hwndDlg, IDC_CLEAR), SW_SHOWNA);
		ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_SHOWNA);
		ShowWindow(GetDlgItem(hwndDlg, IDC_BURN), SW_SHOWNA);
		ShowWindow(GetDlgItem(hwndDlg, IDC_BURN_OPTS), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CANCEL_BURN), SW_HIDE);

		SetDlgItemText(hwndDlg, IDC_CDINFO, WASABI_API_LNGSTRINGW(IDS_CALCULATING));
		
		UpdateWindow(hwndDlg);

		DM_IMAPI_PARAM *pIMAPI = (DM_IMAPI_PARAM*)calloc(1, sizeof(DM_IMAPI_PARAM));
		if (pIMAPI)
		{
			pIMAPI->header.cLetter = cLetter;
			pIMAPI->header.callback = (INT_PTR)hwndDlg;
			pIMAPI->header.uMsg = WM_EX_OPCOMPLETED;
			pIMAPI->header.fnFree = FreeAsyncParam;
			pIMAPI->header.fFlags = DMF_QUERYMEDIATYPE | DMF_QUERYMEDIAINFO;
			br = DriveManager_GetIMAPIInfo(pIMAPI);
		}
		else br = FALSE;
		if (!br) SetDlgItemText(hwndDlg, IDC_CDINFO, WASABI_API_LNGSTRINGW(IDS_DISC_READ_ERROR));
	}
}

static void deleteSelectedItems(HWND hwndDlg, CHAR cLetter)
{
	int idx = LETTERTOINDEX(cLetter);
	for (int i = itemCache[idx].Size - 1;i >= 0;i--)
	{
		if (m_statuslist.GetSelected(i))
		{
			freeRecord(&itemCache[idx].Items[i]);
			int l = itemCache[idx].Size - i - 1;
			if (l > 0) memcpy(&itemCache[idx].Items[i], &itemCache[idx].Items[i + 1], sizeof(itemRecordW)*l);
			itemCache[idx].Size--;
		}
	}
	SetStatus(hwndDlg, cLetter);
	refreshList();
}

static void selectAll()
{
	int l = m_statuslist.GetCount();
	for (int i = 0;i < l;i++) m_statuslist.SetSelected(i);
}

static void playSelectedItems(HWND hwndDlg, int enqueue)
{
	int idx = LETTERTOINDEX(m_cdrom);
	if (!enqueue) SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE);

	for (int i = 0;i < itemCache[idx].Size;i++)
	{
		if (!m_statuslist.GetSelected(i)) continue;

		//send the file to winamp
		COPYDATASTRUCT cds;
		cds.dwData = IPC_PLAYFILEW;
		cds.lpData = (void *)itemCache[idx].Items[i].filename;
		cds.cbData = (DWORD)(sizeof(wchar_t *) * (wcslen(itemCache[idx].Items[i].filename) + 1)); // include space for null char
		SendMessageW(plugin.hwndWinampParent, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
	}

	if (!enqueue) SendMessageW(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_STARTPLAY);
}
BOOL CALLBACK CantBurnProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			wchar_t *message = (wchar_t *)lParam;

			// due to quirks with the more common resource editors, is easier to just store the string
			// internally only with \n and post-process to be \r\n (as here) so it will appear correctly
			// on new lines as is wanted (silly multiline edit controls)
			wchar_t tmp2[1024] = {0}, *t2 = tmp2;
			while(message && *message && (t2 - tmp2 < 1024))
			{
				if(*message == L'\n')
				{
					*t2 = L'\r';
					t2 = CharNextW(t2);
				}
				*t2 = *message;
				message = CharNextW(message);
				t2 = CharNextW(t2);
			}

			SetDlgItemText(hwnd, IDC_MESSAGE2, tmp2);
		}
		return 0;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hwnd, 0);
					break;
				case IDCANCEL:
					EndDialog(hwnd, -1);
					break;
			}
	}
	return 0;
}

HRESULT ResolveShortCut(HWND hwnd, LPCSTR pszShortcutFile, LPSTR pszPath)
{
	HRESULT hres;
	IShellLinkA* psl;
	WIN32_FIND_DATAA wfd;

	*pszPath = 0;   // assume failure

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
	                        IID_IShellLinkA, (void **) & psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		hres = psl->QueryInterface(IID_IPersistFile, (void **) & ppf); // OLE 2!  Yay! --YO
		if (SUCCEEDED(hres))
		{
			wchar_t wsz[MAX_PATH] = {0};
			MultiByteToWideCharSZ(CP_ACP, 0, pszShortcutFile, -1, wsz, MAX_PATH);

			hres = ppf->Load(wsz, STGM_READ);
			if (SUCCEEDED(hres))
			{
				hres = psl->Resolve(hwnd, SLR_ANY_MATCH);
				if (SUCCEEDED(hres))
				{
					char szGotPath[MAX_PATH] = {0};
					lstrcpynA(szGotPath, pszShortcutFile, MAX_PATH);
					hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATAA *) & wfd,
					                    SLGP_SHORTPATH);
					lstrcpynA(pszPath, szGotPath, MAX_PATH);
				}
			}
			ppf->Release();
		}
		psl->Release();
	}
	return SUCCEEDED(hres);
}

HRESULT ResolveShortCut(HWND hwnd, LPCWSTR pszShortcutFile, LPWSTR pszPath)
{
	HRESULT hres;
	IShellLinkW* psl;
	WIN32_FIND_DATAW wfd;

	*pszPath = 0;   // assume failure

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
	                        IID_IShellLinkW, (void **) & psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		hres = psl->QueryInterface(IID_IPersistFile, (void **) & ppf); // OLE 2!  Yay! --YO
		if (SUCCEEDED(hres))
		{
			/*wchar_t wsz[MAX_PATH] = {0};
			MultiByteToWideCharSZ(CP_ACP, 0, pszShortcutFile, -1, wsz, MAX_PATH);*/

			hres = ppf->Load(pszShortcutFile/*wsz*/, STGM_READ);
			if (SUCCEEDED(hres))
			{
				hres = psl->Resolve(hwnd, SLR_ANY_MATCH);
				if (SUCCEEDED(hres))
				{
					wchar_t szGotPath[MAX_PATH] = {0};
					wcsncpy(szGotPath, pszShortcutFile, MAX_PATH);
					hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATAW *) & wfd,
					                    SLGP_SHORTPATH);
					wcsncpy(pszPath, szGotPath, MAX_PATH);
				}
			}
			ppf->Release();
		}
		psl->Release();
	}
	return SUCCEEDED(hres);
}

static int checkFile(const char *file)
{
	//check if the file is supported by winamp
	const char *ext = extension(file);
	if (!ext || !ext[0]) return 0;
	if (strstr(file, "://") && !strstr(file, "cda://")) return 0;

#if 0 // benski> this would be neat to have, but will fail with unicode filenames (which in_mp3 can open anyway)... TODO: make it workable later
	HANDLE hFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile && GetLastError() != ERROR_FILE_NOT_FOUND)
	{
		wchar_t message[1024] = {0};
		StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_FILE_X_CANNOT_BE_BURNED_REASON_NOT_FOUND), AutoWide(file), AutoWide(ext));
		return WASABI_API_DIALOGBOXPARAM(IDD_NOBURN, g_hwnd, CantBurnProc, (LPARAM)message);
	}
	CloseHandle(hFile);
#endif

	char *m_extlist = (char*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_EXTLIST);
	{
		int found = 0;
		char *a = m_extlist;
		while (a && *a)
		{
			if (!lstrcmpiA(a, ext))
			{
				found = 1;
				break;
			}
			a += lstrlenA(a) + 1;
		}
		GlobalFree((HGLOBAL)m_extlist);
		if (!found)
		{
			wchar_t message[1024] = {0};
			StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_FILE_X_CANNOT_BE_BURNED_REASON_FILETYPE_NOT_REGISTERED), AutoWide(file), AutoWide(ext));
			return (int)(INT_PTR)WASABI_API_DIALOGBOXPARAM(IDD_NOBURN, plugin.hwndLibraryParent, CantBurnProc, (LPARAM)message);
		}
	}

	//check for type
	char tmp[64] = {0, };
	getFileInfo(file, "type", tmp, sizeof(tmp) - 1);
	if (tmp[0] && tmp[0] != '0')
	{
		wchar_t message[1024], temp[128] = {0};
		StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_FILE_X_CANNOT_BE_BURNED_REASON_X),AutoWide(file),
						 WASABI_API_LNGSTRINGW_BUF((tmp[0] == '1' ? IDS_VIDEO_FILES_CANNOT_BE_BURNED : IDS_NOT_AN_AUDIO_FILE),temp,128));
		return (int)(INT_PTR)WASABI_API_DIALOGBOXPARAM(IDD_NOBURN, plugin.hwndLibraryParent, CantBurnProc, (LPARAM)message);

	}

	// note: this check is NOT meant as any sort of protection.. It simply saves the user the hassle of an error later
	if (getFileInfo(file, "burnable", tmp, 64) // most plugins don't support this extended file info, so failure is OK
	    && tmp[0] == '0') // if it does support it, then we can check whether or not it's burnable
	{
		wchar_t message[1024] = {0};
		StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_FILE_CANNOT_BE_BURNED), AutoWide(file));
		if (getFileInfo(file, "noburnreason", tmp, 64))
		{
			StringCchPrintfW(message, 1024,
							 WASABI_API_LNGSTRINGW(IDS_FILE_X_CANNOT_BE_BURNED_REASON_X),
							 AutoWide(file), AutoWide(tmp));
		}
		return (int)(INT_PTR)WASABI_API_DIALOGBOXPARAM(IDD_NOBURN, plugin.hwndLibraryParent, CantBurnProc, (LPARAM)message);
	}

	return 1;
}


static int checkFile(const wchar_t *file)
{
	//check if the file is supported by winamp
	const wchar_t *ext = PathFindExtension(file);
	if (!ext || !ext[0]) return 0;
	ext++;
	if (wcsstr(file, L"://") && !wcsstr(file, L"cda://")) return 0;

#if 0 // benski> this would be neat to have, but will fail with unicode filenames (which in_mp3 can open anyway)... TODO: make it workable later
	HANDLE hFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		wchar_t message[1024] = {0};
		StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_FILE_X_CANNOT_BE_BURNED_REASON_NOT_FOUND), file, ext);
		return WASABI_API_DIALOGBOXPARAM(IDD_NOBURN, g_hwnd, CantBurnProc, (LPARAM)message);
	}
	CloseHandle(hFile);
#endif
	wchar_t *m_extlist = (wchar_t*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_EXTLISTW);
	{
		int found = 0;
		wchar_t *a = m_extlist;
		while (a && *a)
		{
			if (!lstrcmpiW(a, ext))
			{
				found = 1;
				break;
			}
			a += lstrlenW(a) + 1;
		}
		GlobalFree((HGLOBAL)m_extlist);
		if (!found)
		{
			wchar_t message[1024] = {0};
			StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_FILE_X_CANNOT_BE_BURNED_REASON_FILETYPE_NOT_REGISTERED), file, ext);
			return (int)(INT_PTR)WASABI_API_DIALOGBOXPARAM(IDD_NOBURN, plugin.hwndLibraryParent, CantBurnProc, (LPARAM)message);
		}
	}

	//check for type
	wchar_t tmp[64] = {0, };
	getFileInfoW(file, L"type", tmp, 64);
	if (tmp[0] && tmp[0] != '0')
	{
		wchar_t message[1024] = {0}, temp[128] = {0};
		StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_FILE_X_CANNOT_BE_BURNED_REASON_X), file,
						 WASABI_API_LNGSTRINGW_BUF((tmp[0] == '1' ? IDS_VIDEO_FILES_CANNOT_BE_BURNED : IDS_NOT_AN_AUDIO_FILE),temp,128));
		return (int)(INT_PTR)WASABI_API_DIALOGBOXPARAM(IDD_NOBURN, plugin.hwndLibraryParent, CantBurnProc, (LPARAM)message);

	}

	// note: this check is NOT meant as any sort of protection.. It simply saves the user the hassle of an error later
	if (getFileInfoW(file, L"burnable", tmp, 64) // most plugins don't support this extended file info, so failure is OK
	    && tmp[0] == '0') // if it does support it, then we can check whether or not it's burnable
	{
		wchar_t message[1024] = {0};
		StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_FILE_CANNOT_BE_BURNED), file);
		if (getFileInfoW(file, L"noburnreason", tmp, 64))
		{
			StringCchPrintfW(message, 1024,
							 WASABI_API_LNGSTRINGW(IDS_FILE_X_CANNOT_BE_BURNED_REASON_X),
							 file, tmp);
		}
		return (int)(INT_PTR)WASABI_API_DIALOGBOXPARAM(IDD_NOBURN, plugin.hwndLibraryParent, CantBurnProc, (LPARAM)message);
	}

	return 1;
}


void cdburn_clearBurner(char driveletter)
{
	emptyRecordList(&itemCache[LETTERTOINDEX(driveletter)]);
}
void cdburn_addfile(char* file, std::vector<GayString*> *files, api_playlistmanager* plManager, ifc_playlistloadercallback *plCB);
void cdburn_addfile(wchar_t* file, std::vector<GayStringW*> *files, api_playlistmanager* plManager, ifc_playlistloadercallback *plCB);
void cdburn_addfolder(const char* folder, std::vector<GayString*> *files, api_playlistmanager* plManager, ifc_playlistloadercallback *plCB);
void cdburn_addfolder(const wchar_t* folder, std::vector<GayStringW*> *files, api_playlistmanager* plManager, ifc_playlistloadercallback *plCB);

void cdburn_appendFile(char *file, char cLetter)
{
	std::vector<GayString*> files;
	waServiceFactory *plmFactory = plugin.service->service_getServiceByGuid(api_playlistmanagerGUID);
	api_playlistmanager *plManager = (plmFactory) ? (api_playlistmanager*)plmFactory->getInterface() : NULL;

	int idx = LETTERTOINDEX(cLetter);
	int validFile = 1;

	if (itemCache[idx].Size > 255) return;
	itemRecordListW newItems = {0, 0, 0};
	PLCallBack plCB;
	plCB.fileList = &files;

	cdburn_addfile(file, &files, (api_playlistmanager*)plManager, (ifc_playlistloadercallback*)&plCB);

	size_t x;

	for (x = 0; x < files.size(); x ++)  // temp record . replace it !!!
	{
		char *fn = files.at(x)->Get();

		validFile = checkFile(fn);
		// can't use switch here cause break won't work
		if (validFile == -1) // bad file and user cancelled
			break;
		if (validFile) // bad file, user skipped
		{
			allocRecordList(&newItems, newItems.Size + 1);
			if (!newItems.Alloc) break;

			char title[2048] = {0};
			basicFileInfoStruct bfis = {fn, 0, 0, title, sizeof(title) - 1,};
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&bfis, IPC_GET_BASIC_FILE_INFO);
			if (bfis.length > 0)
			{
				memset((void *)&(newItems.Items[newItems.Size]), 0, sizeof(itemRecordW));
				newItems.Items[newItems.Size].filename = AutoWideDup(fn);
				newItems.Items[newItems.Size].title = AutoWideDup(title);
				newItems.Items[newItems.Size].length = bfis.length;
				newItems.Size++;
			}
		}
		delete(files.at(x)->Get());
	}

	if (validFile != -1)
		copyRecordList(&itemCache[idx], &newItems);
	refreshList();
	if (m_hwndstatus) SetStatus(m_hwndstatus, cLetter);
	if (plManager) plmFactory->releaseInterface(plManager);
}

void cdburn_appendFile(wchar_t *file, char cLetter)
{
	std::vector<GayStringW*> files;
	waServiceFactory *plmFactory = plugin.service->service_getServiceByGuid(api_playlistmanagerGUID);
	api_playlistmanager *plManager = (plmFactory) ? (api_playlistmanager*)plmFactory->getInterface() : NULL;

	int idx = LETTERTOINDEX(cLetter);
	int validFile = 1;

	if (itemCache[idx].Size > 255) return;
	itemRecordListW newItems = {0, 0, 0};
	PLCallBackW plCB;
	plCB.fileList = &files;

	cdburn_addfile(file, &files, (api_playlistmanager*)plManager, (ifc_playlistloadercallback*)&plCB);

	size_t x;

	for (x = 0; x < files.size(); x ++)  // temp record . replace it !!!
	{
		const wchar_t *fn = files.at(x)->Get();

		validFile = checkFile(fn);
		// can't use switch here cause break won't work
		if (validFile == -1) // bad file and user cancelled
			break;
		if (validFile) // bad file, user skipped
		{
			allocRecordList(&newItems, newItems.Size + 1);
			if (!newItems.Alloc) break;

			wchar_t title[2048] = {0};
			basicFileInfoStructW bfis = {fn, 0, 0, title, ARRAYSIZE(title) - 1,};
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&bfis, IPC_GET_BASIC_FILE_INFOW);
			if (bfis.length > 0)
			{
				memset((void *)&(newItems.Items[newItems.Size]), 0, sizeof(itemRecordW));
				newItems.Items[newItems.Size].filename = _wcsdup(fn);
				newItems.Items[newItems.Size].title = _wcsdup(title);
				newItems.Items[newItems.Size].length = bfis.length;
				newItems.Size++;
			}
		}
		delete(files.at(x)->Get());
	}

	if (validFile != -1)
		copyRecordList(&itemCache[idx], &newItems);
	refreshList();
	if (m_hwndstatus) SetStatus(m_hwndstatus, cLetter);
	if (plManager) plmFactory->releaseInterface(plManager);
}
void cdburn_addfile(char* file, std::vector<GayString*> *files, api_playlistmanager* plManager, ifc_playlistloadercallback *plCB)
{
	if (!_stricmp(extension(file), "lnk"))
	{
		char temp2[MAX_PATH] = {0};
		if (ResolveShortCut(plugin.hwndLibraryParent, file, temp2)) lstrcpynA(file, temp2, MAX_PATH);
		else return;
	}

	if (!_strnicmp(file, "cda://", 6))
	{
		if (strlen(file) == 7)
		{
			int n = 0;
			char buf2[32] = {0};
			getFileInfo(file, "ntracks", buf2, sizeof(buf2));
			n = atoi(buf2);
			if (n > 0 && n < 256)
			{
				for (int x = 0; x < n; x ++)
				{
					char s[64] = {0};
					StringCchPrintfA(s, 64, "%s,%d.cda", file, x + 1);
					files->push_back(new GayString(s));
				}
			}
		}
		else files->push_back(new GayString(file));
	}

	else if (strstr(file, "://"))
	{
		if (plManager && PLAYLISTMANAGER_SUCCESS != plManager->Load(AutoWide(file), plCB))
		{
			files->push_back(new GayString(file));
		}
	}
	else if (!lstrcmpA(file + 1, ":\\"))
	{
		cdburn_addfolder(file, files, plManager, plCB);
	}
	else
	{
		WIN32_FIND_DATAA d = {0};
		HANDLE h =	FindFirstFileA(file, &d);
		if (h != INVALID_HANDLE_VALUE)
		{
			FindClose(h);
			if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				cdburn_addfolder(file, files, plManager, plCB);
			}
			if (plManager && PLAYLISTMANAGER_SUCCESS != plManager->Load(AutoWide(file), plCB))
			{
				files->push_back(new GayString(file));
			}
		}
		else files->push_back(new GayString(file));
	}
}
void cdburn_addfile(wchar_t* file, std::vector<GayStringW*> *files, api_playlistmanager* plManager, ifc_playlistloadercallback *plCB)
{
	if (!_wcsicmp(extensionW(file), L"lnk"))
	{
		wchar_t temp2[MAX_PATH] = {0};
		if (ResolveShortCut(plugin.hwndLibraryParent, file, temp2)) lstrcpyn(file, temp2, MAX_PATH);
		else return;
	}

	if (!_wcsnicmp(file, L"cda://", 6))
	{
		if (wcslen(file) == 7)
		{
			int n = 0;
			wchar_t buf2[32] = {0};
			getFileInfoW(file, L"ntracks", buf2, sizeof(buf2));
			n = _wtoi(buf2);
			if (n > 0 && n < 256)
			{
				for (int x = 0; x < n; x ++)
				{
					wchar_t s[64] = {0};
					StringCchPrintfW(s, 64, L"%s,%d.cda", file, x + 1);
					files->push_back(new GayStringW(s));
				}
			}
		}
		else files->push_back(new GayStringW(file));
	}

	else if (wcsstr(file, L"://"))
	{
		if (plManager && PLAYLISTMANAGER_SUCCESS != plManager->Load(file, plCB))
		{
			files->push_back(new GayStringW(file));
		}
	}
	else if (!lstrcmpW(file + 1, L":\\"))
	{
		cdburn_addfolder(file, files, plManager, plCB);
	}
	else
	{
		WIN32_FIND_DATAW d = {0};
		HANDLE h = FindFirstFileW(file, &d);
		if (h != INVALID_HANDLE_VALUE)
		{
			FindClose(h);
			if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				cdburn_addfolder(file, files, plManager, plCB);
			}
			if (plManager && PLAYLISTMANAGER_SUCCESS != plManager->Load(file, plCB))
			{
				files->push_back(new GayStringW(file));
			}
		}
		else files->push_back(new GayStringW(file));
	}
}
void cdburn_addfolder(const char* folder, std::vector<GayString*> *files, api_playlistmanager* plManager, ifc_playlistloadercallback *plCB)
{
	WIN32_FIND_DATAA d = {0};
	char path[MAX_PATH] = {0};
	PathCombineA(path, folder, "*");

	HANDLE h = FindFirstFileA(path, &d);
	if (h == INVALID_HANDLE_VALUE) return;
	do
	{
		if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (0 == lstrcmpA(d.cFileName, ".") || 0 == lstrcmpA(d.cFileName, "..")) continue;
			GayString pathNew(folder);
			pathNew.Append("\\");
			pathNew.Append(d.cFileName);
			cdburn_addfolder(pathNew.Get(), files, plManager, plCB);
		}
		else
		{
			GayString file(folder);
			file.Append("\\");
			file.Append(d.cFileName);
			cdburn_addfile(file.Get(), files, plManager, plCB);
		}
	}
	while (FindNextFileA(h, &d));
	FindClose(h);
}
void cdburn_addfolder(const wchar_t* folder, std::vector<GayStringW*> *files, api_playlistmanager* plManager, ifc_playlistloadercallback *plCB)
{
	WIN32_FIND_DATAW d = {0};
	wchar_t path[MAX_PATH] = {0};
	PathCombineW(path, folder, L"*");

	HANDLE h = FindFirstFileW(path, &d);
	if (h == INVALID_HANDLE_VALUE) return;
	do
	{
		if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (0 == lstrcmpW(d.cFileName, L".") || 0 == lstrcmpW(d.cFileName, L"..")) continue;
			GayStringW pathNew(folder);
			pathNew.Append(L"\\");
			pathNew.Append(d.cFileName);
			cdburn_addfolder(pathNew.Get(), files, plManager, plCB);
		}
		else
		{
			GayStringW file(folder);
			file.Append(L"\\");
			file.Append(d.cFileName);
			cdburn_addfile((wchar_t*)file.Get(), files, plManager, plCB);
		}
	}
	while (FindNextFileW(h, &d));
	FindClose(h);
}
void cdburn_appendItemRecord(itemRecordList *obj, char cLetter)
{
	int idx = LETTERTOINDEX(cLetter);
	int validFile = 1;
	itemRecordListW newItems = {0, 0, 0};
	BurnAddStatus_Create(obj->Size);

	for (int i = 0;i < obj->Size;i++)
	{
		validFile = checkFile(obj->Items[i].filename);
		if (validFile == -1)
			break;
		if (validFile)
		{
			if (newItems.Size > 255) break;

			allocRecordList(&newItems, newItems.Size + 1);
			if (!newItems.Alloc) return ;

			memset((void *)&(newItems.Items[newItems.Size]), 0, sizeof(itemRecordW));
			newItems.Items[newItems.Size].filename = AutoWideDup(obj->Items[i].filename);

			GayString title;
			if (obj->Items[i].artist) title.Append(obj->Items[i].artist);
			if (title.Get() && title.Get()[0] && obj->Items[i].title && obj->Items[i].title[0])
				title.Append(" - ");
			if (obj->Items[i].title) title.Append(obj->Items[i].title);

			newItems.Items[newItems.Size].title = AutoWideDup(title.Get());

			newItems.Items[newItems.Size].length = obj->Items[i].length;
			newItems.Size++;
			BurnAddStatus_Step(&newItems);
		}
	}
	BurnAddStatus_Done();
	if (validFile != -1)
		copyRecordList(&itemCache[idx], &newItems);
	refreshList();
	if (m_hwndstatus) SetStatus(m_hwndstatus, cLetter);
}

void cdburn_appendItemRecord(itemRecordListW *obj, char cLetter)
{
	int idx = LETTERTOINDEX(cLetter);
	int validFile = 1;
	itemRecordListW newItems = {0, 0, 0};
	BurnAddStatus_Create(obj->Size);

	for (int i = 0;i < obj->Size;i++)
	{
		validFile = checkFile(obj->Items[i].filename);
		if (validFile == -1)
			break;
		if (validFile)
		{
			if (newItems.Size > 255) break;

			allocRecordList(&newItems, newItems.Size + 1);
			if (!newItems.Alloc) return ;

			memset((void *)&(newItems.Items[newItems.Size]), 0, sizeof(itemRecordW));
			newItems.Items[newItems.Size].filename = _wcsdup(obj->Items[i].filename);

			GayStringW title;
			if (obj->Items[i].artist) title.Append(obj->Items[i].artist);
			if (title.Get() && title.Get()[0] && obj->Items[i].title && obj->Items[i].title[0])
				title.Append(L" - ");
			if (obj->Items[i].title) title.Append(obj->Items[i].title);

			newItems.Items[newItems.Size].title = _wcsdup(title.Get());

			newItems.Items[newItems.Size].length = obj->Items[i].length;
			newItems.Size++;
			BurnAddStatus_Step(&newItems);
		}
	}
	BurnAddStatus_Done();
	if (validFile != -1)
		copyRecordList(&itemCache[idx], &newItems);
	refreshList();
	if (m_hwndstatus) SetStatus(m_hwndstatus, cLetter);
}

static void Shell_Free(void *p)
{
	IMalloc *m;
	SHGetMalloc(&m);
	m->Free(p);
}

HWND cdburn_FindBurningHWND(char cLetter)
{
	HWND h = 0;
	while (NULL != (h = FindWindowExW(NULL, h, L"#32770", NULL)))
	{
		if (!GetPropW(h, L"WABURNER")) continue;
		if (((char)(INT_PTR)GetPropW(h, L"DRIVE")) == cLetter) return h;
	}
	return NULL;
}

CHAR cdburn_IsMeBurning(void)
{
	if (pidBurner) 
	{
		HWND h = NULL;
		DWORD pid;
		while (NULL != (h = FindWindowExW(NULL, h, L"#32770", NULL)))
		{
			if (GetPropW(h, L"WABURNER") && GetWindowThreadProcessId(h, &pid) && pid == pidBurner)
					return (CHAR)(INT_PTR)GetPropW(h, L"DRIVE");
		}
	}
	return  0;
}

static void NotifyInfoWindow(HWND hwnd, LPCWSTR pszFileName, BOOL bForceRefresh)
{
	HWND hwndParent;
	hwndParent = GetParent(hwnd);
	if (hwndParent) SendMessageW(hwndParent, WM_SHOWFILEINFO, 
									(WPARAM)((bForceRefresh) ? WISF_FORCE : WISF_NORMAL),
									(LPARAM)pszFileName);
}

static void moveSelItemsUp()
{
	if (DM_MODE_BURNING == DriveManager_GetDriveMode(m_cdrom)) return;

	int idx = LETTERTOINDEX(m_cdrom);
	for (int i = 0;i < itemCache[idx].Size;i++)
	{
		if (m_statuslist.GetSelected(i))
		{
			//swap the 2 items
			if (i > 0)
			{
				itemRecordW tmp = itemCache[idx].Items[i];
				itemCache[idx].Items[i] = itemCache[idx].Items[i - 1];
				itemCache[idx].Items[i - 1] = tmp;
				ListView_SetItemState(m_statuslist.getwnd(), i - 1, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(m_statuslist.getwnd(), i, 0, LVIS_SELECTED);
				ListView_RedrawItems(m_statuslist.getwnd(), i - 1, i);
				if (ListView_GetItemState(m_statuslist.getwnd(), i, LVIS_FOCUSED))
				{
					ListView_SetItemState(m_statuslist.getwnd(), i - 1, LVIS_FOCUSED, LVIS_FOCUSED);
				}
			}
		}
	}
}

static void moveSelItemsDown()
{
	if (DM_MODE_BURNING == DriveManager_GetDriveMode(m_cdrom)) return ;

	int idx = LETTERTOINDEX(m_cdrom);
	for (int i = itemCache[idx].Size - 1;i >= 0;i--)
	{
		if (m_statuslist.GetSelected(i))
		{
			//swap the 2 items
			if (i < (itemCache[idx].Size - 1))
			{
				itemRecordW tmp = itemCache[idx].Items[i];
				itemCache[idx].Items[i] = itemCache[idx].Items[i + 1];
				itemCache[idx].Items[i + 1] = tmp;
				ListView_SetItemState(m_statuslist.getwnd(), i + 1, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(m_statuslist.getwnd(), i, 0, LVIS_SELECTED);
				ListView_RedrawItems(m_statuslist.getwnd(), i, i + 1);
				if (ListView_GetItemState(m_statuslist.getwnd(), i, LVIS_FOCUSED))
				{
					ListView_SetItemState(m_statuslist.getwnd(), i + 1, LVIS_FOCUSED, LVIS_FOCUSED);
				}
			}
		}
	}
}


int g_burn_hack_startburn;

void OnBurnDlgInit(HWND hwndDlg, LPARAM lParam)
{
	m_hwndstatus = hwndDlg;
	m_is_cdrw = 0;
	m_dragging = 0;

	m_cdrom = (char)lParam;

	SendMessageW(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(IDC_BTN_SHOWINFO, BN_EX_GETTEXT), (LPARAM)GetDlgItem(hwndDlg, IDC_BTN_SHOWINFO));

	m_statuslist.setwnd(GetDlgItem(hwndDlg, IDC_LIST2));
	m_statuslist.AddCol(WASABI_API_LNGSTRINGW(IDS_TRACK_NUMBER), g_view_metaconf->ReadInt(L"col_track", 60));
	m_statuslist.AddCol(WASABI_API_LNGSTRINGW(IDS_TITLE), g_view_metaconf->ReadInt(L"col_title", 200));
	m_statuslist.AddCol(WASABI_API_LNGSTRINGW(IDS_LENGTH), g_view_metaconf->ReadInt(L"col_len", 80));
	m_statuslist.AddCol(WASABI_API_LNGSTRINGW(IDS_STATUS), g_view_metaconf->ReadInt(L"col_status", 200));

	childSizer.Init(hwndDlg, burnwnd_rlist, sizeof(burnwnd_rlist) / sizeof(burnwnd_rlist[0]));

	if(m_statuslist.getwnd())
	{
		MLSKINWINDOW sw;
		
		sw.hwndToSkin = m_statuslist.getwnd();
		sw.skinType = SKINNEDWND_TYPE_LISTVIEW;
		sw.style = SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS | SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
		MLSkinWindow(plugin.hwndLibraryParent, &sw);
	}

	refreshList();

	// this will make sure that we've got the cddb logo shown even when using a localised version
	HANDLE hPrev = (HANDLE) SendDlgItemMessage(hwndDlg,IDC_LOGO,STM_SETIMAGE,IMAGE_BITMAP,
					   (LPARAM)LoadImage(plugin.hDllInstance,MAKEINTRESOURCE(IDB_LISTITEM_CDDRIVE),
										 IMAGE_BITMAP,0,0, 0));
	if (hPrev) DeleteObject(hPrev);

	NotifyInfoWindow(hwndDlg, NULL, TRUE); // ignore cache
	SetStatus(hwndDlg, m_cdrom);
	
	if (g_burn_hack_startburn)
	{
		g_burn_hack_startburn = 0;
		PostMessage(hwndDlg, WM_COMMAND, IDC_BURN, 0);
	}

}

void OnBurnNotify(HWND hwndDlg, DWORD notification, DWORD param)
{
	switch (notification)
	{
		case BURN_READY:
			SetStatus(hwndDlg, m_cdrom);
			break;
		case BURN_STATECHANGED:
		{
			wchar_t title[512] = {0};
			const wchar_t *buf = NULL;
			switch (param)
			{
				case BURNERPLAYLIST_BURNCANCELING:
					SetDlgItemText(hwndDlg, IDC_CANCEL_BURN, WASABI_API_LNGSTRINGW(IDS_CANCELLING));
					buf = WASABI_API_LNGSTRINGW_BUF(IDS_BURNING_AUDIO_CANCELLING,title,512);
					break;
				case BURNERPLAYLIST_BURNFINISHING:
					buf = WASABI_API_LNGSTRINGW_BUF(IDS_BURNING_AUDIO_FINISHING,title,512);
					break;
				case BURNERPLAYLIST_DECODEFINISHED:
					buf = WASABI_API_LNGSTRINGW_BUF(IDS_BURNING_AUDIO_DATA_PREP_FINISHED,title,512);
					break;
				case BURNERPLAYLIST_LICENSINGSTARTING:
					buf = WASABI_API_LNGSTRINGW_BUF(IDS_BURNING_AUDIO_VERIFYING_FILES,title,512);
					break;
				case BURNERPLAYLIST_LICENSINGFINISHED:
					buf = WASABI_API_LNGSTRINGW_BUF(IDS_BURNING_AUDIO_VERIFICATION_COMPLETED,title,512);
					break;
				case BURNERPLAYLIST_BURNPROGRESS:
					wchar_t buf2[256] = {0};
					switch (SendMessage(m_burning_other_wnd, WM_BURNGETSTATUS, BURNSTATUS_ERROR, 0))
					{
						case BURNERPLAYLIST_WRITELEADIN:
							buf = WASABI_API_LNGSTRINGW_BUF(IDS_OPENING_DISC_WRITING_LEAD_IN,buf2,256);
							break;
						case BURNERPLAYLIST_WRITELEADOUT:
							buf = WASABI_API_LNGSTRINGW_BUF(IDS_CLOSING_DISC_WRITING_LEAD_OUT,buf2,256);
							break;
						default: break;
					}
					if (buf)
					{
						int percent = (int)(INT_PTR)SendMessage(m_burning_other_wnd, WM_BURNGETSTATUS, BURNSTATUS_PROGRESS, 0);
						percentCompleted = max(percent, percentCompleted);
						StringCchPrintf(title, 512, WASABI_API_LNGSTRINGW(IDS_BURNING_AUDIO_CURRENT_OPERATION), percentCompleted, buf);
					}
					break;
			}
			if (buf) SetDlgItemText(hwndDlg, IDC_CDINFO, title);
		}
		break;
		case BURN_ITEMSTATECHANGED:
			ListView_RedrawItems(m_statuslist.getwnd(), param, param);
			break;
		case BURN_ITEMDECODEPROGRESS:
			ListView_RedrawItems(m_statuslist.getwnd(), param, param);
			{
				wchar_t title[512] = {0};
				int percent = (int)(INT_PTR)SendMessage(m_burning_other_wnd, WM_BURNGETSTATUS, BURNSTATUS_PROGRESS, 0);
				percentCompleted = max(percent, percentCompleted);
				StringCchPrintf(title, 512, WASABI_API_LNGSTRINGW(IDS_BURNING_AUDIO_CD_PREP_DATA), percentCompleted);
				SetDlgItemText(hwndDlg, IDC_CDINFO, title);
			}
			break;
		case BURN_ITEMBURNPROGRESS:
			ListView_RedrawItems(m_statuslist.getwnd(), param, param);
			{
				wchar_t title[512] = {0};
				int percent = (int)(INT_PTR)SendMessage(m_burning_other_wnd, WM_BURNGETSTATUS, BURNSTATUS_PROGRESS, 0);
				percentCompleted = max(percent, percentCompleted);
				StringCchPrintf(title, 512, WASABI_API_LNGSTRINGW(IDS_BURNING_AUDIO_BURNING_DATA), percentCompleted);
				SetDlgItemText(hwndDlg, IDC_CDINFO, title);
			}
			break;
		case BURN_WORKING:
			ListView_RedrawItems(m_statuslist.getwnd(), 0, ListView_GetItemCount(m_statuslist.getwnd()));
			break;
		case BURN_FINISHED:
		{
			wchar_t buf1[128] = {0}, closeStr[16] = {0};
			GetDlgItemText(hwndDlg, IDC_CANCEL_BURN, buf1, ARRAYSIZE(buf1));
			if (lstrcmpi(buf1, WASABI_API_LNGSTRINGW_BUF(IDS_CLOSE,closeStr,16)))
				SetDlgItemText(hwndDlg, IDC_CANCEL_BURN, closeStr);
			wchar_t buf[128] = {0};
			switch (param)
			{
				case BURNERPLAYLIST_SUCCESS:
					WASABI_API_LNGSTRINGW_BUF(IDS_AUDIO_CD_BURNED_SUCCESSFULLY,buf,128);
					break;
				case BURNERPLAYLIST_ABORTED:
					WASABI_API_LNGSTRINGW_BUF(IDS_BURN_ABORTED_BY_USER,buf,128);
					break;
				default:
					WASABI_API_LNGSTRINGW_BUF(IDS_BURNING_FAILED,buf,128);
					break;
			}
			StringCchPrintf(buf1, 128, WASABI_API_LNGSTRINGW(IDS_BURNING_COMPLETED_STATUS_X), buf);
			SetDlgItemText(hwndDlg, IDC_CDINFO, buf1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BURN), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CANCEL_BURN), TRUE);
		}
		break;
		case BURN_DESTROYED:
			EnableWindow(GetDlgItem(hwndDlg, IDC_BURN), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CANCEL_BURN), TRUE);
			m_burning_other_wnd = NULL;
			SetStatus(hwndDlg, m_cdrom);
			break;
		case BURN_CONFIGCHANGED:
			switch (LOWORD(param))
			{
				case BURNCFG_AUTOCLOSE:
					g_config->WriteInt(L"cdburnautoclose", HIWORD(param));
					break;
				case BURNCFG_AUTOEJECT:
					g_config->WriteInt(L"cdburnautoeject", HIWORD(param));
					break;
				case BURNCFG_ADDTODB:
					g_config->WriteInt(L"cdburnautoadddb", HIWORD(param));
					break;
				case BURNCFG_HIDEVIEW:
					g_config->WriteInt(L"cdburnstatuswnd", !HIWORD(param));
					break;
			}
			break;
	}
}

static int CALLBACK WINAPI BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)WASABI_API_APP->path_getWorkingPath());

			// this is not nice but it fixes the selection not working correctly on all OSes
			EnumChildWindows(hwnd, browseEnumProc, 0);
		}
	}
	return 0;
}

wchar_t* BuildFilterList(void)
{
	static wchar_t fileExtensionsString[128] = {L"*.*"};	// "All files\0*.*\0\0"
	wchar_t *temp=fileExtensionsString+lstrlenW(fileExtensionsString) +1;
	lstrcpynW(temp, WASABI_API_LNGSTRINGW(IDS_ALL_FILES), 128);
	*(temp = temp + lstrlenW(temp) + 1) = 0;
	return fileExtensionsString;
}


static void CALLBACK Window_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	HWND hwndList;
	int index, driveIdx;
	wchar_t *pszFileName;
	
	switch(idEvent)
	{
		case TIMER_NOTIFYINFO_ID:
			KillTimer(hwnd, TIMER_NOTIFYINFO_ID);
			hwndList = GetDlgItem(hwnd, IDC_LIST2);
			
			driveIdx = LETTERTOINDEX(m_cdrom);		
			index = (hwndList) ? (INT)SendMessage(hwndList, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_FOCUSED) : -1;
			pszFileName = (index >= 0 && index < itemCache[driveIdx].Size) ? itemCache[driveIdx].Items[index].filename : NULL;
			NotifyInfoWindow(hwnd, pszFileName, FALSE);
			break;
	}
}

static void ListView_OnItemChanged(HWND hwndDlg, NMLISTVIEW *pnmv)
{
	if (LVIF_STATE & pnmv->uChanged)
	{
		if ((LVIS_FOCUSED & pnmv->uOldState) != (LVIS_FOCUSED & pnmv->uNewState))
		{
			KillTimer(hwndDlg, TIMER_NOTIFYINFO_ID);
			SetTimer(hwndDlg, TIMER_NOTIFYINFO_ID, TIMER_NOTIFYINFO_DELAY, Window_TimerProc);
		}
	}
}
static void Window_OnQueryInfo(HWND hwnd)
{
	KillTimer(hwnd, TIMER_NOTIFYINFO_ID);
	NotifyInfoWindow(hwnd, NULL, TRUE);
	SetTimer(hwnd, TIMER_NOTIFYINFO_ID, TIMER_NOTIFYINFO_DELAY, Window_TimerProc);
}

static void Window_OnOperationCompleted(HWND hwnd, DM_NOTIFY_PARAM *phdr)
{
	MEDIAINFO mediaInfo;
	
	if (phdr->cLetter != m_cdrom) return;

	ZeroMemory(&mediaInfo, sizeof(MEDIAINFO));
	mediaInfo.cLetter = m_cdrom;

	switch(phdr->opCode)
	{
		case DMOP_IMAPIINFO:
			if (S_OK == phdr->result)
			{
				DM_IMAPI_PARAM *pIMAPI = (DM_IMAPI_PARAM*)phdr;
												
				if ((0 != pIMAPI->fMediaType && 0 != pIMAPI->fMediaFlags))
				{
					mediaInfo.bInserted = TRUE;
					if (MEDIA_WRITABLE & pIMAPI->fMediaFlags) mediaInfo.bRecordable = TRUE;
					if (MEDIA_RW & pIMAPI->fMediaFlags) mediaInfo.bRewritable = TRUE;
					if (MEDIA_BLANK & pIMAPI->fMediaFlags) mediaInfo.bBlank = TRUE;
					mediaInfo.nSectorsFree = pIMAPI->ulFreeBlocks;
					mediaInfo.nSectorsUsed = pIMAPI->ulNextWritable;
				}
			}
			else GetMediaInfoFromSonic(&mediaInfo);
			FinishSetStatus(hwnd, &mediaInfo);	
			return;
	}
}

static INT_PTR CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	INT_PTR a = dialogSkinner.Handle(hwndDlg, uMsg, wParam, lParam); if (a) return a;
	switch (uMsg)
	{
		case WM_SIZE:
			if (wParam != SIZE_MINIMIZED)
			{
				childSizer.Resize(hwndDlg, burnwnd_rlist, sizeof(burnwnd_rlist) / sizeof(burnwnd_rlist[0]));
			}
			break;
		case WM_BURNNOTIFY:
			OnBurnNotify(hwndDlg, (DWORD)wParam, (DWORD)lParam);
			PostMessage(prevWnd, uMsg, wParam, lParam);
			break;
		case WM_INITDIALOG: OnBurnDlgInit(hwndDlg, lParam);	return 0;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				// link is dead so disabling for the time being
				/*case IDC_LOGO:
					if (HIWORD(wParam) == BN_CLICKED)
						ShellExecute(hwndDlg, L"open", L"http://estore.sonic.com/redirect.asp?id=spaol110103", NULL, L".", 0);
					break;*/
				case IDC_BURN_OPTS:
				{
					RECT r;
					HMENU menu = GetSubMenu(g_context_menus, 6);
					GetWindowRect((HWND)lParam, &r);
					CheckMenuItem(menu, ID_RIPOPTIONS_RIPPINGSTATUSWINDOW, g_config->ReadInt(L"cdburnstatuswnd", 0) ? MF_CHECKED : MF_UNCHECKED);
					CheckMenuItem(menu, ID_RIPOPTIONS_EJECTCDWHENCOMPLETED, g_config->ReadInt(L"cdburnautoeject", 1) ? MF_CHECKED : MF_UNCHECKED);
					CheckMenuItem(menu, ID_BURNOPTIONS_ADDCDTITLESTOLOCALCDDBCACHE, g_config->ReadInt(L"cdburnautoadddb", 1) ? MF_CHECKED : MF_UNCHECKED);
					CheckMenuItem(menu, ID_RIPOPTIONS_CLOSEVIEWWHENCOMPLETE, g_config->ReadInt(L"cdburnautoclose", 0) ? MF_CHECKED : MF_UNCHECKED);
					
					int x = Menu_TrackPopup(plugin.hwndLibraryParent, menu,
											TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN |
											TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
											r.left, r.top, hwndDlg, NULL);
					int val = 0, msgid;
					switch (x)
					{
						case ID_RIPOPTIONS_RIPPINGSTATUSWINDOW:
							val = g_config->ReadInt(L"cdburnstatuswnd", 0);
							g_config->WriteInt(L"cdburnstatuswnd", !val);
							msgid = BURNCFG_HIDEVIEW;
							break;
						case ID_RIPOPTIONS_EJECTCDWHENCOMPLETED:
							val = !g_config->ReadInt(L"cdburnautoeject", 1);
							g_config->WriteInt(L"cdburnautoeject", val);
							msgid = BURNCFG_AUTOEJECT;
							break;
						case ID_BURNOPTIONS_ADDCDTITLESTOLOCALCDDBCACHE:
							val = !g_config->ReadInt(L"cdburnautoadddb", 0);
							g_config->WriteInt(L"cdburnautoadddb", val);
							msgid = 	BURNCFG_ADDTODB;
							break;
						case ID_RIPOPTIONS_CLOSEVIEWWHENCOMPLETE:
							val = !g_config->ReadInt(L"cdburnautoclose", 0);
							g_config->WriteInt(L"cdburnautoclose", val);
							msgid = 	BURNCFG_AUTOCLOSE;
							break;
						default: msgid = 0; break;
					}
					if (msgid)
					{
						HWND h;
						h = cdburn_FindBurningHWND(m_cdrom);
						if (h) 
						{	
							PostMessage(h, WM_BURNCONFIGCHANGED, msgid, val);
							if (BURNCFG_HIDEVIEW == msgid) ShowWindow(h, val ? SW_HIDE : SW_SHOW);
						}
					}
					Sleep(100);
					MSG msg;
					while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
				}
				return 0;
				case IDC_ADD:
					if (DM_MODE_BURNING != DriveManager_GetDriveMode(m_cdrom))
					{
						RECT r;
						GetWindowRect((HWND)lParam, &r);
						int x = Menu_TrackPopup(plugin.hwndLibraryParent, GetSubMenu(g_context_menus, 3),
												TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN |
												TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
												r.left, r.top, hwndDlg, NULL);
						switch (x)
						{
							case ID_BURNADDMENU_FILES:
							{
								OPENFILENAMEW l = {sizeof(l), };
								wchar_t *temp;
								const int len = 256 * 1024 - 128;
								wchar_t *m_extlist = 0;
								m_extlist = (wchar_t*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 1, IPC_GET_EXTLISTW);
								if ((int)(INT_PTR)m_extlist == 1) m_extlist = 0;

								temp = (wchar_t *)GlobalAlloc(GPTR, len);
								l.hwndOwner = hwndDlg;
								l.lpstrFilter = m_extlist ? m_extlist : BuildFilterList();
								l.lpstrFile = temp;
								l.nMaxFile = len - 1;
								l.lpstrTitle = WASABI_API_LNGSTRINGW(IDS_ADD_FILES_TO_BURNING_LIST);
								l.lpstrDefExt = L"";
								l.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();

								l.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
								if (GetOpenFileNameW(&l))
								{
									wchar_t newCurPath[MAX_PATH] = {0};
									GetCurrentDirectoryW(MAX_PATH, newCurPath);
									WASABI_API_APP->path_setWorkingPath(newCurPath);

									if (temp[wcslen(temp) + 1])
									{
										wchar_t buf[MAX_PATH] = {0};
										wchar_t *p = temp;
										wchar_t *path = p;
										p += wcslen(p) + 1;
										while (p && *p)
										{
											if (*path)
												StringCchPrintfW(buf, MAX_PATH, L"%s%s%s", path, path[wcslen(path) - 1] == '\\' ? L"" : L"\\" , p);
											else
												StringCchPrintfW(buf, MAX_PATH, L"%s", p);

											cdburn_appendFile(buf, m_cdrom);
											p += wcslen(p) + 1;
										}
									}
									else
										cdburn_appendFile(temp, m_cdrom);
								}
								GlobalFree(temp);
								if (m_extlist) GlobalFree((HGLOBAL)m_extlist);
								SetStatus(hwndDlg, m_cdrom);
							}
							break;
							case ID_BURNADDMENU_FOLDER:
							{
								BROWSEINFOW bi = {0};
								wchar_t name[MAX_PATH] = {0};
								bi.hwndOwner = hwndDlg;
								bi.pszDisplayName = name;
								bi.lpszTitle = WASABI_API_LNGSTRINGW(IDS_CHOOSE_A_FOLDER_TO_ADD_TO_BURNING_LIST);
								bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
								bi.lpfn = BrowseCallbackProc;
								ITEMIDLIST *idlist = SHBrowseForFolderW(&bi);
								if (idlist)
								{
									wchar_t path[MAX_PATH] = {0};
									SHGetPathFromIDListW(idlist, path);
									Shell_Free(idlist);
									cdburn_appendFile(path, m_cdrom);
									SetStatus(hwndDlg, m_cdrom);
								}
							}
							break;
							case ID_BURNADDMENU_CURRENTPLAYLIST:
							{
								int plsize = (int)(INT_PTR)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
								for (int i = 0;i < plsize;i++)
								{
									wchar_t *name = (wchar_t *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, i, IPC_GETPLAYLISTFILEW);
									cdburn_appendFile(name, m_cdrom);
								}
								SetStatus(hwndDlg, m_cdrom);
							}
							break;
						}
						Sleep(100);
						MSG msg;
						while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
					}
					break;
				case IDC_CLEAR:
					if (DM_MODE_BURNING != DriveManager_GetDriveMode(m_cdrom))
					{
						wchar_t title[32] = {0};
						if (MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_SURE_YOU_WANT_TO_CLEAR_BURNING_LIST),
									   WASABI_API_LNGSTRINGW_BUF(IDS_CONFIRMATION,title,32), MB_YESNO) != IDYES)
							break;
						emptyRecordList(&itemCache[LETTERTOINDEX(m_cdrom)]);
						SetStatus(hwndDlg, m_cdrom);
						refreshList();
					}
					break;
				case IDC_CANCEL_BURN:
				case IDC_BURN:
				{
					HWND h;
					if (NULL != (h = cdburn_FindBurningHWND(m_cdrom)))
					{
						PostMessage(h, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BURN), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CANCEL_BURN), FALSE);

					}
					else doBurnDialog(hwndDlg);
				}
				break;
				case IDC_BTN_SHOWINFO:
					switch(HIWORD(wParam))
					{
						case BN_CLICKED: SendMessageW(GetParent(hwndDlg), WM_COMMAND, wParam, lParam); break;
					}
					break;
			}
			break;

		case WM_CONTEXTMENU:
		{
			if (DM_MODE_BURNING == DriveManager_GetDriveMode(m_cdrom) || m_statuslist.GetCount() == 0) return 0;

			HMENU menu = GetSubMenu(g_context_menus, 4);

			POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

			if (pt.x == -1 || pt.y == -1) // x and y are -1 if the user invoked a shift-f10 popup menu
			{
				RECT itemRect = {0};
				int selected = m_statuslist.GetNextSelected();
				if (selected != -1) // if something is selected we'll drop the menu from there
				{
					m_statuslist.GetItemRect(selected, &itemRect);
					ClientToScreen(m_statuslist.getwnd(), (POINT *)&itemRect);
				}
				else // otherwise we'll drop it from the top-left corner of the listview, adjusting for the header location
				{
					GetWindowRect(m_statuslist.getwnd(), &itemRect);

					HWND hHeader = (HWND)SNDMSG((HWND)wParam, LVM_GETHEADER, 0, 0L);
					RECT headerRect;
					if ((WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) && GetWindowRect(hHeader, &headerRect))
					{
						itemRect.top += (headerRect.bottom - headerRect.top);
					}
				}
				pt.x = itemRect.left;
				pt.y = itemRect.top;
			}

			HWND hHeader = (HWND)SNDMSG((HWND)wParam, LVM_GETHEADER, 0, 0L);
			RECT headerRect;
			if (0 == (WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) || FALSE == GetWindowRect(hHeader, &headerRect))
			{
				SetRectEmpty(&headerRect);
			}

			if (FALSE != PtInRect(&headerRect, pt))
			{
				return 0; 
			}

			int r = Menu_TrackPopup(plugin.hwndLibraryParent, menu,
									TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NONOTIFY,
									pt.x, pt.y, hwndDlg, NULL);
			switch (r)
			{
				case ID_BURNCONTEXTMENU_PLAYSELECTEDITEMS:
					playSelectedItems(hwndDlg, 0);
					break;
				case ID_BURNCONTEXTMENU_ENQUEUESELECTEDITEMS:
					playSelectedItems(hwndDlg, 1);
					break;
				case ID_BURNCONTEXTMENU_SELECTALL:
					selectAll();
					break;
				case ID_BURNCONTEXTMENU_REMOVESELECTEDITEMS:
					if (DM_MODE_BURNING != DriveManager_GetDriveMode(m_cdrom)) deleteSelectedItems(hwndDlg, m_cdrom);
					break;
				case ID_BURNCONTEXTMENU_BURN:
					doBurnDialog(hwndDlg);
					break;
				case ID_BURNCONTEXTMENU_MOVESELECTEDITEMSUP:
					moveSelItemsUp();
					break;
				case ID_BURNCONTEXTMENU_MOVESELECTEDITEMSDOWN:
					moveSelItemsDown();
					break;
			}
			Sleep(100);
			MSG msg;
			while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
			return 1;
		}

		case WM_NOTIFY:
		{
			LPNMHDR l = (LPNMHDR)lParam;
			if (l->idFrom == IDC_LIST2)
			{
				if (l->code == LVN_KEYDOWN)
				{
					LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
					switch (pnkd->wVKey)
					{
						case 38:   //up
							if (GetAsyncKeyState(VK_LMENU))
							{
								moveSelItemsUp();
								return 1;
							}
							break;
						case 40:   //down
							if (GetAsyncKeyState(VK_LMENU))
							{
								moveSelItemsDown();
								return 1;
							}
							break;
						case 46:   //delete
							if (DM_MODE_BURNING != DriveManager_GetDriveMode(m_cdrom)) deleteSelectedItems(hwndDlg, m_cdrom);
							break;
						case 'A':
							if (GetAsyncKeyState(VK_CONTROL))
								selectAll();
							break;
					}
				}
				else if (l->code == NM_DBLCLK)
				{
					if (DM_MODE_BURNING == DriveManager_GetDriveMode(m_cdrom)) return 0;
					playSelectedItems(hwndDlg, (!!g_config->ReadInt(L"enqueuedef", 0)) ^(!!(GetAsyncKeyState(VK_SHIFT)&0x8000)));
				}
				else if (l->code == NM_RETURN)
				{
					if (DM_MODE_BURNING == DriveManager_GetDriveMode(m_cdrom)) return 0;
					playSelectedItems(hwndDlg, 0 ^(!!(GetAsyncKeyState(VK_SHIFT)&0x8000)));
				}
				else if (l->code == LVN_GETDISPINFO)
				{
					NMLVDISPINFO *lpdi = (NMLVDISPINFO*) lParam;
					int item = lpdi->item.iItem;
					int idx = LETTERTOINDEX(m_cdrom);
					if (item < 0 || item >= itemCache[idx].Size) return 0;

					itemRecordW *thisitem = itemCache[idx].Items + item;

					if (lpdi->item.mask & (LVIF_TEXT |  /*LVIF_IMAGE*/0)) // we can always do images too :)
					{
						if (lpdi->item.mask & LVIF_TEXT)
						{
							wchar_t tmpbuf[128] = {0};
							wchar_t *nameptr = 0;
							switch (lpdi->item.iSubItem)
							{
								case 0:
									//track #
									StringCchPrintfW(tmpbuf, 128, L"%d", item + 1);
									nameptr = tmpbuf;
									break;
								case 1:
									//title
									lstrcpynW(tmpbuf, thisitem->title, 128);
									nameptr = tmpbuf;
									break;
								case 2:
									//length
									StringCchPrintfW(tmpbuf, 128, L"%01d:%02d", thisitem->length / 60, thisitem->length % 60);
									nameptr = tmpbuf;
									break;
								case 3:
									DWORD state = (DWORD) SendMessage(m_burning_other_wnd, WM_BURNGETITEMSTATUS, BURNSTATUS_STATE, (LPARAM)item);
									switch (state)
									{
										case BURNERITEM_BURNING:
										case BURNERITEM_DECODING:
											StringCchPrintfW(tmpbuf, 128, L"%s (%d%%)",
															 WASABI_API_LNGSTRINGW((BURNERITEM_BURNING == state) ? IDS_BURNING_ : IDS_PREPARING),
															 (DWORD)SendMessage(m_burning_other_wnd, WM_BURNGETITEMSTATUS, BURNSTATUS_PROGRESS, (LPARAM)item));
											nameptr = tmpbuf;
											break;
										case BURNERITEM_SUCCESS:	break;
										case BURNERITEM_BURNED:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_FINISHED,tmpbuf,128);
											break;
										case BURNERITEM_DECODED:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_PREPARED,tmpbuf,128);
											break;
										case BURNERITEM_SKIPPED:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_SKIPPED,tmpbuf,128);
											break;
										case BURNERITEM_READY:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_SCHEDULED,tmpbuf,128);
											break;
										case BURNERITEM_LICENSING:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_CHECKING_LICENSE,tmpbuf,128);
											break;
										case BURNERITEM_LICENSED:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_LICENSED,tmpbuf,128);
											break;
										case BURNERITEM_ABORTED:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_CANCELLED,tmpbuf,128);
											break;
										case BURNERITEM_FAILED:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_FAILED,tmpbuf,128);
											break;
										case BURNERITEM_CANCELING:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_CANCELLING,tmpbuf,128);
											break;
										case BURNERITEM_BADFILENAME:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_BAD_FILENAME,tmpbuf,128);
											break;
										case BURNERITEM_UNABLEOPENFILE:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_UNABLE_TO_OPEN_FILE,tmpbuf,128);
											break;
										case BURNERITEM_WRITEERROR:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_CACHE_WRITE_FAILED,tmpbuf,128);
											break;
										case BURNERITEM_DECODEERROR:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_UNABLE_TO_FIND_DECODER,tmpbuf,128);
											break;
										case BURNERITEM_ADDSTREAMFAILED:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_CANNOT_ADD_TO_THE_DISC,tmpbuf,128);
											break;
										case BURNERITEM_READSTREAMERROR:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_CACHE_READ_FAILED,tmpbuf,128);
											break;
										default:
											nameptr = WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN_ERROR,tmpbuf,128);
											break;
									}
									//status
									break;
							}
							if (nameptr)
								lstrcpynW(lpdi->item.pszText, nameptr, lpdi->item.cchTextMax);
							else
								lpdi->item.pszText[0] = 0;
						}
					}
					return 0;
				}
				else if (l->code == LVN_BEGINDRAG)
				{
					SetCapture(hwndDlg);
					m_dragging = 1;
					LPNMLISTVIEW nlv = (LPNMLISTVIEW)lParam;
					m_drag_item = nlv->iItem;
				}
				else if (l->code == LVN_ITEMCHANGED) ListView_OnItemChanged(hwndDlg, (NMLISTVIEW*)l);
			}
		}
		break;

		case WM_MOUSEMOVE:
			if (m_dragging)
			{
				POINT p;
				p.x = GET_X_LPARAM(lParam);
				p.y = GET_Y_LPARAM(lParam);
				ClientToScreen(hwndDlg, &p);
				ScreenToClient(m_statuslist.getwnd(), &p);
				int i = m_statuslist.FindItemByPoint(p.x, p.y);
				if (i != -1 && i != m_drag_item)
				{
					if (i > m_drag_item)
					{
						for (int j = 0;j < (i - m_drag_item);j++)
							moveSelItemsDown();
					}
					else
					{
						for (int j = 0;j < (m_drag_item - i);j++)
							moveSelItemsUp();
					}
					m_drag_item = i;
				}
			}
			break;

		case WM_LBUTTONUP:
			if (GetCapture() == hwndDlg)
			{
				ReleaseCapture();
				m_dragging = 0;
			}
			break;

		case WM_DESTROY:
			if (m_statuslist.getwnd())
			{
				g_view_metaconf->WriteInt(L"col_track", m_statuslist.GetColumnWidth(0));
				g_view_metaconf->WriteInt(L"col_title", m_statuslist.GetColumnWidth(1));
				g_view_metaconf->WriteInt(L"col_len", m_statuslist.GetColumnWidth(2));
				g_view_metaconf->WriteInt(L"col_status", m_statuslist.GetColumnWidth(3));
			}

			if (m_burning_other_wnd && IsWindow(m_burning_other_wnd))
			{
				PostMessage(m_burning_other_wnd, WM_BURNUPDATEOWNER, 0, (LPARAM)prevWnd);
				prevWnd = NULL;
			}
			m_hwndstatus = 0;

			if (hPLFont)
			{
				DeleteObject(hPLFont);
				hPLFont = NULL;
			}

			{
				HANDLE hPrev = (HANDLE) SendDlgItemMessage(hwndDlg,IDC_LOGO,STM_SETIMAGE,IMAGE_BITMAP, 0L);
				if (hPrev) DeleteObject(hPrev);
			}
			return 0;
		case WM_ML_CHILDIPC:
			if (lParam == ML_CHILDIPC_DROPITEM && wParam)
			{
				mlDropItemStruct *dis = (mlDropItemStruct *)wParam;
				if (DM_MODE_BURNING == DriveManager_GetDriveMode(m_cdrom))
				{
					dis->result = -1;
					return 0;
				}

				if (dis->type != ML_TYPE_ITEMRECORDLISTW && dis->type != ML_TYPE_ITEMRECORDLIST &&
					dis->type != ML_TYPE_FILENAMES && dis->type != ML_TYPE_FILENAMESW)
				{
					dis->result = -1;
				}
				else
				{
					if (dis->data)
					{
						dis->result = 1;
						if (dis->type == ML_TYPE_ITEMRECORDLIST)
						{
							itemRecordList *obj = (itemRecordList *)dis->data;
							cdburn_appendItemRecord(obj, m_cdrom);
						}
						else if (dis->type == ML_TYPE_ITEMRECORDLISTW)
						{
							itemRecordListW *obj = (itemRecordListW *)dis->data;
							cdburn_appendItemRecord(obj, m_cdrom);
						}
						else if (dis->type == ML_TYPE_FILENAMES) // playlist
						{
							char *p = (char*)dis->data;
							while (p && *p)
							{
								cdburn_appendFile(p, m_cdrom);
								p += strlen(p) + 1;
							}
						}
						else if (dis->type == ML_TYPE_FILENAMESW)
						{
							wchar_t *p = (wchar_t*)dis->data;
							while (p && *p)
							{
								cdburn_appendFile(p, m_cdrom);
								p += wcslen(p) + 1;
							}
						}
					}
				}
			}
			return 0;

		case WM_DROPFILES:
		{
			char temp[2048] = {0};
			HDROP h = (HDROP) wParam;
			int y = DragQueryFileA(h, 0xffffffff, temp, sizeof(temp));

			if (DM_MODE_BURNING == DriveManager_GetDriveMode(m_cdrom))
			{
//				MessageBoxA(hwndDlg,"Cannot add files while burning","CD Burner",MB_OK);
			}
			else for (int x = 0; x < y; x ++)
				{
					DragQueryFileA(h, x, temp, sizeof(temp));
					cdburn_appendFile(temp, m_cdrom);
				}

			DragFinish(h);
		}
		return 0;
		case WM_PAINT:
		{
			int tab[] = { IDC_LIST2 | DCW_SUNKENBORDER, IDC_LOGO | DCW_SUNKENBORDER};
			dialogSkinner.Draw(hwndDlg, tab, 2);
		}
		return 0;
		case WM_ERASEBKGND: return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT
		case WM_QUERYFILEINFO:		Window_OnQueryInfo(hwndDlg); break;
		case WM_EX_OPCOMPLETED:		Window_OnOperationCompleted(hwndDlg, (DM_NOTIFY_PARAM*)lParam);
	}
	return FALSE;
}

static HWND BurnAddStatus_wnd;

static BOOL CALLBACK BurnAddStatus_proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CLOSE)
		DestroyWindow(hwndDlg);
	return 0;
}

void BurnAddStatus_Create(int num)
{
	if (BurnAddStatus_wnd && IsWindow(BurnAddStatus_wnd)) DestroyWindow(BurnAddStatus_wnd);
	BurnAddStatus_wnd = WASABI_API_CREATEDIALOGW(IDD_BURN_ADD_STATUS, plugin.hwndLibraryParent, BurnAddStatus_proc);
	if (!BurnAddStatus_wnd) return ;

	SetTimer(BurnAddStatus_wnd, 1, 100, NULL);
	SendDlgItemMessage(BurnAddStatus_wnd, IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELPARAM(0, num));

	unsigned int start_t = GetTickCount();
	if (start_t >= 0xffffff00) start_t = 0;

	MSG msg;
	while (GetTickCount() < start_t + 100 && GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void BurnAddStatus_Step(itemRecordListW *items)
{
	if (!BurnAddStatus_wnd || !items || items && !items->Size || items && !items->Items) return ;
	SendDlgItemMessage(BurnAddStatus_wnd, IDC_PROGRESS1, PBM_DELTAPOS, 1, 0);

	int l = 0;
	for (int i = 0;i < items->Size;i++)
	{
		l += items->Items[i].length;
	}
	wchar_t buf[512] = {0};
	StringCchPrintf(buf, 512, WASABI_API_LNGSTRINGW(IDS_ADDING_TRACKS_TO_BURNER_TOTAL_LENGTH_X), l / 60, l % 60);
	SetDlgItemText(BurnAddStatus_wnd, IDC_STAT, buf);
}

void BurnAddStatus_Done()
{
	if (!BurnAddStatus_wnd) return ;
	unsigned int start_t = GetTickCount();
	if (start_t >= 0xffffff00) start_t = 0;

	MSG msg;
	while (GetTickCount() < start_t + 1000 && IsWindow(BurnAddStatus_wnd) && GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(BurnAddStatus_wnd);
	BurnAddStatus_wnd = 0;
}

static bool cdrFound(char letter)
{
	wchar_t name[]= L"cda://X.cda";
	wchar_t info[16] = L"";
	name[6] = letter;
	getFileInfoW(name, L"cdtype", info, sizeof(info)/sizeof(wchar_t));
	return !lstrcmpW(info, L"CDR") || !lstrcmpW(info, L"CDRW");
}

static char m_burnwait_letter;
static BOOL CALLBACK BurnWaitProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SetTimer(hwndDlg, 1, 2000, NULL);
			{
				wchar_t buf[512] = {0};
				StringCchPrintf(buf, 512, WASABI_API_LNGSTRINGW(IDS_PLEASE_INSERT_BLANK_RECORDABLE_CD), toupper(m_burnwait_letter));
				SetDlgItemText(hwndDlg, IDC_TEXT, buf);
			}
			return 0;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDCANCEL) 	EndDialog(hwndDlg, 1);
			return 0;
		case WM_TIMER:
			if (cdrFound(m_burnwait_letter)) EndDialog(hwndDlg, 0);
			return 0;
	}
	return 0;
}

int Burn_WaitForCDR(HWND hwndParent, char driveletter) // returns 0 on CD-R found, 1 on cancel
{
	CHAR cMode;
	if (!driveletter) return 1;
	cMode = DriveManager_GetDriveMode(driveletter);
	if (DM_MODE_BURNING == cMode || DM_MODE_RIPPING == cMode)
	{
		return 1; // if burning or ripping, don't fuck with it
	}

	if (cdrFound(driveletter)) return 0;
	if (m_burnwait_letter) return 1;

	m_burnwait_letter = (char)toupper(driveletter);
	int x = (int)(INT_PTR)WASABI_API_DIALOGBOXW(IDD_WAITFORCDR, hwndParent, BurnWaitProc);
	m_burnwait_letter = 0;
	return x;
}
