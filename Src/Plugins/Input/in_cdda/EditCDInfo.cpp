#include "main.h"
#include "cddb.h"
#include "../nu/ListView.h"
#include "cddbinterface.h"
#include "../Agave/Language/api_language.h"
#include "../Winamp/wa_ipc.h"
#include <atlbase.h>
#include <strsafe.h>

int _g_disable_cddb;

#if 0
static DINFO *p;
static char dialogDevice;
W_ListView trackList;

static void Populate(HWND hDlg)
{
	int x;
	trackList.Clear();
	SetDlgItemTextW(hDlg, IDC_TITLE, p->title);
	SetDlgItemTextW(hDlg, IDC_PUBLISHER, p->label);
	SetDlgItemTextW(hDlg, IDC_ARTIST, p->artist);
	SetDlgItemTextW(hDlg, IDC_GENRE, p->genre);
	SetDlgItemTextW(hDlg, IDC_YEAR, p->year);
	CheckDlgButton(hDlg, IDC_COMPILATION, p->compilation?BST_CHECKED:BST_UNCHECKED);
	for (x = 0; x < p->ntracks; x ++)
	{
		trackList.InsertItem(x, p->tracks[x].artist, 0);
		trackList.SetItemText(x, 1, p->tracks[x].title);
	}
}

static HRESULT CALLBACK Cddb_ResultCallback(DWORD cddbId, HRESULT result, ICddbDisc *pDisc, ULONG_PTR user)
{
	CBDATA *pData = (CBDATA*)user;
	DefaultValues(pData->p);

	if(S_OK == result) 
	{		
			
	}
	else if (S_FALSE == result) 
	{
	
	}
	else if (S_MULTIPLE == result)
	{
	
	}
	Populate(pData->hwnd);
	return S_OK;
}

