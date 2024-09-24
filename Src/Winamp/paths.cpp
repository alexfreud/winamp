/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shlobj.h>
#include <shellapi.h>
#include "../Winamp/in2.h"
extern In_Module mod;
#ifdef __cplusplus
extern "C" BOOL UtilGetSpecialFolderPath(HWND hwnd, TCHAR *path, int folder)
#else
BOOL UtilGetSpecialFolderPath(HWND hwnd, TCHAR *path, int folder)
#endif
{
	ITEMIDLIST *pidl; // Shell Item ID List ptr
	IMalloc *imalloc; // Shell IMalloc interface ptr
	BOOL result; // Return value

	if (SHGetSpecialFolderLocation(hwnd, folder, &pidl) != NOERROR)
		return FALSE;

	result = SHGetPathFromIDList (pidl, path);

	if (SHGetMalloc (&imalloc) == NOERROR)
	{
#ifdef __cplusplus
		imalloc->Free(pidl);
		imalloc->Release();
#else
		imalloc->lpVtbl->Free(imalloc, pidl); 		
		imalloc->lpVtbl->Release(imalloc); 
#endif
	}

	return result;
}


/*
This ugly function converts our specially coded paths
environment variables are surrounded with %'s and CSIDLs are surrounded with {}
note that CSIDLs need to be DECIMAL!
*/
#ifdef __cplusplus
extern "C" void ResolveEnvironmentVariables2(wchar_t *string, wchar_t *destString, size_t stringSize)
#else
void ResolveEnvironmentVariables2(TCHAR *string, TCHAR *destString, size_t stringSize)
#endif
{
	//char *saveStart = string;
	wchar_t *p = string;
	int inPercent = 0, inBrace = 0;
	wchar_t environString[MAX_PATH] = {0};
	wchar_t *dest = destString;
	wchar_t helper[MAX_PATH] = {0};
	wchar_t *pEnv = environString;

	*dest = 0;

	while (p && *p)
	{
		if (*p == L'%')
		{
			if (inPercent)
			{
				*pEnv = 0;
				helper[0] = 0;
				GetEnvironmentVariableW(environString, helper, MAX_PATH);
				StringCchCatW(dest, stringSize, helper);
				pEnv = environString;
				string = CharNextW(p);
				*p = 0;
				p = string;
			}
			else
			{
				wchar_t *newP = CharNextW(p);
				*p = 0;
				StringCchCatW(dest, stringSize, string);
				string = p = newP;
			}

			inPercent = !inPercent;
		}
		else if (*p == L'{')
		{
			if (inPercent)
			{
				ptrdiff_t count = CharNextW(p) - p;
				while (count-- && (pEnv - environString) < MAX_PATH)
					*pEnv++ = *p++;
			}
			else
			{
				wchar_t *newP = CharNextW(p);
				*p = 0;
				StringCchCatW(dest, stringSize, string);
				string = p = newP;
				inBrace = 1;
			}
		}
		else if (*p == L'}')
		{
			if (inPercent)
			{
				ptrdiff_t count = CharNextW(p) - p;
				while (count-- && (pEnv - environString) < MAX_PATH)
					*pEnv++ = *p++;
			}
			*pEnv = 0;
			//SHGetSpecialFolderPath(NULL, helper, atoi(environString), FALSE);
#if defined(UNICODE) || defined(_UNICODE)
			SHGetSpecialFolderPath(NULL, helper, StrToInt(environString), FALSE);
			//UtilGetSpecialFolderPath(NULL, helper, _wtoi(environString));
#else
			SHGetSpecialFolderPathW(NULL, helper, StrToIntW(environString), FALSE);
			//UtilGetSpecialFolderPath(NULL, helper, _aoi(environString));
#endif
			StringCchCatW(dest, stringSize, helper);
			pEnv = environString;
			string = CharNextW(p);
			*p = 0;
			p = string;
			inBrace = 0;
		}
		else
		{
			if (inPercent || inBrace)
			{
				ptrdiff_t count = CharNextW(p) - p;
				while (count-- && (pEnv - environString) < MAX_PATH)
					*pEnv++ = *p++;
			}
			else
				p = CharNextW(p);
		}
	}

	StringCchCatW(dest, stringSize, string);
}

#ifndef NO_INPLACE_RESOLVE
#include <malloc.h>
// call this function if you want to modify in-place
#include "main.h"

#ifdef __cplusplus
extern "C" void ResolveEnvironmentVariables(wchar_t *string, size_t stringSize) // 
#else
void ResolveEnvironmentVariables(wchar_t *string, size_t stringSize) // 
#endif
{
	wchar_t *dest = (wchar_t *) alloca(stringSize * sizeof(dest[0]));
	ResolveEnvironmentVariables2(string, dest, stringSize);
	StringCchCopyW(string, stringSize, dest);
}
#endif