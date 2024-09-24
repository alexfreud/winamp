#include "./setupPage.h"
#include "./setupListbox.h"
#include "../common.h"
#include "../resource.h"
#include "../api__ml_online.h"

static ATOM SETUPPAGE_PROP = 0;

#define GetPage(__hwnd) ((SetupPage*)GetPropW((__hwnd), MAKEINTATOM(SETUPPAGE_PROP)))

static INT_PTR WINAPI SetupPage_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



HWND SetupPage_CreateWindow(HWND hParent, SetupPage *page)
{
	return WASABI_API_CREATEDIALOGPARAMW(IDD_SETUPPAGE, hParent, SetupPage_DialogProc, (LPARAM)page);
}

static INT_PTR SetupPage_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM lParam)
{
	if (0 == SETUPPAGE_PROP)
	{
		SETUPPAGE_PROP = GlobalAddAtom(L"omSetupPageProp");
		if (0 == SETUPPAGE_PROP) return FALSE;
	}

	SetupPage *page = (SetupPage*)lParam;
	if (NULL != page && page->AttachWindow(hwnd))
	{
		SetProp(hwnd, MAKEINTATOM(SETUPPAGE_PROP), page);
	}

	return FALSE;
}

static void SetupPage_OnDestroy(HWND hwnd)
{
	SetupPage *page = GetPage(hwnd);
	if (NULL != page)
	{
		page->DetachWindow();
	}
	RemoveProp(hwnd, MAKEINTATOM(SETUPPAGE_PROP));
}

static void SetupPage_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	SetupPage *page;
	switch(controlId)
	{
		case IDC_SERVICELIST:
			switch(eventId)
			{
				case LBN_SELCHANGE:
					page = GetPage(hwnd);
					if (NULL != page)
						page->ListboxSelectionChanged();
					break;
			}
			break;
	}
}

static BOOL SetupPage_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *pmis)
{
	SetupListbox *instance;
	switch(pmis->CtlID)
	{
		case IDC_SERVICELIST:
			instance = SetupListbox::GetInstance(GetDlgItem(hwnd, pmis->CtlID));
			if (NULL != instance)
			{
				return instance->MeasureItem(pmis->itemID, &pmis->itemWidth, &pmis->itemHeight);
			}
			break;
	}
	return FALSE;
}

static BOOL SetupPage_OnDrawItem(HWND hwnd, DRAWITEMSTRUCT *pdis)
{
	SetupListbox *instance;
	switch(pdis->CtlID)
	{
		case IDC_SERVICELIST:
			instance = SetupListbox::GetInstance(pdis->hwndItem);
			if (NULL != instance)
			{
				return instance->DrawItem(pdis->hDC, &pdis->rcItem, pdis->itemID, pdis->itemState, pdis->itemAction); 
			}
			break;				
	}
	return FALSE;
}

static INT_PTR SetupPage_OnCharToItem(HWND hwnd, INT vKey, INT caretPos, HWND hList)
{
	
	if (IDC_SERVICELIST == GetDlgCtrlID(hList)) 
	{
		SetupListbox *instance = SetupListbox::GetInstance(hList);
		if (NULL != instance)  
			return instance->CharToItem(vKey, caretPos);
	}
	return -1;
}

static INT_PTR SetupPage_OnKeyToItem(HWND hwnd, INT vKey, INT caretPos, HWND hList)
{
	if (IDC_SERVICELIST == GetDlgCtrlID(hList)) 
	{
		SetupListbox *instance = SetupListbox::GetInstance(hList);
		if (NULL != instance)  
			return instance->KeyToItem(vKey, caretPos);
	}
	return -1;
}
static void SetupPage_OnUpdateList(HWND hwnd, INT groupId)
{
	HWND hList = GetDlgItem(hwnd, IDC_SERVICELIST);
	if (NULL != hList)
	{	
		SetupListbox *listbox = SetupListbox::GetInstance(hList);
		if (NULL != listbox)
		{
			listbox->UpdateCount();
		}
	}
}
static INT_PTR WINAPI SetupPage_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:		return SetupPage_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:			SetupPage_OnDestroy(hwnd); break;
		case WM_COMMAND:			SetupPage_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_MEASUREITEM:		return SetupPage_OnMeasureItem(hwnd, (MEASUREITEMSTRUCT*)lParam);
		case WM_DRAWITEM:		return SetupPage_OnDrawItem(hwnd, (DRAWITEMSTRUCT*)lParam);
		case WM_CHARTOITEM:		return SetupPage_OnCharToItem(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		case WM_VKEYTOITEM:		return SetupPage_OnKeyToItem(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		case SPM_UPDATELIST:		SetupPage_OnUpdateList(hwnd, (INT)wParam); return TRUE;
	}
	return 0;
}