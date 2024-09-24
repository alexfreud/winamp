#include "Main.h"
#include "ReplayGain.h"
#include "../nu/AutoChar.h"
#include "../nu/MediaLibraryInterface.h"
#include "./resource.h"
#include "./settings.h"
#include "./copyfiles.h"
#include "../winamp/wa_ipc.h"
//#include <primosdk.h>
#include <shlwapi.h>
#include <imapi.h>
#include <imapierror.h>
#include "../nu/ns_wc.h"
#include <vector>
#include <strsafe.h>

#define VERSION_MAJOR			2
#define VERSION_MINOR			0

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(*x))
#endif

typedef struct _LISTENER
{
	HWND hwnd;
	UINT uMsg;
	CHAR cLetter;
} LISTENER;

typedef struct _NAVITEMENUMPARAM
{
	NAVITEMENUMPROC	callback;
	LPARAM			param;
} NAVITEMENUMPARAM;

typedef enum _BTNSTATE
{
	BTNSTATE_NORMAL	= 0,
	BTNSTATE_HILITED = 1,
	BTNSTATE_PRESSED = 2,
	BTNSTATE_DISABLED = 3,
} BTNSTATE;

#define ICON_SIZE_CX	14
#define ICON_SIZE_CY	14

#define NAVBUTTON_STATECHECK_DELAY	100
static int Init();
static void Quit();

HNAVITEM hniMain = NULL;
static LRESULT delay_ml_startup;
static HMLIMGLST hmlilIcons = NULL;
LARGE_INTEGER freq;

#define NAVITEM_PREFIX			L"_ml_disc_"
#define NAVITEM_PREFIX_SIZE		(sizeof(NAVITEM_PREFIX)/sizeof(wchar_t))

#define NCS_EX_SHOWEJECT		0x0100

C_Config *g_config, *g_view_metaconf = NULL;
HMENU g_context_menus;

prefsDlgRecW myPrefsItemCD = {0};
INT_PTR imgIndex = 0;
wchar_t randb[64] = {0};
static wchar_t cdrip[64];

static DWORD g_navStyle = NCS_FULLROWSELECT | NCS_SHOWICONS;
static DWORD riphash = 0;

static std::vector<LPARAM> driveList;

static LISTENER activeListener = { NULL, 0, };
static WNDPROC oldWinampWndProc = NULL;

api_application *WASABI_API_APP = 0;
api_stats *AGAVE_API_STATS = 0;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static UINT uMsgBurnerNotify = 0;
static UINT uMsgRipperNotify = 0;
static UINT uMsgNavStyleUpdate = 0;
static UINT uMsgCopyNotify = 0;

static void CALLBACK Invoke_OnDriveManagerNotify(WORD wCode, INT_PTR param);
static void Plugin_OnMLVisible(BOOL bVisible);
void ShowHideRipBurnParent(void);



static void DriveParam_RegisterDrive(DRIVE *drive)
{
	LPARAM param;
	size_t index;

	param = (LPARAM)drive;
	if (NULL == param)
		return;

	index = driveList.size();
	while(index--)
	{
		if(param == driveList[index])
			return;
	}

	driveList.push_back(param);
}

static void DriveParam_UnregisterDrive(DRIVE *drive)
{
	LPARAM param;
	size_t index;

	param = (LPARAM)drive;
	if (NULL == param)
		return;

	index = driveList.size();
	while(index--)
	{
		if(param == driveList[index])
		{
			driveList.erase(driveList.begin() + index);
			return;
		}
	}
}

static BOOL DriveParam_IsValid(LPARAM param)
{
	if (param > 0x0000FFFF)
	{
		size_t index = driveList.size();
		while(index--)
		{
			if(param == driveList[index])
				return TRUE;
		}
	}
	return FALSE;
}

DRIVE *Plugin_GetDriveFromNavItem(HNAVITEM hItem)
{
	NAVITEM item;
	
	if (!hItem) return NULL;

	item.cbSize = sizeof(NAVITEM);
	item.mask = NIMF_PARAM;
	item.hItem = hItem;
	
	return (MLNavItem_GetInfo(plugin.hwndLibraryParent, &item) && DriveParam_IsValid(item.lParam)) ? 
			(DRIVE*)item.lParam : NULL;
}

HNAVITEM Plugin_GetNavItemFromLetter(CHAR cLetter)
{
	NAVCTRLFINDPARAMS fp = {0};
	wchar_t invariant[32] = {0};

	if (S_OK == StringCchPrintfW(invariant,  sizeof(invariant)/sizeof(wchar_t), L"%s%c", NAVITEM_PREFIX, cLetter))
	{
		fp.cchLength = -1;
		fp.pszName = invariant;
		fp.compFlags = NICF_INVARIANT;

		return MLNavCtrl_FindItemByName(plugin.hwndLibraryParent, &fp);
	}
	return NULL;
}

static BOOL CALLBACK EnumerateNavItemsCB(HNAVITEM hItem, LPARAM param)
{
	DRIVE *pDrive = Plugin_GetDriveFromNavItem(hItem);
	return (pDrive) ? ((NAVITEMENUMPARAM*)param)->callback(hItem, pDrive, ((NAVITEMENUMPARAM*)param)->param) : TRUE;
}

BOOL Plugin_EnumerateNavItems(NAVITEMENUMPROC callback, LPARAM param)
{
	NAVITEMENUMPARAM pluginenum;
	NAVCTRLENUMPARAMS navenum;
	if (!callback) return FALSE;

	pluginenum.callback	= callback;
	pluginenum.param = param;

	navenum.hItemStart = hniMain;
	navenum.lParam = (LPARAM)&pluginenum;
	navenum.enumProc = EnumerateNavItemsCB;

	return MLNavCtrl_EnumItems(plugin.hwndLibraryParent, &navenum);
}

void Plugin_RegisterListener(HWND hwnd, UINT uMsg, CHAR cLetter)
{
	activeListener.hwnd = hwnd;
	activeListener.uMsg = uMsg;
	activeListener.cLetter = cLetter;
}

void Plugin_UnregisterListener(HWND hwnd)
{
	ZeroMemory(&activeListener, sizeof(LISTENER));
}

static BOOL CALLBACK EnumNavItems_OnUIChangeCB(HNAVITEM hItem, DRIVE *pDrive, LPARAM param)
{
	if (pDrive) pDrive->textSize = 0;
	return TRUE;
}

