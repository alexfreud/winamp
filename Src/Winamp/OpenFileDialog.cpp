/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "main.h"
#include "resource.h"
#include "strutil.h"
#include "../nu/AutoChar.h"
#include "api.h"
#include <shlwapi.h>
#include <shobjidl.h>

/*static wchar_t *inc(wchar_t *p, size_t &size, int extra=0)
{
	int len = lstrlenW(p) + extra;
	size-=len;
	p+=len;
	return p;
}*/

#if 0
extern std::vector<In_Module*> in_modules;
void getNewFileVistaPlus(int clearlist, HWND hwnd, const wchar_t *path)
{
    IFileOpenDialog *pFileOpen;
	static int qq;
	if (qq) return;
	qq = 1;
    HRESULT hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));
    if (SUCCEEDED(hr))
    {
		wchar_t titleStr[128] = {0};

		pFileOpen->SetTitle(getStringW((clearlist ? IDS_OFD_OPEN_FILES : IDS_OFD_ADD_FILES_TO_PLAYLIST),titleStr,128));
		pFileOpen->SetDefaultExtension(L"");
		pFileOpen->SetOptions(FOS_FILEMUSTEXIST | FOS_ALLOWMULTISELECT | FOS_NOREADONLYRETURN | FOS_PATHMUSTEXIST);
		if(!clearlist) pFileOpen->SetOkButtonLabel(L"Add");

		IShellItem* shellItem;
		hr = SHCreateItemFromParsingName((!path ? WASABI_API_APP->path_getWorkingPath() : path), 0, IID_IShellItem, reinterpret_cast<void**>(&shellItem));
		if (SUCCEEDED(hr))
		{
			pFileOpen->SetFolder(shellItem);
		}

		wchar_t *fsb = in_getfltstrW(TRUE);
		int filterNum = 0;
		// allocate enough for the number of input plug-ins plus 'all supported' and 'all files'
		COMDLG_FILTERSPEC *fileTypes = new COMDLG_FILTERSPEC[in_modules.size() + 2];
		while(fsb && *fsb && *fsb+1)
		{
			int len = lstrlenW(fsb) + 1;
			wchar_t *fsb2 = fsb + len;
            fileTypes[filterNum].pszName = fsb;
            fileTypes[filterNum].pszSpec = fsb2;
			CharUpperBuffW(fsb2, lstrlenW(fsb2));
			filterNum++;
			fsb += len + lstrlenW(fsb2) + 1;
		}

		pFileOpen->SetFileTypes(filterNum, fileTypes);

		if (hwnd == hMainWindow)
			UninitDragDrops();

        hr = pFileOpen->Show(hwnd);
        if (SUCCEEDED(hr))
        {
			IShellItemArray *pItems;
			hr = pFileOpen->GetResults(&pItems);
            if (SUCCEEDED(hr))
            {
				IEnumShellItems *sItems;
				hr = pItems->EnumItems(&sItems);
				if (SUCCEEDED(hr))
				{
					DWORD numItems = 0;
					pItems->GetCount(&numItems);

					int sp;
					if (clearlist)
					{
						sp = 0; PlayList_delete();
					}
					else sp = PlayList_getlength();

					if(numItems <= 1)
					{
						IShellItem *pItem;
						if(sItems->Next(1, &pItem, NULL) == S_OK)
						{
							LPWSTR folderpath = NULL;
							LPWSTR filepath = NULL;
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filepath);
							if (SUCCEEDED(hr))
							{
								IShellItem *ppItem;
								hr = pItem->GetParent(&ppItem);
								if (SUCCEEDED(hr))
								{
									hr = ppItem->GetDisplayName(SIGDN_FILESYSPATH, &folderpath);
									if (SUCCEEDED(hr))
									{
										WASABI_API_APP->path_setWorkingPath(folderpath);
									}
								}

								if (LoadPlaylist(filepath, 1, 0) == -1) // if not a playlist file
								{
									PlayList_append(filepath, 0);
								}

								CoTaskMemFree(filepath);
								CoTaskMemFree(folderpath);
							}
						}
						pItem->Release();
					}
					else
					{
						size_t size = ((numItems + 1) * MAX_PATH);
						wchar_t *temp = (wchar_t *)GlobalAlloc(GPTR, size * sizeof(wchar_t)), *p = temp;
						IShellItem *pItem;
						while(sItems->Next(1, &pItem, NULL) == S_OK)
						{
							LPWSTR folderpath = NULL;
							LPWSTR filepath = NULL;
							hr = pItem->GetDisplayName(SIGDN_NORMALDISPLAY, &filepath);
							if (SUCCEEDED(hr))
							{
								// replicate how lpstrFile is filled under a multiple select
								// -> selected folder followed by each filename \0 separated
								if(p == temp)
								{
									IShellItem *ppItem;
									hr = pItem->GetParent(&ppItem);
									if (SUCCEEDED(hr))
									{
										hr = ppItem->GetDisplayName(SIGDN_FILESYSPATH, &folderpath);
										if (SUCCEEDED(hr))
										{
											WASABI_API_APP->path_setWorkingPath(folderpath);

											if(StringCchCatW(p, size, folderpath) == S_OK)
											{
												int len = lstrlenW(folderpath) + 1;
												p += len;
												size -= len;
											}
										}
									}
								}

								if(StringCchCatW(p, size, filepath) == S_OK)
								{
									int len = lstrlenW(filepath) + 1;
									p += len;
									size -= len;
								}

								CoTaskMemFree(filepath);
								CoTaskMemFree(folderpath);
							}
							pItem->Release();
						}

						PlayList_addfromdlg(temp);
						if (config_rofiob&1) PlayList_sort(2, sp);

						GlobalFree(temp);
					}

					if (clearlist)
						BeginPlayback();

					sItems->Release();
				}
				pItems->Release();
            }
        }
        pFileOpen->Release();
		GlobalFree(fsb);

		if (hwnd == hMainWindow)
			InitDragDrops();
		for(;;)
		{
			MSG msg;
			if (!PeekMessage(&msg, hMainWindow, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
				break;
		}
    }
	qq = 0;
}
#endif

