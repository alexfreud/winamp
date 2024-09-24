//#define PLUGIN_NAME //"Nullsoft Global Hotkeys"
#define PLUGIN_VERSION L"1.97"

// do this so we can get WM_APPCOMMAND support (needed in 1.6+)
// (have to targetting xp sp1+ so we get the specific play & pause messages
//  though it'll work fine with win2k+ as those messages will never appear)
//////#define _WIN32_WINNT 0x0501
#include "gen_hotkeys.h"
#include "ConfigDlg.h"
#include "HotKey.h"
#include "WACommands.h"
#include <api/service/waServiceFactory.h>

///////////////////////////////////////////////////////////
//  Globals
///////////////////////////////////////////////////////////

// The global plugin instance
winampGeneralPurposePlugin psPlugin =
{
	GPPHDR_VER_U,
	"nullsoft(gen_hotkeys.dll)",
	pluginInit,
	pluginConfig,
	hotkeysClear
};
// Winamp's window procdure
WNDPROC lpWndProcOld = NULL;
static int winampIsUnicode=false;
static int appcommand=false;
// hotkeys
HOTKEY *g_hotkeys = NULL;
DWORD g_dwHotkeys = 0;
// mutex to prevent two gen_hotkeys
HANDLE g_hMutex = NULL;
LPARAM uShellHook = NULL;

// wasabi based services for localisation support
api_service *WASABI_API_SVC = 0;
api_application *WASABI_API_APP = NULL;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static prefsDlgRecW g_prefsItem = {0};
static wchar_t g_titlestr[128];
wchar_t *g_iniFile = 0;

// don't forget to set DEFHKDS_NUM if you change this
HOTKEY_DATA g_defhkds[] = {
                              {MAKEWORD(VK_INSERT, HOTKEYF_ALT | HOTKEYF_CONTROL | HOTKEYF_EXT), -1, L"ghkdc play"},
                              {MAKEWORD(VK_HOME, HOTKEYF_ALT | HOTKEYF_CONTROL | HOTKEYF_EXT), -1, L"ghkdc pause"},
                              {MAKEWORD(VK_END, HOTKEYF_ALT | HOTKEYF_CONTROL | HOTKEYF_EXT), -1, L"ghkdc stop"},
                              {MAKEWORD(VK_PRIOR, HOTKEYF_ALT | HOTKEYF_CONTROL | HOTKEYF_EXT), -1, L"ghkdc prev"},
                              {MAKEWORD(VK_NEXT, HOTKEYF_ALT | HOTKEYF_CONTROL | HOTKEYF_EXT), -1, L"ghkdc next"},
                              {MAKEWORD(VK_UP, HOTKEYF_ALT | HOTKEYF_CONTROL | HOTKEYF_EXT), -1, L"ghkdc vup"},
                              {MAKEWORD(VK_DOWN, HOTKEYF_ALT | HOTKEYF_CONTROL | HOTKEYF_EXT), -1, L"ghkdc vdown"},
                              {MAKEWORD(VK_RIGHT, HOTKEYF_ALT | HOTKEYF_CONTROL | HOTKEYF_EXT), -1, L"ghkdc forward"},
                              {MAKEWORD(VK_LEFT, HOTKEYF_ALT | HOTKEYF_CONTROL | HOTKEYF_EXT), -1, L"ghkdc rewind"},
                              {MAKEWORD('J', HOTKEYF_ALT | HOTKEYF_CONTROL | HOTKEYF_EXT), -1, L"ghkdc jump"},
                              {MAKEWORD('L', HOTKEYF_ALT | HOTKEYF_CONTROL | HOTKEYF_EXT), -1, L"ghkdc file"},
                              //multimedia keyboard stuff
                              {2226, -1, L"ghkdc stop"},
                              {2227, -1, L"ghkdc play/pause"},
                              {2225, -1, L"ghkdc prev"},
                              {2224, -1, L"ghkdc next"},
                          };

#define TIMER_ID 0x8855

static int m_genhotkeys_add_ipc;

///////////////////////////////////////////////////////////
//  DLL entry function
///////////////////////////////////////////////////////////

#ifndef _DEBUG
BOOL WINAPI _DllMainCRTStartup(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hInst);
	return TRUE;
}
#endif

