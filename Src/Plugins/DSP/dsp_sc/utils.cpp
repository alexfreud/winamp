#include <windows.h>
#include <uxtheme.h>
#include "utils.h"
#include "api.h"

#include <time.h>
#include <string>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <fstream>
#include <sstream>
#include <shlwapi.h>

#include <winamp/wa_ipc.h>
#include <winamp/dsp.h>
#include "resource/resource.h"

extern winampDSPModule module;

// Config file
char IniName[MAX_PATH] = {0},
	 IniEncName[MAX_PATH] = {0},
	 IniDir[MAX_PATH] = {0},
	 PluginDir[MAX_PATH] = {0};
wchar_t IniDirW[MAX_PATH] = {0},
		PluginDirW[MAX_PATH] = {0},
		SharedDirW[MAX_PATH] = {0};
HANDLE NextTracks[NUM_OUTPUTS] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
HANDLE SaveEncoded[NUM_OUTPUTS] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};

static bool IsVista = false,
			checked = false;

bool IsVistaUp() {
	if (checked == false) {
		OSVERSIONINFO version = {0};
		version.dwOSVersionInfoSize = sizeof(version);
		if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));
		IsVista = (version.dwMajorVersion >= 6);
		checked = true;
	}
	return IsVista;
}

UINT ver = -1;
UINT GetWinampVersion(HWND winamp)
{
	if(ver == -1)
	{
		return (ver = SendMessage(winamp, WM_WA_IPC, 0, IPC_GETVERSION));
	}
	return ver;
}

char* LocalisedStringA(UINT uID, char *str, size_t maxlen) {
	if (WASABI_API_LNG) {
		if (!str) {
			return WASABI_API_LNGSTRING(uID);
		} else {
			return WASABI_API_LNGSTRING_BUF(uID, str, maxlen);
		}
	} else {
		__declspec(thread) static char *tmp;
		char* strtmp = 0;
		if (!str) {
			if (!tmp) tmp = (char *)malloc(1024*sizeof(char));
			strtmp = tmp;
			maxlen = 1024;
		} else {
			strtmp = str;
		}
		LoadStringA(module.hDllInstance, uID, strtmp, maxlen);
		return strtmp;
	}
}

wchar_t* LocalisedString(UINT uID, wchar_t *str, size_t maxlen) {
	if (WASABI_API_LNG) {
		if (!str) {
			return WASABI_API_LNGSTRINGW(uID);
		} else {
			return WASABI_API_LNGSTRINGW_BUF(uID, str, maxlen);
		}
	} else {
		__declspec(thread) static wchar_t *tmp;
		wchar_t* strtmp = 0;
		if (!str) {
			if (!tmp) tmp = (wchar_t *)malloc(1024*sizeof(wchar_t));
			strtmp = tmp;
			maxlen = 1024;
		} else {
			strtmp = str;
		}
		LoadStringW(module.hDllInstance, uID, strtmp, maxlen);
		return strtmp;
	}
}

HWND LocalisedCreateDialog(HINSTANCE instance, UINT dialog_id, HWND hWndParent, DLGPROC DlgProc, LPARAM user_id) {
	if (WASABI_API_LNG) {
		return WASABI_API_CREATEDIALOGPARAMW(dialog_id, hWndParent, DlgProc, user_id);
	} else {
		return CreateDialogParamW(instance, MAKEINTRESOURCEW(dialog_id), hWndParent, DlgProc, user_id);
	}
}

INT_PTR LocalisedDialogBox(HINSTANCE hDllInstance, UINT dialog_id, HWND hWndParent, DLGPROC lpDialogFunc) {
	if (WASABI_API_LNG) {
		return WASABI_API_DIALOGBOXW(dialog_id, hWndParent, lpDialogFunc);
	} else {
		return DialogBoxW(hDllInstance, MAKEINTRESOURCEW(dialog_id), hWndParent, lpDialogFunc);
	}
}

