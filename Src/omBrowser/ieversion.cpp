#include "main.h"
#include "./ieversion.h"

#include <shlwapi.h>
#include <strsafe.h>

HRESULT MSIE_GetVersionString(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return E_INVALIDARG;
	
	HKEY hKey = NULL;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Internet Explorer", 0,  
							   STANDARD_RIGHTS_READ | KEY_QUERY_VALUE, &hKey);

	if (ERROR_SUCCESS == result)
	{
		DWORD cbBuffer = sizeof(WCHAR) * cchBufferMax;
		result = RegQueryValueEx(hKey, L"svcVersion", NULL, NULL, (BYTE*)pszBuffer, &cbBuffer); 
		if (ERROR_SUCCESS != result
			|| L'\0' == *pszBuffer)
		{
			cbBuffer = sizeof(WCHAR) * cchBufferMax;
			result = RegQueryValueEx(hKey, L"Version", NULL, NULL, (BYTE*)pszBuffer, &cbBuffer); 
		}
		RegCloseKey(hKey);
	}

	if (ERROR_SUCCESS != result)
	{
		*pszBuffer = L'\0';
		return HRESULT_FROM_WIN32(result);
	}

	return S_OK;
}

HRESULT MSIE_GetVersion(INT *majorOut, INT *minorOut, INT *buildOut, INT *subBuildOut)
{	
	INT szVersion[4] = { 0, 0, 0, 0};
	WCHAR szBuffer[64] = {0};
	HRESULT hr = MSIE_GetVersionString(szBuffer, ARRAYSIZE(szBuffer));
	if (SUCCEEDED(hr)) 
	{
		INT index = 0;
		LPWSTR block = szBuffer;
		LPWSTR cursor = block;

		for(;;)
		{
			if (L'\0' == *cursor)
			{
				if (block != cursor && FALSE != StrToIntEx(block,STIF_DEFAULT, &szVersion[index]))
					index++;
				break;
			}
			else if (L'.' == *cursor)
			{
				*cursor = L'\0';
				if (block != cursor && FALSE != StrToIntEx(block,STIF_DEFAULT, &szVersion[index]))
				{
					index++;
					if (index == ARRAYSIZE(szVersion))
						break; // too many numbers
				}
				cursor++;
				block = cursor;
			}
			cursor++;
		}

		if (index < ARRAYSIZE(szVersion))
			hr = E_FAIL;
	}

	if (NULL != majorOut) *majorOut = szVersion[0];
	if (NULL != minorOut) *minorOut = szVersion[1];
	if (NULL != buildOut) *buildOut = szVersion[2];
	if (NULL != subBuildOut) *subBuildOut = szVersion[3];

	return hr;
}