static void UpdatedNavStyles(void)
{
	g_navStyle = MLNavCtrl_GetStyle(plugin.hwndLibraryParent);
	if (0 != g_view_metaconf->ReadInt(TEXT("showeject"), 1)) g_navStyle |= NCS_EX_SHOWEJECT;
}

static BOOL CALLBACK EnumerateNavItemsRemoveCB(HNAVITEM hItem, DRIVE *pDrive, LPARAM param)
{
	if(pDrive)
	{
		MLNavCtrl_DeleteItem(plugin.hwndLibraryParent,hItem);
		Plugin_EnumerateNavItems(EnumerateNavItemsRemoveCB, 0);
	}
	return TRUE;
}

static BOOL Plugin_QueryOkToQuit()
{
	CHAR szLetters[24] = {0};
	INT c = DriveManager_GetDriveList(szLetters, ARRAYSIZE(szLetters));
	while(c-- > 0)
	{
		INT msgId;
		if (cdrip_isextracting(szLetters[c])) msgId = IDS_YOU_ARE_CURRENTLY_RIPPING_AUDIO_CD_MUST_CANCEL_TO_CLOSE_WINAMP;
		else if (MLDisc_IsDiscCopying(szLetters[c]))  msgId = IDS_YOU_ARE_CURRENTLY_COPYING_DATA_CD_MUST_CANCEL_TO_CLOSE_WINAMP;
		else msgId = 0;
		if (msgId)
		{
			wchar_t buffer[512] = {0};
			StringCchPrintfW(buffer, 512, WASABI_API_LNGSTRINGW(msgId), szLetters[c]);
			MessageBoxW(plugin.hwndWinampParent, buffer, WASABI_API_LNGSTRINGW(IDS_NOTIFICATION),
					MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST);
			return FALSE;
		}
	}
	return TRUE;
}

LRESULT CALLBACK WinampWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsgNavStyleUpdate == uMsg)
	{
		if(!wParam)
		{
			UpdatedNavStyles();
			Plugin_EnumerateNavItems(EnumNavItems_OnUIChangeCB, 0);
		}
		else
		{
			Plugin_EnumerateNavItems(EnumerateNavItemsRemoveCB, 0);
			ShowHideRipBurnParent();
			DriveManager_Uninitialize(0);
			DriveManager_Initialize(Invoke_OnDriveManagerNotify, TRUE);
			Plugin_OnMLVisible((BOOL)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_IS_VISIBLE, 0));
		}
	}
	else if (uMsgBurnerNotify == uMsg) // burner broadcast message LOWORD(wParam) = drive letter, lParam = (BOOL)bStarted. if bStarted = TRUE - burning started, otherwise burning finished
	{
		if (0 == HIWORD(wParam))
		{
			DriveManager_SetDriveMode((CHAR)LOWORD(wParam), (0 != lParam) ? DM_MODE_BURNING : DM_MODE_READY);
		}
	}
	else if (uMsgRipperNotify == uMsg)
	{
		if (HIWORD(wParam)) // another instance of winamp quering
		{
			if (LOWORD(wParam) && cdrip_isextracting((CHAR)LOWORD(wParam))) SendNotifyMessage((HWND)lParam, uMsgRipperNotify, LOWORD(wParam), (LPARAM)TRUE);
			else
			{
				CHAR cLetter;
				cLetter = (CHAR)cdrip_isextracting(-1);
				if (cLetter) SendNotifyMessage((HWND)lParam, uMsgRipperNotify, cLetter, (LPARAM)TRUE);
			}
		}	
		else 
		{
			DriveManager_SetDriveMode((CHAR)LOWORD(wParam), (0 != lParam) ? DM_MODE_RIPPING : DM_MODE_READY);
		}
	}
	else if (uMsgCopyNotify == uMsg)
	{
		if (HIWORD(wParam))
		{
			if (LOWORD(wParam) && MLDisc_IsDiscCopying((CHAR)LOWORD(wParam))) SendNotifyMessage((HWND)lParam, uMsgCopyNotify, LOWORD(wParam), (LPARAM)TRUE);
			else
			{
				CHAR szLetters[24] = {0};
				INT c = DriveManager_GetDriveList(szLetters, ARRAYSIZE(szLetters));
				while(c-- > 0)
				{
					if (MLDisc_IsDiscCopying(szLetters[c])) 
						SendNotifyMessage((HWND)lParam, uMsgCopyNotify, szLetters[c], (LPARAM)TRUE);
				}
			}
		}
		else 
		{
			DriveManager_SetDriveMode((CHAR)LOWORD(wParam), (0 != lParam) ? DM_MODE_COPYING : DM_MODE_READY);
		}
	}
	else if (WM_WA_IPC == uMsg)
	{
		switch(lParam)
		{
			case IPC_SKIN_CHANGED:
			case IPC_CB_RESETFONT:
				UpdatedNavStyles();
				Plugin_EnumerateNavItems(EnumNavItems_OnUIChangeCB, 0);
				break;
			case IPC_FILE_TAG_MAY_HAVE_UPDATED:
			case IPC_FILE_TAG_MAY_HAVE_UPDATEDW:
				if (activeListener.hwnd) SendMessageW(activeListener.hwnd, activeListener.uMsg, (WPARAM)lParam, (LPARAM)wParam);
				break;
		}
		if(lParam == delay_ml_startup)
		{
			if(!wParam)
			{
				PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 1, delay_ml_startup);
			}
			else if(wParam == 1)
			{
				// TODO: benski> temp-hack fix for now -- if (SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_IS_VISIBLE, 0))
				{
					DriveManager_Initialize(Invoke_OnDriveManagerNotify, TRUE);
					MLDisc_InitializeCopyData();

					DriveManager_Resume(TRUE);
					SendNotifyMessage(HWND_BROADCAST, uMsgBurnerNotify, MAKEWPARAM(0, 0xffff), (LPARAM)plugin.hwndWinampParent);
					SendNotifyMessage(HWND_BROADCAST, uMsgRipperNotify, MAKEWPARAM(0, 0xffff), (LPARAM)plugin.hwndWinampParent);
					SendNotifyMessage(HWND_BROADCAST, uMsgCopyNotify, MAKEWPARAM(0, 0xffff), (LPARAM)plugin.hwndWinampParent);
				}
			}
		}
	}
	return (oldWinampWndProc) ? CallWindowProcW(oldWinampWndProc, hwnd, uMsg, wParam, lParam) : DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static DM_UNITINFO2_PARAM unitinfo;