// about the most reliable way i can find to get the Winamp window as it could
// have been started with the /CLASS= parameter which then means it won't be
// 'Winamp v1.x' so instead go for a fixed child window which will always be
// there (and deals with other apps who create a 'fake' Winamp window (like AIMP)
// and there are two versions to cope with classic or modern skins being used.
HWND hwndWinamp = 0;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	char name[24];
	GetClassName(hwnd,name,24);
	// this check will only work for classic skins
	if (!strnicmp(name, "Winamp PE", 24)) {
		HWND child = GetWindow(GetWindow(hwnd, GW_CHILD), GW_CHILD);
		GetClassName(child, name, 24);
		// this check improves reliability of this check against players
		// like KMPlayer which also create a fake playlist editor window
		if (!strnicmp(name, "WinampVis", 24) || strnicmp(name, "TSkinPanel", 24)) {
			hwndWinamp = GetWindow(hwnd, GW_OWNER);
			return FALSE;
		}
	} else if (!strnicmp(name, "BaseWindow_RootWnd", 24)) {
		// this check will only work for modern skins
		HWND child = GetWindow(GetWindow(hwnd, GW_CHILD), GW_CHILD);
		GetClassName(child, name, 24);
		if (!strnicmp(name, "Winamp PE", 24)) {
			hwndWinamp = GetWindow(hwnd, GW_OWNER);
			return FALSE;
		}
	} else if (!strnicmp(name, "Winamp v1.x", 24)) {
		// this check will fail if /CLASS= was used on Winamp
		HWND child = GetWindow(hwnd, GW_CHILD);
		GetClassName(child, name, 24);
		if (!strnicmp(name, "WinampVis", 24)) {
			hwndWinamp = hwnd;
			return FALSE;
		}
	}
	return TRUE;
}

HWND GetWinampHWND(HWND winamp) {
	// if no HWND is passed then attemp to find it
	if (!IsWindow(winamp)) {
		// but only do the enumeration again if we have an invalid HWND cached
		if (!IsWindow(hwndWinamp)) {
			hwndWinamp = 0;
			EnumThreadWindows(GetCurrentThreadId(), EnumWindowsProc, 0);
		}
		return hwndWinamp;
	} else {
		return (hwndWinamp = winamp);
	}
}

HINSTANCE GetMyInstance() {
	MEMORY_BASIC_INFORMATION mbi = {0};
	if (VirtualQuery(GetMyInstance, &mbi, sizeof(mbi))) {
		return (HINSTANCE)mbi.AllocationBase;
	}
	return NULL;
}

char* GetIniDirectory(HWND winamp) {
	if (!IniDir[0]) {
		// this gets the string of the full ini file path
		strncpy(IniDir, (char*)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETINIDIRECTORY), MAX_PATH);
	}
	return IniDir;
}

wchar_t* GetIniDirectoryW(HWND winamp) {
	if (!IniDirW[0]) {
		// this gets the string of the full ini file path
		wcsncpy(IniDirW, (wchar_t*)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETINIDIRECTORYW), MAX_PATH);
	}
	return IniDirW;
}

char* GetPluginDirectory(HWND winamp) {
	// this gets the string of the full plug-in folder path
	strncpy(PluginDir, (char*)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORY), MAX_PATH);
	return PluginDir;
}

wchar_t* GetPluginDirectoryW(HWND winamp) {
	// this gets the string of the full plug-in folder path
	wcsncpy(PluginDirW, (wchar_t*)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORYW), MAX_PATH);
	return PluginDirW;
}

wchar_t* GetSharedDirectoryW(HWND winamp) {
	// this gets the string of the full shared dll folder path
	wchar_t* str = (wchar_t*)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETSHAREDDLLDIRECTORYW);
	if (str > (wchar_t*)65536) {
		wcsncpy(SharedDirW, str, MAX_PATH);
	} else {
		// and on older versions of Winamp we revert to the plug-ins folder path
		wcsncpy(SharedDirW, GetPluginDirectoryW(winamp), MAX_PATH);
	}
	return SharedDirW;
}

