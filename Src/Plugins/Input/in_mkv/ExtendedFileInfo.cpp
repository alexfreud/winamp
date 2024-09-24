#include <bfc/platform/types.h>
#include <windows.h>
#include "api__in_mkv.h"
#include "MKVInfo.h"
#include <strsafe.h>
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
		if ((len>3 && L'.' == fn[len-4]))
		{
			p = &fn[len - 3];
			if (!_wcsicmp(p, L"MKV") && S_OK == StringCchCopyW(dest, destlen, WASABI_API_LNGSTRINGW(IDS_FAMILY_STRING))) return 1;
		}
		else if (len>4 && L'.' == fn[len-5])
		{
			p = &fn[len - 4];
			if (!_wcsicmp(p, L"webm") && S_OK == StringCchCopyW(dest, destlen, WASABI_API_LNGSTRINGW(IDS_FAMILY_STRING_WEBM))) return 1;
		}
		return 0;
	}
	else
	{
		MKVInfo info;
		dest[0]=0;
		if (!_stricmp(data, "length"))
		{
			if (info.Open(fn))
				StringCchPrintf(dest, destlen, L"%d", info.GetLengthMilliseconds());
		}
		else if (!_stricmp(data, "bitrate"))
		{
			if (info.Open(fn))
				StringCchPrintf(dest, destlen, L"%d", info.GetBitrate());
		}
		else if (!_stricmp(data, "title"))
		{
			return 1;
		}
		else if (!_stricmp(data, "width"))
		{

			if (info.Open(fn))
			{
				int width;
				if (info.GetWidth(width))
					StringCchPrintf(dest, destlen, L"%d", width);

			}
		}
		else if (!_stricmp(data, "height"))
		{
			if (info.Open(fn))
			{
				int height;
				if (info.GetHeight(height))
					StringCchPrintf(dest, destlen, L"%d", height);

			}
		}
		else
			return 0;
		return 1;
	}

	return 0;
}