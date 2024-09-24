#include "./loginConfig.h"
#include "./common.h"

#include <shlwapi.h>
#include <strsafe.h>

#define BOOL2HRESULT(__result) ((FALSE != (__result)) ? S_OK : S_FALSE)


LoginConfig::LoginConfig()
	: ref(1), configPath(NULL), pathValidated(FALSE), buffer(NULL)
{

}

LoginConfig::~LoginConfig()
{
	LoginBox_FreeAnsiString(configPath);
	LoginBox_FreeString((LPWSTR)buffer);
}


HRESULT LoginConfig::CreateInstance(LoginConfig **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	WCHAR szFile[MAX_PATH] = {0};
	HRESULT hr;

	hr = LoginBox_GetConfigPath(szFile, FALSE);
	if (FAILED(hr)) return hr;

	if (FALSE == PathAppend(szFile, L"loginBox.ini"))
		return E_UNEXPECTED;

	LPSTR configPath;
	hr = LoginBox_WideCharToMultiByte(CP_UTF8, 0, szFile, -1, NULL, NULL, &configPath);
	if (FAILED(hr)) return hr;

	*instance = new LoginConfig();
	if (NULL == *instance)
	{
		LoginBox_FreeAnsiString(configPath);
		return E_OUTOFMEMORY;
	}
	else
	{
		(*instance)->configPath = configPath;
	}

	return S_OK;
}

ULONG LoginConfig::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginConfig::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT LoginConfig::GetPath(LPCSTR *ppPath)
{
	if (NULL == ppPath)
		return E_POINTER;
	
	*ppPath = configPath;
	return S_OK;
}

DWORD LoginConfig::ReadAnsiStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize)
{
	return GetPrivateProfileStringA(lpSectionName, lpKeyName, lpDefault, lpReturnedString, nSize, configPath);
}

UINT LoginConfig::ReadInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nDefault)
{
	return GetPrivateProfileIntA(lpSectionName, lpKeyName, nDefault, configPath);
}


HRESULT LoginConfig::WriteAnsiStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpString)
{
	if (NULL == configPath || L'\0' == *configPath) 
		return E_UNEXPECTED;

	if (FALSE == pathValidated)
	{
		LPWSTR pszTest;
		if (SUCCEEDED(LoginBox_MultiByteToWideChar(CP_UTF8, 0, configPath, -1, &pszTest)))
		{
			PathRemoveFileSpec(pszTest);
			LoginBox_EnsurePathExist(pszTest);
			pathValidated = TRUE;

			LoginBox_FreeString(pszTest);
		}
	}

	if (0 != WritePrivateProfileStringA(lpSectionName, lpKeyName, lpString, configPath))
		return S_OK;

	DWORD errorCode = GetLastError();
	return HRESULT_FROM_WIN32(errorCode);
}

HRESULT LoginConfig::WriteInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nValue)
{
	CHAR szBuffer[32] = {0};
	HRESULT hr = StringCchPrintfA(szBuffer, ARRAYSIZE(szBuffer), "%d", nValue);
	if (FAILED(hr)) return hr;

	return WriteAnsiStr(lpSectionName, lpKeyName, szBuffer);
}