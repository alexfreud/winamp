#include "main.h"
#include "./skinnedMenuThreadInfo.h"
#include "./skinnedMenu.h"
#include "./skinnedMenuWnd.h"
#ifndef _DEBUG
  #include <new>
#endif

static DWORD tlsIndex = TLS_OUT_OF_INDEXES;

	size_t ref;
	HHOOK attachHook;
	SkinnedMenu *attachMenu;
	HHOOK validationHook;
	SkinnedMenuWnd *validationWindow;
	khash_t(intptr_map) *windowMap;
	khash_t(int_set) *claimedIdSet;
	unsigned int lastAssignedId;
	HMENU  activeMeasureMenu;

SkinnedMenuThreadInfo::SkinnedMenuThreadInfo()
	: ref(1), attachHook(NULL), attachMenu(NULL), validationHook(NULL),
	  validationWindow(NULL), lastAssignedId((unsigned int)-100),
	  activeMeasureMenu(NULL)
{
	windowMap = kh_init(intptr_map);
	claimedIdSet = kh_init(int_set);
}

SkinnedMenuThreadInfo::~SkinnedMenuThreadInfo()
{
	if (TLS_OUT_OF_INDEXES != tlsIndex)
		TlsSetValue(tlsIndex, NULL);

	if (NULL != attachHook) 
	{
		UnhookWindowsHookEx(attachHook);
		attachHook = NULL;
	}

	if (NULL != validationHook) 
	{
		UnhookWindowsHookEx(validationHook);
		validationHook = NULL;
	}

	if (NULL != windowMap)
		kh_destroy(intptr_map, windowMap);

	if (NULL != claimedIdSet)
		kh_destroy(int_set, claimedIdSet);
}

HRESULT SkinnedMenuThreadInfo::GetInstance(BOOL allowCreate, SkinnedMenuThreadInfo **instance)
{
	HRESULT hr;
	SkinnedMenuThreadInfo *self;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (TLS_OUT_OF_INDEXES == tlsIndex)
	{
		tlsIndex = TlsAlloc();
		if (TLS_OUT_OF_INDEXES == tlsIndex) 
			return E_OUTOFMEMORY;

		self = NULL;

		if (FALSE != TlsSetValue(tlsIndex, NULL))
			SetLastError(ERROR_SUCCESS);
	}
	else
	{
		self = (SkinnedMenuThreadInfo*)TlsGetValue(tlsIndex);
	}

	if (NULL == self)
	{
		unsigned long errorCode;
		errorCode = GetLastError();
		if (ERROR_SUCCESS != errorCode)
		{
			hr = HRESULT_FROM_WIN32(errorCode);
		}
		else
		{
			if (FALSE == allowCreate)
			{
				self = NULL;
				hr = S_FALSE;
			}
			else
			{
				#ifndef _DEBUG
				self = new (std::nothrow) SkinnedMenuThreadInfo();
				#else
				self = new SkinnedMenuThreadInfo();
				#endif
				if (NULL == self)
					hr = E_OUTOFMEMORY;
				else
				{
					if (FALSE == TlsSetValue(tlsIndex, self))
					{
						errorCode = GetLastError();
						hr = HRESULT_FROM_WIN32(errorCode);
						self->Release();
						self = NULL;
					}
					else
					{
						hr = S_OK;
					}
				}
			}
		}
	}
	else
	{
		self->AddRef();
		hr = S_OK;
	}

	*instance = self;
	return hr;
}

size_t SkinnedMenuThreadInfo::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t SkinnedMenuThreadInfo::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

BOOL SkinnedMenuThreadInfo::SetAttachHook(SkinnedMenu *menu)
{
	if (NULL == menu)
		return FALSE;

	if (NULL != attachHook)
		return FALSE;

	attachMenu = menu;
	attachHook = SetWindowsHookEx(WH_CALLWNDPROC, SkinnedMenuThreadInfo_AttachHookCb, NULL, GetCurrentThreadId()); 
	if (NULL == attachHook) 
	{
		attachMenu = FALSE;
		return FALSE;
	}

	return TRUE;
}

