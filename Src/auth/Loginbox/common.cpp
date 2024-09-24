#include "./common.h"
#include "../api.h"

#include "../../winamp/accessibilityConfigGroup.h"

#include <shlwapi.h>
#include <strsafe.h>


LPWSTR LoginBox_MallocString(size_t cchLen)
{
	return (LPWSTR)calloc(cchLen, sizeof(WCHAR));
}

void LoginBox_FreeString(LPWSTR pszString)
{
	if (NULL != pszString)
	{
		free(pszString);
	}
}

void LoginBox_FreeStringSecure(LPWSTR pszString)
{
	if (NULL != pszString)
	{
		size_t size = LoginBox_GetAllocSize(pszString);
		if (0 != size) 
			SecureZeroMemory(pszString, size);

		free(pszString);
	}
}

LPWSTR LoginBox_ReAllocString(LPWSTR pszString, size_t cchLen)
{
	return (LPWSTR)realloc(pszString, sizeof(WCHAR) * cchLen);
}

LPWSTR LoginBox_CopyString(LPCWSTR pszSource)
{
	if (NULL == pszSource)
		return NULL;

	INT cchSource = lstrlenW(pszSource) + 1;
		
	LPWSTR copy = LoginBox_MallocString(cchSource);
	if (NULL != copy)
	{
		CopyMemory(copy, pszSource, sizeof(WCHAR) * cchSource);
	}
	return copy;
}

LPSTR LoginBox_MallocAnsiString(size_t cchLen)
{
	return (LPSTR)calloc(cchLen, sizeof(CHAR));
}

LPSTR LoginBox_CopyAnsiString(LPCSTR pszSource)
{
	if (NULL == pszSource)
		return NULL;

	INT cchSource = lstrlenA(pszSource) + 1;
		
	LPSTR copy = LoginBox_MallocAnsiString(cchSource);
	if (NULL != copy)
	{
		CopyMemory(copy, pszSource, sizeof(CHAR) * cchSource);
	}
	return copy;

}
void LoginBox_FreeAnsiString(LPSTR pszString)
{
	LoginBox_FreeString((LPWSTR)pszString);
}

void LoginBox_FreeAnsiStringSecure(LPSTR pszString)
{
	LoginBox_FreeStringSecure((LPWSTR)pszString);
}

size_t LoginBox_GetAllocSize(void *memory)
{
	return (NULL != memory) ? _msize(memory) : 0;
}

size_t LoginBox_GetStringMax(LPWSTR pszString)
{
	return LoginBox_GetAllocSize(pszString)/sizeof(WCHAR);
}

size_t LoginBox_GetAnsiStringMax(LPSTR pszString)
{
	return LoginBox_GetAllocSize(pszString)/sizeof(CHAR);
}

HRESULT LoginBox_WideCharToMultiByte(UINT codePage, DWORD dwFlags, LPCWSTR lpWideCharStr, INT cchWideChar, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar, LPSTR *ppResult)
{
	if (NULL == ppResult)
		return E_POINTER;

	INT resultMax = WideCharToMultiByte(codePage, dwFlags, lpWideCharStr, cchWideChar, NULL, 0, lpDefaultChar, lpUsedDefaultChar);
	if (0 == resultMax) 
	{
		DWORD errorCode = GetLastError();
		*ppResult = NULL;
		return HRESULT_FROM_WIN32(errorCode);
	}

	if (cchWideChar > 0)
		resultMax++;


	*ppResult = LoginBox_MallocAnsiString(resultMax);
	if (NULL == *ppResult) return E_OUTOFMEMORY;
	resultMax = WideCharToMultiByte(codePage, dwFlags, lpWideCharStr, cchWideChar, *ppResult, resultMax, lpDefaultChar, lpUsedDefaultChar);
	if (0 == resultMax)
	{
		DWORD errorCode = GetLastError();
		LoginBox_FreeAnsiString(*ppResult);
		*ppResult = NULL;
		return HRESULT_FROM_WIN32(errorCode);
	}

	if (cchWideChar > 0)
		(*ppResult)[resultMax] = '\0';

	return S_OK;
}

