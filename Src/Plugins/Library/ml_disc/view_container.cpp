#include "main.h"
#include "./resource.h"
#include "../nu/DialogSkinner.h"
#include "../nu/AutoWideFn.h"
#include "./commandbar.h"
#include "./settings.h"
#include "../nu/trace.h"

//#include <primosdk.h>
#include <windowsx.h>
#include <strsafe.h>

#define MINIINFO_HEIGHT		100

#define IDC_CHILDVIEW		0x1000
#define IDC_MINIINFO		0x1001
#define IDC_COMMAND_BAR		0x1002

#define WINDOWS_SPACING		3

#define  CONTAINER_PROPW	L"CONTAINER"

typedef struct _CONTAINER
{
	CHAR cLetter;
	INT	typeChild;
	BOOL bInfoVisible;
	HANDLE hLastQuery;
} CONTAINER;

static UINT msgNotify = 0;
#define GetContainer(__hwnd) ((CONTAINER*)GetPropW(__hwnd, CONTAINER_PROPW))
static INT_PTR WINAPI DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// public function
typedef struct _CONTAINER_PARAM
{
	CHAR cLetter;
	BOOL bQueryInfo;
}CONTAINER_PARAM;

HWND CreateContainerWindow(HWND hwndParent, CHAR cLetter, BOOL bQueryInfo)
{
	CONTAINER_PARAM param;
	param.cLetter = cLetter;
	param.bQueryInfo = bQueryInfo;
	return WASABI_API_CREATEDIALOGPARAMW(IDD_VIEW_CONTAINER, hwndParent, DlgProc, (LPARAM)&param);
}

static void ViewContainer_LayoutChildren(HWND hdlg, BOOL bRedraw)
{
	RECT rc;
	HWND hctrl;
	HDWP hdwp;
	DWORD flags;
	
	CONTAINER *pc = GetContainer(hdlg);
	if (!pc) return;

	if (!GetClientRect(hdlg, &rc)) return;
	rc.right -= 2;

	flags = SWP_NOACTIVATE | ((bRedraw) ? 0 : SWP_NOREDRAW);

	hdwp = BeginDeferWindowPos(3);
	if (!hdwp) return;

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_COMMAND_BAR)))
	{
		INT height = CommandBar_GetBestHeight(hctrl);
		hdwp = DeferWindowPos(hdwp, hctrl, HWND_BOTTOM, rc.left, rc.bottom - (height), rc.right - rc.left, height, flags);
		rc.bottom -= (height + WINDOWS_SPACING);
	}
	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_MINIINFO)) && pc->bInfoVisible)
	{		
		INT height = MINIINFO_HEIGHT;
		hdwp = DeferWindowPos(hdwp, hctrl, HWND_TOP, rc.left, rc.bottom - height, rc.right - rc.left, height, flags);
		rc.bottom -= (height + WINDOWS_SPACING);
	}

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_CHILDVIEW)))
	{		
		hdwp = DeferWindowPos(hdwp, hctrl, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, flags);
	}

	EndDeferWindowPos(hdwp);
}

static void CreateChild(HWND hParent, INT typeId, LPARAM param, BOOL fCreateAlways = TRUE)
{
	DM_NOTIFY_PARAM hdr;
	CONTAINER *pc = GetContainer(hParent);
	if (!pc) return;

	HWND hChild = GetDlgItem(hParent, IDC_CHILDVIEW);
    if (hChild)
	{
		if (pc->typeChild == typeId && !fCreateAlways) return;
		DestroyWindow(hChild);
	}

	hChild = NULL;
	pc->typeChild = 0;

	switch(typeId)
	{
		case IDD_VIEW_INFO:
			hChild = CreateInfoWindow(hParent, (CHAR)(0xFF & param));
			break;
		case IDD_VIEW_WAIT:
			hChild = CreateWaitWindow(hParent, (CHAR)(0xFF & param));
			break;
		case IDD_VIEW_CDROM:
			if (param <= 0xFF)		
			{
				ZeroMemory(&hdr, sizeof(DM_NOTIFY_PARAM));
				hdr.cLetter = (CHAR)(0xFF & (INT)param);
				hdr.opCode = DMOP_GENERAL;
				param = (INT_PTR)&hdr;
			}
			hChild = CreateCDViewWindow(hParent, (DM_NOTIFY_PARAM*)param);
			break;
		case IDD_VIEW_CDROM_EX2:
			hChild = CreateCDRipWindow(hParent, (CHAR)(0xFF & param));
			break;
		case IDD_VIEW_CDROM_DATA:
			hChild = CreateCdDataViewWindow(hParent, (CHAR)(0xFF & param));
			break;
	}
	TRACE_FMT(L"Creating cd view (%d)\n", typeId);
	if (hChild)
	{	
		pc->typeChild = typeId;
		SetWindowLongPtrW(hChild, GWLP_ID, IDC_CHILDVIEW); 
		ViewContainer_LayoutChildren(hParent, TRUE);
		ShowWindow(hChild, SW_SHOWNA);
	}
}

