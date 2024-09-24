// Elevator.cpp : Implementation of WinMain

#include "stdafx.h"
#include "resource1.h"
#include "FileTypeRegistrar.h"
#include "ElevatorFactory.h"
#include <sddl.h>
#include <strsafe.h>

HRESULT RegisterServer(HINSTANCE hInstance);
HRESULT UnregisterServer(HINSTANCE hInstance);

DWORD g_allLocks = 0;
int APIENTRY WinMain(HINSTANCE hInstance,
										 HINSTANCE hPrevInstance,
										 LPSTR     lpCmdLine,
										 int       nCmdShow)
{
	if (lpCmdLine == NULL || !*lpCmdLine)
	{
		MSGBOXPARAMSW msgbx = {sizeof(MSGBOXPARAMS),0};
		msgbx.lpszText = L"Winamp Elevator\nCopyright © 2008-2014 Winamp SA";
		msgbx.lpszCaption = L"About...";
		msgbx.lpszIcon = MAKEINTRESOURCEW(IDI_ICON1);
		msgbx.hInstance = GetModuleHandle(0);
		msgbx.dwStyle = MB_USERICON;
		MessageBoxIndirectW(&msgbx);
		return 0;
	}

	if ( lpCmdLine != NULL && ( _strcmpi( lpCmdLine, "/RegServer" ) == 0 
		|| _strcmpi( lpCmdLine, "-RegServer" ) == 0 )) {
		RegisterServer(hInstance);
		return 0;
	}

	if ( lpCmdLine != NULL && ( _strcmpi( lpCmdLine, "/UnregServer" ) == 0 
		|| _strcmpi( lpCmdLine, "-UnregServer" ) == 0 )) {
		UnregisterServer(hInstance);
		return 0;
	}
	CoInitialize(NULL);    

	if(lpCmdLine && (strstr(lpCmdLine, "/Embedding") || strstr(lpCmdLine, "-Embedding")))
	{
		ElevatorFactory cf; 
		DWORD regID = 0;
		CoRegisterClassObject(CLSID_WFileTypeRegistrar,(IClassFactory*)&cf, CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &regID);
		MSG ms;
		while(GetMessage(&ms, 0, 0, 0))
		{
			TranslateMessage(&ms);
			DispatchMessage(&ms);
		}

		CoRevokeClassObject(regID);   
	}

	CoUninitialize();   
	return 0;
}

extern FileTypeRegistrar registrar;

void Lock()
{
	++g_allLocks;
}

void UnLock()
{
	--g_allLocks;
	if(g_allLocks == 0 )//&& registrar.refCount == 0/* benski> a hack, for now */) 
	{
		PostQuitMessage(0);
	}
}

static BOOL GetAccessPermissionsForLUAServer(SECURITY_DESCRIPTOR **ppSD)
{
// Local call permissions to IU, SY
    LPWSTR lpszSDDL = L"O:BAG:BAD:(A;;0x3;;;IU)(A;;0x3;;;SY)";
    SECURITY_DESCRIPTOR *pSD;
    *ppSD = NULL;

    if (ConvertStringSecurityDescriptorToSecurityDescriptorW(lpszSDDL, SDDL_REVISION_1, (PSECURITY_DESCRIPTOR *)&pSD, NULL))
    {
        *ppSD = pSD;
        return TRUE;
    }

    return FALSE;
}

// hKey is the HKCR\AppID\{GUID} key
static BOOL SetAccessPermissions(HKEY hkey, PSECURITY_DESCRIPTOR pSD)
{
    BOOL bResult = FALSE;
    DWORD dwLen = GetSecurityDescriptorLength(pSD);
    LONG lResult;
    lResult = RegSetValueExA(hkey, 
				"AccessPermission",
				0,
				REG_BINARY,
				(BYTE*)pSD,
				dwLen);
    if (lResult != ERROR_SUCCESS) goto done;
    bResult = TRUE;
done:
    return bResult;
}

static BOOL GetLaunchActPermissionsWithIL (SECURITY_DESCRIPTOR **ppSD)
{
	// Allow World Local Launch/Activation permissions. Label the SD for LOW IL Execute UP
	LPWSTR lpszSDDL = L"O:BAG:BAD:(A;;0xb;;;WD)S:(ML;;NX;;;LW)";
	SECURITY_DESCRIPTOR *pSD;
    if (ConvertStringSecurityDescriptorToSecurityDescriptorW(lpszSDDL, SDDL_REVISION_1, (PSECURITY_DESCRIPTOR *)&pSD, NULL))
    {
        *ppSD = pSD;
        return TRUE;
    }
	return FALSE;
}