void GetDefaultNextTracksLogFile(HWND winamp, int bufferLen, wchar_t* buffer, int index) {
	snwprintf(buffer, bufferLen, L"%s\\Plugins\\dsp_sc_nexttracks_%d.log", GetIniDirectoryW(winamp), index+1);
}

char* GetSCIniFile(HWND winamp) {
	if (!IniName[0]) {

		// allows support for multiple instances of the dsp_sc.dll
		// without the settings being saved into the same section
		char dll_name[MAX_PATH] = {"dsp_sc"};
		if (GetModuleFileName(module.hDllInstance, dll_name, MAX_PATH)) {
			PathStripPath(dll_name);
			PathRemoveExtension(dll_name);
		}
		snprintf(IniName, MAX_PATH, "%s\\Plugins\\%s.ini", GetIniDirectory(winamp), dll_name);
	}
	return IniName;
}

wchar_t* GetSCLogFile(HWND winamp, int bufferLen, wchar_t* logFile, int index) {
	snwprintf(logFile, bufferLen, L"%s\\Plugins\\dsp_sc_%d.log", GetIniDirectoryW(winamp), index + 1);
	return logFile;
}

char* CreateLogFileMessage(char* buffer, wchar_t* message, int* len) {
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	char d[100], t[100], msg[1024];
	GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &sysTime, "yyyy'-'MM'-'dd", d, 99);
	GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &sysTime, "HH':'mm':'ss", t, 99);

	std::string utf8 = ConvertToUTF8(message);
	char* m = (char*)utf8.c_str();
	char* n = msg;
	while (m && *m) {
		if (m && *m && *m == '\n') {
			*n = ' ';
		} else if (m) {
			if (n) *n = *m;
		}
		m = CharNext(m);
		n = CharNext(n);
	}
	*n = 0;

	*len = snprintf(buffer, 1024, "%s %s\t%s\r\n", d, t, msg);
	return buffer;
}