HRESULT LoginBox_MultiByteToWideChar(UINT codePage, DWORD dwFlags, LPCSTR lpMultiByteStr, INT cbMultiByte, LPWSTR *ppResult)
{
	if (NULL == ppResult)
		return E_POINTER;

	INT resultMax = MultiByteToWideChar(codePage, dwFlags, lpMultiByteStr, cbMultiByte, NULL, 0);
	if (0 == resultMax) 
	{
		DWORD errorCode = GetLastError();
		*ppResult = NULL;
		return HRESULT_FROM_WIN32(errorCode);
	}

	if (cbMultiByte > 0)
		resultMax++;

	*ppResult = LoginBox_MallocString(resultMax);
	if (NULL == *ppResult) return E_OUTOFMEMORY;
	resultMax = MultiByteToWideChar(codePage, dwFlags, lpMultiByteStr, cbMultiByte, *ppResult, resultMax);
	if (0 == resultMax)
	{
		DWORD errorCode = GetLastError();
		LoginBox_FreeString(*ppResult);
		*ppResult = NULL;
		return HRESULT_FROM_WIN32(errorCode);
	}

	if (cbMultiByte > 0)
		(*ppResult)[resultMax] = L'\0';

	return S_OK;
}

HRESULT LoginBox_GetConfigPath(LPWSTR pszConfig, BOOL fEnsureExist)
{
	if (NULL == pszConfig) 
		return E_INVALIDARG;

	LPCWSTR pszWinamp;
	pszWinamp = (NULL != WASABI_API_APP) ? WASABI_API_APP->path_getUserSettingsPath(): NULL;
	if (NULL == pszWinamp) 
		return E_FAIL;	
	
	if (NULL == PathCombine(pszConfig, pszWinamp, L"Plugins\\loginBox"))
		return E_FAIL;
	
	if (FALSE != fEnsureExist)
	{
		HRESULT hr;
		hr = LoginBox_EnsurePathExist(pszConfig);
		if (FAILED(hr)) return hr;
	}
	return S_OK;
}

HRESULT LoginBox_EnsurePathExist(LPCWSTR pszDirectory)
{
	DWORD ec = ERROR_SUCCESS;
	UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

	if (0 == CreateDirectory(pszDirectory, NULL))
	{
		ec = GetLastError();
		if (ERROR_PATH_NOT_FOUND == ec)
		{
			LPCWSTR pszBlock = pszDirectory;
			WCHAR szBuffer[MAX_PATH] = {0};
			
			LPCTSTR pszCursor = PathFindNextComponent(pszBlock);
			ec = (pszCursor == pszBlock || S_OK != StringCchCopyN(szBuffer, ARRAYSIZE(szBuffer), pszBlock, (pszCursor - pszBlock))) ?
					ERROR_INVALID_NAME : ERROR_SUCCESS;
			
			pszBlock = pszCursor;
			
			while (ERROR_SUCCESS == ec && NULL != (pszCursor = PathFindNextComponent(pszBlock)))
			{
				if (pszCursor == pszBlock || S_OK != StringCchCatN(szBuffer, ARRAYSIZE(szBuffer), pszBlock, (pszCursor - pszBlock)))
					ec = ERROR_INVALID_NAME;

				if (ERROR_SUCCESS == ec && !CreateDirectory(szBuffer, NULL))
				{
					ec = GetLastError();
					if (ERROR_ALREADY_EXISTS == ec) ec = ERROR_SUCCESS;
				}
				pszBlock = pszCursor;
			}
		}

		if (ERROR_ALREADY_EXISTS == ec) 
			ec = ERROR_SUCCESS;
	}

	SetErrorMode(errorMode);
	SetLastError(ec);
	return HRESULT_FROM_WIN32(ec);
}