static LRESULT CALLBACK EditProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NOTIFYFORMAT:
		return NFR_UNICODE;

	case WM_INITDIALOG:
		trackList.setwnd(GetDlgItem(hDlg, IDC_TRACKS));

		RECT rect1, rect2;
		GetWindowRect(GetDlgItem(hDlg, IDC_EDITARTIST), &rect1);
		GetWindowRect(GetDlgItem(hDlg, IDC_EDITTEXT), &rect2);

		trackList.AddCol(WASABI_API_LNGSTRINGW(plugin.hDllInstance,IDS_ARTIST), rect2.left-rect1.left-3);
		trackList.AddCol(WASABI_API_LNGSTRINGW(plugin.hDllInstance,IDS_TITLE), (rect2.right-rect2.left)-18);

		if (_g_disable_cddb) EnableWindow(GetDlgItem(hDlg, IDC_GETCDDB), 0);
		SendDlgItemMessage(hDlg, IDC_TITLE, EM_LIMITTEXT, 512, 0);
		SendDlgItemMessage(hDlg, IDC_PUBLISHER, EM_LIMITTEXT, 512, 0);
		SendDlgItemMessage(hDlg, IDC_ARTIST, EM_LIMITTEXT, 512, 0);
		SendDlgItemMessage(hDlg, IDC_YEAR, EM_LIMITTEXT, 8, 0);
		SendDlgItemMessage(hDlg, IDC_GENRE, EM_LIMITTEXT, 64, 0);
		SendDlgItemMessage(hDlg, IDC_EDITTEXT, EM_LIMITTEXT, 512, 0);
		SendDlgItemMessage(hDlg, IDC_EDITARTIST, EM_LIMITTEXT, 512, 0);

		if (GetCDDBInfo(p, dialogDevice))
		{
			int x;
			SetDlgItemText(hDlg, IDC_TITLE, WASABI_API_LNGSTRINGW(line.hDllInstance,IDS_TITLE));
			SetDlgItemText(hDlg, IDC_ARTIST, WASABI_API_LNGSTRINGW(line.hDllInstance,IDS_ALBUM_ARTIST));
			for (x = 0; x < p->ntracks; x ++)
			{
				wchar_t buf[128] = {0};
				StringCchPrintfW(buf, 128, L"Track %d", x + 1);
				trackList.InsertItem(x, "Artist", 0);
				trackList.SetItemText(x, 1, buf);
			}
		}
		else
		{
			Populate(hDlg);
		}

		return 1;
	case WM_NOTIFY:
	{
		LPNMHDR l = (LPNMHDR)lParam;
		switch (l->code)
		{
		case LVN_ITEMCHANGED:
		{
			LPNMLISTVIEW lvNotif = (LPNMLISTVIEW)l;
			if (lvNotif->uNewState & LVIS_SELECTED)
			{
				wchar_t buf[512] = {0};
				trackList.GetText(trackList.GetNextSelected(-1), 0, buf, 512);
				SetDlgItemTextW(hDlg, IDC_EDITARTIST, buf);
				trackList.GetText(trackList.GetNextSelected(-1), 1, buf, 512);
				SetDlgItemTextW(hDlg, IDC_EDITTEXT, buf);
			}
		}
		break;
		}
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_GETCDDB:
			{
				wchar_t szTOC[2048] = {0};
				if (Cddb_CalculateTOC(p, szTOC, sizeof(szTOC)/sizeof(wchar_t)))
				{
					CBDATA data = { p, hDlg };
					HRESULT hr = Cddb_DoLookup(p->CDDBID, szTOC, hDlg, Cddb_ResultCallback, 
						CDDB_NOCACHE | CDDB_MODAL | CDDB_UI_MULTIPLE, (ULONG_PTR)&data);
				} 
				else hr = CDDB_E_BADTOC;
			}
			break;
		case IDC_CDTEXT:
			if (DoCDText(p, dialogDevice))
			{
				Populate(hDlg);
			}
			break;
		case IDCANCEL: EndDialog(hDlg, 1); break;
		case IDOK:
		{
			int x;
			GetDlgItemTextW(hDlg, IDC_TITLE, p->title, TITLE_SIZE);
			GetDlgItemTextW(hDlg, IDC_PUBLISHER, p->label, 511); p->label[511] = 0;
			GetDlgItemTextW(hDlg, IDC_ARTIST, p->artist, ARTIST_SIZE);
			GetDlgItemTextW(hDlg, IDC_GENRE, p->genre, 63); p->genre[63] = 0;
			GetDlgItemTextW(hDlg, IDC_YEAR, p->year, 8);
			p->compilation = !!IsDlgButtonChecked(hDlg, IDC_COMPILATION);
			for (x = 0; x < p->ntracks; x ++)
			{
				trackList.GetText(x, 0, p->tracks[x].artist, ARTIST_SIZE);
				trackList.GetText(x, 1, p->tracks[x].title, TITLE_SIZE);
			}
			AddToDatabase(p);
		}

		EndDialog(hDlg, 0);
		break;
		case IDC_REMOVE:
			RemoveFromDatabase(p);
			EndDialog(hDlg, 1); break;

		case IDC_EDITTEXT:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				wchar_t buf[512] = {0};
				int p = trackList.GetNextSelected(-1);
				if (p != LB_ERR)
				{
					GetDlgItemTextW(hDlg, IDC_EDITTEXT, buf, 512);
					trackList.SetItemText(p, 1, buf);
				}
			}
			break;

		case IDC_EDITARTIST:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				wchar_t buf[512] = {0};
				int p = trackList.GetNextSelected(-1);
				if (p != LB_ERR)
				{
					GetDlgItemTextW(hDlg, IDC_EDITARTIST, buf, 512);
					trackList.SetItemText(p, 0, buf);

					if (!IsDlgButtonChecked(hDlg, IDC_COMPILATION))
					{
						wchar_t buf2[512] = {0};
						GetDlgItemTextW(hDlg, IDC_ARTIST, buf2, 512);
						if (lstrcmp(buf, buf2))
							CheckDlgButton(hDlg, IDC_COMPILATION, BST_CHECKED);
					}
				}
			}
			break;
		case IDC_ARTIST:
			if (HIWORD(wParam) == EN_CHANGE && !IsDlgButtonChecked(hDlg, IDC_COMPILATION))
			{
				wchar_t buf[512] = {0};
				GetDlgItemTextW(hDlg, IDC_ARTIST, buf, 512);

				for (int x = 0; x < p->ntracks; x ++)
				{
					trackList.SetItemText(x, 0, buf);
				}
				if (trackList.GetNextSelected(-1) != LB_ERR)
				{
					SetDlgItemTextW(hDlg, IDC_EDITARTIST, buf);

				}
			}
			break;

		}
		return 0;
	}
	return 0;
}

