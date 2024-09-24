#include "main.h"
#include "./skinnedmenu.h"
#include "./skinnedmenuwnd.h"


SkinnedMenu::SkinnedMenu()
{	
	hwndOwner = NULL;
	skinStyle = SMS_NORMAL;
	hmlil = NULL;
	width = 0;

	if (FAILED(SkinnedMenuThreadInfo::GetInstance(TRUE, &threadInfo)))
		threadInfo = NULL;
}

SkinnedMenu::~SkinnedMenu(void)
{	
	if (NULL != threadInfo)
	{
		threadInfo->RemoveAttachHook(this);
		threadInfo->Release();
	}
}

HWND SkinnedMenu::WindowFromHandle(HMENU menu)
{	
	HWND hwnd;
	SkinnedMenuThreadInfo *threadInfo;

	if (S_OK != SkinnedMenuThreadInfo::GetInstance(FALSE, &threadInfo))
		return NULL;

	hwnd = threadInfo->FindMenuWindow(menu);

	threadInfo->Release();

	return hwnd;
}

BOOL SkinnedMenu::InitializeHook(HWND hwndOwner, UINT skinStyle, HMLIMGLST hmlil, INT width, MENUCUSTOMIZEPROC _customProc, ULONG_PTR customParam)
{
	if (NULL == threadInfo)
		return FALSE;

	if (FALSE != threadInfo->IsAttachHookActive())
		return FALSE;
	
	this->hwndOwner = hwndOwner;
	this->hmlil = hmlil;
	this->width = width;
	this->skinStyle = skinStyle;
	this->customProc = _customProc;
	this->customParam = customParam;

	return threadInfo->SetAttachHook(this);
}

BOOL SkinnedMenu::TrackMenuPopupEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, UINT skinStyle, 
								   HMLIMGLST hmlil, INT width, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam)
{
	if (NULL == hwnd || 
		!InitializeHook(hwnd, skinStyle, hmlil, width, customProc, customParam)) 
	{
		return FALSE;
	}
	
	return TrackPopupMenuEx(hmenu, fuFlags, x, y, hwnd, lptpm);
}

BOOL SkinnedMenu::AttachToHwnd(HWND hwndMenu)
{
	SkinnedMenuWnd *psw = new SkinnedMenuWnd(skinStyle, hmlil, width, customProc, customParam);
	if (!psw || !psw->Attach(hwndMenu, hwndOwner))
	{
		delete(psw);
		return FALSE;
	}

	return TRUE;
}