HRESULT LoginBox_GetWindowText(HWND hwnd, LPWSTR *ppszText, UINT *pcchText)
{
	if (NULL == ppszText) return E_POINTER;
	if (NULL == hwnd) return E_INVALIDARG;
	
	UINT cchText = (UINT)SNDMSG(hwnd, WM_GETTEXTLENGTH, 0, 0L);
	
	cchText++;
	*ppszText = LoginBox_MallocString(cchText);
	if (NULL == *ppszText)
	{
		if (NULL != pcchText) *pcchText = 0;
		return E_OUTOFMEMORY;
	}
	
	cchText = (UINT)SNDMSG(hwnd, WM_GETTEXT, (WPARAM)cchText, (LPARAM)*ppszText);
	if (NULL != pcchText)
		*pcchText = cchText;
	
	return S_OK;
}

BOOL LoginBox_PrintWindow(HWND hwnd, HDC hdc, UINT flags)
{
	typedef BOOL (WINAPI *PRINTWINDOW)(HWND /*hwnd*/, HDC /*hdc*/, UINT /*nFlags*/);
	static PRINTWINDOW printWindow = NULL;
	static HMODULE moduleUser32 = NULL;
	if (NULL == moduleUser32)
	{
		moduleUser32 = GetModuleHandle(L"USER32");
		if (NULL == moduleUser32) return FALSE;
		
		printWindow = (PRINTWINDOW)GetProcAddress(moduleUser32, "PrintWindow");
	}
	
	return (NULL != printWindow && FALSE != printWindow(hwnd, hdc, flags));

}


BOOL LoginBox_MessageBeep(UINT beepType)
{
	BOOL result = FALSE;
	ifc_configitem *beepEnabled = AGAVE_API_CONFIG->GetItem(accessibilityConfigGroupGUID, L"modalbeep");
	if (NULL != beepEnabled)
	{
		if (false != beepEnabled->GetBool())
		{
			result = MessageBeep(beepType);
		}
		beepEnabled->Release();
	}

	return result;
}

HRESULT LoginBox_IsStringEqualEx(LCID locale, BOOL ignoreCase, LPCWSTR str1, LPCWSTR str2)
{
	if ((NULL == str1) != (NULL == str2))
		return S_FALSE;
	
	if (NULL != str1 && CSTR_EQUAL != CompareString(locale, (FALSE != ignoreCase) ? NORM_IGNORECASE : 0, str1, -1, str2, -1))
		return S_FALSE;

	return S_OK;
}

UINT LoginBox_GetCurrentTime()
{
	SYSTEMTIME st;
	FILETIME ft;

	GetSystemTime(&st);
	if(FALSE == SystemTimeToFileTime(&st, &ft))
		return 0;
	
	ULARGE_INTEGER t1;
	t1.LowPart = ft.dwLowDateTime;
	t1.HighPart = ft.dwHighDateTime;
	
	return (UINT)((t1.QuadPart - 116444736000000000) / 10000000);
}

HRESULT LoginBox_GetCurrentLang(LPSTR *ppLang)
{
	if (NULL == ppLang)
		return E_POINTER;

	if (NULL == WASABI_API_LNG)
		return E_UNEXPECTED;
	
	LPCWSTR lang = WASABI_API_LNG->GetLanguageIdentifier(LANG_LANG_CODE);
	
	if (NULL != lang && L'\0' != *lang)
		return LoginBox_WideCharToMultiByte(CP_UTF8, 0, lang, -1, NULL, NULL, ppLang);
	
	*ppLang = NULL;
	return S_OK;
	
}

