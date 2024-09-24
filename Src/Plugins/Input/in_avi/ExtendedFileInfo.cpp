#include <bfc/platform/types.h>
#include <windows.h>
#include "api__in_avi.h"
#include "win32_avi_reader.h"
#include "../nsavi/metadata.h"
#include "../nu/ns_wc.h"
#include <strsafe.h>
#include "resource.h"

static void ReadMetadata(nsavi::Metadata &metadata, uint32_t id, wchar_t *dest, size_t destlen)
{
	nsavi::Info *info=0;
	const char *str = 0;
	if (metadata.GetInfo(&info) == nsavi::READ_OK && (str = info->GetMetadata(id)))
	{
		MultiByteToWideCharSZ(CP_ACP/*UTF8*/, 0, str, -1, dest, (int)destlen);
	}
	else
		dest[0]=0;
}

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
		if (!_wcsicmp(p, L"AVI") && S_OK == StringCchCopyW(dest, destlen, WASABI_API_LNGSTRINGW(IDS_FAMILY_STRING))) return 1;
		return 0;
	}
	else
	{
		AVIReaderWin32 reader;
		if (reader.Open(fn) == nsavi::READ_OK) 
		{
			nsavi::Metadata metadata(&reader); 
			uint32_t riff_type;
			metadata.GetRIFFType(&riff_type); // need to call this to get the party started
			// TODO: cache metadata object

			if (!_stricmp(data, "length"))
			{
				int time_ms;
				if (metadata.GetDuration(&time_ms) == nsavi::READ_OK)
					StringCchPrintf(dest, destlen, L"%d", time_ms);
				else
					dest[0]=0;
			}
			else if (!_stricmp(data, "height"))
			{
				nsavi::HeaderList header_list;
				if (metadata.GetHeaderList(&header_list) == nsavi::READ_OK && header_list.avi_header && header_list.avi_header->height)
					StringCchPrintf(dest, destlen, L"%d", header_list.avi_header->height);
				else
					dest[0]=0;
			}
			else if (!_stricmp(data, "width"))
			{
				nsavi::HeaderList header_list;
				if (metadata.GetHeaderList(&header_list) == nsavi::READ_OK && header_list.avi_header && header_list.avi_header->width)
					StringCchPrintf(dest, destlen, L"%d", header_list.avi_header->width);
				else
					dest[0]=0;
			}
			else if (!_stricmp(data, "bitrate"))
			{
				int time_ms = 0;
				uint64_t file_length = 0;
				if (metadata.GetDuration(&time_ms) == nsavi::READ_OK
					&& (file_length = reader.GetContentLength())
					&& time_ms > 0 && file_length > 0)
				{
					uint64_t bitrate = 8ULL * file_length / (uint64_t)time_ms;
					StringCchPrintf(dest, destlen, L"%I64u", bitrate);
				}
				else
					dest[0]=0;
			}
			else if (!_stricmp(data, "artist"))
			{
				ReadMetadata(metadata, nsaviFOURCC('I','A','R','T'), dest, destlen);
			}
			else if (!_stricmp(data, "publisher"))
			{
				ReadMetadata(metadata, nsaviFOURCC('I','P','U','B'), dest, destlen);
			}
			else if (!_stricmp(data, "album"))
			{
				ReadMetadata(metadata, nsaviFOURCC('I','A','L','B'), dest, destlen);
			}
			else if (!_stricmp(data, "composer"))
			{
				ReadMetadata(metadata, nsaviFOURCC('I','C','O','M'), dest, destlen);
			}
			else if (!_stricmp(data, "genre"))
			{
				ReadMetadata(metadata, nsaviFOURCC('I','G','N','R'), dest, destlen);
			}
			else if (!_stricmp(data, "comment"))
			{
				ReadMetadata(metadata, nsaviFOURCC('I','C','M','T'), dest, destlen);
			}
			else if (!_stricmp(data, "title"))
			{
				ReadMetadata(metadata, nsaviFOURCC('I','N','A','M'), dest, destlen);
			}
			else if (!_stricmp(data, "tool"))
			{
				ReadMetadata(metadata, nsaviFOURCC('I','S','F','T'), dest, destlen);
			}
			else if (!_stricmp(data, "copyright"))
			{
				ReadMetadata(metadata, nsaviFOURCC('I','C','O','P'), dest, destlen);
			}
			else
			{
				reader.Close();
				return 0;
			}

			reader.Close();
			return 1;
		}
	}

	return 0;
}