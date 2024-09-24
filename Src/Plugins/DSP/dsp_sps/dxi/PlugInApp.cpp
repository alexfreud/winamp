// PlugInApp.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "PlugInApp.h"

////////////////////////////////////////////////////////////////////////////////

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pv)
{
	return DllEntryPoint( hInstance, ulReason, pv );
}

////////////////////////////////////////////////////////////////////////////////

LONG recursiveDeleteKey( HKEY hKeyParent,					// Parent of key to delete
                         const char* lpszKeyChild )	// Key to delete
{
	// Open the child.
	HKEY hKeyChild ;
	LONG lRes = RegOpenKeyEx( hKeyParent, lpszKeyChild, 0, KEY_ALL_ACCESS, &hKeyChild );
	if (lRes != ERROR_SUCCESS)
	{
		return lRes;
	}

	// Enumerate all of the decendents of this child.
	FILETIME time;
	char szBuffer[ 256 ];
	DWORD dwSize = 256;
	while (RegEnumKeyEx( hKeyChild, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time ) == S_OK)
	{
		// Delete the decendents of this child.
		lRes = recursiveDeleteKey( hKeyChild, szBuffer );
		if (lRes != ERROR_SUCCESS)
		{
			// Cleanup before exiting.
			RegCloseKey( hKeyChild );
			return lRes;
		}
		dwSize = 256;
	}

	// Close the child.
	RegCloseKey( hKeyChild );

	// Delete this child.
	return RegDeleteKey( hKeyParent, lpszKeyChild );
}

////////////////////////////////////////////////////////////////////////////////

static const char* s_pszReg = "CakewalkPlugIns\\";

extern CFactoryTemplate g_Templates[];
extern int g_cTemplates;

////////////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
	HKEY					hKey = 0;
	char					sz[ _MAX_PATH ];
	OLECHAR				wsz[ _MAX_PATH ];
	char					szCLSID[ 64 ];
	ITypeLib*			pTypeLib = 0;
	int					i = 0;
	HRESULT				hr = E_FAIL;

	// Do DirectShow registration
	hr = AMovieDllRegisterServer2( TRUE );
	if (FAILED( hr ))
		goto DONE;

	// Get our full pathname, converting to multibyte
	GetModuleFileName( g_hInst, sz, sizeof sz );
	if (0 == MultiByteToWideChar( CP_ACP, 0, sz, _MAX_PATH, wsz, _MAX_PATH ))
		goto DONE;
		
	// Iterate over all exported CLSIDs
	for (i = 0; i < g_cTemplates; i++)
	{
		CFactoryTemplate* pT = &g_Templates[ i ];

		if (NULL != pT->m_pAMovieSetup_Filter)
		{
			// For backwards compatability, instantiate all servers and get hold of
			// IAMovieSetup (if implemented) and call IAMovieSetup.Register() method
			if (NULL != pT->m_lpfnNew)
			{
				IAMovieSetup* pSetup = 0;
				if (SUCCEEDED( CoCreateInstance( *(pT->m_ClsID), 0, CLSCTX_INPROC_SERVER,
					IID_IAMovieSetup, (void**)&pSetup ) ))
				{
					pSetup->Register();
					pSetup->Release();
				}
			}

			// Convert the CLSID to an ANSI string
			StringFromGUID2( *(pT->m_ClsID), wsz, sizeof wsz );
			if (0 == WideCharToMultiByte( CP_ACP, 0, wsz, -1, szCLSID, sizeof szCLSID, NULL, NULL ))
				goto DONE;
		
			// Add {...} to HKEY_CLASSES_ROOT\<s_pszReg>
			strcpy( sz, s_pszReg );
			strcat( sz, szCLSID );
			if (ERROR_SUCCESS != RegCreateKey( HKEY_CLASSES_ROOT, sz, &hKey ))
				goto DONE;

			// {...}\Description = <description text>
			if (0 == WideCharToMultiByte( CP_ACP, 0, pT->m_Name, -1, sz, sizeof sz, NULL, NULL ))
				goto DONE;
			RegSetValueEx( hKey, "Description", 0, REG_SZ, (BYTE*)sz, strlen(sz) );

			// Written for backwards compatability with SONAR 1.x and Pro Audio:
			// {...}\HelpFilePath = ""
			// {...}\HelpFileTopic = ""
			*sz = 0;	
			RegSetValueEx( hKey, "HelpFilePath", 0, REG_SZ, (BYTE*)sz, 1 );
			RegSetValueEx( hKey, "HelpFileTopic", 0, REG_SZ, (BYTE*)sz, 1 );

			RegCloseKey( hKey );
			hKey = 0;
		}
	}

	hr = S_OK;

DONE:


	if (hKey)
		RegCloseKey( hKey );

	return hr;
}

////////////////////////////////////////////////////////////////////////////////

STDAPI DllUnregisterServer()
{	
	char					sz[ _MAX_PATH ];
	OLECHAR				wsz[ _MAX_PATH ];
	char					szCLSID[ 64 ];
	int					i = 0;
	HRESULT				hr = E_FAIL;

	// Do DirectShow unregistration
	hr = AMovieDllRegisterServer2( FALSE );
	if (FAILED( hr ))
		goto DONE;

	// Iterate over all exported CLSIDs
	for (i = 0; i < g_cTemplates; i++)
	{
		CFactoryTemplate* pT = &g_Templates[ i ];

		// For backwards compatability, instantiate all servers and get hold of
		// IAMovieSetup (if implemented) and call IAMovieSetup.Register() method
		if (NULL != pT->m_lpfnNew)
		{
			IAMovieSetup* pSetup = 0;
			if (SUCCEEDED( CoCreateInstance( *(pT->m_ClsID), 0, CLSCTX_INPROC_SERVER,
														IID_IAMovieSetup, (void**)&pSetup ) ))
			{
				pSetup->Unregister();
				pSetup->Release();
			}
		}

		// Convert the CLSID to an ANSI string
		StringFromGUID2( *(pT->m_ClsID), wsz, sizeof wsz );
		if (0 == WideCharToMultiByte( CP_ACP, 0, wsz, -1, szCLSID, sizeof szCLSID, NULL, NULL ))
			goto DONE;
		
		// Delete HKEY_CLASSES_ROOT\<s_pszReg>
		strcpy( sz, s_pszReg );
		strcat( sz, szCLSID );
		recursiveDeleteKey( HKEY_CLASSES_ROOT, sz );
	}

	hr = S_OK;

DONE:

	return hr;
}

////////////////////////////////////////////////////////////////////////////////
