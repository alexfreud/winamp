#include "nsid3v2/nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include <new>
#include <strsafe.h>


/*
================== Tag ================== 
=                                       =
========================================= 
*/
#if 0 // save for reference
int NSID3v2_Frame_Text_SetUTF16(nsid3v2_frame_t f, const wchar_t *value)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	size_t len = wcslen(value);
	size_t bytes = len * 2 + 1; // leave 1 byte for encoding
	if (bytes < len) // woops, integer overflow
		return NErr_Error;

	size_t datalen;
	void *data;
	int ret = frame->NewData(bytes, &data, &datalen);
	if (ret == NErr_Success)
	{
		uint8_t *data8 = (uint8_t *)data;
		data8[0]=1; // set encoding to UTF-16
		memcpy(data8+1, value, len*2);
	}
	return ret;
}

int NSID3v2_Frame_UserText_SetUTF16(nsid3v2_frame_t f, const wchar_t *description, const wchar_t *value)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	size_t value_len = wcslen(value);
	size_t description_len = wcslen(description);
	size_t bytes = (value_len + description_len + 1) * 2 + 1; // leave 1 byte for encoding

	size_t datalen;
	void *data;
	int ret = frame->NewData(bytes, &data, &datalen);
	if (ret == NErr_Success)
	{
		uint8_t *data8 = (uint8_t *)data;
		data8[0]=1; // set encoding to UTF-16
		wcscpy((wchar_t *)(data8+1), description); // guaranteed to be room
		memcpy(data8+1+1+description_len*2, value, value_len*2);
	}
	return ret;
}

int NSID3v2_Frame_UserText_SetLatin(nsid3v2_frame_t f, const char *description, const char *value)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	size_t value_len = strlen(value);
	size_t description_len = strlen(description);
	size_t bytes = (value_len + description_len + 1) + 1; // leave 1 byte for encoding

	size_t datalen;
	void *data;
	int ret = frame->NewData(bytes, &data, &datalen);
	if (ret == NErr_Success)
	{
		uint8_t *data8 = (uint8_t *)data;
		data8[0]=0; // set encoding to ISO-8859-1
		strcpy((char *)(data8+1), description); // guaranteed to be room
		memcpy(data8+1+1+description_len, value, value_len);
	}
	return ret;
}
#endif