static char szDesc[1024];
static DWORD szTypes[64];

static void CALLBACK FreeParam(DM_NOTIFY_PARAM *phdr)
{
	if (!phdr)  return;
	switch(phdr->opCode)
	{
		case DMOP_TITLE:
			if (((DM_TITLE_PARAM*)phdr)->pszTitle) free(((DM_TITLE_PARAM*)phdr)->pszTitle);
			break;
	}
	free(phdr);
}
static void RegisterIcons()
{
	MLIMAGELISTCREATE mlilCreate;
	MLIMAGESOURCE mlis;
	MLIMAGELISTITEM mlilItem;

	if (hmlilIcons) return;

	mlilCreate.cx = ICON_SIZE_CX;
	mlilCreate.cy = ICON_SIZE_CY;
	mlilCreate.cInitial = 5;
	mlilCreate.cGrow = 1;
	mlilCreate.cCacheSize = 3;
	mlilCreate.flags = MLILC_COLOR24;

	hmlilIcons = MLImageList_Create(plugin.hwndLibraryParent, &mlilCreate);
	if (!hmlilIcons) return;
	
	ZeroMemory(&mlilItem, sizeof(MLIMAGELISTITEM));
	mlilItem.cbSize	= sizeof(MLIMAGELISTITEM);
	mlilItem.hmlil = hmlilIcons;
	mlilItem.filterUID = MLIF_FILTER1_UID;
	mlilItem.pmlImgSource = &mlis;

	ZeroMemory(&mlis, sizeof(MLIMAGESOURCE));
	mlis.cbSize		= sizeof(MLIMAGESOURCE);
	mlis.type		= SRC_TYPE_PNG;
	mlis.hInst		= plugin.hDllInstance;

	mlis.lpszName	= MAKEINTRESOURCEW(IDB_NAVITEM_CDROM);
    MLImageList_Add(plugin.hwndLibraryParent, &mlilItem);

	mlis.lpszName	= MAKEINTRESOURCEW(IDB_EJECT_NORMAL);
    MLImageList_Add(plugin.hwndLibraryParent, &mlilItem);

	mlis.lpszName	= MAKEINTRESOURCEW(IDB_EJECT_HILITED);
    MLImageList_Add(plugin.hwndLibraryParent, &mlilItem);
	
	mlis.lpszName	= MAKEINTRESOURCEW(IDB_EJECT_PRESSED);
    MLImageList_Add(plugin.hwndLibraryParent, &mlilItem);
	
	mlis.lpszName	= MAKEINTRESOURCEW(IDB_EJECT_DISABLED);
    MLImageList_Add(plugin.hwndLibraryParent, &mlilItem);

}


static BOOL UpdateTitle(CHAR cLetter, LPCWSTR pszTitle)
{
	HNAVITEM hItem;
	NAVITEM item;
	DRIVE *pDrive;
	
	hItem = Plugin_GetNavItemFromLetter(cLetter);
	if (!hItem) return FALSE;
		
	pDrive = Plugin_GetDriveFromNavItem(hItem);
	if (!pDrive) return FALSE;
	
	if (S_OK != StringCchCopyW(pDrive->szTitle, sizeof(pDrive->szTitle)/sizeof(wchar_t), (pszTitle) ? pszTitle : L"")) return FALSE;
	
	pDrive->textSize = 0;
	
	item.cbSize	= sizeof(NAVITEM);
	item.mask	= NIMF_TEXT;
	item.hItem	= hItem;
	item.pszText = pDrive->szTitle;
		
	return MLNavItem_SetInfo(plugin.hwndLibraryParent,  &item);
	
}
static void QueryTitle(CHAR cLetter)
{
	DM_TITLE_PARAM *pdtp;
	
	pdtp = (DM_TITLE_PARAM*)calloc(4, sizeof(DM_TITLE_PARAM));
	if (pdtp)
	{
		pdtp->header.callback = (INT_PTR)Invoke_OnDriveManagerNotify;
		pdtp->header.cLetter = cLetter;
		pdtp->header.uMsg = NULL;
		pdtp->header.fnFree = FreeParam;
		pdtp->cchTitle = 256;
		pdtp->pszTitle = (wchar_t*)calloc(pdtp->cchTitle, sizeof(wchar_t));

		DriveManager_QueryTitle(pdtp);
	}
}
static void Drive_OnAdded(CHAR cLetter)
{
	wchar_t szInvariant[32] = {0};
	
	DRIVE	*pDrive;
	NAVINSERTSTRUCT nis = {0};
	wchar_t szDriveType[32] = {0}, szDriveCap[64] = {0};

	pDrive = (DRIVE*)calloc(1, sizeof(DRIVE));
	if (!pDrive) return;

	StringCchPrintfW(szInvariant, sizeof(szInvariant)/sizeof(wchar_t), L"%s%c", NAVITEM_PREFIX, cLetter);

	pDrive->cLetter = cLetter;
	pDrive->cMode = DriveManager_GetDriveMode(cLetter);

	WASABI_API_LNGSTRINGW_BUF(IDS_CD, szDriveType, sizeof(szDriveType)/sizeof(wchar_t));
	WASABI_API_LNGSTRINGW_BUF(IDS_DRIVE_CAP, szDriveCap, sizeof(szDriveCap)/sizeof(wchar_t));
	StringCchPrintfW(pDrive->szTitle, sizeof(pDrive->szTitle)/sizeof(wchar_t),  L"%s %s (%C:)", szDriveType, szDriveCap, (WCHAR)cLetter);
				
	if (NULL == hmlilIcons) RegisterIcons();
	
	ZeroMemory(&nis, sizeof(NAVINSERTSTRUCT));
	nis.hParent		= hniMain;
	nis.item.cbSize = sizeof(NAVITEM);
	nis.item.pszText		= pDrive->szTitle;
    nis.item.pszInvariant	= szInvariant;
	nis.item.mask			= NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_PARAM | NIMF_STYLE;
	nis.item.style			= NIS_CUSTOMDRAW | NIS_WANTSETCURSOR | NIS_WANTHITTEST;
	nis.item.styleMask		= nis.item.style;
	nis.item.lParam			= (LPARAM)pDrive;
	
	if (NULL != MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis))
	{
		DriveParam_RegisterDrive(pDrive);
	}
}

static void Drive_OnChanged(CHAR cLetter)
{
	QueryTitle(cLetter);
}