void getNewFile(int clearlist, HWND hwnd, const wchar_t *path) // if clearlist is 1, playlist is cleared
{
	// TODO - Egg was having stability issues with this so keeping disabled until resolved
	// on Vista and up use the IFileOpenDialog interface as it
	// allows us to get around the MAX_PATH file filter issues
	/*if(IsVista())
	{
		getNewFileVistaPlus(clearlist, hwnd, path);
		return;
	}*/

	// otherwise revert to using the older pre-Vista methods
	const int len = 256 * 1024 - 128;

	wchar_t oldCurPath[MAX_PATH] = {0}, titleStr[128] = {0};

	GetCurrentDirectoryW(MAX_PATH, oldCurPath);
	wchar_t *temp, *fsb;
	OPENFILENAMEW l = {sizeof(OPENFILENAMEW),0};
	static int q;
	if (q) return;
	q = 1;
	temp = (wchar_t *)GlobalAlloc(GPTR, len * sizeof(wchar_t));
	l.lStructSize = sizeof(l);
	l.hwndOwner = hwnd;
	l.lpstrFilter = fsb = in_getfltstrW(FALSE);

	l.lpstrFile = temp;
	l.nMaxFile = len - 1;
	l.lpstrTitle = getStringW((clearlist ? IDS_OFD_OPEN_FILES : IDS_OFD_ADD_FILES_TO_PLAYLIST),titleStr,128);
	l.lpstrDefExt = L"";
	l.lpstrInitialDir = path;
	if (!l.lpstrInitialDir) l.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();

	l.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT
	          | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	if (hwnd == hMainWindow)
		UninitDragDrops();
	if (GetOpenFileNameW(&l))
	{
		wchar_t newCurPath[MAX_PATH] = {0};
		GetCurrentDirectoryW(MAX_PATH, newCurPath);
		WASABI_API_APP->path_setWorkingPath(newCurPath);
		int sp;
		if (clearlist)
		{
			sp = 0; PlayList_delete();
		}
		else sp = PlayList_getlength();
		if (temp[lstrlenW(temp)+1])
		{
			PlayList_addfromdlg(temp);
			if (config_rofiob&1) PlayList_sort(2, sp);
		}
		else
		{
			if (LoadPlaylist(temp, 1, 0) == -1) // if not a playlist file
			{
				PlayList_append(temp, 0);
			}
		}
		if (clearlist)
			BeginPlayback();
	}
	SetCurrentDirectoryW(oldCurPath);

	if (hwnd == hMainWindow)
		InitDragDrops();
	for(;;)
	{
		MSG msg;
		if (!PeekMessage(&msg, hMainWindow, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			break;
	}
	GlobalFree(fsb);
	GlobalFree(temp);
	q = 0;
}

static wchar_t loc[FILENAME_SIZE];
int ResizeComboBoxDropDown(HWND hwndDlg, UINT id, const char* str, int width){
	SIZE size = {0};
	HWND control = GetDlgItem(hwndDlg, id);
	HDC hdc = GetDC(control);
	// get and select parent dialog's font so that it'll calculate things correctly
	HFONT font = (HFONT)SendMessageW(hwndDlg,WM_GETFONT,0,0), oldfont = (HFONT)SelectObject(hdc,font);
	GetTextExtentPoint32A(hdc, str, lstrlenA(str)+1, &size);

	int ret = width;
	if(size.cx > width)
	{
		SendDlgItemMessage(hwndDlg, id, CB_SETDROPPEDWIDTH, size.cx, 0);
		ret = size.cx;
	}

	SelectObject(hdc, oldfont);
	ReleaseDC(control, hdc);
	return ret;
}

int ResizeComboBoxDropDownW(HWND hwndDlg, UINT id, const wchar_t *str, int width){
	SIZE size = {0};
	HWND control = GetDlgItem(hwndDlg, id);
	HDC hdc = GetDC(control);
	// get and select parent dialog's font so that it'll calculate things correctly
	HFONT font = (HFONT)SendMessageW(hwndDlg,WM_GETFONT,0,0), oldfont = (HFONT)SelectObject(hdc,font);
	GetTextExtentPoint32W(hdc, str, (int) wcslen(str)+1, &size);

	int ret = width;
	if(size.cx > width)
	{
		SendDlgItemMessage(hwndDlg, id, CB_SETDROPPEDWIDTH, size.cx, 0);
		ret = size.cx;
	}

	SelectObject(hdc, oldfont);
	ReleaseDC(control, hdc);
	return ret;
}

static LRESULT WINAPI locProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			if(lParam) SetWindowTextW(hwndDlg, getStringW(IDS_ADD_URL, NULL, 0));
			SendDlgItemMessageW(hwndDlg, IDC_URL_NEW, CB_LIMITTEXT, sizeof(loc)/sizeof(*loc) - 1, 0);
			{
				int width = 0, x = 0;
				HWND h = GetDlgItem(hwndDlg, IDC_URL_NEW);
				SendMessageW(h, CB_SETHORIZONTALEXTENT, 400, 0);
				while(1)
				{
					char s[123] = {0};
					wchar_t name[FILENAME_SIZE] = {0};
					StringCchPrintfA(s, 123, "RecentURL%d", x + 1);
					_r_sW(s, name, 123);
					if (!name[0]) break;
					SendMessageW(h, CB_ADDSTRING, 0, (LPARAM)name);
					width = ResizeComboBoxDropDownW(hwndDlg, IDC_URL_NEW, name, width);
					x++;
				}

				// show add / open loc window and restore last position as applicable
				POINT pt = {loc_rect.left, loc_rect.top};
				if (!windowOffScreen(hwndDlg, pt))
					SetWindowPos(hwndDlg, HWND_TOP, loc_rect.left, loc_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
			}

			SetDlgItemTextW(hwndDlg, IDC_URL_NEW, loc);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					GetDlgItemTextW(hwndDlg, IDC_URL_NEW, loc, sizeof(loc)/sizeof(*loc) - 1);
					{
						wchar_t *p = loc;
						while (IsCharSpaceW(*p)) p = CharNextW(p);
						if (!*p) EndDialog(hwndDlg, 1);
						else recent_add(loc);
					}
				case IDCANCEL:
					GetWindowRect(hwndDlg, &loc_rect);
					EndDialog(hwndDlg, (LOWORD(wParam) == IDCANCEL));
					return FALSE;
				case IDRESET:
					if (LPMessageBox(hwndDlg, IDS_RESET_URLS, IDS_RESET_URLS_TITLE, MB_ICONQUESTION | MB_YESNO) == IDYES)
					{
						int x = 1;
						while(1)
						{
							char s[123] = {0};
							wchar_t name[FILENAME_SIZE] = {0};
							StringCchPrintfA(s, 123, "RecentURL%d", x);
							_r_sW(s, name, 123);
							_w_sW(s, 0);
							if (!name[0]) break;
							x++;
						}

						// clear out but keep what's been currently entered so as not to annoy people...
						GetDlgItemTextW(hwndDlg, IDC_URL_NEW, loc, ARRAYSIZE(loc));
						SendDlgItemMessage(hwndDlg, IDC_URL_NEW, CB_RESETCONTENT, 0, 0);
						SetDlgItemTextW(hwndDlg, IDC_URL_NEW, loc);
					}
					return FALSE;
			}
			return FALSE;
		case WM_CLOSE:
			EndDialog(hwndDlg, 2);
			return FALSE;
	}
	return 0;
}

