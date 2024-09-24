#ifndef NULLSOFT_WINAMP_STRING_BUILDER_HEADER
#define NULLSOFT_WINAMP_STRING_BUILDER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class StringBuilder
{
public:
	StringBuilder();
	~StringBuilder();

public:
	HRESULT Allocate(size_t newSize);
	void Clear(void);
	LPCWSTR Get(void);
	HRESULT Set(size_t index, WCHAR value);
	HRESULT Append(LPCWSTR pszString);
	

protected:
	LPWSTR buffer;
	LPWSTR cursor;
	size_t allocated;
	size_t remaining;
};

#endif //NULLSOFT_WINAMP_STRING_BUILDER_HEADER