static void Drive_OnRemoved(CHAR cLetter)
{
	HNAVITEM hItem;
	hItem = Plugin_GetNavItemFromLetter(cLetter);
	if (hItem) MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, hItem);
	if (riphash && (0xFF & (riphash >> 24)) == (UCHAR)cLetter) riphash = 0;
}

static void Drive_OnModeChanged(CHAR cLetter, CHAR cMode)
{
	HNAVITEM hItem;
	DRIVE *pDrive;

	hItem = Plugin_GetNavItemFromLetter(cLetter);
	if (!hItem) return;
		
	pDrive = Plugin_GetDriveFromNavItem(hItem);
	if (pDrive) 
	{
		NAVITEMINAVLIDATE inv;

		pDrive->cMode = cMode;

		inv.fErase = FALSE;
		inv.hItem = hItem;
		inv.prc = NULL;
		MLNavItem_Invalidate(plugin.hwndLibraryParent,  &inv);
	}
}

static void Medium_OnAdded(CHAR cLetter)
{
	if (riphash && (0xFF & (riphash >> 24)) == (UCHAR)cLetter) riphash = 0;
	QueryTitle(cLetter);
}

static void Medium_OnRemoved(CHAR cLetter)
{
	if (riphash && (0xFF & (riphash >> 24)) == (UCHAR)cLetter) riphash = 0;
	QueryTitle(cLetter);
}

static void OnDriveMangerOpCompleted(DM_NOTIFY_PARAM *phdr)
{
	switch(phdr->opCode)
	{
		case DMOP_TITLE:
			if (S_OK == phdr->result) UpdateTitle(phdr->cLetter, ((DM_TITLE_PARAM*)phdr)->pszTitle);
			break;
	}
}

static void CALLBACK OnDriveManagerNotify(ULONG_PTR param)
{
	WORD code;
	CHAR cLetter;
	
	code = LOWORD(param);
	cLetter = (CHAR)(0xFF & HIWORD(param));
	
	switch(code)
	{
		case DMW_DRIVEADDED:		Drive_OnAdded(cLetter); break;
		case DMW_DRIVEREMOVED:	Drive_OnRemoved(cLetter); break;
		case DMW_DRIVECHANGED:	Drive_OnChanged(cLetter); break;
		case DMW_MEDIUMARRIVED:	Medium_OnAdded(cLetter); break;
		case DMW_MEDIUMREMOVED:	Medium_OnRemoved(cLetter); break;
		case DMW_MODECHANGED:	Drive_OnModeChanged(cLetter, ((CHAR)(HIWORD(param)>>8))); break;
		
	}
	if (activeListener.hwnd && (0 == activeListener.cLetter || cLetter == activeListener.cLetter)) 
		PostMessageW(activeListener.hwnd, activeListener.uMsg, (WPARAM)code, (LPARAM)HIWORD(param));
}

static void CALLBACK Invoke_OnDriveManagerNotify(WORD wCode, INT_PTR param)
{
	switch(wCode)
	{
		case DMW_DRIVEADDED:
		case DMW_DRIVEREMOVED:
		case DMW_DRIVECHANGED:
		case DMW_MEDIUMARRIVED:
		case DMW_MEDIUMREMOVED:
		case DMW_MODECHANGED:
			if (GetCurrentThreadId() != GetWindowThreadProcessId(plugin.hwndLibraryParent, NULL))
			{
				HANDLE htWA = (WASABI_API_APP) ? WASABI_API_APP->main_getMainThreadHandle() : NULL;
				if (htWA)
				{
					QueueUserAPC(OnDriveManagerNotify, htWA, MAKELONG(wCode, (WORD)(param)));
					CloseHandle(htWA);
				}
			}
			else OnDriveManagerNotify(MAKELONG(wCode, (WORD)(param)));
			break;
		case DMW_OPCOMPLETED: OnDriveMangerOpCompleted((DM_NOTIFY_PARAM*)param); 	break;
	}
}

void ShowHideRipBurnParent(void)
{
	BOOL bVal;
	if (S_OK == Settings_GetBool(C_GLOBAL, GF_SHOWPARENT, &bVal) && bVal)
	{
		if(!hniMain)
		{
			NAVINSERTSTRUCT nis;
			ZeroMemory(&nis, sizeof(NAVITEM));
			nis.item.cbSize = sizeof(NAVITEM);
			nis.item.pszText = WASABI_API_LNGSTRINGW_BUF(IDS_RIP_AND_BURN, randb,64);
			nis.item.pszInvariant = NAVITEM_PREFIX L"main";
			nis.item.mask = NIMF_TEXT | NIMF_STYLE | NIMF_TEXTINVARIANT | NIMF_PARAM;
			nis.item.style = NIS_HASCHILDREN;
			nis.item.styleMask = nis.item.style;
			nis.item.lParam = 0L;
			hniMain = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);
		}
	}
	else
	{
		if(hniMain)
		{
			MLNavCtrl_DeleteItem(plugin.hwndLibraryParent,hniMain);
		}
		hniMain = NULL;
	}
}

