#ifndef NULLSOFT_AUTOURLH
#define NULLSOFT_AUTOURLH

#include <windows.h>
#include "AutoChar.h"

/* benski> i'm sure there's a nice optimized way of doing this, but I need to implement it _right now_ */
#define HEXCASE(d) case 0x##d: return #@d
inline char quickhex(unsigned char in)
{
	switch (in)
	{
		HEXCASE(0);
		HEXCASE(1);
		HEXCASE(2);
		HEXCASE(3);
		HEXCASE(4);
		HEXCASE(5);
		HEXCASE(6);
		HEXCASE(7);
		HEXCASE(8);
		HEXCASE(9);
		HEXCASE(A);
		HEXCASE(B);
		HEXCASE(C);
		HEXCASE(D);
		HEXCASE(E);
		HEXCASE(F);

	}
	return 0;
}

/* encodes a UTF-8 string into a buffer */
inline void AutoUrl_Encode(const char *in, char *out, size_t len)
{
	if (!len)
		return;

	char *dest=out;
	const unsigned char *src = (const unsigned char *)in;
	while (*src && --len)
	{
		if ((*src >= 'A' && *src <= 'Z') ||
		        (*src >= 'a' && *src <= 'z') ||
		        (*src >= '0' && *src <= '9') || *src == '.' || *src == '_' || *src == '-' || *src == '~')
		{
			*dest++=*src++;
		}
		else if (len > 2)
		{
			int i = *src++;
			*dest++ = '%';
			int b = (i >> 4) & 15;
			if (b < 10) *dest++ = (char)('0' + b);
			else *dest++ = (char)('A' + b - 10);
			b = i & 15;
			if (b < 10) *dest++ = (char)('0' + b);
			else *dest++ = (char)('A' + b - 10);
		}
		else
			break;
	}
	*dest=0;
}

inline char *AutoUrlDupN(const wchar_t *convert, size_t len)
{
	if (!convert)
		return 0;
	AutoCharN utf8(convert, len, CP_UTF8);
	size_t size = strlen(utf8)*3+1; // one byte might get encoded to 3 bytes, so we'll malloc for worst-case

	char *url= (char *)malloc(size*sizeof(char));
	AutoUrl_Encode(utf8, url, size);
	return url;
}

inline char *AutoUrlDup(const wchar_t *convert)
{
	if (!convert)
		return 0;
	AutoChar utf8(convert, CP_UTF8);
	size_t size = strlen(utf8)*3+1; // one byte might get encoded to 3 bytes, so we'll malloc for worst-case

	char *url= (char *)malloc(size*sizeof(char));
	AutoUrl_Encode(utf8, url, size);
	return url;
}

inline char *AutoUrlDup(const char *utf8)
{
	if (!utf8)
		return 0;

	size_t size = strlen(utf8)*3+1; // one byte might get encoded to 3 bytes, so we'll malloc for worst-case

	char *url= (char *)malloc(size*sizeof(char));
	AutoUrl_Encode(utf8, url, size);
	return url;
}

class AutoUrl
{
public:

	AutoUrl(const wchar_t *convert) : narrow(0)
	{
		narrow = AutoUrlDup(convert);
	}
	AutoUrl(const wchar_t *convert, size_t len) : narrow(0)
	{
		narrow = AutoUrlDupN(convert, len);
	}
	AutoUrl(const char *convert) : narrow(0)
	{
		narrow = AutoUrlDup(convert);
	}
	AutoUrl(const AutoUrl &convert) : narrow(0)
	{
		if (convert.narrow)
			narrow = _strdup(convert.narrow);
	}
	~AutoUrl()
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