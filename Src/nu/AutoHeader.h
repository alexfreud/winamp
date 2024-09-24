#ifndef NULLSOFT_AUTOHEADERH
#define NULLSOFT_AUTOHEADERH

#include <windows.h>
#include "AutoChar.h"



/* encodes a UTF-8 string into a buffer, returns a pointer to the end of the string */
inline char *AutoHeader_Encode(const char *in, char *out, size_t len)
{
	if (!len)
		return 0;

	char *dest=out;
	const unsigned char *src = (const unsigned char *)in;
	while (*src && --len)
	{
		if ((*src >= 'A' && *src <= 'Z') ||
		        (*src >= 'a' && *src <= 'z') ||
		        (*src >= '0' && *src <= '9') || *src == '.' || *src == '-' || *src == '~')
		{
			*dest++=*src++;
		}
		else if (len > 2)
		{
			int i = *src++;
			*dest++ = '=';
			int b = (i >> 4) & 15;
			if (b < 10) *dest++ = '0' + b;
			else *dest++ = 'A' + b - 10;
			b = i & 15;
			if (b < 10) *dest++ = '0' + b;
			else *dest++ = 'A' + b - 10;
		}
		else
			break;
	}
	*dest=0;
	return dest;
}


inline char *AutoHeaderDup(const wchar_t *convert)
{
	if (!convert)
		return 0;

	BOOL failed=FALSE;
	int n = WideCharToMultiByte(28591, WC_NO_BEST_FIT_CHARS, convert, -1, 0, 0, 0, &failed);
	if (n && !failed)
	{
		char *url = (char *)malloc(n + 1);
		WideCharToMultiByte(28591, WC_NO_BEST_FIT_CHARS, convert, -1, url, n+1, 0, 0);
		return url;
	}

	AutoChar utf8(convert, CP_UTF8);
	size_t size = strlen(utf8)*3+1; // one byte might get encoded to 3 bytes, so we'll malloc for worst-case

	char *url= (char *)malloc((9  /*?utf-8?q?*/+ size + 2 /*?=*/)*sizeof(char));
	memcpy(url, "?utf-8?q?", 9);
	char *end = AutoHeader_Encode(utf8, url+9, size);
	strcpy(end, "?=");
	return url;
}

class AutoHeader
{
public:

	AutoHeader(const wchar_t *convert) : narrow(0)
	{
		narrow = AutoHeaderDup(convert);
	}
	AutoHeader(const AutoHeader &convert) : narrow(0)
	{
		if (convert.narrow)
			narrow = _strdup(convert.narrow);
	}
	~AutoHeader()
	{
		free(narrow);
		narrow=0;
	}
	operator const char *()
	{
		return narrow;
	}
	operator char *()
	{
		return narrow;
	}
private:
	char *narrow;
};

#endif