int Init()
{
	QueryPerformanceFrequency(&freq);

	// get the Application service
	waServiceFactory *sf = plugin.service->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) WASABI_API_APP = (api_application *)sf->getInterface();

	// loader so that we can get the localisation service api for use
	sf = plugin.service->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	sf = plugin.service->service_getServiceByGuid(AnonymousStatsGUID);
	if (sf) AGAVE_API_STATS = reinterpret_cast<api_stats*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,MlDiscLangGUID);

	mediaLibrary.library = plugin.hwndLibraryParent;
	mediaLibrary.winamp = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	static wchar_t szDescription[256];
	StringCchPrintf(szDescription, ARRAYSIZE(szDescription),
					WASABI_API_LNGSTRINGW(IDS_NULLSOFT_RIP_AND_BURN), VERSION_MAJOR, VERSION_MINOR);
	plugin.description = (char*)szDescription;

	// add to Winamp preferences
	myPrefsItemCD.dlgID = IDD_PREFSCDRIPFR;
	myPrefsItemCD.name = WASABI_API_LNGSTRINGW_BUF(IDS_CD_RIPPING,cdrip,64);
	myPrefsItemCD.proc = (void*)CDRipPrefsProc;
	myPrefsItemCD.hInst = WASABI_API_LNG_HINST;
	myPrefsItemCD.where = 6; // media library
	SENDWAIPC(plugin.hwndWinampParent, IPC_ADD_PREFS_DLGW, (WPARAM)&myPrefsItemCD);

	wchar_t szIniFile[MAX_PATH],
			*INI_DIR = (wchar_t*)SENDWAIPC(plugin.hwndWinampParent, IPC_GETINIDIRECTORYW, 0);
	
	PathCombine(szIniFile, INI_DIR, TEXT("Plugins\\gen_ml.ini"));
	g_config = new C_Config(szIniFile);

	PathCombine(szIniFile, INI_DIR, TEXT("Plugins\\ml\\cdrom.vmd"));
	g_view_metaconf = new C_Config(szIniFile);

	g_context_menus = WASABI_API_LOADMENU(IDR_CONTEXTMENUS);

	oldWinampWndProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(plugin.hwndWinampParent, GWLP_WNDPROC, (LONG)(LONG_PTR)WinampWndProc);

	if (!uMsgBurnerNotify) uMsgBurnerNotify = RegisterWindowMessageA("WABURNER_BROADCAST_MSG");
	if (!uMsgRipperNotify) uMsgRipperNotify = RegisterWindowMessageA("WARIPPER_BROADCAST_MSG");
	if (!uMsgCopyNotify) uMsgCopyNotify = RegisterWindowMessageA("WACOPY_BROADCAST_MSG");
	if (!uMsgNavStyleUpdate) uMsgNavStyleUpdate = RegisterWindowMessageW(L"ripburn_nav_update");

	UpdatedNavStyles();
	ShowHideRipBurnParent();

	delay_ml_startup = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"ml_disc_delay", IPC_REGISTER_WINAMP_IPCMESSAGE);
	PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, delay_ml_startup);
	return ML_INIT_SUCCESS;
}

void Quit()
{
	DriveManager_Uninitialize(4000); // allow to wait for 4 sec.
	MLDisc_ReleaseCopyData();
	delete(g_view_metaconf);
	g_view_metaconf = 0;

	delete(g_config);
	g_config = NULL;

	if (rgThread)
	{
		QueueUserAPC(QuitThread, rgThread, 0);
		WaitForSingleObject(rgThread, INFINITE);
		CloseHandle(rgThread);
		rgThread = 0;
	}

	waServiceFactory *sf = plugin.service->service_getServiceByGuid(AnonymousStatsGUID);
	if (sf) sf->releaseInterface(AGAVE_API_STATS);
}

int getFileInfo(const char *filename, const char *metadata, char *dest, int len)
{
	dest[0] = 0;
	extendedFileInfoStruct efis = { filename, metadata, dest, len, };
	return (int)(INT_PTR)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM) & efis, IPC_GET_EXTENDED_FILE_INFO); //will return 1 if wa2 supports this IPC call
}

int getFileInfoW(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, int len)
{
	if (dest && len)
		dest[0] = 0;
	extendedFileInfoStructW efis = { filename, metadata, dest, len, };
	return (int)(INT_PTR)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&efis, IPC_GET_EXTENDED_FILE_INFOW); //will return 1 if wa2 supports this IPC call
}


void Plugin_ShowRippingPreferences(void)
{
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&myPrefsItemCD, IPC_OPENPREFSTOPAGE);
}
BOOL Plugin_IsExtractScheduled(CHAR cLetter)
{
	BOOL result;
	if (riphash && (0xFF & (riphash >> 24)) == (UCHAR)cLetter)
	{
		DWORD mediumSN;
		char devname[] = "X:\\";
		devname[0] = cLetter;	
		result = (GetVolumeInformationA(devname, NULL, 0, &mediumSN, NULL, NULL, NULL, 0) && (0x00FFFFFF & riphash) == mediumSN); 
		riphash = 0;
	}
	else result = FALSE;
	return result;
}


static int Root_OnContextMenu(HNAVITEM hItem, HWND hHost, POINTS pts)
{
	HMENU hMenu = GetSubMenu(g_context_menus, 7);
	if (!hMenu) return 0;

	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(plugin.hwndLibraryParent, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}

	int r = Menu_TrackPopup(plugin.hwndLibraryParent, hMenu,
							TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NONOTIFY,
							pt.x, pt.y, hHost, NULL);
	switch (r)
	{
		case ID_NAVIGATION_PREFERENCES: Plugin_ShowRippingPreferences(); return 1;
		case ID_NAVIGATION_HELP: SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, L"https://help.winamp.com/hc/articles/8111574760468-CD-Ripping-with-Winamp"); return 1;
		break;
	}
	return 0;
}

