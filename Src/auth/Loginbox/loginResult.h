#ifndef NULLSOFT_AUTH_LOGINRESULT_HEADER
#define NULLSOFT_AUTH_LOGINRESULT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class LoginData;

class __declspec(novtable) LoginResult : public IUnknown
{

public:
	typedef void (CALLBACK *Callback)(LoginResult *result);

protected:
	LoginResult() {}
	~LoginResult() {}

public:
	
	virtual HRESULT GetLoginData(LoginData **loginData) = 0;
	
	virtual HRESULT GetWaitHandle(HANDLE *handle) = 0;
	virtual HRESULT GetUser(void **user) = 0;
	virtual HRESULT	RequestAbort(BOOL fDrop) = 0;
	virtual HRESULT IsCompleted() = 0;
	virtual HRESULT IsAborting() = 0;
	

};

#endif //NULLSOFT_AUTH_LOGINRESULT_HEADER