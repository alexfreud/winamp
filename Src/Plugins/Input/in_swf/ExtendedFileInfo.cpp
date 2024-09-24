#include "main.h"
#include "resource.h"
#include <shlwapi.h>
#include <strsafe.h>
#include "api.h"

#define TESTKEYWORD(__keyword, __string)\
	(CSTR_EQUAL == CompareStringA(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),\
								NORM_IGNORECASE, (__keyword), -1, (__string), -1))

extern "C" __declspec( dllexport ) int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, int destlen)
{
	if (TESTKEYWORD("type", data))
	{
		if (NULL != dest)
		{
			int index = 0;
			if (destlen > 1)
				dest[index++] = L'1';
			dest[index] = L'\0';
		}
		return 1;
	}
	else if (TESTKEYWORD("family", data))
	{
		LPCWSTR e, p(NULL);
		e = PathFindExtensionW(fn);
		if (L'.' != *e) return 0;
		e++;
		
		if (CSTR_EQUAL == CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 
				NORM_IGNORECASE, e, -1, L"SWF", -1))
		{
			if (S_OK == StringCchCopyW(dest, destlen, WASABI_API_LNGSTRINGW(IDS_FAMILY_STRING))) return 1;
			//p = L"Shockwave Flash";
		}
		//if (p && S_OK == StringCchCopyW(dest, destlen, p)) return 1;
		return 0;
	}
	return 0;
}