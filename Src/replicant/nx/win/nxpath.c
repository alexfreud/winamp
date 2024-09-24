#include "nxpath.h"
#include <shlwapi.h>
#include "foundation/error.h"

static const wchar_t *FindExtension(nx_uri_t filename)
{
	size_t position;
	if (!filename || !filename->string || !filename->len)
		return 0;

	position=filename->len;
	while (position--)
	{
		wchar_t c = filename->string[position];
		if (c == '.')
			return &filename->string[position+1];
		if (c == '/' || c == '\\')
			return 0;
	}
	return 0;
}

static const wchar_t *FindFilename(nx_uri_t filename)
{
	size_t position;
	if (!filename || !filename->string || !filename->len)
		return 0;

	position=filename->len;
	while (position--)
	{
		wchar_t c = filename->string[position];
		if (c == '/' || c == '\\')
			return &filename->string[position+1];
	}
	return 0;
}

size_t NXPathMatchExtensionList(nx_uri_t filename, nx_string_t *extension_list, size_t num_extensions)
{
	const wchar_t *ext = FindExtension(filename);
	if (ext && *ext)
	{
		size_t i;
		for (i=0;i<num_extensions;i++)
		{
#if WINVER >= 0x0600
			if (CompareStringOrdinal(ext, -1, extension_list[i]->string, -1, TRUE) == CSTR_EQUAL)
				return i;
#else
			if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, ext, -1, extension_list[i]->string, -1) == CSTR_EQUAL)
				return i;
#endif
		}
	}
	return num_extensions;
}

int NXPathMatchExtension(nx_uri_t filename, nx_string_t extension)
{
	const wchar_t *ext = FindExtension(filename);
	if (ext && *ext)
	{
#if WINVER >= 0x0600
		if (CompareStringOrdinal(ext, -1, extension->string, -1, TRUE) == CSTR_EQUAL)
			return NErr_Success;
#else
		if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, ext, -1, extension->string, -1) == CSTR_EQUAL)
			return NErr_Success;
#endif
	}
	return NErr_False;
}

int NXPathProtocol(nx_uri_t filename, const char *protocol)
{
	if (PathIsURL(filename->string))
	{
		wchar_t protocol_from_filename[100];
		DWORD protocol_length=100;
		if (UrlGetPart(filename->string, protocol_from_filename, &protocol_length, URL_PART_SCHEME, 0) == S_OK)
		{
			DWORD i;
			for (i=0;i<protocol_length;i++)
			{
				if ((wchar_t)(protocol[i]) != protocol_from_filename[i])
					return NErr_False;
			}
			return NErr_Success;
		}
	}
	return NErr_False;
}

int NXPathIsURL(nx_uri_t filename)
{
	if (PathIsURL(filename->string))
		return NErr_True;
	else
		return NErr_False;
}

int NXPathIsRelative(nx_uri_t filename)
{
	if (filename->len >= 3 && filename->string[1] == L':' && (filename->string[2] == L'\\' || filename->string[2] == L'/'))
		return NErr_False;

	if (filename->len >= 1 && (filename->string[2] == L'\\' || filename->string[2] == L'/'))
		return NErr_False;

	return NErr_True;
}

