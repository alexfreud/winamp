#include "main.h"

int NXURICreateWithUTF16(nx_uri_t *out, const wchar_t *utf16)
{
	nx_string_t nx_filename = 0;
	nx_uri_t uri_filename = 0;

	int ret = NXStringCreateWithUTF16(&nx_filename, utf16);
	if (ret != NErr_Success)
		return ret;

	ret = NXURICreateWithNXString(&uri_filename, nx_filename);
	NXStringRelease(nx_filename);
	if (ret != NErr_Success)
		return ret;
	
	*out = uri_filename;
	return NErr_Success;
}