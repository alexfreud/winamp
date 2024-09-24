#include "nxuri.h"
#include <stdlib.h>
#include "foundation/atomics.h"
#include "foundation/error.h"
#include "nxstring.h" // for string_heap
#include "foundation/atomics.h"

HANDLE string_heap;

int NXStringFree(HANDLE heap, nx_string_t str);

nx_uri_t NXURIRetain(nx_uri_t string)
{
	if (!string)
	{
		return 0;
	}

	nx_atomic_inc(&string->ref_count);
	return string;
}

void NXURIRelease(nx_uri_t string)
{
	if (string)
	{
		if (nx_atomic_dec(&string->ref_count) == 0)
		{
			NXStringFree(string_heap, (nx_string_t)string);
		}
	}
}

// don't include null terminator here
nx_uri_t NXURIMalloc(size_t characters)
{
	return (nx_uri_t)NXStringMalloc(characters);
}

int NXURICreateWithNXString(nx_uri_t *uri, nx_string_t string)
{
	if (!string)
	{
		return NErr_Empty;
	}

	*uri = NXURIRetain((nx_uri_t)string);
	return NErr_Success;
}

int NXURICreateFromPath(nx_uri_t *uri, const wchar_t *filename, const nx_uri_t path)
{
	size_t filename_length = wcslen(filename);
	size_t path_length = path->len;
	size_t total_length = filename_length + path_length; /* TODO: check for overflow */
	int need_slash = 1; 
	nx_uri_t output=0;
	if (path_length && (path->string[path_length-1] == '/' || path->string[path_length-1] == '\\'))
	{
		need_slash=0;
	}
	else
	{
		total_length++; /* TODO: check for overflow */
	}

	output = NXURIMalloc(total_length);
	if (!output)
	{
		return NErr_OutOfMemory;
	}

	wmemcpy(output->string, path->string, path_length);
	if (need_slash)
	{
		output->string[path_length]='\\'; /* TODO: URL detection to know whether to add / or \\ */
		wcscpy(&output->string[path_length+1], filename);
	}
	else
	{
		wcscpy(&output->string[path_length], filename);
	}

	*uri = output;
	return NErr_Success;
}

int NXURICreateWithPath(nx_uri_t *uri, const nx_uri_t filename, const nx_uri_t path)
{
	size_t filename_length = filename->len;
	size_t path_length = path->len;
	size_t total_length = filename_length + path_length; /* TODO: check for overflow */
	int need_slash = 1; 
	nx_uri_t output=0;
	if (path_length && (path->string[path_length-1] == '/' || path->string[path_length-1] == '\\'))
	{
		need_slash=0;
	}
	else
	{
		total_length++; /* TODO: check for overflow */
	}

	output = NXURIMalloc(total_length);
	if (!output)
	{
		return NErr_OutOfMemory;
	}

	wmemcpy(output->string, path->string, path_length);
	if (need_slash)
	{
		output->string[path_length]='\\'; /* TODO: URL detection to know whether to add / or \\ */
		wcscpy(&output->string[path_length+1], filename->string);
	}
	else
	{
		wcscpy(&output->string[path_length], filename->string);
	}

	*uri = output;
	return NErr_Success;
}

int NXURIGetNXString(nx_string_t *string, nx_uri_t uri)
{
	*string = (nx_string_t)NXURIRetain(uri);
	return NErr_Success;
}

static const wchar_t *FindFilename(nx_uri_t filename)
{
	size_t position;
	if (!filename || !filename->string || !filename->len)
	{
		return 0;
	}

	position=filename->len;
	while (position--)
	{
		wchar_t c = filename->string[position];
		if (c == '/' || c == '\\')
		{
			return &filename->string[position+1];
		}
	}
	return 0;
}

int NXURICreateTempForFilepath(nx_uri_t *out_temp, nx_uri_t filename)
{
	nx_uri_t new_uri;
	size_t path_length = 0;
	wchar_t temp_part[64] = {0};
#if _WIN32_WINNT >= 0x600
	int temp_length = wsprintf(temp_part, L".%x-%I64x-%d.tmp", GetCurrentThreadId(), GetTickCount64(), rand());
#else
	int temp_length = wsprintf(temp_part, L".%x-%Ix-%d.tmp", GetCurrentThreadId(), GetTickCount(), rand());
#endif
	const wchar_t *filepart = FindFilename(filename);
	if (filepart)
	{
		path_length = (filepart - filename->string);
	}
	else
	{
		path_length=0;
	}
	new_uri = NXURIMalloc(path_length+temp_length);
	if (!new_uri)
	{
		return NErr_OutOfMemory;
	}
	wmemcpy(new_uri->string, filename->string, path_length);
	wmemcpy(new_uri->string+path_length, temp_part, temp_length);
	new_uri->string[path_length+temp_length]=0;
	*out_temp = new_uri;
	return NErr_Success;
}

int NXURICreateWithUTF8(nx_uri_t *value, const char *utf8)
{
	nx_string_t nx_filename;
	nx_uri_t uri_filename;

	int ret = NXStringCreateWithUTF8(&nx_filename, utf8);
	if (ret != NErr_Success)
	{
		return ret;
	}

	ret = NXURICreateWithNXString(&uri_filename, nx_filename);
	NXStringRelease(nx_filename);
	if (ret != NErr_Success)
		return ret;

	*value = uri_filename;
	return NErr_Success;
}

int NXURICreateRemovingFilename(nx_uri_t *out_uri, nx_uri_t filename)
{
	nx_uri_t new_uri;
	size_t path_length;

	const wchar_t *filepart = FindFilename(filename);
	if (filepart)
	{
		path_length = (filepart - filename->string);
	}
	else
	{
		path_length=0;
	}
	new_uri = NXURIMalloc(path_length);
	if (!new_uri)
	{
		return NErr_OutOfMemory;
	}
	wmemcpy(new_uri->string, filename->string, path_length);
	new_uri->string[path_length]=0;
	*out_uri = new_uri;
	return NErr_Success;
}

int NXURICreateTemp(nx_uri_t *out_temp)
{
	return NXURICreateTempWithExtension(out_temp, "tmp");
}

int NXURICreateTempWithExtension(nx_uri_t *out_temp, const char *extension)
{
	nx_uri_t new_uri;
	wchar_t temppath[MAX_PATH-14] = {0}; // MAX_PATH-14 'cause MSDN said so
	int path_length = GetTempPathW(MAX_PATH-14, temppath);
	wchar_t temp_part[64] = {0};
#if _WIN32_WINNT >= 0x600
	int temp_length = wsprintf(temp_part, L".%x-%I64x-%d.%S", GetCurrentThreadId(), GetTickCount64(), rand(), extension);
#else
	int temp_length = wsprintf(temp_part, L".%x-%Ix-%d.%S", GetCurrentThreadId(), GetTickCount(), rand(), extension);
#endif
	new_uri = NXURIMalloc(path_length+temp_length);
	if (!new_uri)
	{
		return NErr_OutOfMemory;
	}
	wmemcpy(new_uri->string, temppath, path_length);
	wmemcpy(new_uri->string+path_length, temp_part, temp_length);
	new_uri->string[path_length+temp_length]=0;
	*out_temp = new_uri;
	return NErr_Success;
}

size_t NXURIGetLength(nx_uri_t string)
{
	return (string ? string->len : 0);
}