HDWP LoginBox_LayoutButtonBar(HDWP hdwp, HWND hwnd, const INT *buttonList, UINT buttonCount, const RECT *prcClient, LONG buttonHeight, LONG buttonSpace, BOOL fRedraw, RECT *prcResult)
{
	if (NULL == hdwp && NULL == prcClient)
		return NULL;
	
	RECT rect;
	CopyRect(&rect, prcClient);
	
	LONG top = rect.bottom - buttonHeight;
	if (top < rect.top) top = rect.top;
	LONG height = rect.bottom - top;
	LONG width;
	LONG right = rect.right;

	if (NULL == buttonList || 0 == buttonCount)
	{
		if (NULL != prcResult)
			SetRect(prcResult, right, top, rect.right, top + height);

		return (NULL != hdwp) ? hdwp :(HDWP)TRUE;
	}


	UINT flags = SWP_NOACTIVATE | SWP_NOZORDER;
	if (FALSE == fRedraw) flags |= SWP_NOREDRAW;

	WCHAR szText[256] = {0};
	INT cchText;
	HFONT font(NULL), fontOrig;
	SIZE textSize;

	RECT buttonRect;
	while(buttonCount--)
	{
		HWND hControl = GetDlgItem(hwnd, buttonList[buttonCount]);
		if (NULL == hControl || 0 == (WS_VISIBLE & GetWindowStyle(hControl)) || 
			FALSE == GetWindowRect(hControl, &buttonRect))
		{
			continue;
		}

		if (right != rect.right)
			 right -= buttonSpace;

		width = buttonRect.right - buttonRect.left;

		cchText = (INT)SendMessage(hControl, WM_GETTEXT, (WPARAM)ARRAYSIZE(szText), (LPARAM)szText);
		if (cchText > 0)
		{
			HDC hdc = GetDCEx(hControl, NULL,  DCX_CACHE | DCX_WINDOW | DCX_NORESETATTRS);
			if (NULL != hdc)
			{
				if (NULL == font)
					font = (HFONT)SendMessage(hControl, WM_GETFONT, 0, 0L);

				fontOrig = (HFONT)SelectObject(hdc, font);

				if (FALSE != GetTextExtentPoint32W(hdc, szText, cchText, &textSize))
				{
					width = textSize.cx + 4*LoginBox_GetAveCharWidth(hdc);
				}

				SelectObject(hdc, fontOrig);
				ReleaseDC(hControl, hdc);
			}
		}
		
		if (width < 75) 
			width = 75;

		if (NULL != hdwp)
		{
			hdwp = DeferWindowPos(hdwp, hControl, NULL, right - width, top, width, height, flags);
			if (NULL == hdwp) return NULL;
		}

		right -= width;
	}

	if (NULL != prcResult)
		SetRect(prcResult, right, top, rect.right, top + height);

	return (NULL != hdwp) ? hdwp :(HDWP)TRUE;
}

BYTE LoginBox_GetSysFontQuality()
{
	BOOL smoothingEnabled;
	if (FALSE == SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &smoothingEnabled, 0) ||
		FALSE == smoothingEnabled)
	{
		return DEFAULT_QUALITY;
	}
    
    OSVERSIONINFO vi;
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (FALSE == GetVersionEx(&vi)) 
		return DEFAULT_QUALITY;

	if (vi.dwMajorVersion > 5 || (vi.dwMajorVersion == 5 && vi.dwMinorVersion >= 1))
	{
		UINT smootingType;
	    if (FALSE == SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &smootingType, 0))
			return DEFAULT_QUALITY;
	    
	    if (FE_FONTSMOOTHINGCLEARTYPE == smootingType)
			return CLEARTYPE_QUALITY;
	}

	return ANTIALIASED_QUALITY;
}

INT LoginBox_GetAveStrWidth(HDC hdc, INT cchLen)
{
	const char szTest[] = 
	{ 
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P', 'Q','R','S','T','U','V','W','X','Y','Z',
		'a','b','c','d','e','f','g','h','i','j','k','l', 'm','n','o','p','q','r','s','t','u','v','w','x','y','z'
	};

	SIZE textSize;
	if (FALSE == GetTextExtentPointA(hdc, szTest, ARRAYSIZE(szTest) -1, &textSize))
		return 0;

	INT result;
	if (1 == cchLen)
	{
		result = (textSize.cx + ARRAYSIZE(szTest)/2)/ARRAYSIZE(szTest);
	}
	else
	{
		result = MulDiv(cchLen, textSize.cx + ARRAYSIZE(szTest)/2, ARRAYSIZE(szTest));
		if (0 != result)
		{
			TEXTMETRIC tm;
			if (FALSE != GetTextMetrics(hdc, &tm))
				result += tm.tmOverhang;
		}
	}
	return result;
}

INT LoginBox_GetAveCharWidth(HDC hdc)
{
	return LoginBox_GetAveStrWidth(hdc, 1);
}