///////////////////////////////////////////////////////////
//  Plugin functions
///////////////////////////////////////////////////////////

int pluginInit()
{
	RegisterShellHookWindow(psPlugin.hwndParent);
	uShellHook = RegisterWindowMessage(TEXT("SHELLHOOK"));

	g_iniFile = (wchar_t*)SendMessage(psPlugin.hwndParent, WM_WA_IPC, 0, IPC_GETINIFILEW);

	// loader so that we can get the localisation service api for use
	WASABI_API_SVC = (api_service*)SendMessage(psPlugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (WASABI_API_SVC == (api_service*)1 || 
		NULL == WASABI_API_SVC) 
	{
		return 1;
	}

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(psPlugin.hDllInstance,GenHotkeysLangGUID);

	m_genhotkeys_add_ipc = (int)SendMessage(	psPlugin.hwndParent, WM_WA_IPC, (WPARAM) &"GenHotkeysAdd", IPC_REGISTER_WINAMP_IPCMESSAGE);

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription, ARRAYSIZE(szDescription),
					 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_GLOBAL_HOTKEYS), PLUGIN_VERSION);
	psPlugin.description = (char*)szDescription;

	appcommand = GetPrivateProfileIntW(L"gen_hotkeys", L"appcommand", 0, g_iniFile);

	// Save Winamp's window procedure
	winampIsUnicode = IsWindowUnicode(psPlugin.hwndParent);
	lpWndProcOld = (WNDPROC)(LONG_PTR)GetWindowLongPtr(psPlugin.hwndParent, GWLP_WNDPROC);
	if (winampIsUnicode)
		SetWindowLongPtrW(psPlugin.hwndParent, GWLP_WNDPROC, (LONGX86)(LONG_PTR)WndProc);
	else
		SetWindowLongPtrA(psPlugin.hwndParent, GWLP_WNDPROC, (LONGX86)(LONG_PTR)WndProc);

	InitCommands();

	SetTimer(psPlugin.hwndParent, TIMER_ID, 10, NULL); //call hotkeysInit() when all plugins are loaded

	//register prefs screen
	g_prefsItem.dlgID = IDD_CONFIG;
	g_prefsItem.name = WASABI_API_LNGSTRINGW_BUF(IDS_GHK_TITLE_STR,g_titlestr,128);
	g_prefsItem.proc = ConfigProc;
	g_prefsItem.hInst = WASABI_API_LNG_HINST;
	// delay the adding of this
	// for some reason when changing a lang pack it can cause the WM_DESTROY of ConfigProc
	// to be called which completely messes up the ghk list and so can wipe it :(
	SendMessage(psPlugin.hwndParent, WM_WA_IPC, (WPARAM) &g_prefsItem, IPC_ADD_PREFS_DLGW);

	return 0;
}

int hotkeysLoad(HOTKEY_DATA *hkds, DWORD num, int do_register, int verbose /*=1*/)
{
	unsigned int uFailed = 0;

	delete [] g_hotkeys;
	g_hotkeys = NULL;

	if (num)
	{
		g_hotkeys = new HOTKEY[num];
		memset(g_hotkeys, 0, num * sizeof(HOTKEY));
	}
	if (!g_hotkeys)
	{
		g_dwHotkeys = 0;
		return 1;
	}

	g_dwHotkeys = num;

	wchar_t moreStr[64] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_GHK_HOTKEY_REG_FAILED_MORE,moreStr,64);
	size_t bufLen = 256 + wcslen(moreStr) + 1;
	wchar_t* szFailed = (wchar_t*)malloc(bufLen*sizeof(wchar_t));
	WASABI_API_LNGSTRINGW_BUF(IDS_GHK_HOTKEY_REG_FAILED,szFailed,bufLen);
	DWORD dwLeft = 256 - lstrlenW(szFailed);
	DWORD dwFailedWithNoMsg = 0;

	for (DWORD i = 0; i < num; i++)
	{
		g_hotkeys[i].hkd = hkds[i];

		if (g_hotkeys[i].hkd.iCommand < 0)
			// action not loaded yet for this hotkey
			continue;

		if (do_register && RegisterHotkey(g_hotkeys + i))
		{
			if (verbose && dwLeft)
			{
				wchar_t szTemp[1024] = {0};
				bool unicode = 0;
				char* name = GetCommandName(g_hotkeys[i].hkd.iCommand, &unicode);
				StringCchPrintfW(szTemp, 1024, (unicode?L"\n\t%s":L"\n\t%S"), name);
				DWORD dwLen = lstrlenW(szTemp);
				if (dwLen < dwLeft)
				{
					StringCchCatW(szFailed, bufLen, szTemp);
					dwLeft -= dwLen;
				}
				else
					dwFailedWithNoMsg++;
			}

			g_hotkeys[i].failed = TRUE;
			uFailed++;
		}
	}

	if (verbose && uFailed)
	{
		if (dwFailedWithNoMsg)
			StringCchCatW(szFailed, bufLen, moreStr);
		HWND parent = (HWND)SendMessage(psPlugin.hwndParent, WM_WA_IPC, 0, IPC_GETDIALOGBOXPARENT);
			if (parent == 0 || parent == (HWND)1)
				parent=psPlugin.hwndParent;

		MessageBoxW(parent, szFailed, g_titlestr, MB_OK | MB_ICONSTOP | MB_TOPMOST);
	}
	free(szFailed);

	return uFailed;
}