void DBEdit(DINFO *ps, HWND hwnd, int l, char device)
{
	static int i = 0;
	if (i) return ;
	i = 1;
	p = ps;
	dialogDevice = device;
	_g_disable_cddb = l;
	if (!WASABI_API_DIALOGBOX(IDD_EDIT, hwnd, (DLGPROC)EditProc))
	{
		PostMessage(line.hMainWindow, WM_USER, (WPARAM)L"cda://", 247 /*IPC_REFRESHPLCACHE*/);
	}
	i = 0;
}

#endif
#if 0

typedef struct _EDITDATA
{
	DINFO	*p;
	INT		modified;
	CHAR	cDevice;

} EDITDATA;

#define GET_DATA(__hwnd) (EDITDATA*)(LONG_PTR)GetWindowLongPtrW((__hwnd), GWLP_USERDATA);


#define SET_IF(hwndDlg, id, data) if (data) SetDlgItemText(hwndDlg, id, data); else SetDlgItemText(hwndDlg, id, L"");
static void Fill(HWND hwndDlg, const DINFO *info)
{
	SET_IF(hwndDlg, IDC_TITLE, info->title);
	SET_IF(hwndDlg, IDC_ARTIST, info->artist);

	if (info->discnum)
		SetDlgItemInt(hwndDlg, IDC_DISC, info->discnum, FALSE);
	else
		SetDlgItemText(hwndDlg, IDC_DISCS, L"");

	if (info->numdiscs)
		SetDlgItemInt(hwndDlg, IDC_DISCS, info->numdiscs, FALSE);
	else
		SetDlgItemText(hwndDlg, IDC_DISCS, L"");

	SET_IF(hwndDlg, IDC_YEAR, info->year);
	SET_IF(hwndDlg, IDC_LABEL, info->label);
	SET_IF(hwndDlg, IDC_NOTES, info->notes);
	SET_IF(hwndDlg, IDC_GENRE, info->genre);

	CheckDlgButton(hwndDlg, IDC_COMPILATION, (info->compilation)?BST_CHECKED:BST_UNCHECKED);
	SendDlgItemMessage(hwndDlg, IDC_TRACKLIST, LB_RESETCONTENT, 0, 0);

	for (int x = 0; x < info->ntracks; x ++)
	{
		wchar_t buf[1100] = {0};
		if (!info->tracks[x].title)
			StringCchPrintfW(buf, 1100, L"%d.", x+1);
		else if (info->tracks[x].artist && info->tracks[x].artist[0] && wcscmp(info->tracks[x].artist, info->artist))
			StringCchPrintfW(buf, 1100, L"%d. %s - %s", x+1, info->tracks[x].artist, info->tracks[x].title);
		else
			StringCchPrintfW(buf, 1100, L"%d. %s", x+1, info->tracks[x].title);
		SendDlgItemMessageW(hwndDlg, IDC_TRACKLIST, LB_ADDSTRING, 0, (LPARAM)buf);

	}
}

static HRESULT CALLBACK Cddb_LookupCallback(HRESULT result, ICddbDisc *pDisc, DWORD *pdwAutoCloseDelay, ULONG_PTR user)
{
	HWND hwnd = (HWND)user;
	EDITDATA *pData = GET_DATA(hwnd);
	if (!pData) return E_INVALIDARG;

	if(S_OK == result) 
	{	
		pData->modified = 1;
		DefaultValues(pData->p);
		GetDiscInfo(pDisc, pData->p);
		StoreDisc(pData->p->CDDBID, pDisc);
		ICddbCacheManager* pCache;
		HRESULT hr = Cddb_GetICacheManger((void**)&pCache);
		if (SUCCEEDED(hr))
		{
			CComBSTR toc;
			pDisc->get_Toc(&toc);
			hr = pCache->StoreDiscByToc(toc, pDisc);
			pCache->Release();
		}
		Fill(hwnd, pData->p);
	}
	else 
	{
		*pdwAutoCloseDelay = AUTOCLOSE_NEVER;
	}
	CddbCache_SetDisc(pData->p, result);
	return S_OK;
}