BOOL LoginBox_GetWindowBaseUnits(HWND hwnd, INT *pBaseUnitX, INT *pBaseUnitY)
{
	INT baseunitX(0), baseunitY(0);
	BOOL result = FALSE;

	if (NULL != hwnd)
	{		
		HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != hdc)
		{
			HFONT font = (HFONT)SNDMSG(hwnd, WM_GETFONT, 0, 0L);
			HFONT fontOrig = (HFONT)SelectObject(hdc, font);
			
			TEXTMETRIC tm;
			if (FALSE != GetTextMetrics(hdc, &tm))
			{
				baseunitY = tm.tmHeight;
				baseunitX = LoginBox_GetAveCharWidth(hdc);
				result = TRUE;
			}

			SelectObject(hdc, fontOrig);
			ReleaseDC(hwnd, hdc);
		}
	}

	if (NULL != pBaseUnitX) *pBaseUnitX = baseunitX;
	if (NULL != pBaseUnitY) *pBaseUnitY = baseunitY;

	return result;

}

INT LoginBox_GetWindowTextHeight(HWND hwnd, INT paddingDlgUnit)
{
	if (NULL == hwnd) return 0;
	
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == hdc) return 0;

	INT height = 0;

	HFONT font = (HFONT)SNDMSG(hwnd, WM_GETFONT, 0, 0L);
	HFONT fontOrig = (HFONT)SelectObject(hdc, font);

	TEXTMETRIC tm;
	if (FALSE != GetTextMetrics(hdc, &tm))
	{
		height = tm.tmHeight;
		if (0 != paddingDlgUnit)
			height += MulDiv(2 * paddingDlgUnit, tm.tmHeight, 8);
	}


	SelectObject(hdc, fontOrig);
	ReleaseDC(hwnd, hdc);

	return height;

}
BOOL LoginBox_GetWindowTextSize(HWND hwnd, INT idealWidth, INT *pWidth, INT *pHeight)
{
	INT width(0), height(0);
	BOOL result = FALSE;

	if (NULL != hwnd)
	{		
		HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != hdc)
		{
			HFONT font = (HFONT)SNDMSG(hwnd, WM_GETFONT, 0, 0L);
			HFONT fontOrig = (HFONT)SelectObject(hdc, font);
			
			LPWSTR pszText;
			UINT cchText;
			if (SUCCEEDED(LoginBox_GetWindowText(hwnd, &pszText, &cchText)))
			{
				if (0 == cchText)
				{
					TEXTMETRIC tm;
					if (FALSE != GetTextMetrics(hdc, &tm))
					{
						height = tm.tmHeight;
						width = 0;
						result = TRUE;
					}
				}
				else
				{
					RECT rect;
					SetRect(&rect, 0, 0, idealWidth, 0);
					if (0 != DrawText(hdc, pszText, cchText, &rect, DT_CALCRECT | DT_NOPREFIX | DT_WORDBREAK))
					{
						width = rect.right - rect.left;
						height = rect.bottom  - rect.top;
						result = TRUE;
					}

				}
				LoginBox_FreeString(pszText);
			}

			SelectObject(hdc, fontOrig);
			ReleaseDC(hwnd, hdc);
		}
	}

	if (NULL != pWidth) *pWidth = width;
	if (NULL != pHeight) *pHeight = height;

	return result;
}

BOOL LoginBox_OpenUrl(HWND hOwner, LPCWSTR pszUrl, BOOL forceExternal)
{
	if (NULL == WASABI_API_WINAMP)
		return FALSE;

	HCURSOR hCursor = LoadCursor(NULL, IDC_APPSTARTING);
	if (NULL != hCursor) 
		hCursor = SetCursor(hCursor);

	BOOL result;

	if (FALSE != forceExternal)
	{
		HINSTANCE hInst = ShellExecute(hOwner, L"open", pszUrl, NULL, NULL, SW_SHOWNORMAL);
		result = ((INT_PTR)hInst > 32) ? TRUE: FALSE;
	}
	else
	{
		HRESULT hr = WASABI_API_WINAMP->OpenUrl(hOwner, pszUrl);
		result = SUCCEEDED(hr);
	}
		
	if (NULL != hCursor) 
		SetCursor(hCursor);

	return result;
}