static volatile LONG initted=0;
void hotkeysInit()
{
	if (initted)
		return;
	initted=1;

	int enabled = GetPrivateProfileIntW(L"gen_hotkeys", L"enabled", 0, g_iniFile);

	// base the mutex on the current winamp install
	// (makes it work better with mutliple winamp installs otherwise the older
	//  "Winamp - gen_hotkeys.dll ^&*#@" mutex prevents different hotkeys from
	//  being initialised between the different installs without going to prefs)
	char mutexStr[MAX_PATH] = {0}, ghkFilename[MAX_PATH] = {0};
	GetModuleFileName(psPlugin.hDllInstance, ghkFilename, MAX_PATH);
	StringCchPrintf(mutexStr, MAX_PATH, "Winamp - %s ^&*#@", ghkFilename);
	g_hMutex = CreateMutex(0, TRUE, mutexStr);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		enabled = 0;

	int i;

	for (i = 0; i < DEFHKDS_NUM; i++)
	{
		g_defhkds[i].iCommand = GetCommandIdx(g_defhkds[i].szCommand);
		g_defhkds[i].szCommand = 0;
	}

	int l = GetPrivateProfileIntW(L"gen_hotkeys", L"nbkeys", -1, g_iniFile);
	int ver = GetPrivateProfileIntW(L"gen_hotkeys", L"version", 1, g_iniFile);
	if (l != -1)
	{
		if (ver == 2)
		{
			HOTKEY_DATA *hkds = new HOTKEY_DATA[l]; // TODO: could alloca this
			if (hkds)
			{
				for (i = 0; i < l; i++)
				{
					wchar_t tmp[1024] = {0}, action[1024] = {0};
					StringCchPrintfW(tmp, 1024, L"action%d", i);
					GetPrivateProfileStringW(L"gen_hotkeys", tmp, L"", action, 1024, g_iniFile);
					StringCchPrintfW(tmp, 1024, L"hotkey%d", i);
					int hotkey = GetPrivateProfileIntW(L"gen_hotkeys", tmp, 0, g_iniFile);

					hkds[i].dwHotKey = hotkey;

					hkds[i].iCommand = GetCommandIdx(action);
					if (hkds[i].iCommand >= 0)
						hkds[i].szCommand = NULL;
					else
						hkds[i].szCommand = _wcsdup(action);
				}

				hotkeysLoad(hkds, l, enabled, 0);
				delete [] hkds;
			}
		}
		// legacy support
		else if (ver == 1)
		{
			HOTKEY_DATA *hkds =  new HOTKEY_DATA[l]; // TODO: could alloca this
			if (hkds) 
			{
				int nb = 0;
				for (i = 0;i < l;i++)
				{
					wchar_t tmp[512] = {0};
					StringCchPrintfW(tmp, 512, L"msg%d", i);
					unsigned int msg = GetPrivateProfileIntW(L"gen_hotkeys", tmp, 0, g_iniFile);
					if (msg)
					{
						StringCchPrintfW(tmp, 512, L"wparam%d", i);
						WPARAM wparam = GetPrivateProfileIntW(L"gen_hotkeys", tmp, 0, g_iniFile);
						StringCchPrintfW(tmp, 512, L"lparam%d", i);
						LPARAM lparam = GetPrivateProfileIntW(L"gen_hotkeys", tmp, 0, g_iniFile);
						for (int j = 0;WACommands[j].name;j++)
						{
							if (WACommands[j].uMsg == msg && WACommands[j].wParam == wparam && WACommands[j].lParam == lparam)
							{
								StringCchPrintfW(tmp, 512, L"hotkey%d", i);
								int hotkey = GetPrivateProfileIntW(L"gen_hotkeys", tmp, 0, g_iniFile);
								hkds[nb].dwHotKey = hotkey;
								hkds[nb].iCommand = j;
								hkds[nb].szCommand = NULL;
								nb++;
								break;
							}
						}
					}
				}
				hotkeysLoad(hkds, nb, enabled, 0);
				delete [] hkds;
			}
		}
	}
	else
	{
		// load defaults
		hotkeysLoad(g_defhkds, DEFHKDS_NUM, enabled, 0);
	}
}