static int Plugin_OnContextMenu(HNAVITEM hItem, HWND hHost, POINTS pts, CHAR cLetter)
{
	HMENU hMenu = GetSubMenu(g_context_menus, 2);
	if (!hMenu) return 0;

	MENUITEMINFO mii = { sizeof(MENUITEMINFO), };
	mii.fMask = MIIM_STATE;
	if (GetMenuItemInfo(hMenu, ID_CDROMMENU_EJECTCD, FALSE, &mii))
	{
		mii.fState &= ~(MFS_ENABLED | MFS_DISABLED);
		mii.fState |= ((DM_MODE_READY == DriveManager_GetDriveMode(cLetter)) ? MFS_ENABLED : MFS_DISABLED);
		SetMenuItemInfo(hMenu, ID_CDROMMENU_EJECTCD, FALSE, &mii);
	}

	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(plugin.hwndLibraryParent, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}

	int r = Menu_TrackPopup(plugin.hwndLibraryParent, hMenu,
							TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NONOTIFY,
							pt.x, pt.y, hHost, NULL);
	switch (r)
	{
		case ID_CDROMMENU_EXTRACT_CONFIGURE: Plugin_ShowRippingPreferences(); return 1;
		case ID_CDROMMENU_EXTRACT_HELP:  SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, L"https://help.winamp.com/hc/articles/8111574760468-CD-Ripping-with-Winamp"); return 1;
		case ID_CDROMMENU_PLAYALL:
		case ID_CDROMMENU_ENQUEUEALL:
		{
			int enq = r == ID_CDROMMENU_ENQUEUEALL;
			itemRecordList obj = {0, };
			saveCDToItemRecordList(cLetter, &obj, NULL);
			mlSendToWinampStruct p;
			p.type = ML_TYPE_CDTRACKS;
			p.enqueue = enq | 2;
			p.data = &obj;
			SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SENDTOWINAMP, (WPARAM)&p);
			freeRecordList(&obj);
		}
		break;
		case ID_CDROMMENU_EXTRACT_EXTRACTALL:
			riphash = 0;
			if (hItem)
			{
				if (hItem == MLNavCtrl_GetSelection(plugin.hwndLibraryParent))
				{
					HWND hwnd = (HWND)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_GETCURRENTVIEW, 0);
					if (hwnd && SendMessageW(hwnd, WM_EXTRACTDISC, cLetter, 0)) break;
				}
				
				char devname[] = "X:\\";
				devname[0] = cLetter;
				if (!GetVolumeInformationA(devname, NULL, 0, &riphash, NULL, NULL, NULL, 0)) riphash = 0;
				if (riphash) riphash = ((0x00FFFFFF & riphash) | (cLetter << 24));
				MLNavItem_Select(plugin.hwndLibraryParent, hItem);
			}
			break;
		case ID_CDROMMENU_EJECTCD:
			{
				CHAR cMode;
				cMode = DriveManager_GetDriveMode(cLetter);
				if (DM_MODE_READY != cMode)
				{
					wchar_t title[32] = {0};
					MessageBox(plugin.hwndLibraryParent,
							   WASABI_API_LNGSTRINGW((DM_MODE_RIPPING == cMode) ? IDS_ERROR_CD_RIP_IN_PROGRESS : IDS_ERROR_CD_BURN_IN_PROGRESS),
							   WASABI_API_LNGSTRINGW_BUF(IDS_CD_EJECT,title,32), 0);
					return FALSE;
				}
				else DriveManager_Eject(cLetter, DM_EJECT_REMOVE);
			}
			break;
	}
	Sleep(100);
	MSG msg;
	while (PeekMessageW(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
	return 0;
}

static DWORD resumeTick = 0; // this is cheating
static void Plugin_OnMLVisible(BOOL bVisible)
{
	if (bVisible)
	{		
		DriveManager_Resume(TRUE);
		resumeTick = GetTickCount();
		SendNotifyMessage(HWND_BROADCAST, uMsgBurnerNotify, MAKEWPARAM(0, 0xffff), (LPARAM)plugin.hwndWinampParent);
		SendNotifyMessage(HWND_BROADCAST, uMsgRipperNotify, MAKEWPARAM(0, 0xffff), (LPARAM)plugin.hwndWinampParent);
		SendNotifyMessage(HWND_BROADCAST, uMsgCopyNotify, MAKEWPARAM(0, 0xffff), (LPARAM)plugin.hwndWinampParent);
		return;	
	}
	else DriveManager_Suspend();
}

static HWND Plugin_OnViewCreate(HNAVITEM hItem, HWND hwndParent)
{
	if (hItem == hniMain)
	{
		return WASABI_API_CREATEDIALOGW(IDD_VIEW_RIPBURN, hwndParent, view_ripburnDialogProc);
	}
	else
	{
		DRIVE *pDrive = Plugin_GetDriveFromNavItem(hItem);
		if (pDrive) return CreateContainerWindow(hwndParent, pDrive->cLetter, ((GetTickCount() - resumeTick) > 100));
		resumeTick = 0;
	}
	return NULL;
}
static BOOL Plugin_OnNavItemDelete(HNAVITEM hItem)
{
	DRIVE *pDrive = Plugin_GetDriveFromNavItem(hItem);
	if (!pDrive) return FALSE;
	DriveParam_UnregisterDrive(pDrive);
	free(pDrive);
	return TRUE;
}

static BOOL Plugin_OnNavItemClick(HNAVITEM hItem, UINT nAction, HWND hwndParent)
{
	return FALSE;
}

static INT Plugin_OnNavCustomDraw(HNAVITEM hItem, NAVITEMDRAW *pnicd, LPARAM lParam)
{
	static INT indent = 0;
	DRIVE *pDrive;
	
	if (FALSE == DriveParam_IsValid(lParam))
		return FALSE;

	pDrive = (DRIVE*)lParam;

	if (0 == indent) indent = MLNavCtrl_GetIndent(plugin.hwndLibraryParent);
	switch(pnicd->drawStage)
	{
		case NIDS_PREPAINT: 
			if (pnicd->prc->bottom > 0 && pnicd->prc->bottom > pnicd->prc->top)
			{
				HIMAGELIST himl;
				INT realIndex, l, t, r;
				MLIMAGELISTREALINDEX mlilRealIndex;

				himl = MLImageList_GetRealList(plugin.hwndLibraryParent, hmlilIcons);

				mlilRealIndex.cbSize = sizeof(MLIMAGELISTREALINDEX);
				mlilRealIndex.hmlil = hmlilIcons;
				mlilRealIndex.rgbBk = GetBkColor(pnicd->hdc);
				mlilRealIndex.rgbFg = GetTextColor(pnicd->hdc);

				t = pnicd->prc->top + (pnicd->prc->bottom - pnicd->prc->top - ICON_SIZE_CY)/2;
				l = pnicd->prc->left + (indent*pnicd->iLevel) + 3;
				r = pnicd->prc->right;

				mlilRealIndex.mlilIndex = 0;
				realIndex = ((NCS_SHOWICONS & g_navStyle) && himl && l < pnicd->prc->right) ? 
									MLImageList_GetRealIndex(plugin.hwndLibraryParent, &mlilRealIndex) : -1;	
				if (-1 != realIndex) // draw icon
				{
					if (ImageList_Draw(himl, realIndex, pnicd->hdc, l, t, ILD_NORMAL))
					{
						ExcludeClipRect(pnicd->hdc, l, t, l + ICON_SIZE_CX, t + ICON_SIZE_CY);
						l += (ICON_SIZE_CX + 5);
					}
				}

				pDrive->bEjectVisible = FALSE;
				mlilRealIndex.mlilIndex = 1 + ((DM_MODE_READY == pDrive->cMode) ?  pDrive->nBtnState : BTNSTATE_DISABLED);
				realIndex = ((NCS_EX_SHOWEJECT & g_navStyle) && himl && (r - l) > (24 + 6 + ICON_SIZE_CX)) ? 
									MLImageList_GetRealIndex(plugin.hwndLibraryParent, &mlilRealIndex) : -1;
				if (-1 != realIndex)
				{
					if (ImageList_Draw(himl, realIndex, pnicd->hdc, r - (ICON_SIZE_CX + 2), t, ILD_NORMAL))
					{
						r -= (ICON_SIZE_CX + 2);
						ExcludeClipRect(pnicd->hdc, r, t, r + ICON_SIZE_CX, t + ICON_SIZE_CY);
						r -= 4;
						pDrive->bEjectVisible = TRUE;
					}
				}
				
				if (*pDrive->szTitle && l < r)
				{
					RECT rt;
					INT  textH, textW;
					COLORREF rgbOld(0), rgbBkOld(0);

					if (!pDrive->textSize || (pDrive->textOrigWidth > r-l-3 && pDrive->itemWidth > (pnicd->prc->right - pnicd->prc->left)) || 
							(LOWORD(pDrive->textSize) != pDrive->textOrigWidth && pDrive->itemWidth < (pnicd->prc->right - pnicd->prc->left)))
					{
						NAVITEM item;
						item.cbSize = sizeof(NAVITEM);
						item.mask = NIMF_TEXT;
						item.hItem = hItem;
						item.cchTextMax = sizeof(pDrive->szTitle)/sizeof(wchar_t);
						item.pszText = pDrive->szTitle;
						MLNavItem_GetInfo(plugin.hwndLibraryParent, &item);
						{
							if (pDrive->szTitle != item.pszText) 
							{
								StringCchCopyW(pDrive->szTitle, sizeof(pDrive->szTitle)/sizeof(wchar_t), item.pszText);
							}
							if (!pDrive->textSize)
							{
								SetRect(&rt, 0, 0, 1, 1);
								DrawTextW(pnicd->hdc, pDrive->szTitle, -1, &rt, 	DT_SINGLELINE | DT_CALCRECT);
								pDrive->textOrigWidth = rt.right - rt.left;
							}
							SetRect(&rt, 0, 0, r - l - 3, 1);
							textH = DrawTextW(pnicd->hdc, pDrive->szTitle, -1, &rt, 	DT_SINGLELINE|DT_CALCRECT|DT_END_ELLIPSIS|DT_MODIFYSTRING);
							textW = rt.right - rt.left;
							pDrive->textSize = (DWORD)MAKELONG(textW, textH);
						}
					}
					else 
					{
						textW = LOWORD(pDrive->textSize);
						textH = HIWORD(pDrive->textSize);
					}

					if (0 == (NCS_FULLROWSELECT & g_navStyle) && ((NIS_SELECTED | NIS_DROPHILITED) & pnicd->itemState))
					{
						rgbOld = SetTextColor(pnicd->hdc, pnicd->clrText);
						rgbBkOld = SetBkColor(pnicd->hdc, pnicd->clrTextBk);
					}
					
					if (r > (l + textW + 7)) r = l + textW + 7;
					
					SetRect(&rt, l, pnicd->prc->top, r, pnicd->prc->bottom);
					
					t = pnicd->prc->top + (pnicd->prc->bottom - pnicd->prc->top - textH)/2;
					ExtTextOutW(pnicd->hdc, rt.left + 2, t, ETO_CLIPPED | ETO_OPAQUE, &rt, pDrive->szTitle, lstrlenW(pDrive->szTitle), 0);
					if (0 == (NCS_FULLROWSELECT & g_navStyle) && (NIS_FOCUSED & pnicd->itemState) && 
						0 == (0x1 /*UISF_HIDEFOCUS*/ & (INT)SendMessageW(MLNavCtrl_GetHWND(plugin.hwndLibraryParent), 0x129 /*WM_QUERYUISTATE*/, 0, 0L)))
					{
						DrawFocusRect(pnicd->hdc, &rt);
					}
					ExcludeClipRect(pnicd->hdc, rt.left, rt.top, rt.right, rt.bottom);
								
					
					if (0 == (NCS_FULLROWSELECT & g_navStyle) && ((NIS_SELECTED | NIS_DROPHILITED) & pnicd->itemState))
					{
						if (rgbOld != pnicd->clrText) SetTextColor(pnicd->hdc, rgbOld);
						if (rgbBkOld != pnicd->clrTextBk) SetBkColor(pnicd->hdc, rgbBkOld);
					}
				}
				
				pDrive->itemWidth = (WORD)(pnicd->prc->right - pnicd->prc->left);

				if (NCS_FULLROWSELECT & g_navStyle)
				{
					ExtTextOutW(pnicd->hdc, 0, 0, ETO_OPAQUE, pnicd->prc, L"", 0, 0);
					return NICDRF_SKIPDEFAULT;
				}
				else ExcludeClipRect(pnicd->hdc, pnicd->prc->left, pnicd->prc->top, pnicd->prc->right, pnicd->prc->bottom);
			}
			break;
		case NIDS_POSTPAINT: 
			break;
	}
	return NICDRF_DODEFAULT;
}

static HNAVITEM	hItemActive = NULL;

static BOOL GetEjectBtnRect(HNAVITEM hItem, RECT *prc)
{
	NAVITEMGETRECT navRect;
	navRect.fItem = FALSE;
	navRect.hItem = hItem;

	if (!hItem || !prc || !MLNavItem_GetRect(plugin.hwndLibraryParent, &navRect)) return FALSE;

	navRect.rc.right -= 2;
	navRect.rc.left = navRect.rc.right - ICON_SIZE_CX;
	navRect.rc.top += (navRect.rc.bottom - navRect.rc.top - ICON_SIZE_CY)/2;
	navRect.rc.bottom = navRect.rc.top + ICON_SIZE_CY;

	CopyRect(prc, &navRect.rc);
	return TRUE;
}

static INT_PTR Plugin_OnNavHitTest(HNAVITEM hItem, NAVHITTEST *pnavHitTest, LPARAM lParam)
{
	DRIVE *pDrive;

	if (FALSE == DriveParam_IsValid(lParam))
		return FALSE;

	pDrive = (DRIVE*)lParam;

	if ((NAVHT_ONITEMRIGHT | NAVHT_ONITEM) & pnavHitTest->flags)
	{
		RECT rb;

		if (pDrive->bEjectVisible && GetEjectBtnRect(hItem, &rb) && 
				pnavHitTest->pt.x >= rb.left && pnavHitTest->pt.x <= rb.right && 
				pnavHitTest->pt.y >= rb.top && pnavHitTest->pt.y <= rb.bottom)
		{
			pnavHitTest->flags = NAVHT_NOWHERE;
			pnavHitTest->hItem = NULL;
		}
	}
	return 1;
}

static void CALLBACK NavButton_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	DRIVE *pDrive;
	POINT pt;
	RECT rb;
	
	pDrive = (hItemActive) ? Plugin_GetDriveFromNavItem(hItemActive) : NULL;
	if (!pDrive || (BYTE)BTNSTATE_NORMAL == pDrive->nBtnState || !pDrive->bEjectVisible || !GetEjectBtnRect(hItemActive, &rb)) 
	{
		KillTimer(NULL, idEvent);
		return;
	}

	GetCursorPos(&pt);
	MapWindowPoints(HWND_DESKTOP, MLNavCtrl_GetHWND(plugin.hwndLibraryParent), &pt, 1);


	if (pt.x < rb.left || pt.x > rb.right || pt.y < rb.top || pt.y > rb.bottom)
	{
		NAVITEMINAVLIDATE inv;
		
		KillTimer(NULL, idEvent);
		
		inv.fErase = FALSE;
		inv.hItem = hItemActive;
		inv.prc = &rb;

		hItemActive = NULL;
		pDrive->nBtnState = BTNSTATE_NORMAL;

		MLNavItem_Invalidate(plugin.hwndLibraryParent, &inv);
	}
}