BOOL SkinnedMenuThreadInfo::RemoveAttachHook(SkinnedMenu *menu)
{
	if (NULL == attachHook)
		return FALSE;

	if (menu != attachMenu)
		return FALSE;

	UnhookWindowsHookEx(attachHook);

	attachHook = NULL;
	attachMenu = NULL;
				
	return TRUE;
}

BOOL SkinnedMenuThreadInfo::IsAttachHookActive()
{
	return (NULL != attachHook);
}

LRESULT SkinnedMenuThreadInfo::AttachHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (HC_ACTION == nCode)
	{		
		CWPSTRUCT *pcwp = (CWPSTRUCT*)lParam;
		if (WM_NCCREATE == pcwp->message)
		{
			wchar_t szName[128] = {0};
			if (GetClassNameW(pcwp->hwnd, szName, ARRAYSIZE(szName)) && 
				CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, szName, -1, L"#32768", -1))
			{
				LRESULT result;
				HHOOK hookCopy;
				SkinnedMenu *menuCopy;

				menuCopy = attachMenu;
				hookCopy = attachHook;

				attachHook = NULL;
				attachMenu = NULL;

				result = CallNextHookEx(hookCopy, nCode, wParam, lParam);
				UnhookWindowsHookEx(hookCopy);
						
				if (NULL != menuCopy)
					menuCopy->AttachToHwnd(pcwp->hwnd);
				
				return result;
			}
		}
	}

	return CallNextHookEx(attachHook, nCode, wParam, lParam);
}

BOOL SkinnedMenuThreadInfo::SetValidationHook(SkinnedMenuWnd *window)
{
	HMENU prevMenu;

	if (NULL == window)
		return FALSE;

	if (NULL != validationHook)
		return FALSE;

	validationWindow = window;
	prevMenu = SetActiveMeasureMenu(window->GetMenuHandle());

	validationHook = SetWindowsHookEx(WH_CALLWNDPROC, SkinnedMenuThreadInfo_ValidationHookCb, NULL, GetCurrentThreadId()); 
	if (NULL == validationHook) 
	{
		validationWindow = FALSE;
		SetActiveMeasureMenu(prevMenu);
		return FALSE;
	}
	
	return TRUE;
}

BOOL SkinnedMenuThreadInfo::RemoveValidationHook(SkinnedMenuWnd *window)
{
	if (NULL == validationHook)
		return FALSE;

	if (window != validationWindow)
		return FALSE;


	UnhookWindowsHookEx(validationHook);

	validationWindow = NULL;
	validationHook = NULL;
				
	return TRUE;
}

BOOL SkinnedMenuThreadInfo::IsValidationHookActive()
{
	return (NULL != validationHook);
}

LRESULT SkinnedMenuThreadInfo::ValidationHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (HC_ACTION == nCode)
	{		
		CWPSTRUCT *pcwp = (CWPSTRUCT*)lParam;

		if (WM_MEASUREITEM == pcwp->message ||
			WM_DRAWITEM == pcwp->message)
		{			
			if (NULL != validationWindow && NULL != pcwp->lParam)
			{
				BOOL validationCompleted(FALSE);

				if (WM_MEASUREITEM == pcwp->message)
				{
					MEASUREITEMSTRUCT *measureItem;
					measureItem = (MEASUREITEMSTRUCT*)pcwp->lParam;
					if (ODT_MENU == measureItem->CtlType && 
						validationWindow->GetMenuHandle() == GetActiveMeasureMenu())
					{
						validationCompleted = TRUE;
					}
				}
				else
				{
					DRAWITEMSTRUCT *drawItem;
					drawItem = (DRAWITEMSTRUCT*)pcwp->lParam;

					if (ODT_MENU == drawItem->CtlType && 
						validationWindow->GetMenuHandle() == (HMENU)drawItem->hwndItem)
					{
						validationCompleted = TRUE;
					}
				}
			
				if (FALSE != validationCompleted)
				{
					LRESULT result;
					HHOOK hookCopy;
					SkinnedMenuWnd *windowCopy;

					windowCopy = validationWindow;
					hookCopy = validationHook;

					validationHook = NULL;
					validationWindow = NULL;

					if (NULL != windowCopy && windowCopy->GetOwnerWindow() != pcwp->hwnd)
					{
						windowCopy->SetOwnerWindow(pcwp->hwnd);
					}

					result = CallNextHookEx(hookCopy, nCode, wParam, lParam);
					UnhookWindowsHookEx(hookCopy);
					

					return result;
				}
			}
		}
	}

	return CallNextHookEx(attachHook, nCode, wParam, lParam);
}

