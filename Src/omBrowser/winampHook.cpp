#include "main.h"
#include "./winampHook.h"
#include "./ifc_winamphook.h"
#include "../winamp/wa_ipc.h"

#include <windows.h>

#define WINAMP_REFRESHSKIN              40291
#define WHPROCRECOVERY					L"WaHookProcRecovery"

static ATOM WAWNDATOM = 0;
#define GetWinampHook(__hwnd) ((WinampHook*)GetProp((__hwnd), MAKEINTATOM(WAWNDATOM)))

WinampHook::WinampHook(HWND hwndWinamp)
	: ref(1), hwnd(hwndWinamp), originalProc(NULL), flags(0), lastCookie(0)
{
}

WinampHook::~WinampHook()
{
	DetachFromWinamp();

	if (0 != WAWNDATOM)
	{
		GlobalDeleteAtom(WAWNDATOM);
		WAWNDATOM = 0;
	}
}

HRESULT WinampHook::CreateInstance(HWND hwndWinamp, WinampHook **instance)
{
	if (NULL == instance) return E_POINTER;

	*instance = NULL;

	if (NULL == hwndWinamp || FALSE == IsWindow(hwndWinamp))
		return E_INVALIDARG;

	*instance = new WinampHook(hwndWinamp);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

ULONG WinampHook::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG WinampHook::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

HRESULT WinampHook::RegisterCallback(ifc_winamphook *callback, UINT *cookie)
{
	if (NULL == cookie) return E_POINTER;
	*cookie = 0;

	if (NULL == callback) return E_INVALIDARG;

	if (FAILED(AttachToWinamp()))
		return E_UNEXPECTED;

	*cookie = ++lastCookie;

	callbackMap.insert({ *cookie, callback });
	callback->AddRef();

	return S_OK;
}

HRESULT WinampHook::UnregisterCallback(UINT cookie)
{
	if (0 == cookie) return E_INVALIDARG;

	for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
	{
		if (cookie == iter->first)
		{
			ifc_winamphook *hook = iter->second;
			callbackMap.erase(iter);
			
			if (NULL != hook) 
				hook->Release();

			return S_OK;
		}
	}
	return S_FALSE;
}

HWND WinampHook::GetWinamp()
{
	return hwnd;
}

LRESULT WinampHook::CallPrevWinampProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (NULL == originalProc || NULL == hwnd) return 0;
	return (0 != (flagUnicode & flags)) ? 
		CallWindowProcW(originalProc, hwnd, uMsg, wParam, lParam) : 
		CallWindowProcA(originalProc, hwnd, uMsg, wParam, lParam);
}

LRESULT WinampHook::CallDefWinampProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (NULL == hwnd) return 0;
	return (0 != (flagUnicode & flags)) ? 
		DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
		DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

HRESULT WinampHook::AttachToWinamp()
{
	if (NULL == hwnd || !IsWindow(hwnd)) return E_UNEXPECTED;

	if (0 == WAWNDATOM)
	{
		 WAWNDATOM = GlobalAddAtom(L"WinampHook");
		 if (0 == WAWNDATOM) return E_FAIL;
	}

	WinampHook *hookInstance = GetWinampHook(hwnd);
	if (this == hookInstance) return S_FALSE;

	if (NULL != hookInstance || NULL != originalProc) 
		return E_FAIL;

	flags = 0;
	if (IsWindowUnicode(hwnd)) flags |= flagUnicode;

	originalProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)WinampWindowProc);
	if (NULL == originalProc || FALSE == SetProp(hwnd, MAKEINTATOM(WAWNDATOM), this))
	{
		if (NULL != originalProc)
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)originalProc);
		return E_FAIL;
	}

	return S_OK;
}

