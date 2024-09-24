#include "main.h"
#include "resource.h"
#include "../nu/DialogSkinner.h"
#include "../nu/ChildSizer.h"
#include "config.h"
#include ".\driveListBox.h"
#include ".\infoBox.h"
#include ".\primosdk_helper.h"
#include <strsafe.h>

static ChildWndResizeItem ripburn_rlist[]=
{
	{IDC_LBL_DRIVES,			0x0000},
	{IDC_LIST_DRIVES,			0x0001},
	{IDC_LBL_INFO_DRIVE,		0x0010},
	{IDC_LBL_INFO_MEDIUM,		0x0011},
	{IDC_LBL_DRIVE_LETTER_VAL,	0x0010},
	{IDC_LBL_DRIVE_DESCRIPTION_VAL,0x0010},
	{IDC_LBL_DRIVE_BUS_VAL,		0x0010},
	{IDC_LBL_DRIVE_TYPES_VAL,	0x0010},
	{IDC_LBL_MEDIUM_UPDATE,		0x0010},
	{IDC_LBL_MEDIUM_CAPACITY_VAL,		0x0010},
	{IDC_LBL_MEDIUM_TRACKN_VAL,	0x0010},
	{IDC_LBL_MEDIUM_ERASEABLE_VAL,	0x0010},
	{IDC_LBL_MEDIUM_RECORDABLE_VAL,0x0010},
	{IDC_LBL_MEDIUM_FORMAT_VAL,	0x0010},
	{IDC_LBL_MEDIUM_ADDINFO_VAL,	0x0010},
	{IDC_LBL_MEDIUM_DISC_VAL,	0x0010},
	{IDC_BTN_REFRESH,			0x0101},
};

static DriveListBox	*driveListBox = NULL;
static MLInfoBox		*driveInfo = NULL;
static MLInfoBox		*mediumInfo = NULL;

static HBRUSH lblHeaderBrush = NULL;
static HBRUSH lblValueBrush = NULL;

static UINT msgNotify = 0;
static CHAR activeDrive = 0x00;

static void CALLBACK FreeAsyncParam(DM_NOTIFY_PARAM *phdr)
{
	DM_UNITINFO_PARAM *pui = NULL;
	DM_UNITINFO2_PARAM *pui2 = NULL;
	if(!phdr) return;
	
	switch(phdr->opCode)
	{
		case DMOP_UNITINFO:
			pui = (DM_UNITINFO_PARAM*)phdr;
			if (pui->pszDesc) free(pui->pszDesc);
			break;
		case DMOP_UNITINFO2:
			pui2 = (DM_UNITINFO2_PARAM*)phdr;
			if (pui2->pdwTypes) free(pui2->pdwTypes);
			break;
	}
	free(phdr);
}