static HWND CreateMiniInfoWindow(HWND hwndParent)
{
	RECT rw;
	HWND hwnd;
	WEBINFOCREATE wic;

	GetWindowRect(hwndParent, &rw);

	wic.hwndParent = hwndParent;
	wic.uMsgQuery = WM_QUERYFILEINFO;
	wic.ctrlId = IDC_MINIINFO;
	wic.cx = rw.right - rw.left - 3;
	wic.cy = MINIINFO_HEIGHT;
	wic.x = 0;
	wic.y = (rw.bottom - rw.top) - MINIINFO_HEIGHT - 1;
	
	hwnd = (HWND)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_CREATEWEBINFO, (WPARAM)&wic);
	if (hwnd) 
	{
		SetWindowLongPtrW(hwnd, GWL_EXSTYLE, GetWindowLongPtrW(hwnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);
		MLSkinWindow2(plugin.hwndLibraryParent, hwnd, SKINNEDWND_TYPE_WINDOW, SWS_USESKINCOLORS);
	}

	return hwnd;
}

static void CALLBACK FreeAsyncParam(DM_NOTIFY_PARAM *phdr)
{
	DM_MCI_PARAM *pmci;
	switch(phdr->opCode)
	{
		case DMOP_MCIINFO:
			pmci = (DM_MCI_PARAM*)phdr;
			if (pmci->pTracks) free(pmci->pTracks);
			break;
		case DMOP_IMAPIINFO:
			break;
	}
	free(phdr);
}

static void QueryDriveRecordable(HWND hwnd, CHAR cLetter)
{
	CONTAINER *pc = GetContainer(hwnd);
	if (!pc) return;

	DWORD dwType;
	dwType = DriveManager_GetDriveType(cLetter);
	if (0 == (DRIVE_CAP_UNKNOWN & dwType))
	{
		CreateChild(hwnd, ((DRIVE_CAP_R | DRIVE_CAP_RW) & dwType) ? IDD_VIEW_CDROM_BURN : IDD_VIEW_CDROM, (LPARAM)cLetter);
		return;
	}
	else
	{
		DM_UNITINFO_PARAM *pui;

		pui = (DM_UNITINFO_PARAM*)calloc(1, sizeof(DM_UNITINFO_PARAM));
		if (pui)
		{
			pui->header.callback = (INT_PTR)hwnd;
			pui->header.uMsg = msgNotify;
			pui->header.cLetter = cLetter;
			pui->header.fnFree = FreeAsyncParam;
			pc->hLastQuery = pui;
			if (!DriveManager_GetUnitInfo(pui)) CreateChild(hwnd, IDD_VIEW_CDROM, (LPARAM)cLetter);
		}
	}
}

