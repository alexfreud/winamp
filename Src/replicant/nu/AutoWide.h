#ifndef AUTOWIDEH
#define AUTOWIDEH
#ifdef WIN32
#include <windows.h>
#include <stdlib.h>

inline wchar_t *AutoWideDup(const char *convert, UINT codePage=CP_ACP)
{
	if (!convert)
		return 0;


	wchar_t *wide = 0;

	int size = MultiByteToWideChar(codePage, 0, convert, -1, 0,0);
	if (!size)
		return 0;

	wide = (wchar_t *)malloc(size*sizeof(wchar_t));
	if (!MultiByteToWideChar(codePage, 0, convert, -1, wide,size))
	{
		free(wide);
		wide=0;
	}
	return wide;
}

class AutoWide
{
public:
	AutoWide(const char *convert, UINT codePage=CP_ACP) :  wide(0)
	{
		wide = AutoWideDup(convert, codePage);
	}
	~AutoWide()
	{
		free(wide);
		wide=0;
	}
	operator wchar_t *()
	{
		return wide;
	}
private:
	wchar_t *wide;
};

#endif

#endif
