#include "./common.h"
#include "./setupDetails.h"
#include "./setupServicePanel.h"

EXTERN_C ATOM DETAILS_PROP = 0;
HMODULE hEditModule = NULL;


BOOL SetupDetails_Initialize()
{
	if (0 == DETAILS_PROP)
	{
		DETAILS_PROP = GlobalAddAtom(L"omSetupDetailsProp");
		if (0 == DETAILS_PROP) return FALSE;
	}

	if (NULL == (hEditModule = LoadLibrary(L"riched20.dll")))
		return FALSE;

	return TRUE;
}

void SetupDetails_Uninitialize()
{
	if (NULL != hEditModule)
	{
		FreeLibrary(hEditModule);
		hEditModule = NULL;
	}

	if (0 != DETAILS_PROP)
	{
		GlobalDeleteAtom(DETAILS_PROP);
		DETAILS_PROP = 0;
	}
}

void SetupDetails_SetDescription(HWND hEdit, LPCWSTR pszText)
{	
	SetWindowText(hEdit, pszText);

	DWORD originalStyle = GetWindowStyle(hEdit);
	DWORD windowStyle = originalStyle & ~WS_VSCROLL;

	INT lineCount = (INT)SendMessage(hEdit, EM_GETLINECOUNT, 0, 0L);
	if (lineCount > 0)
	{
		INT charIndex = (INT)SendMessage(hEdit, EM_LINEINDEX, (WPARAM)(lineCount - 1), 0L);
		if (-1 != charIndex)
		{
			LRESULT result = SendMessage(hEdit, EM_POSFROMCHAR, charIndex, 0L);
			POINTS pts = MAKEPOINTS(result);
			RECT clientRect;
			if (GetClientRect(hEdit, &clientRect) && pts.y > (clientRect.bottom - 14))
			{
				windowStyle |= WS_VSCROLL;				
			}
		}
	}
	
	if (windowStyle != originalStyle)
	{
		SetWindowLongPtr(hEdit, GWL_STYLE, windowStyle);
		SetWindowPos(hEdit, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
	}

	ShowWindow(hEdit, SW_HIDE);
	if (0 != ShowWindow(hEdit,  (L'\0' != *pszText) ? SW_SHOWNA : SW_HIDE))
		InvalidateRect(hEdit, NULL, TRUE);
}

HWND SetupDetails_CreateServiceView(HWND hParent, LPCWSTR pszName, ifc_omservice *service)
{
	return ServicePanel::CreateInstance(hParent, pszName, service, NULL);
}

BOOL SetupDetails_GetUniqueName(HWND hwnd, LPWSTR pszBuffer, UINT cchBufferMax)
{
	return (BOOL)SendMessage(hwnd, NSDM_GETUNIQUENAME, (WPARAM)cchBufferMax, (LPARAM)pszBuffer);
}