static INT_PTR Plugin_OnNavSetCursor(HNAVITEM hItem, LPARAM lParam)
{
	POINT pt;
	DRIVE *pDrive;
	BYTE state;
	RECT rb;

	if (FALSE == DriveParam_IsValid(lParam))
		return FALSE;

	pDrive = (DRIVE*)lParam;

	if (DM_MODE_READY != pDrive->cMode || !pDrive->bEjectVisible || !GetEjectBtnRect(hItem, &rb)) return -1;
    
	GetCursorPos(&pt);

	MapWindowPoints(HWND_DESKTOP, MLNavCtrl_GetHWND(plugin.hwndLibraryParent), &pt, 1);
	if (pt.x >= rb.left && pt.x <= rb.right && pt.y >= rb.top && pt.y <= rb.bottom)
	{			
		state = (BYTE)((0x8000 & GetAsyncKeyState( GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON)) ? 
					BTNSTATE_PRESSED : BTNSTATE_HILITED);
	}
	else state = BTNSTATE_NORMAL;

	if (pDrive->nBtnState != state)
	{
		
		NAVITEMINAVLIDATE inv;

		if ((BYTE)BTNSTATE_PRESSED == pDrive->nBtnState && BTNSTATE_HILITED == state)
		{
			DriveManager_Eject(pDrive->cLetter, DM_EJECT_CHANGE);
		}
		
		if (pDrive->timerId)
		{
			KillTimer(NULL, pDrive->timerId);
			pDrive->timerId = 0;
		}
		if (hItemActive)
		{
			DRIVE *pDriveOld = Plugin_GetDriveFromNavItem(hItemActive);
			if (pDriveOld)
			{
				RECT rb2;
				if (pDriveOld->timerId) 
				{
					KillTimer(NULL, pDriveOld->timerId);
					pDriveOld->timerId = NULL;
				}
				if ((BYTE)BTNSTATE_NORMAL != pDriveOld->nBtnState && GetEjectBtnRect(hItemActive, &rb2))
				{
					pDriveOld->nBtnState = BTNSTATE_NORMAL;
					inv.fErase = FALSE;
					inv.hItem = hItemActive;
					inv.prc = &rb2;
					MLNavItem_Invalidate(plugin.hwndLibraryParent, &inv);
				}
				hItemActive = NULL;
			}
		}

		if (BTNSTATE_NORMAL != state)
		{			
			hItemActive = hItem;
			pDrive->timerId = SetTimer(NULL, 0, NAVBUTTON_STATECHECK_DELAY, NavButton_TimerProc);
		}
		
		pDrive->nBtnState = state;
		
		inv.fErase = FALSE;
		inv.hItem = hItem;
		inv.prc = &rb;
		MLNavItem_Invalidate(plugin.hwndLibraryParent, &inv);
	
	}

	
	return -1;
}