static BOOL SetLaunchActPermissions(HKEY hkey, PSECURITY_DESCRIPTOR pSD)
{
    BOOL bResult = FALSE;
    DWORD dwLen = GetSecurityDescriptorLength(pSD);
    LONG lResult;
    lResult = RegSetValueExA(hkey, 
					"LaunchPermission",
					0,
					REG_BINARY,
					(BYTE*)pSD,
					dwLen);
    if (lResult != ERROR_SUCCESS) goto done;
    bResult = TRUE;
done:
    return bResult;
};

static HRESULT UnregisterComponent(const CLSID &clsid, LPCWSTR pszVersionIndProgId, LPCWSTR pszProgId);
static HRESULT RegisterComponent(HMODULE hModule, const CLSID &clsid, LPCWSTR pszFriendlyName, 	 LPCWSTR pszVersionIndProgId, LPCWSTR pszProgId);

static const WCHAR szComponentFriendlyName[]		= L"Winamp Elevator";
static const WCHAR szVersionIndependentProgId[]		= L"Elevator.WFileTypeRegistrar2";
static const WCHAR szProgId[]						= L"Elevator.WFileTypeRegistrar2.1";

HRESULT RegisterServer(HINSTANCE hInstance)
{
	HRESULT hr(S_OK);
	hr = RegisterComponent(hInstance, CLSID_WFileTypeRegistrar, szComponentFriendlyName,  szVersionIndependentProgId, szProgId);
	return hr;
}

HRESULT UnregisterServer(HINSTANCE hInstance)
{
	HRESULT hr(S_OK);
	hr = UnregisterComponent(CLSID_WFileTypeRegistrar, szVersionIndependentProgId, szProgId);
	return hr;
}

static BOOL WriteRegKey(HKEY hKeyParent, LPCWSTR pszKey, LPCWSTR pszValue, HKEY *phKey = NULL)
{
	HKEY hKey;
	LONG result;
	result = RegCreateKeyExW(hKeyParent, pszKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
	if (ERROR_SUCCESS != result || !hKey) return FALSE;
	if (pszValue) 
	{		
		result = RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)pszValue, (DWORD)(sizeof(wchar_t)*(1 + lstrlenW(pszValue))));
	}
	if (!phKey) RegCloseKey(hKey);
	else *phKey = hKey;

	return (ERROR_SUCCESS == result);
}

static BOOL WriteRegValue(HKEY hKeyParent, LPCWSTR pszKey, LPCWSTR pszEntry, LPCWSTR pszValue)
{
	HKEY hKey;
	LONG result;
	result = RegOpenKeyExW(hKeyParent, pszKey, 0, KEY_SET_VALUE, &hKey);
	if (ERROR_SUCCESS != result || !hKey) return FALSE;
	if (pszValue) 
	{		
		result = RegSetValueEx(hKey, pszEntry, 0, REG_SZ, (BYTE*)pszValue, (DWORD)(sizeof(wchar_t)*(1 + lstrlenW(pszValue))));
	}
	RegCloseKey(hKey);
	return (ERROR_SUCCESS == result);
}

static BOOL WriteRegValue(HKEY hKeyParent, LPCWSTR pszKey, LPCWSTR pszEntry, DWORD pszValue)
{
	HKEY hKey;
	LONG result;
	result = RegOpenKeyExW(hKeyParent, pszKey, 0, KEY_SET_VALUE, &hKey);
	if (ERROR_SUCCESS != result || !hKey) return FALSE;
	if (pszValue) 
	{		
		result = RegSetValueEx(hKey, pszEntry, 0, REG_DWORD, (BYTE*)&pszValue, sizeof(DWORD));
	}
	RegCloseKey(hKey);
	return (ERROR_SUCCESS == result);
}

static LONG DeleteRegKey(HKEY hKeyParent, LPCWSTR pszKey)
{	
	HKEY hKey;
	LONG result;
	FILETIME time = {0};
	WCHAR szBuffer[512] = {0};
	DWORD dwSize = sizeof(szBuffer)/sizeof(WCHAR);

	result = RegOpenKeyExW(hKeyParent, pszKey, 0, KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS , &hKey);
	if (ERROR_SUCCESS != result) return result;
	while (ERROR_SUCCESS == RegEnumKeyExW(hKey, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time))
	{
		if (ERROR_SUCCESS != (result = DeleteRegKey(hKey, szBuffer)))
		{
			RegCloseKey(hKey);
			return result;
		}
		dwSize = sizeof(szBuffer)/sizeof(WCHAR);
	}

	RegCloseKey(hKey);
	return RegDeleteKeyW(hKeyParent, pszKey);
}