static void QueryDiscInfo(HWND hwnd, CHAR cLetter)
{
	DM_MCI_PARAM *pmci;
	CHAR cMode;

	CONTAINER *pc = GetContainer(hwnd);
	if (!pc) return;

	cMode = DriveManager_GetDriveMode(cLetter);

	switch(cMode)
	{
		case DM_MODE_RIPPING: 
			if (cdrip_isextracting(cLetter)) CreateChild(hwnd, IDD_VIEW_CDROM_EX2, cLetter); 
			else CreateChild(hwnd, IDD_VIEW_INFO, cLetter);
			return;

		case DM_MODE_COPYING:
			if (IDD_VIEW_CDROM_DATA == pc->typeChild) return;
			break;
		case DM_MODE_READY:
			if (IDD_VIEW_CDROM_DATA == pc->typeChild) return;
			break;
	}

	CreateChild(hwnd, IDD_VIEW_WAIT, cLetter);

	pmci = (DM_MCI_PARAM*)calloc(1, sizeof(DM_MCI_PARAM));
    if (pmci)
	{
		pmci->header.callback = (INT_PTR)hwnd;
		pmci->header.uMsg = msgNotify;
		pmci->header.cLetter = cLetter;
		pmci->header.fnFree = FreeAsyncParam;
		pmci->header.fFlags = DMF_READY | DMF_MEDIUMPRESENT | DMF_MODE | DMF_TRACKCOUNT | DMF_TRACKSINFO;
		pmci->nTracks = 256;
		pmci->pTracks = (DWORD*)calloc(pmci->nTracks, sizeof(DWORD));

		pc->hLastQuery = pmci;

		if (!DriveManager_GetMCIInfo(pmci))
		{
			pc->hLastQuery = NULL;
			// display error dialog
		}
	}
}

static void Medium_OnArrived(HWND hwnd, CHAR cLetter)
{
	QueryDiscInfo(hwnd, cLetter);
}

static void Medium_OnRemoved(HWND hwnd, CHAR cLetter)
{
	QueryDriveRecordable(hwnd, cLetter);
}

static void Drive_OnModeChanged(HWND hwnd, CHAR cLetter, CHAR cMode)
{	
	QueryDiscInfo(hwnd, cLetter); 
	HWND hctrl = GetDlgItem(hwnd, IDC_CHILDVIEW);
	if (hctrl) SendMessage(hctrl, WM_COMMAND, MAKEWPARAM(ID_DRIVE_MODE_CHANGED, 0), (LPARAM)hwnd);
}

static void GetInfo_Completed(HWND hwnd, DM_NOTIFY_PARAM *phdr)
{
	DM_MCI_PARAM *pmci;
	CONTAINER *pc = GetContainer(hwnd);
	if (!pc || phdr != pc->hLastQuery) return;

	switch(phdr->opCode)
	{
		case DMOP_MCIINFO:
			pmci = (DM_MCI_PARAM*)phdr;
			if (0 == phdr->result)
			{
				if (pmci->bMediumPresent && pmci->nTracks > 0) 
				{
					int i;
					for (i = 0; i < pmci->nTracks; i++) if (0x80000000 & pmci->pTracks[i]) break;
					if (i == pmci->nTracks) CreateChild(hwnd, IDD_VIEW_CDROM_DATA, (LPARAM)phdr->cLetter);
					else CreateChild(hwnd, IDD_VIEW_CDROM, (LPARAM)phdr);
				}
				else  QueryDriveRecordable(hwnd, phdr->cLetter);
			}
			else {/*go to error view*/ }
			break;
		case DMOP_UNITINFO:
			CreateChild(hwnd, ( 0 == phdr->result && Drive_IsRecorderType(((DM_UNITINFO_PARAM*)phdr)->dwType)) ? IDD_VIEW_CDROM_BURN : IDD_VIEW_CDROM, (LPARAM)phdr->cLetter);
			break;
	}
}

static BOOL ViewContainer_OnGetMiniInfoEnabled(HWND hdlg)
{
	return g_config->ReadInt(TEXT("useminiinfo2"), 0);
}

static INT_PTR Window_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	CONTAINER_PARAM *param = (CONTAINER_PARAM*)lParam;
	CONTAINER *pc = (CONTAINER*)calloc(1, sizeof(CONTAINER));
	if (pc)
	{
		pc->cLetter = param->cLetter;
		SetPropW(hwnd, CONTAINER_PROPW, pc);
	}

	MLSkinWindow2(plugin.hwndLibraryParent, hwnd, SKINNEDWND_TYPE_DIALOG, SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	BOOL bVal = FALSE;
	if (S_OK == Settings_GetBool(C_GLOBAL, GF_SHOWINFO, &bVal) && bVal)
		SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_BTN_SHOWINFO, BN_CLICKED), (LPARAM)NULL);

	if (!msgNotify) msgNotify = RegisterWindowMessageW(L"cdrom_notify_msg");
	Plugin_RegisterListener(hwnd, msgNotify, pc->cLetter);

	if (param->bQueryInfo) QueryDiscInfo(hwnd, pc->cLetter);

	if (WASABI_API_APP)
	{
		HACCEL hAccel = WASABI_API_LOADACCELERATORSW(IDR_ACCELERATOR_VIEW);
		if (hAccel) WASABI_API_APP->app_addAccelerators(hwnd, &hAccel, 1, TRANSLATE_MODE_CHILD);
		WASABI_API_APP->ActiveDialog_Register(hwnd);
	}

	return 0;
}