HRESULT WinampHook::DetachFromWinamp()
{
	if (NULL == hwnd || !IsWindow(hwnd)) return E_UNEXPECTED;
	if (0 == WAWNDATOM) return E_FAIL;

	for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
	{
		if (NULL != iter->second) 
			iter->second->Release();
	}
	callbackMap.clear();

	WinampHook *hookInstance = GetWinampHook(hwnd);
	if (this != hookInstance) return E_FAIL;

	RemoveProp(hwnd, MAKEINTATOM(WAWNDATOM));
	WNDPROC currentProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
	if (currentProc == WinampWindowProc)
	{
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)originalProc);
	}
	else
	{
		SetProp(hwnd, WHPROCRECOVERY, (HANDLE)originalProc);
	}
	originalProc = NULL;
	flags = 0;
	return S_OK;
}

LRESULT WinampHook::OnWinampDestroy()
{
	WNDPROC proc = originalProc;
	DetachFromWinamp();
	return (IsWindowUnicode(hwnd)) ? 
			CallWindowProcW(proc, hwnd, WM_DESTROY, 0, 0L) : 
			CallWindowProcA(proc, hwnd, WM_DESTROY, 0, 0L);
}

LRESULT WinampHook::OnWinampIPC(UINT commandId, WPARAM param)
{
	switch(commandId)
	{
		case IPC_CB_RESETFONT:
			for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
				if (NULL != iter->second) iter->second->ResetFont();
			break;

		case IPC_HOOK_OKTOQUIT:
			for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
				if (NULL != iter->second && S_FALSE == iter->second->IsQuitAllowed()) return 0;
			break;

		case IPC_SKIN_CHANGED:
			{
				WCHAR szBuffer[MAX_PATH*2] = {0};
				SENDWAIPC(hwnd, IPC_GETSKINW, szBuffer);
				for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
					if (NULL != iter->second) iter->second->SkinChanged(szBuffer);
			}
			break;
		
		case IPC_FF_ONCOLORTHEMECHANGED:
			if (FALSE != IS_INTRESOURCE(param))
				param = 0L;
			for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
				if (NULL != iter->second) iter->second->SkinColorChange((LPCWSTR)param);
			break;

		case IPC_FILE_TAG_MAY_HAVE_UPDATED:
			{
				WCHAR szBuffer[MAX_PATH*2] = {0};
				param = (0 != MultiByteToWideChar(CP_ACP, 0, (char*)param, -1, szBuffer, ARRAYSIZE(szBuffer))) ?
						(WPARAM)szBuffer : 0L;

				for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
					if (NULL != iter->second) iter->second->FileMetaChange((LPCWSTR)param);
			}
			break;

		case IPC_FILE_TAG_MAY_HAVE_UPDATEDW:
			for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
				if (NULL != iter->second) iter->second->FileMetaChange((LPCWSTR)param);
			break;
	}

	return CallPrevWinampProc(WM_WA_IPC, param, (LPARAM)commandId);
}

void WinampHook::OnWinampCommand(UINT commandId, UINT controlId, HWND hControl)
{
	switch(commandId)
	{
		case WINAMP_REFRESHSKIN:
			for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
				if (NULL != iter->second) iter->second->SkinChanging();
			break;
	}
}

void WinampHook::OnSysColorChange()
{
	for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
		if (NULL != iter->second) iter->second->SysColorChange();
}

LRESULT WinampHook::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:		return OnWinampDestroy();
		case WM_WA_IPC:			return OnWinampIPC((UINT)lParam, wParam);
		case WM_COMMAND:		OnWinampCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_SYSCOLORCHANGE:	OnSysColorChange(); break;
	}
	return CallPrevWinampProc(uMsg, wParam, lParam);
}

static LRESULT CALLBACK WinampWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WinampHook *hookInstance = GetWinampHook(hwnd);
	if (NULL == hookInstance)
	{
		WNDPROC recovery = (WNDPROC)GetProp(hwnd, WHPROCRECOVERY);
		if (NULL != recovery)
		{			
			return (IsWindowUnicode(hwnd)) ? 
				CallWindowProcW(recovery, hwnd, uMsg, wParam, lParam) : 
				CallWindowProcA(recovery, hwnd, uMsg, wParam, lParam);
		}

		return (IsWindowUnicode(hwnd)) ? 
			DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
			DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}

	return hookInstance->WindowProc(uMsg, wParam, lParam);
}