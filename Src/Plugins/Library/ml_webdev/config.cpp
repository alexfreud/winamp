#include "main.h"
#include "./config.h"
#include "./wasabi.h"

#include <shlwapi.h>
#include <strsafe.h>

#define CONFIG_SUFFIX		L"Plugins\\webDev"

static LPCSTR Config_GetPath()
{
	static LPSTR configPath = NULL;
	if (NULL == configPath)
	{
		LPCWSTR p = (NULL != WASABI_API_APP) ? WASABI_API_APP->path_getUserSettingsPath() : NULL;
		if (NULL != p)
		{
			WCHAR szBuffer[MAX_PATH * 2] = {0};
			if (0 != PathCombine(szBuffer, p, CONFIG_SUFFIX))
			{
				OMUTILITY->EnsurePathExist(szBuffer);
				PathAppend(szBuffer, L"config.ini");
				configPath = Plugin_WideCharToMultiByte(CP_UTF8, 0, szBuffer, -1, NULL, NULL);
			}
		}
	}

	return configPath;
}

DWORD Config_ReadStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize)
{
	return GetPrivateProfileStringA(lpSectionName, lpKeyName, lpDefault, lpReturnedString, nSize, Config_GetPath());
}

UINT Config_ReadInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nDefault)
{
	return GetPrivateProfileIntA(lpSectionName, lpKeyName, nDefault, Config_GetPath());
}

HRESULT Config_WriteStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpString)
{
	LPCSTR configPath = Config_GetPath();
	if (NULL == configPath || '\0' == *configPath) 
		return E_UNEXPECTED;
	
	if (0 != WritePrivateProfileStringA(lpSectionName, lpKeyName, lpString, configPath))
		return S_OK;

	DWORD errorCode = GetLastError();
	return HRESULT_FROM_WIN32(errorCode);
}

HRESULT Config_WriteInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nValue)
{
	char szBuffer[32];
	HRESULT hr = StringCchPrintfA(szBuffer, ARRAYSIZE(szBuffer), "%d", nValue);
	if (FAILED(hr)) return hr;

	return Config_WriteStr(lpSectionName, lpKeyName, szBuffer);
}

HRESULT Config_WriteSection(LPCSTR lpSectionName, LPCSTR lpData)
{
	LPCSTR configPath = Config_GetPath();
	if (NULL == configPath || '\0' == *configPath) 
		return E_UNEXPECTED;
	
	if (0 == WritePrivateProfileSectionA(lpSectionName, lpData, configPath))
		return S_OK;

	DWORD errorCode = GetLastError();
	return HRESULT_FROM_WIN32(errorCode);
}