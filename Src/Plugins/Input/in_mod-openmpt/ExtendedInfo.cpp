#include <bfc/platform/types.h>
#include <windows.h>
#include <strsafe.h>
#include "api__in_mod.h"
#include "resource.h"
#include <libopenmpt/libopenmpt.h>
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
#include <libopenmpt/libopenmpt_stream_callbacks_file.h>

static wchar_t *open_filename = 0;
static openmpt_module *info_mod = 0;
openmpt_module *OpenMod(const wchar_t *filename);

extern "C" __declspec(dllexport)
	int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, size_t destlen)
{
	if (!_stricmp(data, "type"))
	{
		dest[0]='0';
		dest[1]=0;
		return 1;
	}
	else if (!_stricmp(data, "family"))
	{
		size_t len;
		const wchar_t *p;
		if (!fn || !fn[0]) {
			return 0;
		}
		len = wcslen(fn);
		if (len < 4 || L'.' != fn[len - 4]) {
			return 0;
		}
		p = &fn[len - 3];
		const char *tracker = openmpt_get_tracker_name(AutoChar(p));
		if (tracker && *tracker) {
			*dest = 0;
			MultiByteToWideCharSZ(CP_UTF8, 0, tracker, -1, dest, (int)destlen);
			openmpt_free_string(tracker);
			return 1;
		}
		openmpt_free_string(tracker);
		return 0;
	} else {
		if (!open_filename || _wcsicmp(open_filename,fn)) {
			free(open_filename);
			open_filename = _wcsdup(fn);
			openmpt_module_destroy(info_mod);
			info_mod = 0;
			info_mod = OpenMod(fn);
		}
		int retval = 0;

		if (!_stricmp(data, "length")) {
			double seconds = openmpt_module_get_duration_seconds(info_mod);
			StringCchPrintf(dest, destlen, L"%.0f", seconds*1000.0);
			retval = 1;
		} else if (!_stricmp(data, "artist")) {
			const char *value = openmpt_module_get_metadata(info_mod, "artist");
			MultiByteToWideCharSZ(CP_UTF8, 0, value, -1, dest, (int)destlen);
			openmpt_free_string(value);
			retval = 1;
		} else if (!_stricmp(data, "tool")) {
			const char *value = openmpt_module_get_metadata(info_mod, "tracker");
			MultiByteToWideCharSZ(CP_UTF8, 0, value, -1, dest, (int)destlen);
			openmpt_free_string(value);
			retval = 1;
		} else if (!_stricmp(data, "title")) {
			const char *value = openmpt_module_get_metadata(info_mod, "title");
			MultiByteToWideCharSZ(CP_UTF8, 0, value, -1, dest, (int)destlen);
			openmpt_free_string(value);
			retval = 1;
		} else if (!_stricmp(data, "year")) {
			const char *value = openmpt_module_get_metadata(info_mod, "date");
			MultiByteToWideCharSZ(CP_UTF8, 0, value, -1, dest, (int)destlen);
			openmpt_free_string(value);
			retval = 1;
		} else if (!_stricmp(data, "comment")) {
			const char *value = openmpt_module_get_metadata(info_mod, "message");
			MultiByteToWideCharSZ(CP_UTF8, 0, value, -1, dest, (int)destlen);
			openmpt_free_string(value);
			retval = 1;
		}

		return retval;
	}

	return 0;
}