#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_MENU_THREAD_INFO_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_MENU_THREAD_INFO_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./klib/khash.h"

#ifdef  _WIN64
 KHASH_MAP_INIT_INT64(intptr_map, HWND);
#else
 KHASH_MAP_INIT_INT(intptr_map, HWND);
#endif

KHASH_SET_INIT_INT(int_set)

class SkinnedMenu;
class SkinnedMenuWnd;

class SkinnedMenuThreadInfo
{
protected:
	SkinnedMenuThreadInfo();
	~SkinnedMenuThreadInfo();

public:
	static HRESULT GetInstance(BOOL allowCreate, SkinnedMenuThreadInfo **instance);

public:
	size_t AddRef();
	size_t Release();

	BOOL SetAttachHook(SkinnedMenu *menu);
	BOOL RemoveAttachHook(SkinnedMenu *menu);
	BOOL IsAttachHookActive();

	BOOL SetValidationHook(SkinnedMenuWnd *window);
	BOOL RemoveValidationHook(SkinnedMenuWnd *window);
	BOOL IsValidationHookActive();
	
	BOOL RegisterMenu(HMENU menu, HWND window); 
	BOOL UnregisterMenu(HMENU menu);
	HWND FindMenuWindow(HMENU menu);

	void ClaimId(unsigned int id);
	void ReleaseId(unsigned int id);
	unsigned int GetAvailableId();

	HMENU SetActiveMeasureMenu(HMENU menu);
	HMENU GetActiveMeasureMenu();

protected:
	LRESULT AttachHook(int nCode, WPARAM wParam, LPARAM lParam);
	LRESULT ValidationHook(int nCode, WPARAM wParam, LPARAM lParam);

protected:
	friend static LRESULT CALLBACK SkinnedMenuThreadInfo_AttachHookCb(int nCode, WPARAM wParam, LPARAM lParam);
	friend static LRESULT CALLBACK SkinnedMenuThreadInfo_ValidationHookCb(int nCode, WPARAM wParam, LPARAM lParam);

protected:
	size_t ref;
	HHOOK attachHook;
	SkinnedMenu *attachMenu;
	HHOOK validationHook;
	SkinnedMenuWnd *validationWindow;
	khash_t(intptr_map) *windowMap;
	khash_t(int_set) *claimedIdSet;
	unsigned int lastAssignedId;
	HMENU  activeMeasureMenu;
};

#endif //NULLOSFT_MEDIALIBRARY_SKINNED_MENU_THREAD_INFO_HEADER