static void Window_OnDestroy(HWND hdlg)
{
	Plugin_UnregisterListener(hdlg);
	if (WASABI_API_APP) WASABI_API_APP->app_removeAccelerators(hdlg);

	HWND hctrl = GetDlgItem(hdlg, IDC_MINIINFO);
	if (hctrl && IsWindow(hctrl)) SENDMLIPC(hctrl, ML_IPC_WEBINFO_RELEASE, 0);

	CONTAINER *pc = GetContainer(hdlg);
	if (pc)
	{
		Settings_SetBool(C_GLOBAL, GF_SHOWINFO, pc->bInfoVisible);
		RemovePropW(hdlg, CONTAINER_PROPW);
		free(pc);
	}
}

static void Window_OnDisplayChange(HWND hdlg, INT dpi, INT resX, INT resY)
{		
	SendDlgItemMessageW(hdlg, IDC_CHILDVIEW, WM_DISPLAYCHANGE, dpi, MAKELPARAM(resX, resY));
	SendDlgItemMessageW(hdlg, IDC_MINIINFO, WM_DISPLAYCHANGE, dpi, MAKELPARAM(resX, resY));
	SendDlgItemMessageW(hdlg, IDC_COMMAND_BAR, WM_DISPLAYCHANGE, dpi, MAKELPARAM(resX, resY));
}

static void ViewContainer_OnWindowPosChanged(HWND hdlg, WINDOWPOS *pwp)
{
	RECT rc, rw;
	HWND hctrl;
	HDWP hdwp;
	DWORD flags;

	CONTAINER *pc = GetContainer(hdlg);
	if (!pc) return;

	if (SWP_FRAMECHANGED & pwp->flags) 
	{
		ViewContainer_LayoutChildren(hdlg, (SWP_NOREDRAW & pwp->flags));
		return;
	}

	if (0 != (SWP_NOSIZE & pwp->flags)) return;

	if (!GetClientRect(hdlg, &rc)) return;

	rc.right -= 2;

	flags = SWP_NOACTIVATE | SWP_NOZORDER | ((SWP_NOREDRAW | SWP_NOCOPYBITS) & pwp->flags);

	hdwp = BeginDeferWindowPos(3);

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_COMMAND_BAR)) && GetWindowRect(hctrl, &rw))
	{		
		hdwp = DeferWindowPos(hdwp, hctrl, HWND_BOTTOM, rc.left, rc.bottom - (rw.bottom - rw.top), rc.right - rc.left, (rw.bottom - rw.top), flags);
		rc.bottom -= (rw.bottom - rw.top + WINDOWS_SPACING);
	}

	if (pc->bInfoVisible && NULL != (hctrl = GetDlgItem(hdlg, IDC_MINIINFO)) && GetWindowRect(hctrl, &rw))
	{			
		hdwp = DeferWindowPos(hdwp, hctrl, HWND_BOTTOM, rc.left, rc.bottom - (rw.bottom - rw.top), rc.right - rc.left, (rw.bottom - rw.top), flags);
		rc.bottom -= (rw.bottom - rw.top + WINDOWS_SPACING);
	}

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_CHILDVIEW)))
	{		
		hdwp = DeferWindowPos(hdwp, hctrl, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, flags | SWP_NOMOVE);
	}

	EndDeferWindowPos(hdwp);
}