static HRESULT CALLBACK Cddb_EditCallback(HRESULT result, ICddbDisc *pDisc, DWORD *pdwAutoCloseDelay, ULONG_PTR user)
{
	HWND hwnd = (HWND)user;
	EDITDATA *pData = GET_DATA(hwnd);
	if (!pData) return E_INVALIDARG;
			
	if (FAILED(result))
	{
		*pdwAutoCloseDelay = AUTOCLOSE_NEVER;
		return S_OK;
	}
	
	if (SUCCEEDED(result))
	{
		HRESULT hr(S_OK);
		wchar_t szTOC[2048] = {0};
		ICDDBControl  *pControl;
		CDDBUIFlags uiFlags = UI_EDITMODE;
			
		if (!Cddb_CalculateTOC(pData->p, szTOC, sizeof(szTOC)/sizeof(wchar_t))) hr = CDDB_E_BADTOC;
		if (SUCCEEDED(hr)) hr = Cddb_GetIControl((void**)&pControl);
		if (SUCCEEDED(hr))
		{
			if (!pDisc) 
			{
				uiFlags = UI_SUBMITNEW;
				hr = pControl->GetSubmitDisc(szTOC, 0, 0, &pDisc);
				if (FAILED(hr)) pDisc = NULL;
			}
			else pDisc->AddRef();
			
			if (pDisc)
			{
				Cddb_DisplayDiscInfo(pDisc, &uiFlags, hwnd);
				if (uiFlags & UI_DATA_CHANGED)
				{
					ICddbCacheManager* pCache;
					hr = Cddb_GetICacheManger((void**)&pCache);
					if (SUCCEEDED(hr))
					{
						hr = pCache->StoreDiscByToc(szTOC, pDisc);
						pCache->Release();
					}
									
					pData->modified = 1;
					DefaultValues(pData->p);
					GetDiscInfo(pDisc, pData->p);
					StoreDisc(pData->p->CDDBID, pDisc);
					Fill(hwnd, pData->p);
					CddbCache_SetDisc(pData->p, S_OK);
				}
				pDisc->Release();
			}
			pControl->Release();
		}
		
	}
	return S_OK;
}

static LRESULT CALLBACK EditProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	EDITDATA *pData;

	switch (message)
	{
		case WM_INITDIALOG:
			{
				pData = (EDITDATA*)lParam;
				if (!pData) { EndDialog(hwndDlg, 0); return 0; }
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
				pData->modified = 0;
				Fill(hwndDlg, pData->p);
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_CDDB:
					{			
						wchar_t szTOC[2048] = {0};
						pData = GET_DATA(hwndDlg);
					
						if (pData && Cddb_CalculateTOC(pData->p, szTOC, sizeof(szTOC)/sizeof(wchar_t)))
						{
							UINT flags = CDDB_NOCACHE | CDDB_UI_MODAL | CDDB_UI_MULTIPLE | CDDB_UI_RESULT_MODAL;
							HRESULT hr = Cddb_DoLookup(szTOC, hwndDlg, Cddb_LookupCallback, flags, (ULONG_PTR)hwndDlg);
							if (FAILED(hr)) Cddb_DisplayResultDlg(hwndDlg, hr, AUTOCLOSE_NEVER, flags);
						} 			
					}
					break;
				case IDC_EDIT:
					{
						wchar_t szTOC[2048] = {0};
						pData = GET_DATA(hwndDlg);

						if (pData && Cddb_CalculateTOC(pData->p, szTOC, sizeof(szTOC)/sizeof(wchar_t)))
						{
							UINT flags = CDDB_UI_MODAL | CDDB_UI_MULTIPLE | CDDB_UI_RESULT_MODAL;
							HRESULT hr = Cddb_DoLookup(szTOC, hwndDlg, Cddb_EditCallback, flags, (ULONG_PTR)hwndDlg);
							if (FAILED(hr)) Cddb_DisplayResultDlg(hwndDlg, hr, AUTOCLOSE_NEVER, flags);
						} 		
					}
					break;
				case IDCANCEL:
					pData = (EDITDATA*)(LONG_PTR)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					EndDialog(hwndDlg, (pData) ? pData->modified : 0);
					break;
			}
			break;
	}
	return 0;
}
bool CDEdit(CHAR cDevice, DINFO *ps, HWND hwnd)
{
	EDITDATA data;
	ZeroMemory(&data, sizeof(EDITDATA));
	data.cDevice = cDevice;
	data.p = ps;

	if (WASABI_API_DIALOGBOXPARAM(IDD_EDIT, hwnd, (DLGPROC)EditProc, (LPARAM)&data))
	{
		PostMessageW(line.hMainWindow, WM_USER, (WPARAM)L"cda://", IPC_REFRESHPLCACHE);
	}
	return ( 0 != data.modified);
}
#endif