HRESULT RegisterComponent(HMODULE hModule, const CLSID &clsid, LPCWSTR pszFriendlyName, LPCWSTR pszVersionIndProgId, LPCWSTR pszProgId)
{
	SECURITY_DESCRIPTOR *sd;
	HKEY hKey, hKey2, hKey3;
	BOOL br;
	WCHAR szBuffer[MAX_PATH] = {0}, szCLSID[64] = {0};

	if (!StringFromGUID2(clsid, szCLSID, sizeof(szCLSID)/sizeof(WCHAR))) return E_OUTOFMEMORY;
	StringCchPrintfW(szBuffer, sizeof(szBuffer)/sizeof(WCHAR), L"SOFTWARE\\Classes\\CLSID\\%s", szCLSID);

	if (!WriteRegKey(HKEY_LOCAL_MACHINE, szBuffer, pszFriendlyName, &hKey)) return S_FALSE;

	if (!GetModuleFileNameW(hModule, szBuffer, sizeof(szBuffer)/sizeof(WCHAR))) return S_FALSE;

	RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\AppID\\{3B29AB5C-52CB-4a36-9314-E3FEE0BA7468}", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey3, NULL);
	if (GetAccessPermissionsForLUAServer(&sd))
		SetAccessPermissions(hKey3, sd);
	if (GetLaunchActPermissionsWithIL(&sd))
		SetLaunchActPermissions(hKey3, sd);
	WriteRegValue(hKey3, NULL, NULL, szBuffer);
	RegCloseKey(hKey3);

	RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\AppID\\elevator.exe", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey3, NULL);
	WriteRegValue(hKey3, NULL, L"AppID", L"{3B29AB5C-52CB-4a36-9314-E3FEE0BA7468}");
	RegCloseKey(hKey3);		

	wchar_t localizedString[MAX_PATH+15] = {0};
	StringCbPrintf(localizedString, sizeof(localizedString), L"@%s,-%u", szBuffer, IDS_WINAMP);

	br = (WriteRegKey(hKey, L"LocalServer32" , szBuffer, &hKey2) &&
			WriteRegValue(hKey2, NULL, L"ThreadingModel", L"Both") &&
			WriteRegKey(hKey, L"ProgID", pszProgId) &&
			WriteRegKey(hKey, L"VersionIndependentProgID", pszVersionIndProgId)
			&& WriteRegValue(hKey, NULL, L"LocalizedString", localizedString)						
			&& WriteRegValue(hKey, NULL, L"AppId", L"{3B29AB5C-52CB-4a36-9314-E3FEE0BA7468}")						
			&& WriteRegKey(hKey, L"Elevation", 0)
			//&& WriteRegValue(hKey, L"Elevation", L"IconReference", 
			&& WriteRegValue(hKey, L"Elevation", L"Enabled", 1)
			);
	RegCloseKey(hKey);
	if (hKey2) RegCloseKey(hKey2);
	if (!br) return S_FALSE;

	if (!WriteRegKey(HKEY_CLASSES_ROOT, pszVersionIndProgId, pszFriendlyName, &hKey)) return S_FALSE;
	br = (WriteRegKey(hKey, L"CLSID", szCLSID) &&
			WriteRegKey(hKey, L"CurVer", pszProgId));
	RegCloseKey(hKey);
	if (!br) return S_FALSE;

	if (!WriteRegKey(HKEY_CLASSES_ROOT, pszProgId, pszFriendlyName, &hKey)) return S_FALSE;
	br = WriteRegKey(hKey, L"CLSID", szCLSID);

	RegCloseKey(hKey);

	if (!br) return S_FALSE;

	return S_OK;
}

static HRESULT UnregisterComponent(const CLSID &clsid, LPCWSTR pszVersionIndProgId, LPCWSTR pszProgId)
{
	LONG result;
	WCHAR szBuffer[MAX_PATH] = {0}, szCLSID[64] = {0}; 
	if (!StringFromGUID2(clsid, szCLSID, sizeof(szCLSID)/sizeof(WCHAR))) return E_OUTOFMEMORY;
	StringCchPrintfW(szBuffer, sizeof(szBuffer)/sizeof(WCHAR), L"CLSID\\%s", szCLSID);

	result = DeleteRegKey(HKEY_CLASSES_ROOT, szBuffer);
	if (ERROR_SUCCESS != result && ERROR_FILE_NOT_FOUND != result) return S_FALSE;

	result = DeleteRegKey(HKEY_CLASSES_ROOT, pszVersionIndProgId);
	if (ERROR_SUCCESS != result && ERROR_FILE_NOT_FOUND != result) return S_FALSE;

	result = DeleteRegKey(HKEY_CLASSES_ROOT, pszProgId);
	if (ERROR_SUCCESS != result && ERROR_FILE_NOT_FOUND != result) return S_FALSE;

	return S_OK;
}