static void UpdateDriveInfo(HWND hwndDlg, CHAR cLetter)
{
	DM_NOTIFY_PARAM header = {0};
	DM_UNITINFO_PARAM *pui = NULL;
	DM_UNITINFO2_PARAM *pui2 = NULL;
	DM_DISCINFOEX_PARAM *pdi = NULL;
	DM_DISCINFO2_PARAM *pdi2 = NULL;
	wchar_t message[128] = {0};

	activeDrive = cLetter;

	if(!PrimoSDKHelper_IsLoaded())
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_NO_INFO_AVAILABLE,message,128);
	}

	SetDlgItemTextA(hwndDlg, IDC_LBL_DRIVE_LETTER_VAL, &cLetter);
	SetDlgItemText(hwndDlg, IDC_LBL_DRIVE_DESCRIPTION_VAL, message);
	SetDlgItemText(hwndDlg, IDC_LBL_DRIVE_BUS_VAL, message);
	SetDlgItemText(hwndDlg, IDC_LBL_DRIVE_TYPES_VAL, message);
	SetDlgItemTextA(hwndDlg, IDC_LBL_MEDIUM_DISC_VAL, NULL);
	SetDlgItemTextA(hwndDlg, IDC_LBL_MEDIUM_CAPACITY_VAL, NULL);
	SetDlgItemTextA(hwndDlg, IDC_LBL_MEDIUM_FORMAT_VAL, NULL);
	SetDlgItemTextA(hwndDlg, IDC_LBL_MEDIUM_ERASEABLE_VAL, NULL);
	SetDlgItemTextA(hwndDlg, IDC_LBL_MEDIUM_RECORDABLE_VAL, NULL);
	SetDlgItemTextA(hwndDlg, IDC_LBL_MEDIUM_TRACKN_VAL, NULL);
	SetDlgItemTextA(hwndDlg, IDC_LBL_MEDIUM_ADDINFO_VAL, NULL);

	if (0 == activeDrive) return;

	ZeroMemory(&header, sizeof(DM_NOTIFY_PARAM));
	
	header.callback = (INT_PTR)hwndDlg;
	header.uMsg		= msgNotify;
	header.cLetter	= cLetter;
	header.fnFree	= FreeAsyncParam;
    
	// request unitinfo
	pui = (DM_UNITINFO_PARAM*)calloc(1, sizeof(DM_UNITINFO_PARAM));
	if (pui)
	{
		CopyMemory(&pui->header, &header, sizeof(DM_NOTIFY_PARAM));
		pui->header.fFlags = DMF_DESCRIPTION;
		pui->cchDesc = 128;
		pui->pszDesc = (CHAR*)calloc(pui->cchDesc, sizeof(CHAR));
		DriveManager_GetUnitInfo(pui);
	}

	// request unitinfo2
	pui2 = (DM_UNITINFO2_PARAM*)calloc(1, sizeof(DM_UNITINFO2_PARAM));
	if (pui2)
	{
		CopyMemory(&pui2->header, &header, sizeof(DM_NOTIFY_PARAM));
		pui2->header.fFlags = DMF_TYPES;
		pui2->nTypes = 32;
		pui2->pdwTypes = (DWORD*)calloc(pui2->nTypes, sizeof(DWORD));
		DriveManager_GetUnitInfo2(pui2);
	}

	// request discinfoex
	pdi = (DM_DISCINFOEX_PARAM*)calloc(1, sizeof(DM_DISCINFOEX_PARAM));
	if (pdi)
	{
		CopyMemory(&pdi->header, &header, sizeof(DM_NOTIFY_PARAM));
		pdi->header.fFlags = DMF_DRIVEMODE_DAO | DMF_MEDIUMTYPE | DMF_MEDIUMFORMAT | DMF_TRACKS | DMF_USED | DMF_FREE;
		DriveManager_GetDiscInfoEx(pdi);
	}

	// request discinfo2
	pdi2 = (DM_DISCINFO2_PARAM*)calloc(1, sizeof(DM_DISCINFO2_PARAM));
	if (pdi2)
	{
		CopyMemory(&pdi2->header, &header, sizeof(DM_NOTIFY_PARAM));
		pdi2->header.fFlags = DMF_MEDIUM | DMF_MEDIUMEX;
		DriveManager_GetDiscInfo2(pdi2);
	}
}

static BOOL CALLBACK EnumerateNavItemsCB(HNAVITEM hItem, DRIVE *pDrive, LPARAM param)
{
	if (!param) return FALSE;
	if (pDrive) PostMessageW((HWND)param, msgNotify, (WPARAM)DMW_DRIVEADDED, (LPARAM)pDrive->cLetter);
	return TRUE;
}

static void SwitchControlVisible(HWND hwndDlg, INT ctrlId, RECT *prcParent, BOOL hide, BOOL bInvalidate = FALSE)
{
	HWND hwndCtrl = GetDlgItem(hwndDlg, ctrlId);
	
	if (hwndCtrl)
	{
		if (hide) ShowWindow(hwndCtrl, SW_HIDE);
		else
		{
			RECT rc;
			GetWindowRect(hwndCtrl, &rc);
			
			BOOL bVisible = ((prcParent->right > rc.right) && (prcParent->bottom > rc.bottom)); 
			if (bVisible != IsWindowVisible(hwndCtrl)) ShowWindow(hwndCtrl, (bVisible) ? SW_SHOWNORMAL : SW_HIDE);
			if (bVisible && bInvalidate) InvalidateRect(hwndCtrl, NULL, TRUE);
		}
	}
}

