#include <windows.h>
#include "Winamp.h"
#include "resource.h"
#include "WinampFactory.h"
#define INITGUID
#include <guiddef.h>
#include <strsafe.h>

//D9C17076-9F55-49b5-8BEB-6A857931E62C
DEFINE_GUID(CLSID_Winamp,0xD9C17076,0x9F55,0x49b5,0x8B,0xEB,0x6A,0x85,0x79,0x31,0xE6,0x2C);

static HRESULT UnregisterComponent(const CLSID &clsid);
static HRESULT RegisterComponent(HMODULE hModule, const CLSID &clsid, LPCWSTR pszFriendlyName);

static const WCHAR szComponentFriendlyName[]		= L"Winamp Application Detector";

static HINSTANCE hMainInstance=0;
static DWORD regID = 0;

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
   if (dwReason == DLL_PROCESS_ATTACH)
	 {
      hMainInstance=hInstance;
			
			
		//	CoRegisterClassObject(CLSID_Winamp,(IClassFactory*)&cf, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &regID);
	 }
   return TRUE;    // ok
}

STDAPI DllRegisterServer()
{
	HRESULT hr(S_OK);
	hr = RegisterComponent(hMainInstance, CLSID_Winamp, szComponentFriendlyName);
	return hr;
}

STDAPI DllCanUnloadNow()
{
	return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
	    HRESULT hr = E_OUTOFMEMORY; 
    *ppv = NULL; 
 
    WinampFactory *winampFactory = new WinampFactory(); 
    if (winampFactory != NULL)   { 
        hr = winampFactory->QueryInterface(riid, ppv); 
        winampFactory->Release(); 
    } 
    return hr;

}

STDAPI DllUnregisterServer()
{
	HRESULT hr(S_OK);
	hr = UnregisterComponent(CLSID_Winamp);
	return hr;
}

static BOOL WriteRegKey(HKEY hKeyParent, LPCWSTR pszKey, LPCWSTR pszValue, HKEY *phKey = NULL)
{
	HKEY hKey;
	LONG result;
	result = RegCreateKeyExW(hKeyParent, pszKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
	if (ERROR_SUCCESS != result || !hKey) return FALSE;
	if (pszValue) {		
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
	FILETIME time;
	WCHAR szBuffer[512];
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

static HRESULT RegisterComponent(HMODULE hModule, const CLSID &clsid, LPCWSTR pszFriendlyName)
{
	HKEY hKey, hKey2=0;
	BOOL br;
	WCHAR szBuffer[MAX_PATH], szCLSID[64]; 
	
	if (!StringFromGUID2(clsid, szCLSID, sizeof(szCLSID)/sizeof(WCHAR))) return E_OUTOFMEMORY;
	StringCchPrintfW(szBuffer, sizeof(szBuffer)/sizeof(WCHAR), L"CLSID\\%s", szCLSID);
		
	if (!WriteRegKey(HKEY_CLASSES_ROOT, szBuffer, pszFriendlyName, &hKey)) return E_ACCESSDENIED;
			
	if (!GetModuleFileNameW(hModule, szBuffer, sizeof(szBuffer)/sizeof(WCHAR))) return S_FALSE;
	//wchar_t localizedString[MAX_PATH+15];
	//StringCbPrintf(localizedString, sizeof(localizedString), L"@%s,-%u", szBuffer, IDS_WINAMP);

	br = (WriteRegKey(hKey, L"InProcServer32" , szBuffer, &hKey2) &&
		WriteRegValue(hKey2, NULL, L"ThreadingModel", L"Both"));
	RegCloseKey(hKey);
	if (hKey2) RegCloseKey(hKey2);
	if (!br) return E_ACCESSDENIED;

	return S_OK;
}

static HRESULT UnregisterComponent(const CLSID &clsid)
{
	LONG result;
	WCHAR szBuffer[MAX_PATH], szCLSID[64]; 
	if (!StringFromGUID2(clsid, szCLSID, sizeof(szCLSID)/sizeof(WCHAR))) return E_OUTOFMEMORY;
	StringCchPrintfW(szBuffer, sizeof(szBuffer)/sizeof(WCHAR), L"CLSID\\%s", szCLSID);

	result = DeleteRegKey(HKEY_CLASSES_ROOT, szBuffer);
	if (ERROR_SUCCESS != result && ERROR_FILE_NOT_FOUND != result) return S_FALSE;


	return S_OK;
}
