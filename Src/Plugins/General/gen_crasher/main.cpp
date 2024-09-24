// Winamp error feedback plugin
// Copyright (C) 2005 Nullsoft

//#define PLUGIN_DESC "Nullsoft Error Feedback"
#define PLUGIN_VER L"1.16"

#include ".\main.h"
#include "configDlg.h"
#include "crashDlg.h"
#include "api__gen_crasher.h"
#include "../nu/ServiceWatcher.h"

ServiceWatcher watcher;
Settings settings;
prefsDlgRecW prefItem = {0};
char *winampVersion;
static wchar_t prefsTitle[64];

// wasabi based services for localisation support
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
api_syscb *WASABI_API_SYSCB = 0;
api_application *WASABI_API_APP = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

int init(void);
void config(void);
void quit(void);

extern "C" winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER_U,
	"nullsoft(gen_crasher.dll)",
	init,
	config,
	quit,
};

extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() {	return &plugin; }

int init(void)
{
	if (!settings.IsOk()) return GEN_INIT_FAILURE;

	// loader so that we can get the localisation service api for use
	WASABI_API_SVC = (api_service*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (!WASABI_API_SVC || WASABI_API_SVC == (api_service *)1)
		return GEN_INIT_FAILURE;

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) WASABI_API_SYSCB = reinterpret_cast<api_syscb*>(sf->getInterface());

	watcher.WatchWith(WASABI_API_SVC);
	watcher.WatchFor(&WASABI_API_LNG, languageApiGUID);
	WASABI_API_SYSCB->syscb_registerCallback(&watcher);

	sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,GenCrasherLangGUID);

	static wchar_t szDescription[256];
	swprintf(szDescription, ARRAYSIZE(szDescription),
			 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_ERROR_FEEDBACK), PLUGIN_VER);
	plugin.description = (char*)szDescription;

	//register prefs screen
	prefItem.dlgID = IDD_CONFIG;
	prefItem.name = WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_FEEDBACK,prefsTitle,64);
	prefItem.proc = (void*) ConfigDlgProc;
	prefItem.hInst = WASABI_API_LNG_HINST;
	prefItem.where = -1;
	SendMessageA(plugin.hwndParent, WM_WA_IPC, (WPARAM) &prefItem, IPC_ADD_PREFS_DLGW);
	winampVersion = (char *)SendMessageA(plugin.hwndParent,WM_WA_IPC,0,IPC_GETVERSIONSTRING);
	return GEN_INIT_SUCCESS;
}

void config(void)
{
	SendMessage(plugin.hwndParent,WM_WA_IPC,(WPARAM)&prefItem,IPC_OPENPREFSTOPAGE);
}

void quit(void)
{
	watcher.StopWatching();
	watcher.Clear();

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) sf->releaseInterface(WASABI_API_LNG);
	WASABI_API_LNG=0;
}

int StartHandler(wchar_t* iniPath)
{	
	settings.SetPath(iniPath);
	if (!settings.Load()) 
	{
		if (!(settings.CreateDefault(iniPath) && settings.Save()))
		{
			//OutputDebugString(L"Feedback plugin - unable to read settings. Error feedback disabled.\r\n");
		}
	}
	SetUnhandledExceptionFilter(FeedBackFilter);
	//OutputDebugString(L"Error FeedBack started.\r\n");
	return 0;
}

PEXCEPTION_POINTERS gExceptionInfo;

LONG WINAPI FeedBackFilter( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
	if (alreadyProccessing) return EXCEPTION_CONTINUE_EXECUTION;
	
	alreadyProccessing = TRUE;
	gExceptionInfo = pExceptionInfo; 

	// show user dialog
	HWND hwnd;
	if (WASABI_API_LNG)
		hwnd = WASABI_API_CREATEDIALOGPARAMW(IDD_CRASHDLG, NULL, CrashDlgProc, (LPARAM)WASABI_API_ORIG_HINST);
	else 
	#ifdef _M_IX86
		hwnd = CreateDialogParam(plugin.hDllInstance, MAKEINTRESOURCE(IDD_CRASHDLG), NULL, CrashDlgProc, (LPARAM)plugin.hDllInstance);
	#endif
	#ifdef _M_X64
		hwnd = CreateDialogParam(plugin.hDllInstance, MAKEINTRESOURCE(IDD_CRASHDLG), NULL, (DLGPROC)CrashDlgProc, (LPARAM)plugin.hDllInstance);
	#endif

	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
	while (IsWindow(hwnd))
	{
		MSG msg;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		if (IsDialogMessage(hwnd, &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}