static void ripburn_OnDisplayChanges(HWND hwndDlg)
{
	driveListBox->SetColors(dialogSkinner.Color(WADLG_ITEMBG), 
							dialogSkinner.Color(WADLG_ITEMBG),
							dialogSkinner.Color(WADLG_ITEMFG),
							dialogSkinner.Color(WADLG_ITEMFG),
							dialogSkinner.Color(WADLG_WNDFG));
	driveInfo->SetColors(	dialogSkinner.Color(WADLG_ITEMBG), 
							dialogSkinner.Color(WADLG_LISTHEADER_FONTCOLOR),
							dialogSkinner.Color(WADLG_LISTHEADER_BGCOLOR));
	mediumInfo->SetColors(	dialogSkinner.Color(WADLG_ITEMBG), 
							dialogSkinner.Color(WADLG_LISTHEADER_FONTCOLOR),
							dialogSkinner.Color(WADLG_LISTHEADER_BGCOLOR));

	if (lblHeaderBrush) DeleteObject(lblHeaderBrush);
	lblHeaderBrush = NULL;

	if (lblValueBrush) DeleteObject(lblValueBrush);
	lblValueBrush = NULL;

	// fixes the view not updating correctly on colour theme changes, etc
	// NOTE: ideal would be using a LayoutWindows(..) method which would
	//		 help to resolve this as things can be offloaded to gen_ml...
	RECT rc;
	GetClientRect(hwndDlg, &rc);
	RedrawWindow(hwndDlg, &rc, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_ERASENOW | RDW_UPDATENOW);
}

static void ripburn_OnInitDialog(HWND hwndDlg)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_DRIVES);

	driveListBox = new DriveListBox(IDC_LIST_DRIVES);
	driveListBox->SetImages(plugin.hDllInstance, IDB_LISTBOX_BACK, IDB_LISTITEM_CDDRIVE);
	driveListBox->Init(hwndList);

	driveInfo = new MLInfoBox();
	driveInfo->Init(GetDlgItem(hwndDlg, IDC_LBL_INFO_DRIVE));

	mediumInfo = new MLInfoBox();
	mediumInfo->Init(GetDlgItem(hwndDlg, IDC_LBL_INFO_MEDIUM));

	UpdateDriveInfo(hwndDlg, 0);

	childSizer.Init(hwndDlg,ripburn_rlist,sizeof(ripburn_rlist)/sizeof(ripburn_rlist[0]));
	ripburn_OnDisplayChanges(hwndDlg);

	if (!msgNotify) msgNotify = RegisterWindowMessageW(L"ripburn_notify_msg");

	Plugin_EnumerateNavItems(EnumerateNavItemsCB, (LPARAM)hwndDlg);
	Plugin_RegisterListener(hwndDlg, msgNotify, 0);
}

static void ripburn_OnDestroy(HWND hwndDlg)
{
	Plugin_UnregisterListener(hwndDlg);

	HWND hwndLB = GetDlgItem(hwndDlg, IDC_LIST_DRIVES);
	if (hwndLB)
	{
		INT index = (int)(INT_PTR)SendMessageW(hwndLB, LB_GETCURSEL, 0,0);
		DWORD data = (LB_ERR != index) ? (DWORD)SendMessageW(hwndLB, LB_GETITEMDATA, index, 0) : 0; 
		if (data) g_config->WriteInt(L"last_drive", (CHAR)(0xFF & data));
	}

	if (lblHeaderBrush) DeleteObject(lblHeaderBrush);
	lblHeaderBrush = NULL;
	if (lblValueBrush) DeleteObject(lblValueBrush);
	lblValueBrush = NULL;

	if (driveListBox) delete(driveListBox);
	driveListBox = NULL;
	if (driveInfo) delete(driveInfo);
	driveInfo = NULL;
	if (mediumInfo) delete(mediumInfo);
	mediumInfo = NULL;
}

