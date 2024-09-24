#ifndef NULLSOFT_AUTH_LOGIN_PROVIDER_ENUMERATOR_HEADER
#define NULLSOFT_AUTH_LOGIN_PROVIDER_ENUMERATOR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class LoginProvider;

class LoginProviderEnumerator
{

protected:
	LoginProviderEnumerator();
	~LoginProviderEnumerator();

public:
	static HRESULT CreateInstance(LoginProvider **list, size_t size, LoginProviderEnumerator **instance);

public:
	ULONG AddRef();
	ULONG Release();

	HRESULT Next(ULONG listSize, LoginProvider **elementList, ULONG *elementCount);
	HRESULT Reset(void);
	HRESULT Skip(ULONG elementCount);

protected:
	ULONG ref;
	size_t cursor;
	size_t size;
	LoginProvider **list;

};

#endif //NULLSOFT_AUTH_LOGIN_PROVIDER_ENUMERATOR_HEADER