void StartNextTracks(int index, wchar_t* file) {
	NextTracks[index] = CreateFileW(file, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
	if (NextTracks[index] != INVALID_HANDLE_VALUE) {
		// reset the file on loading things
		SetFilePointer(NextTracks[index], 0, NULL, FILE_BEGIN);
		SetEndOfFile(NextTracks[index]);
	}
}

void WriteNextTracks(int index, HWND winamp, std::vector<int> nextListIdx, std::vector<std::wstring> nextList, bool xml) {
	if (NextTracks[index] != INVALID_HANDLE_VALUE) {
		DWORD written;

		// reset the file so if there are no tracks then that'll be set
		SetFilePointer(NextTracks[index], 0, NULL, FILE_BEGIN);
		SetEndOfFile(NextTracks[index]);

		std::stringstream s;
		if (xml) {
			s << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<nexttracks>\n";
		}

		if (!nextList.empty()) {
			std::vector<std::wstring>::const_iterator i = nextList.begin();
			std::vector<int>::const_iterator idx = nextListIdx.begin();
			for (int count = 1; i != nextList.end(); ++i, ++idx, count++) {
				wchar_t *file=(wchar_t*)SendMessage(module.hwndParent, WM_WA_IPC, (*idx), IPC_GETPLAYLISTFILEW);
				if (xml) {
					std::string filepath = ConvertToUTF8Escaped(file);
					s << "\t<file seq=\"" << count << "\">" << filepath << "</file>\n\t";
					std::string next = ConvertToUTF8Escaped((*i).c_str());
					s << "<title seq=\"" << count << "\">" << next << "</title>\n";
				} else {
					std::string rawfilepath = ConvertToUTF8(file);
					WriteFile(NextTracks[index], rawfilepath.c_str(), rawfilepath.length(), &written, 0);
					WriteFile(NextTracks[index], "\r\n", 2, &written, 0);
				}
			}
		}

		if (xml) {
			s << "</nexttracks>\n";
			WriteFile(NextTracks[index], s.str().data(), s.str().length(), &written, 0);
		}
	}
}

void StopNextTracks(int index) {
	if (NextTracks[index] != INVALID_HANDLE_VALUE) {
		CloseHandle(NextTracks[index]);
		NextTracks[index] = INVALID_HANDLE_VALUE;
	}
}

void StartSaveEncoded(int index, wchar_t* file) {
	SaveEncoded[index] = CreateFileW(file, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
	if (SaveEncoded[index] != INVALID_HANDLE_VALUE) {
		// reset the file on loading things
		SetFilePointer(SaveEncoded[index], 0, NULL, FILE_BEGIN);
		SetEndOfFile(SaveEncoded[index]);
	}
}

void WriteSaveEncoded(int index, LPCVOID buffer, int bufferLen) {
	if (SaveEncoded[index] != INVALID_HANDLE_VALUE) {
		DWORD written;
		WriteFile(SaveEncoded[index], buffer, bufferLen, &written, 0);
	}
}

void StopSaveEncoded(int index) {
	if (SaveEncoded[index] != INVALID_HANDLE_VALUE) {
		CloseHandle(SaveEncoded[index]);
		SaveEncoded[index] = INVALID_HANDLE_VALUE;
	}
}

void StartLogging(int index, int clearOnStart) {
	wchar_t name[MAX_PATH];
	logFiles[index] = CreateFileW(GetSCLogFile(module.hwndParent, ARRAYSIZE(name), name, index), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
	if (logFiles[index] != INVALID_HANDLE_VALUE) {
		// clear the file when started
		if (clearOnStart) {
			SetFilePointer(logFiles[index], 0, NULL, FILE_BEGIN);
			SetEndOfFile(logFiles[index]);
		} else {
			SetFilePointer(logFiles[index], 0, NULL, FILE_END);
		}

		int len = 0;
		DWORD written;
		char buf[1024];
		CreateLogFileMessage(buf, L"Logging starting", &len);
		WriteFile(logFiles[index], buf, len, &written, 0);
	}
}

void StopLogging(int index) {
	if (logFiles[index] != INVALID_HANDLE_VALUE) {
		int len = 0;
		DWORD written;
		char buf[1024];
		CreateLogFileMessage(buf, L"Logging stopping\r\n", &len);
		WriteFile(logFiles[index], buf, len, &written, 0);
		CloseHandle(logFiles[index]);
	}
	logFiles[index] = INVALID_HANDLE_VALUE;
}

BOOL IsDirectMouseWheelMessage(const UINT uMsg) {
	static UINT WINAMP_WM_DIRECT_MOUSE_WHEEL = WM_NULL;

	if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL) {
		WINAMP_WM_DIRECT_MOUSE_WHEEL = RegisterWindowMessageW(L"WINAMP_WM_DIRECT_MOUSE_WHEEL");
		if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
			return FALSE;
	}

	return (WINAMP_WM_DIRECT_MOUSE_WHEEL == uMsg);
}

HWND ActiveChildWindowFromPoint(HWND hwnd, POINTS cursor_s, const int *controls, size_t controlsCount) {	
	POINT pt = {0};
	RECT controlRect = {0};
	POINTSTOPOINT(pt, cursor_s);

	while (controlsCount--) {
		HWND controlWindow = GetDlgItem(hwnd, controls[controlsCount]);
		if (NULL != controlWindow &&
			FALSE != GetClientRect(controlWindow, &controlRect)) {
			MapWindowPoints(controlWindow, HWND_DESKTOP, (POINT*)&controlRect, 2);
			if (FALSE != PtInRect(&controlRect, pt)) {	
				unsigned long windowStyle;
				windowStyle = (unsigned long)GetWindowLongPtrW(controlWindow, GWL_STYLE);
				if (WS_VISIBLE == ((WS_VISIBLE | WS_DISABLED) & windowStyle))
					return controlWindow;	
				break;
			}
		}
	}
	return NULL;
}

BOOL DirectMouseWheel_ProcessDialogMessage(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam) {
	if (FALSE != IsDirectMouseWheelMessage(uMsg)) {
		const int controls[] = {
			IDC_MUSSLIDER,
			IDC_MUS2SLIDER,
			IDC_MICSLIDER,
			IDC_FADESLIDER,
			IDC_MICFADESLIDER,
		};
		HWND targetWindow = ActiveChildWindowFromPoint(hwnd, MAKEPOINTS(lParam), controls, ARRAYSIZE(controls));
		if (NULL != targetWindow) {
			SendMessage(targetWindow, WM_MOUSEWHEEL, wParam, lParam);
			SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, (long)TRUE);
			return TRUE;
		}
	}
	return FALSE;
}


static HCURSOR link_hand_cursor;
LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"link_proc"), hwndDlg, uMsg, wParam, lParam);
	// override the normal cursor behaviour so we have a hand to show it is a link
	if (uMsg == WM_SETCURSOR) {
		if ((HWND)wParam == hwndDlg) {
			if (!link_hand_cursor) {
				link_hand_cursor = LoadCursor(NULL, IDC_HAND);
			}
			SetCursor(link_hand_cursor);
			return TRUE;
		}
	}
	return ret;
}