static void ripburn_OnSize(HWND hwndDlg, int cx, int cy)
{	
	RECT box;

	GetWindowRect(GetDlgItem(hwndDlg, IDC_LBL_INFO_DRIVE), &box);
	BOOL hide = FALSE;

	SwitchControlVisible(hwndDlg, IDC_LBL_DRIVE_LETTER, &box, FALSE);
	SwitchControlVisible(hwndDlg, IDC_LBL_DRIVE_DESCRIPTION, &box, FALSE);
	SwitchControlVisible(hwndDlg, IDC_LBL_DRIVE_BUS, &box, FALSE);
	SwitchControlVisible(hwndDlg, IDC_LBL_DRIVE_TYPES,&box, FALSE);
	SwitchControlVisible(hwndDlg, IDC_LBL_DRIVE_LETTER_VAL, &box, hide, TRUE);
	SwitchControlVisible(hwndDlg, IDC_LBL_DRIVE_DESCRIPTION_VAL, &box, hide, TRUE);
	SwitchControlVisible(hwndDlg, IDC_LBL_DRIVE_BUS_VAL, &box, hide, TRUE);
	SwitchControlVisible(hwndDlg, IDC_LBL_DRIVE_TYPES_VAL, &box, hide, TRUE);
	
	GetWindowRect(GetDlgItem(hwndDlg, IDC_LBL_INFO_MEDIUM), &box);
	hide = IsWindowVisible(GetDlgItem(hwndDlg, IDC_LBL_MEDIUM_UPDATE));
	if (hide) InvalidateRect(GetDlgItem(hwndDlg, IDC_LBL_MEDIUM_UPDATE), NULL, TRUE);

	if(PrimoSDKHelper_IsLoaded())
	/*{
		ShowWindow(GetDlgItem(hwndDlg, IDC_LBL_MEDIUM_UPDATE), SW_SHOW);
	}
	else*/
	{
		ShowWindow(GetDlgItem(hwndDlg, IDC_LBL_MEDIUM_UPDATE), SW_HIDE);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_CAPACITY_VAL, &box, hide, TRUE);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_FORMAT_VAL, &box, hide, TRUE);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_ERASEABLE_VAL, &box, hide, TRUE);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_RECORDABLE_VAL,&box, hide, TRUE);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_TRACKN_VAL, &box, hide, TRUE);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_DISC_VAL, &box, hide, TRUE);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_ADDINFO_VAL, &box, hide, TRUE);

		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_TYPE, &box, hide);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_CAPACITY, &box, hide);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_FORMAT, &box, hide);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_ERASEABLE, &box, hide);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_RECORDABLE, &box, hide);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_TRACKN, &box, hide);
		SwitchControlVisible(hwndDlg, IDC_LBL_MEDIUM_ADDINFO, &box, hide);
	}
}

static int LabelColoring(HDC hdc, HWND hwndCtrl)
{
	switch(GetDlgCtrlID(hwndCtrl))
	{
		case IDC_LBL_DRIVES:
			if(!lblHeaderBrush) lblHeaderBrush = CreateSolidBrush(dialogSkinner.Color(WADLG_LISTHEADER_BGCOLOR));
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, dialogSkinner.Color(WADLG_LISTHEADER_FONTCOLOR));
			return (BOOL)(INT_PTR)lblHeaderBrush;
		case IDC_LBL_MEDIUM_NOINFO:
		case IDC_LBL_MEDIUM_CAPACITY_VAL:
		case IDC_LBL_MEDIUM_TRACKN_VAL:
		case IDC_LBL_MEDIUM_ERASEABLE_VAL:
		case IDC_LBL_MEDIUM_RECORDABLE_VAL:
		case IDC_LBL_MEDIUM_FORMAT_VAL:
		case IDC_LBL_MEDIUM_DISC_VAL:
		case IDC_LBL_MEDIUM_ADDINFO_VAL:
		case IDC_LBL_DRIVE_LETTER_VAL:
		case IDC_LBL_DRIVE_DESCRIPTION_VAL:
		case IDC_LBL_DRIVE_BUS_VAL:
		case IDC_LBL_DRIVE_TYPES_VAL:
		case IDC_LBL_MEDIUM_UPDATE:
			if(!lblValueBrush) lblValueBrush = CreateSolidBrush(dialogSkinner.Color(WADLG_ITEMBG));
			SetBkColor(hdc, dialogSkinner.Color(WADLG_ITEMBG));
			SetTextColor(hdc, dialogSkinner.Color(WADLG_ITEMFG));
			return (BOOL)(INT_PTR)lblValueBrush;
		case IDC_LBL_MEDIUM_CAPACITY:
		case IDC_LBL_MEDIUM_TRACKN:
		case IDC_LBL_MEDIUM_ERASEABLE:
		case IDC_LBL_MEDIUM_RECORDABLE:
		case IDC_LBL_MEDIUM_FORMAT:
		case IDC_LBL_MEDIUM_ADDINFO:
		case IDC_LBL_DRIVE_LETTER:
		case IDC_LBL_DRIVE_DESCRIPTION:
		case IDC_LBL_DRIVE_BUS:
		case IDC_LBL_DRIVE_TYPES:
		case IDC_LBL_MEDIUM_TYPE:
			if(!lblValueBrush) lblValueBrush = CreateSolidBrush(dialogSkinner.Color(WADLG_ITEMBG));
			SetBkMode(hdc, TRANSPARENT);
		//	SetBkColor(hdc, dialogSkinner.Color(WADLG_ITEMBG));
			SetTextColor(hdc, dialogSkinner.Color(WADLG_ITEMFG));
			return (BOOL)(INT_PTR)lblValueBrush;
	}
	return FALSE;
}