void writePrivateProfileInt(wchar_t *section, int val)
{
	wchar_t s[32] = {0};
	StringCchPrintfW(s, 32, L"%d", val);
	WritePrivateProfileStringW(L"gen_hotkeys", section, s, g_iniFile);
}

void hotkeysSave(HOTKEY_DATA *hkds, DWORD num)
{
	int enabled = GetPrivateProfileIntW(L"gen_hotkeys", L"enabled", 0, g_iniFile);
	appcommand = GetPrivateProfileIntW(L"gen_hotkeys", L"appcommand", 0, g_iniFile);
	WritePrivateProfileStringW(L"gen_hotkeys", NULL, NULL, g_iniFile);
	writePrivateProfileInt(L"nbkeys", num);
	writePrivateProfileInt(L"version", 2);
	writePrivateProfileInt(L"enabled", enabled);
	writePrivateProfileInt(L"appcommand", appcommand);
	if (!hkds || !num)
	{
		return ;
	}
	for (size_t i = 0; i < num; i++)
	{
		wchar_t tmp[1024] = {0};
		StringCchPrintfW(tmp, 1024, L"action%d", i);
		if (hkds[i].iCommand >= 0)
			WritePrivateProfileStringW(L"gen_hotkeys", tmp, GetCommandId(hkds[i].iCommand), g_iniFile);
		else if (hkds[i].szCommand)
			WritePrivateProfileStringW(L"gen_hotkeys", tmp, hkds[i].szCommand, g_iniFile);
		StringCchPrintfW(tmp, 1024, L"hotkey%d", i);
		writePrivateProfileInt(tmp, hkds[i].dwHotKey);
	}
}

void pluginConfig()
{
	SendMessage(psPlugin.hwndParent, WM_WA_IPC, (WPARAM) &g_prefsItem, IPC_OPENPREFSTOPAGE);
}

void hotkeysClear()
{
	DeregisterShellHookWindow(psPlugin.hwndParent);

	if (!g_hotkeys)
		return;

	// Unregister all of the hot keys, and delete all of our unique hot key identifiers
	for (DWORD i = 0; i < g_dwHotkeys; i++)
	{
		UnregisterHotkey(g_hotkeys + i);
	}
	delete [] g_hotkeys;
	g_hotkeys = NULL;
}

///////////////////////////////////////////////////////////
//  Plugin export function
///////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() { return &psPlugin; }

