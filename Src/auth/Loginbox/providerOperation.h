#ifndef NULLSOFT_AUTH_LOGIN_PROVIDER_OPERATION_HEADER
#define NULLSOFT_AUTH_LOGIN_PROVIDER_OPERATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class LoginProvider;
class LoginProviderEnumerator;

class LoginProviderOperation
{
public:
	typedef enum
	{
		operationDelete = 0,
		operationReplace = 1,
	} OperationCode;

protected:
	LoginProviderOperation(LoginProvider *pSource, LoginProvider *pTarget, UINT uCode);
	~LoginProviderOperation();

public:
	static HRESULT CreateDeleteOperation(LoginProvider *pRemove, LoginProviderOperation **instance);
	static HRESULT CreateReplaceOperation(LoginProvider *pSource, LoginProvider *pTarget, LoginProviderOperation **instance);
	static HRESULT CreateFromUpdate(LoginProvider *active, LoginProviderEnumerator *enumerator, LoginProviderOperation **instance);

public:
	ULONG AddRef();
	ULONG Release();

	UINT GetCode();
	HRESULT GetSource(LoginProvider **provider);
	HRESULT GetTarget(LoginProvider **provider);

protected:
	ULONG ref;
	LoginProvider *source;
	LoginProvider *target;
	UINT code;

};

#endif //NULLSOFT_AUTH_LOGIN_PROVIDER_OPERATION_HEADER