static void Drive_OnAdded(HWND hwndDlg, CHAR cLetter)
{
	HWND hwndLB = GetDlgItem(hwndDlg, IDC_LIST_DRIVES);
	if (IsWindow(hwndLB))
	{
		wchar_t str[] = {cLetter, 0x00};
		INT index = (INT)SendMessageW(hwndLB, LB_ADDSTRING, 0, (LPARAM)str);
		if (LB_ERR != index)
		{
			SendMessageW(hwndLB, LB_SETITEMDATA, index, (LPARAM)cLetter); 

			INT idxSelection = (int)(INT_PTR)SendMessageW(hwndLB, LB_GETCURSEL, 0,0);
			if (LB_ERR == idxSelection && cLetter == g_config->ReadInt(L"last_drive", cLetter))
			{
				if (LB_ERR != SendMessageW(hwndLB, LB_SETCURSEL, index, 0L))
				{
					UpdateDriveInfo(hwndDlg, cLetter);
				}
			}
			
			// request unitinfo
			DM_UNITINFO_PARAM *pui = (DM_UNITINFO_PARAM*)calloc(1, sizeof(DM_UNITINFO_PARAM));
			if (pui)
			{
				pui->header.callback = (INT_PTR)hwndDlg;
				pui->header.uMsg = msgNotify;
				pui->header.cLetter = cLetter;
				pui->header.fnFree = FreeAsyncParam;
				DriveManager_GetUnitInfo(pui);
			}
		}
	}
}

static INT GetListBoxIndex(HWND hwndLB, CHAR cLetter)
{
	wchar_t str[] = {cLetter, 0x00};
	return (cLetter && hwndLB) ? (INT)SendMessageW(hwndLB, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)str) : LB_ERR;
}

static void Drive_OnRemoved(HWND hwndDlg, CHAR cLetter)
{
	HWND hwndLB = GetDlgItem(hwndDlg, IDC_LIST_DRIVES);

	if (IsWindow(hwndLB))
	{
		INT index = GetListBoxIndex(hwndLB, cLetter);
		if (LB_ERR != index) SendMessageW(hwndLB, LB_DELETESTRING, (WPARAM)index, 0L);
	}
}

