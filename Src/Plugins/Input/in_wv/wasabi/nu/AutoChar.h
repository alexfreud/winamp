#ifndef NULLSOFT_AUTOCHARH
#define NULLSOFT_AUTOCHARH
#ifdef WIN32
#include <windows.h>

inline char *AutoCharDupN(const wchar_t *convert, size_t len, UINT codePage = CP_ACP, UINT flags=0)
{
	if (!convert)
		return 0;
	int size = WideCharToMultiByte(codePage, flags, convert, (int)len, 0, 0, NULL, NULL);

	if (!size)
		return 0;

	char *narrow = (char *)malloc((size+1)*sizeof(char));

	if (!WideCharToMultiByte(codePage, flags, convert, (int)len, narrow, size, NULL, NULL))
	{
		free(narrow);
		narrow=0;
	}
	else
		narrow[size]=0;

	return narrow;
}

inline char *AutoCharDup(const wchar_t *convert, UINT codePage = CP_ACP, UINT flags=0)
{
	if (!convert)
		return 0;
	int size = WideCharToMultiByte(codePage, flags, convert, -1, 0, 0, NULL, NULL);

	if (!size)
		return 0;

	char *narrow = (char *)malloc(size*sizeof(char));

	if (!WideCharToMultiByte(codePage, flags, convert, -1, narrow, size, NULL, NULL))
	{
		free(narrow);
		narrow=0;
	}
	return narrow;
}

class AutoChar
{
public:
	AutoChar(const wchar_t *convert, UINT codePage = CP_ACP, UINT flags=0) : narrow(0)
	{
		narrow = AutoCharDup(convert, codePage, flags);
	}
	~AutoChar()
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
protected:
	AutoChar() : narrow(0)
	{
	}
	char *narrow;
};

class AutoCharN : public AutoChar
{
public:
	AutoCharN(const wchar_t *convert, size_t len, UINT codePage = CP_ACP, UINT flags=0)
	{
		narrow = AutoCharDupN(convert, len, codePage, flags);
	}
};
#else
#include <stdlib.h>
#include <wchar.h>

inline char *AutoCharDup(const wchar_t *convert)
{
	if (!convert)
		return 0;

	size_t size = wcslen(convert)+1;

	if (!size)
		return 0;

	char *narrow = (char *)malloc(size*sizeof(char));

	if (wcstombs(narrow, convert, size) == (size_t)-1)
	{
		free(narrow);
		narrow=0;
	}
	return narrow;
}


class AutoChar
{
public:

	AutoChar(const wchar_t *convert) : narrow(0)
	{
		narrow = AutoCharDup(convert);
	}
	~AutoChar()
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

#endif