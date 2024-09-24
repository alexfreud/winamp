#ifndef NULLOSFT_ONLINEMEDIA_FORCE_URL_HEADER
#define NULLOSFT_ONLINEMEDIA_FORCE_URL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class ForceUrl
{
public:
	ForceUrl();
	~ForceUrl();

public:
	HRESULT Set(UINT serviceId, LPCWSTR pszUrl);
	HRESULT Peek(UINT serviceId, LPWSTR *pszUrl);
	HRESULT Remove(UINT serviceId);

	void FreeString(LPWSTR pszValue);
private:
	UINT id;
	LPWSTR url;
};

#endif //NULLOSFT_ONLINEMEDIA_FORCE_URL_HEADER