static void GetInfo_Completed(HWND hwndDlg, DM_NOTIFY_PARAM *phdr)
{
	wchar_t szBuffer[256] = {0};

	DM_UNITINFO_PARAM *pui = NULL;
	DM_UNITINFO2_PARAM *pui2 = NULL;
	DM_DISCINFOEX_PARAM *pdi = NULL;
	DM_DISCINFO2_PARAM *pdi2 = NULL;

	switch(phdr->opCode)
	{
		case DMOP_UNITINFO:
			pui = (DM_UNITINFO_PARAM*)phdr;
			if (0 == phdr->result)
			{	
				HWND hwndLB = GetDlgItem(hwndDlg, IDC_LIST_DRIVES);
				if (hwndLB)
				{
					INT idxLB = GetListBoxIndex(hwndLB, phdr->cLetter);
					if (LB_ERR != idxLB) 
					{
						DWORD data = MAKELONG(phdr->cLetter, pui->dwType);
						if (data != (DWORD)SendMessage(hwndLB ,LB_GETITEMDATA, idxLB, 0))
						{
							if (LB_ERR != SendMessageW(hwndLB, LB_SETITEMDATA, idxLB, (LPARAM)data))
							{
								RECT rc;
								SendMessageW(hwndLB, LB_GETITEMRECT, idxLB, (LPARAM)&rc);
								InvalidateRect(hwndLB, &rc, FALSE);
								UpdateWindow(hwndLB);
							}
						}
					}
				}
				if (activeDrive == phdr->cLetter && pui->pszDesc) SetDlgItemTextA(hwndDlg, IDC_LBL_DRIVE_DESCRIPTION_VAL, (pui->cchDesc > 0) ? pui->pszDesc : "");
			}
			break;
		case DMOP_UNITINFO2:
			pui2 = (DM_UNITINFO2_PARAM*)phdr;
			if (0 == phdr->result && activeDrive == phdr->cLetter)
			{				
				SetDlgItemTextW(hwndDlg, IDC_LBL_DRIVE_BUS_VAL, Drive_GetBusTypeString(pui2->dwBusType));
				szBuffer[0] = 0x00;
				for (int i = 0; i < pui2->nTypes; i++)
				{
					if (0 != i) StringCchCatW(szBuffer, sizeof(szBuffer)/sizeof(wchar_t), L", "); 
					StringCchCatW(szBuffer, sizeof(szBuffer)/sizeof(wchar_t), Drive_GetTypeString(pui2->pdwTypes[i]));
				}
				SetDlgItemTextW(hwndDlg, IDC_LBL_DRIVE_TYPES_VAL, szBuffer);
			}
			break;
		case DMOP_DISCINFO:
			pdi = (DM_DISCINFOEX_PARAM*)phdr;
			if (0 == phdr->result && activeDrive == phdr->cLetter)
			{
				StringCchPrintfW(szBuffer, sizeof(szBuffer)/sizeof(wchar_t),
								 WASABI_API_LNGSTRINGW(IDS_X_OF_X_SECTORS_FREE),
								 pdi->dwFree, pdi->dwUsed +  pdi->dwFree);
				SetDlgItemTextW(hwndDlg, IDC_LBL_MEDIUM_CAPACITY_VAL, szBuffer);
				SetDlgItemInt(hwndDlg, IDC_LBL_MEDIUM_TRACKN_VAL, pdi->dwTracks, FALSE);
				SetDlgItemText(hwndDlg, IDC_LBL_MEDIUM_ERASEABLE_VAL, WASABI_API_LNGSTRINGW((pdi->bErasable) ? IDS_YES : IDS_NO));
				SetDlgItemText(hwndDlg, IDC_LBL_MEDIUM_RECORDABLE_VAL, WASABI_API_LNGSTRINGW((Medium_IsRecordableType(pdi->dwMediumType)) ? IDS_YES : IDS_NO));
				SetDlgItemText(hwndDlg, IDC_LBL_MEDIUM_ADDINFO_VAL, Medium_GetTypeString(pdi->dwMediumType));
				SetDlgItemText(hwndDlg, IDC_LBL_MEDIUM_FORMAT_VAL, Medium_GetFormatString(pdi->dwMediumFormat));
			}
			break;
		case DMOP_DISCINFO2:
			pdi2 = (DM_DISCINFO2_PARAM*)phdr;
			if (0 == phdr->result && activeDrive == phdr->cLetter)
			{
				SetDlgItemTextW(hwndDlg, IDC_LBL_MEDIUM_DISC_VAL, Medium_GetPhysicalTypeString(pdi2->dwMediumEx));
			}
			break;
	}
}

static void View_OnPluginNotify(HWND hwndDlg, WORD wCode, INT_PTR param)
{
	switch(wCode)
	{
		case DMW_DRIVEADDED:	Drive_OnAdded(hwndDlg, (CHAR)param); break;
		case DMW_DRIVEREMOVED:	Drive_OnRemoved(hwndDlg, (CHAR)param); break;
		case DMW_MEDIUMARRIVED:
		case DMW_MEDIUMREMOVED:	if ((CHAR)param == activeDrive) UpdateDriveInfo(hwndDlg, activeDrive); break;
		case DMW_OPCOMPLETED:
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			GetInfo_Completed(hwndDlg, (DM_NOTIFY_PARAM*)param);
			break;	
	}
}