///////////////////////////////////////////////////////////
//  DLL Windows message handling procedure
///////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == uShellHook && appcommand && wParam == HSHELL_APPCOMMAND)
	{
		// WM_APPCOMMAND info:: http://msdn2.microsoft.com/en-us/library/ms646275.aspx
		int cmd = GET_APPCOMMAND_LPARAM(lParam);
		switch (cmd)
		{
			case APPCOMMAND_MEDIA_PLAY_PAUSE:
			{
				int playing = (int)SendMessage(psPlugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING);
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM((playing ? WINAMP_BUTTON3 : WINAMP_BUTTON2),0), 0);
			}
				break;
			case APPCOMMAND_MEDIA_NEXTTRACK:
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON5,0), 0);
				return TRUE;
			case APPCOMMAND_MEDIA_PREVIOUSTRACK:
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON1,0), 0);
				return TRUE;
			case APPCOMMAND_MEDIA_STOP:
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON4,0), 0);
				return TRUE;
			case APPCOMMAND_VOLUME_DOWN:
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(WINAMP_VOLUMEDOWN,0), 0);
				return TRUE;
			case APPCOMMAND_VOLUME_UP:
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(WINAMP_VOLUMEUP,0), 0);
				return TRUE;
			/*case APPCOMMAND_VOLUME_MUTE:
				return TRUE;*/

			// info on play/pause (xp sp1+) commands:: http://msdn2.microsoft.com/en-us/library/bb417078.aspx
			case APPCOMMAND_MEDIA_PLAY:
			{
				int playing = (int)SendMessage(psPlugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING);
				 // play if not currently playing/are stopped
				if(!playing) SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2,0), 0);
				// if paused then start playing again
				else if(playing == 3) SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON3,0), 0);
				// otherwise do nothing if playing already (playing == 1)
			}
				return TRUE;
			case APPCOMMAND_MEDIA_PAUSE:
			{
				int playing = (int)SendMessage(psPlugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING);
				// if playing then we pause
				if(playing == 1) SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON3,0), 0);
				// otherwise do nothing if already stopped or paused (playing == 0 || playing == 3)
			}
				return TRUE;
			case APPCOMMAND_MEDIA_FAST_FORWARD:
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(WINAMP_FFWD5S,0), 0);
				return TRUE;
			case APPCOMMAND_MEDIA_REWIND:
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(WINAMP_REW5S,0), 0);
				return TRUE;
		}
	}

	switch (message)
	{
		case WM_TIMER:
			if (wParam == TIMER_ID)
			{
				KillTimer(hwnd, TIMER_ID);
				hotkeysInit();
				return 0;
			}
			break;

		case WM_HOTKEY:
			if (g_hotkeys)
			{
				for (unsigned int i = 0; i < g_dwHotkeys; i++)
				{
					if (g_hotkeys[i].atom == wParam)
					{
						DoCommand(g_hotkeys[i].hkd.iCommand);
						return 0;
					}
				}
			}
			break;

		case WM_WA_IPC:
			if (lParam == m_genhotkeys_add_ipc && m_genhotkeys_add_ipc > 65536)
			{
				int cmd = AddCommand((genHotkeysAddStruct *) wParam);
				if (cmd > 0)
				{
					for (unsigned int i = 0; i < g_dwHotkeys; i++)
					{
						if (g_hotkeys[i].hkd.iCommand < 0
							&& g_hotkeys[i].hkd.szCommand
							&& !lstrcmpiW(g_hotkeys[i].hkd.szCommand, GetCommandId(cmd)))
						{
							g_hotkeys[i].hkd.iCommand = cmd;
							free(g_hotkeys[i].hkd.szCommand);
							g_hotkeys[i].hkd.szCommand = NULL;
							int enabled = GetPrivateProfileIntW(L"gen_hotkeys", L"enabled", 0, g_iniFile);
							if (enabled) RegisterHotkey(&g_hotkeys[i]);
						}
					}
				}
				return cmd;
			}

			// handles the weird inc/dec current rating (since it's locked to 0-5
			// we can just wrap around and correct the real value passed on to the api)
			else if(lParam == IPC_SETRATING && (wParam == -1 || wParam == 6))
			{
				LRESULT curRating = SendMessage(hwnd, WM_WA_IPC,
												SendMessage(hwnd, WM_WA_IPC, 0, IPC_GETLISTPOS),
												IPC_GETRATING);
				if(wParam == 6)	// increment
				{
					wParam = curRating+1;
				}
				else // decrement
				{
					wParam = curRating-1;
				}
			}
			break;
	}

	// If we don't know how to handle this message, let WinAMP do it for us
	if (winampIsUnicode)
		return CallWindowProcW(lpWndProcOld, hwnd, message, wParam, lParam);
	else
		return CallWindowProcA(lpWndProcOld, hwnd, message, wParam, lParam);
}