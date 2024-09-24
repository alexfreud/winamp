#ifndef NULLSOFT_AUTH_LOGIN_STATUS_HEADER
#define NULLSOFT_AUTH_LOGIN_STATUS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../nu/Vectors.h"

class LoginStatus
{

protected:
	LoginStatus(HWND hTarget);
	~LoginStatus();

public: 
	static HRESULT CreateInstance(HWND hTarget, LoginStatus **instance);

public:
	ULONG AddRef();
	ULONG Release();

	UINT Add(BSTR status);
	BOOL Set(UINT cookie, BSTR status);
	void Remove(UINT cookie);

	BOOL AttachWindow(HWND hTarget);
	BOOL DetachWindow();

protected:
	BOOL UpdateWindowText();
	UINT GetNextCookie();

protected:
	typedef struct __Record
	{
		UINT cookie;
		BSTR text;
	} Record;

	typedef Vector<Record> RecordList;

protected:
	ULONG ref;
	HWND hwnd;
	RecordList list;
	CRITICAL_SECTION lock;

};

#endif //NULLSOFT_AUTH_LOGIN_STATUS_HEADER