static INT_PTR ListBox_OnKeyPressed(HWND hwndDlg, HWND hwndLB, WORD wKey, INT iCurret)
{
	switch(wKey)
	{
		case VK_F5: DriveManager_Update(TRUE); return -2;
		case VK_SPACE: 
			PostMessageW(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_LIST_DRIVES,LBN_DBLCLK), (LPARAM)hwndLB); 
			return -2;
	}
	if (wKey >= 'A' && wKey <= 'Z') 
	{
		INT index = GetListBoxIndex(hwndLB, (CHAR)wKey);
		return (LB_ERR != index) ? index : -2;
	}

	return -1; // do default
}

INT_PTR CALLBACK view_ripburnDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	INT_PTR a;

	if (uMsg == WM_CTLCOLORSTATIC ) 
	{
		a = LabelColoring((HDC)wParam, (HWND) lParam);
		if (a) return a;
	}

	a = driveListBox->HandleMsgProc(uMsg,wParam,lParam); if (a) return a;
	a = dialogSkinner.Handle(hwndDlg,uMsg,wParam,lParam); if (a) return a;

	switch(uMsg)
	{
		case WM_INITDIALOG:
			ripburn_OnInitDialog(hwndDlg);
			break;
		case WM_DISPLAYCHANGE:
			ripburn_OnDisplayChanges(hwndDlg);
			break;
		case WM_SIZE:
			if (wParam != SIZE_MINIMIZED) 
			{
				childSizer.Resize(hwndDlg,ripburn_rlist,sizeof(ripburn_rlist)/sizeof(ripburn_rlist[0]));
				ripburn_OnSize(hwndDlg, LOWORD(lParam), HIWORD(lParam));
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_LIST_DRIVES:
					if (HIWORD(wParam) == LBN_SELCHANGE)
					{
						INT index = (int)(INT_PTR)SendMessage((HWND)lParam, LB_GETCURSEL, 0,0);
						DWORD data = (LB_ERR != index) ? (DWORD)SendMessage((HWND)lParam ,LB_GETITEMDATA, index, 0) : 0; 
						if (data) UpdateDriveInfo(hwndDlg, (CHAR)(0xFF & data));
					}
					else if (HIWORD(wParam) == LBN_DBLCLK)
					{
						INT index = (int)(INT_PTR)SendMessage((HWND)lParam, LB_GETCURSEL, 0,0);
						DWORD data = (LB_ERR != index) ? (DWORD)SendMessage((HWND)lParam ,LB_GETITEMDATA, index, 0) : 0; 
						HNAVITEM hItem = (data) ? Plugin_GetNavItemFromLetter((CHAR)(0xFF & data)) : NULL;
						if (hItem) MLNavItem_Select(plugin.hwndLibraryParent, hItem);
					}
					break;
				case IDC_BTN_REFRESH:
					if (HIWORD(wParam) == BN_CLICKED) DriveManager_Update(TRUE);
					break;
			}
			break;
		case WM_PAINT:
			{
				int tab[] = { IDC_LIST_DRIVES | DCW_SUNKENBORDER,
							  IDC_LBL_DRIVES | DCW_SUNKENBORDER,
							  IDC_LBL_INFO_DRIVE | DCW_SUNKENBORDER,
							  IDC_LBL_INFO_MEDIUM | DCW_SUNKENBORDER};
				dialogSkinner.Draw(hwndDlg, tab, 4);
			}
			return 0;
		case WM_DESTROY:
			ripburn_OnDestroy(hwndDlg);
			break;
		case WM_ERASEBKGND:
			return 0;
		case WM_VKEYTOITEM:
			return ListBox_OnKeyPressed(hwndDlg, (HWND)lParam, LOWORD(wParam), HIWORD(wParam));
	}
	if (msgNotify == uMsg)
		View_OnPluginNotify(hwndDlg, (WORD)wParam, (INT_PTR)lParam);
	
	return FALSE;
}