static BOOL Plugin_OnConfig(void)
{
	SendMessageW(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&myPrefsItemCD, IPC_OPENPREFSTOPAGE);
	return TRUE;
}

static INT_PTR pluginMessageProc(int msg, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	HNAVITEM hItem;

	if (msg >= ML_MSG_TREE_BEGIN && msg <= ML_MSG_TREE_END)
	{
		hItem = (msg < ML_MSG_NAVIGATION_FIRST) ? MLNavCtrl_FindItemById(plugin.hwndLibraryParent, param1) : (HNAVITEM)param1;
		if (!hItem) return 0;
	} else hItem = NULL;

	switch (msg)
	{
		case ML_MSG_TREE_ONCREATEVIEW:	return (INT_PTR)Plugin_OnViewCreate(hItem, (HWND)param2); 
		case ML_MSG_NAVIGATION_ONDELETE:	return (INT_PTR)Plugin_OnNavItemDelete(hItem);
		case ML_MSG_NAVIGATION_ONCUSTOMDRAW:	return (INT_PTR)Plugin_OnNavCustomDraw(hItem, (NAVITEMDRAW*)param2, (LPARAM)param3); 
		case ML_MSG_NAVIGATION_ONHITTEST:	return (INT_PTR)Plugin_OnNavHitTest(hItem, (NAVHITTEST*)param2, (LPARAM)param3); 
		case ML_MSG_NAVIGATION_ONSETCURSOR:	return (INT_PTR)Plugin_OnNavSetCursor(hItem, (LPARAM)param3); 
		case ML_MSG_NAVIGATION_CONTEXTMENU:
		{
			DRIVE *pDrive;
			if (hniMain && (hItem == hniMain)) 
				return Root_OnContextMenu(hItem, (HWND)param2, MAKEPOINTS(param3));

			//Plugin Item
			pDrive = Plugin_GetDriveFromNavItem(hItem);
			if (pDrive) 
				return Plugin_OnContextMenu(hItem, (HWND)param2, MAKEPOINTS(param3), pDrive->cLetter);

			return 0;
		}
		case ML_MSG_TREE_ONCLICK:		return (INT_PTR)Plugin_OnNavItemClick(hItem, (UINT)param2, (HWND)param3);
		case ML_MSG_CONFIG:				return (INT_PTR)Plugin_OnConfig();
		case ML_MSG_MLVISIBLE:			Plugin_OnMLVisible((BOOL)param1); break;
		case ML_MSG_NOTOKTOQUIT:		if (!Plugin_QueryOkToQuit()) { return TRUE; } break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
extern "C" winampMediaLibraryPlugin plugin = 
{ 	
	MLHDR_VER, 
	"nullsoft(ml_disc.dll)", 
	Init, 
	Quit, 
	pluginMessageProc, 	
	0, 
	0, 
	0, 
};

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}