LRESULT getNewLocation(int clearlist, HWND hwnd) // if clearlist is 1, playlist is cleared
{
	static int q;
	if (q) return 0;
	q = 1;
	if (!LPDialogBoxParamW(IDD_OPENLOC, hwnd, (DLGPROC)locProc, !clearlist))
	{
		wchar_t *beg = loc;
		wchar_t *l = loc;
		wchar_t buf[FILENAME_SIZE] = {0};
		wchar_t *pEnd;

		while (IsCharSpaceW(*l)) l = CharNextW(l);

		if (!wcsstr(l, L":/") && !wcsstr(l, L":\\"))
		{
			StringCchPrintfW(buf, FILENAME_SIZE, L"http://%s", l);
			l = buf;
			beg = buf;
		}

		if (clearlist < 0)
		{
			int psize = WideCharToMultiByte(CP_ACP, 0, l, -1, 0, 0, NULL, NULL);
			char* p = (char*)GlobalAlloc(GPTR, psize);
			WideCharToMultiByte(CP_ACP, 0, l, -1, p, psize, NULL, NULL);

			//trim any trailing spaces
			//l = p + lstrlen(p) - 1;
			if (IsCharSpaceW(*l))
			{
				while (IsCharSpaceW(*l))
					l = CharPrevW(beg, l);
				*CharNextW(l) = 0;
			}

			q = 0;
			return (LRESULT)p;
		}

		pEnd = GetLastCharacterW(l);
		if (IsCharSpaceW(*pEnd))
		{
			while (IsCharSpaceW(*pEnd))
				pEnd = CharPrevW(l, pEnd);
			*CharNextW(pEnd) = 0;
		}

		/* benski> CUT because 'idir' doesn't seem to be used
		int idir=0;
		HANDLE h;
		WIN32_FIND_DATA d;
		h =	FindFirstFile(l,&d);
		if (h!=INVALID_HANDLE_VALUE)
		{
		FindClose(h);
		if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
		idir=1;
		}
		}
		*/
		if (clearlist)
			PlayList_delete();
		PlayList_appendthing(l, 0, 0);
		if (clearlist)
			BeginPlayback();
	}
	q = 0;
	return 0;
}