void link_startsubclass(HWND hwndDlg, UINT id) {
	HWND ctrl = GetDlgItem(hwndDlg, id);
	if (!GetPropW(ctrl, L"link_proc")) {
		SetPropW(ctrl, L"link_proc", (HANDLE)SetWindowLongPtrW(ctrl, GWLP_WNDPROC, (LONG_PTR)link_handlecursor));
	}
}

BOOL link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_DRAWITEM) {
		DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
		if (di->CtlType == ODT_BUTTON) {
			wchar_t wt[123];
			int y;
			RECT r;

			// due to the fun of theming and owner drawing we have to get the background colour
			if (isthemethere){
				HTHEME hTheme = OpenThemeData(hwndDlg, L"Tab");
				if (hTheme) {
					DrawThemeParentBackground(di->hwndItem, di->hDC, &di->rcItem);
					CloseThemeData(hTheme);
				}
			}

			HPEN hPen, hOldPen;
			GetDlgItemTextW(hwndDlg, wParam, wt, ARRAYSIZE(wt));

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
			return TRUE;
		}
	}
	return FALSE;
}

#include <map>
class xmlEscapes: public std::map<char,std::string>
{
public:
	xmlEscapes() {  		
		(*this)['<'] = "&lt;";
		(*this)['>'] = "&gt;";
		(*this)['&'] = "&amp;";
		(*this)['\''] = "&apos;";
		(*this)['"'] = "&quot;";
	}
};

static const xmlEscapes gsXmlEscapes;

// this will only be receiving an already converted
// string so no need to do the commented part again
char* escapeXML(const char* s) {
	static char result[2048] = {0};
	memset(&result, 0, 2048);
	int len = strlen(s);
	for (int x = 0, y = 0; x < len; x++)
	{
		xmlEscapes::const_iterator i = gsXmlEscapes.find(s[x]);
		if (i != gsXmlEscapes.end()) {
			strcat(&result[y-1], (*i).second.c_str());
			y += (*i).second.size();
		} else if (s[x] >= 0 && s[x] <= 31 && s[x] != 9 && s[x] != 10 && s[x] != 13) {
			// strip out characters which aren't supported by the DNAS
			// (only allow backspace, linefeed and carriage return)
			#ifdef DEBUG
			result[y] = '\xEF';
			y++;
			result[y] = '\xBF';
			y++;
			result[y] = '\xBD';
			y++;
			#endif
		} else if ((x < len - 2) && s[x] == '\xEF' && s[x+1] == '\xBF' && s[x+2] == '\xBF') {
			// and any UTF-8 boms which are in there (seen it happen!)
			x+=2;
			#ifdef DEBUG
			result[y] = '\xEF';
			y++;
			result[y] = '\xBF';
			y++;
			result[y] = '\xBD';
			y++;
			#endif
		} else {
			result[y] = s[x];
			y++;
		}
	}
	return result;
}