static void ViewContainer_ShowMiniInfo(HWND hdlg, BOOL bShow)
{
	CONTAINER *pc = GetContainer(hdlg);
	if (!pc) return;

	pc->bInfoVisible = FALSE;

	HWND hInfo = GetDlgItem(hdlg, IDC_MINIINFO);
	if (bShow && !hInfo) hInfo = CreateMiniInfoWindow(hdlg);

	if (hInfo)
	{
		if (bShow) 
		{
			pc->bInfoVisible = TRUE;
			ViewContainer_LayoutChildren(hdlg, TRUE);
			ShowWindow(hInfo, SW_SHOWNA);
			UpdateWindow(hInfo);
		}
		else
		{			
			ShowWindow(hInfo, SW_HIDE); 
			ViewContainer_LayoutChildren(hdlg, TRUE);
		}
	}

	HWND hctrl = GetDlgItem(hdlg, IDC_CHILDVIEW);
	if (hctrl && NULL != (hctrl = GetDlgItem(hctrl, IDC_BTN_SHOWINFO)))
		SetWindowTextW(hctrl, WASABI_API_LNGSTRINGW((bShow) ? IDS_HIDE_INFO : IDS_SHOW_INFO));
}

static void Window_OnCommand(HWND hdlg, INT eventId, INT ctrlId, HWND hctrl)
{
	CONTAINER *pc = GetContainer(hdlg);
	if (!pc) return;

	switch(ctrlId)
	{		
		case IDC_BTN_SHOWINFO:
			switch(eventId)
			{
				case BN_CLICKED:
					ViewContainer_ShowMiniInfo(hdlg, !pc->bInfoVisible);
					break;
				case BN_EX_GETTEXT:
				{
					if (IsWindow(hctrl))
					{
						// to ensure we're able to remove this for all views, we check this here
						if (!ViewContainer_OnGetMiniInfoEnabled(hdlg))
							DestroyWindow(hctrl);
						else
							SetWindowTextW(hctrl, WASABI_API_LNGSTRINGW((pc->bInfoVisible) ? IDS_HIDE_INFO : IDS_SHOW_INFO));
					}
					break;
				}
			}
			break;
		case ID_MINIINFO_SHOW:	ViewContainer_ShowMiniInfo(hdlg, !pc->bInfoVisible); 	break;
		case ID_EJECT_DISC:		DriveManager_Eject(pc->cLetter, DM_EJECT_CHANGE); break;
		case ID_COPY_SELECTION: SendDlgItemMessageW(hdlg, IDC_CHILDVIEW, WM_COMMAND, MAKEWPARAM(ctrlId, eventId), (LPARAM)hctrl); break;
	}
}

static void Window_OnFileTagUpdated(HWND hdlg, WORD wCode, INT_PTR param)
{
	HWND hChild = GetDlgItem(hdlg, IDC_CHILDVIEW);
    if (hChild && IsWindow(hChild))
	{
		SendMessageW(hChild, WM_TAGUPDATED, 0, 
			(LPARAM)((IPC_FILE_TAG_MAY_HAVE_UPDATED == wCode) ? AutoWideFn((LPCSTR)param) : (LPCWSTR)param));
	}
}

static void Window_OnPluginNotify(HWND hdlg, WORD wCode, INT_PTR param)
{
	switch(wCode)
	{
		case DMW_MEDIUMARRIVED:	Medium_OnArrived(hdlg, (CHAR)param); break;
		case DMW_MEDIUMREMOVED:	Medium_OnRemoved(hdlg, (CHAR)param); break;
		case DMW_OPCOMPLETED:	GetInfo_Completed(hdlg, (DM_NOTIFY_PARAM*)param); break;	
		case DMW_MODECHANGED:	Drive_OnModeChanged(hdlg, (CHAR)LOWORD(param), (CHAR)(LOWORD(param) >> 8)); break;
		case IPC_FILE_TAG_MAY_HAVE_UPDATED:
		case IPC_FILE_TAG_MAY_HAVE_UPDATEDW:	 Window_OnFileTagUpdated(hdlg, wCode, param); break;
	}
}

static INT_PTR Window_OnSendFileInfo(HWND hdlg, LPCWSTR pszFileName, UINT fFlags)
{
	WEBINFOSHOW wis;
	HWND hInfo = GetDlgItem(hdlg, IDC_MINIINFO);
	if (!hInfo) return 0;

	wis.pszFileName = pszFileName;;
	wis.fFlags = fFlags; 
	return (INT_PTR)SENDMLIPC(hInfo, ML_IPC_WEBINFO_SHOWINFO, (WPARAM)&wis);
}