BOOL SkinnedMenuThreadInfo::RegisterMenu(HMENU menu, HWND window)
{
	int code;
	khint_t key;

	if (NULL == menu)
		return FALSE;

	if (NULL == windowMap)
		return FALSE;

	key = kh_put(intptr_map, windowMap, (intptr_t)menu, &code);
	kh_val(windowMap, key) = window;
	
	return TRUE;
}

BOOL SkinnedMenuThreadInfo::UnregisterMenu(HMENU menu)
{
	khint_t key;

	if (NULL == menu)
		return FALSE;

	if (NULL == windowMap)
		return FALSE;
	
	key = kh_get(intptr_map, windowMap, (intptr_t)menu);
	if (kh_end(windowMap) == key)
		return FALSE;

	kh_del(intptr_map, windowMap, key);
	return TRUE;
}

HWND SkinnedMenuThreadInfo::FindMenuWindow(HMENU menu)
{
	khint_t key;

	if (NULL == menu)
		return NULL;

	if (NULL == windowMap)
		return NULL;
	
	key = kh_get(intptr_map, windowMap, (intptr_t)menu);
	if (kh_end(windowMap) == key)
		return NULL;

	return kh_val(windowMap, key);
}

void SkinnedMenuThreadInfo::ClaimId(unsigned int id)
{
	int code;
	kh_put(int_set, claimedIdSet, id, &code);
}

void SkinnedMenuThreadInfo::ReleaseId(unsigned int id)
{
	khint_t key;
	key = kh_get(int_set, claimedIdSet, id);
	if (kh_end(claimedIdSet) != key)
		kh_del(int_set, claimedIdSet, key);
}

unsigned int SkinnedMenuThreadInfo::GetAvailableId()
{
	khint_t key;
	unsigned int originalId;
		
	lastAssignedId--;
	if ((unsigned int)-1 == lastAssignedId)
		lastAssignedId--;

	originalId = lastAssignedId;

	for(;;)
	{
		key = kh_get(int_set, claimedIdSet, lastAssignedId);
		if (kh_end(claimedIdSet) == key)
			return lastAssignedId;

		lastAssignedId--;
		if ((unsigned int)-1 == lastAssignedId)
			lastAssignedId--;

		if (lastAssignedId == originalId)
			break;
	}

	return (unsigned int)-1;
}

HMENU SkinnedMenuThreadInfo::SetActiveMeasureMenu(HMENU menu)
{
	HMENU prevMenu;
	prevMenu = activeMeasureMenu;
	activeMeasureMenu = menu;
	return prevMenu;

}

HMENU SkinnedMenuThreadInfo::GetActiveMeasureMenu()
{
	return activeMeasureMenu;
}

static LRESULT CALLBACK SkinnedMenuThreadInfo_AttachHookCb(int nCode, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	SkinnedMenuThreadInfo *self;

	if (S_OK != SkinnedMenuThreadInfo::GetInstance(FALSE, &self))
		return 0;

	result = self->AttachHook(nCode, wParam, lParam);

	self->Release();
	return result;
}

static LRESULT CALLBACK SkinnedMenuThreadInfo_ValidationHookCb(int nCode, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	SkinnedMenuThreadInfo *self;

	if (S_OK != SkinnedMenuThreadInfo::GetInstance(FALSE, &self))
		return 0;

	result = self->ValidationHook(nCode, wParam, lParam);

	self->Release();
	return result;
}