char* ConvertToUTF8Escaped(const wchar_t *str) {
	static char utf8tmp[1024] = {0};
	memset(&utf8tmp, 0, sizeof(utf8tmp));
	WideCharToMultiByte(CP_UTF8, 0, str, -1, utf8tmp, sizeof(utf8tmp), 0, 0);
	return escapeXML(utf8tmp);
}

char* ConvertToUTF8(const wchar_t *str) {
	static char utf8tmp2[1024] = {0};
	memset(&utf8tmp2, 0, sizeof(utf8tmp2));
	WideCharToMultiByte(CP_UTF8, 0, str, -1, utf8tmp2, sizeof(utf8tmp2), 0, 0);
	return utf8tmp2;
}

int ConvertFromUTF8(const char *src, wchar_t *dest, int destlen) {
	if (destlen == 0)
		return MultiByteToWideChar(CP_UTF8, 0, src, -1, dest, destlen);
	int converted = MultiByteToWideChar(CP_UTF8, 0, src, -1, dest, destlen-1);
	if (!converted)
		return 0;
	dest[converted]=0;
	return converted+1;
}

DWORD GetPrivateProfileStringUTF8(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName) {
	char tmp[MAX_PATH] = {0};
	GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, tmp, nSize, lpFileName);
	return ConvertFromUTF8(tmp, lpReturnedString, nSize);
}


void ShowWindowDlgItem(HWND hDlg, int nIDDlgItem, int nCmdShow) {
	ShowWindow(GetDlgItem(hDlg, nIDDlgItem), nCmdShow);
}

void EnableWindowDlgItem(HWND hDlg, int nIDDlgItem, BOOL bEnable) {
	EnableWindow(GetDlgItem(hDlg, nIDDlgItem), bEnable);
}

template<typename S,typename F>
std::vector<S> tokenizer_if(const S &ins,F isdelimiter) throw()
{
	std::vector<S> result;
	S accum;

	for(typename S::const_iterator i = ins.begin(); i != ins.end(); ++i)
	{
		if (!isdelimiter(*i))
		{
			accum.push_back(*i);// was +=
		}
		else
		{
			if (!accum.empty())
			{
				result.push_back(accum);
				accum = S();
			}
		}
	}

	if (!accum.empty())
		result.push_back(accum);
	return result;
}

template<typename S>
inline std::vector<S> tokenizer(const S &ins,typename S::value_type delim) throw()
	{ return tokenizer_if(ins,bind1st(std::equal_to<typename S::value_type>(),delim)); }

extern char sourceVersion[64];
bool CompareVersions(char *verStr)
{
	bool needsUpdating = false;
	if (verStr && *verStr)
	{
		std::vector<std::string> newVerStr = tokenizer(std::string(verStr), '.');
		std::vector<std::string> curVerStr = tokenizer(std::string(sourceVersion), '.');
		int newVer[] = {::atoi(newVerStr[0].c_str()), ::atoi(newVerStr[1].c_str()), ::atoi(newVerStr[2].c_str()), ::atoi(newVerStr[3].c_str())},
			curVer[] = {::atoi(curVerStr[0].c_str()), ::atoi(curVerStr[1].c_str()), ::atoi(curVerStr[2].c_str()), ::atoi(curVerStr[3].c_str())};

		// look to compare from major to minor parts of the version strings
		// 2.x.x.x vs 3.x.x.x
		if (newVer[0] > curVer[0]) {
			needsUpdating = true;
		}
		// 2.0.x.x vs 2.2.x.x
		else if((newVer[0] == curVer[0]) && (newVer[1] > curVer[1])) {
			needsUpdating = true;
		}
		// 2.0.0.x vs 2.0.1.x
		else if((newVer[0] == curVer[0]) && (newVer[1] == curVer[1]) && (newVer[2] > curVer[2])) {
			needsUpdating = true;
		}
		// 2.0.0.29 vs 2.0.0.30
		else if((newVer[0] == curVer[0]) && (newVer[1] == curVer[1]) && (newVer[2] == curVer[2]) && (newVer[3] > curVer[3])) {
			needsUpdating = true;
		}
	}
	return needsUpdating;
}