static void Window_OnQueryFileInfo(HWND hdlg)
{
	HWND hChild = GetDlgItem(hdlg, IDC_CHILDVIEW);
	if(!hChild || !PostMessageW(hChild, WM_QUERYFILEINFO, 0, 0L)) Window_OnSendFileInfo(hdlg, L"", WISF_FORCE);
}

INT_PTR CALLBACK CommandBar_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static HWND ViewContainer_OnCreateCommandBar(HWND hdlg, CMDBARCREATESTRUCT *pcbcs)
{
	if (!pcbcs || !pcbcs->fnDialogProc) return NULL;
	HWND hwndBar = WASABI_API_CREATEDIALOGPARAMW(pcbcs->resourceId, hdlg, CommandBar_DialogProc, (LPARAM)pcbcs);
	if (hwndBar)
	{
		SetWindowLongPtrW(hwndBar, GWLP_ID, IDC_COMMAND_BAR);
		ViewContainer_LayoutChildren(hdlg, TRUE);
		ShowWindow(hwndBar, SW_SHOWNA);
	}
	return hwndBar;
}

static BOOL ViewContainer_OnDestroyCommandBar(HWND hdlg)
{
	HWND hwndBar = GetDlgItem(hdlg, IDC_COMMAND_BAR);
	if (!hwndBar) return FALSE;
			
	DestroyWindow(hwndBar);
	ViewContainer_LayoutChildren(hdlg, TRUE);
	return TRUE;
}

static BOOL ViewContainer_OnGetMiniInfoVisible(HWND hdlg)
{
	HWND hctrl = GetDlgItem(hdlg, IDC_MINIINFO);
	return (hctrl && IsWindowVisible(hctrl));
}

static BOOL ViewContainer_OnExtractDisc(HWND hdlg, WPARAM wParam, LPARAM lParam)
{
	HWND hChild = GetDlgItem(hdlg, IDC_CHILDVIEW);
	return (hChild) ? (BOOL)SendMessageW(hChild, WM_EXTRACTDISC, wParam, lParam) : FALSE;
}

static INT_PTR WINAPI DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return (INT_PTR)Window_OnInitDialog(hdlg, (HWND)wParam, lParam);
		case WM_DESTROY:			Window_OnDestroy(hdlg); break;
		case WM_DISPLAYCHANGE:		Window_OnDisplayChange(hdlg, (INT)wParam, LOWORD(lParam), HIWORD(lParam)); break;
		case WM_WINDOWPOSCHANGED:	ViewContainer_OnWindowPosChanged(hdlg, (WINDOWPOS*)lParam); return TRUE;
		case WM_COMMAND:			Window_OnCommand(hdlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam); break;
		case WM_QUERYFILEINFO:		Window_OnQueryFileInfo(hdlg); return 1;
		case WM_SHOWFILEINFO:		MSGRESULT(hdlg, Window_OnSendFileInfo(hdlg, (LPCWSTR)lParam, (UINT)wParam));
		case WM_EXTRACTDISC:		MSGRESULT(hdlg, ViewContainer_OnExtractDisc(hdlg, wParam, lParam));
		case VCM_CREATECOMMANDBAR:	MSGRESULT(hdlg, ViewContainer_OnCreateCommandBar(hdlg, (CMDBARCREATESTRUCT*)lParam));
		case VCM_DESTROYCOMMANDBAR:	MSGRESULT(hdlg, ViewContainer_OnDestroyCommandBar(hdlg));
		case VCM_GETCOMMANDBAR:		MSGRESULT(hdlg, GetDlgItem(hdlg, IDC_COMMAND_BAR));
		case VCM_GETMININFOENABLED: MSGRESULT(hdlg, ViewContainer_OnGetMiniInfoEnabled(hdlg));
		case VCM_GETMININFOVISIBLE: MSGRESULT(hdlg, ViewContainer_OnGetMiniInfoVisible(hdlg));
		case WM_USER + 0x200:		MSGRESULT(hdlg, TRUE);
	}

	if (msgNotify == uMsg) Window_OnPluginNotify(hdlg, (WORD)wParam, (INT_PTR)lParam);
	return 0;
}