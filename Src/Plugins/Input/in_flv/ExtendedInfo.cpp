#include <bfc/platform/types.h>
#include <windows.h>
#include <strsafe.h>
#include "api__in_flv.h"
#include "resource.h"

extern "C" __declspec(dllexport)
	int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, size_t destlen)
{
	if (!_stricmp(data, "type"))
	{
		dest[0]='1';
		dest[1]=0;
		return 1;
	}
	else if (!_stricmp(data, "family"))
	{
		int len;
		const wchar_t *p;
		if (!fn || !fn[0]) return 0;
		len = lstrlenW(fn);
		if (len < 4 || L'.' != fn[len - 4])  return 0;
		p = &fn[len - 3];
		if (!_wcsicmp(p, L"FLV") && S_OK == StringCchCopyW(dest, destlen, WASABI_API_LNGSTRINGW(IDS_FAMILY_STRING))) return 1;
		return 0;
	}

	return 0;
}