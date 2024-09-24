#ifndef NULLSOFT_AUTH_LOGIN_UPDATE_HEADER
#define NULLSOFT_AUTH_LOGIN_UPDATE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../nu/ptrlist.h"

class LoginDownloadResult;
class LoginStatus;

class LoginUpdate
{

protected:
	LoginUpdate(HWND hLoginbox);
	~LoginUpdate();

public:
	static HRESULT CreateInstance(HWND hLoginbox, LoginUpdate **instance);

public:
	ULONG AddRef();
	ULONG Release();

	HRESULT Start();
	HRESULT Abort();

protected:
	void DownloadCompleted(LoginDownloadResult *result);
	

protected:
	typedef nu::PtrList<LoginDownloadResult> DownloadList;
	friend static void CALLBACK LoginUpdate_DownloadCompleted(LoginDownloadResult *result, void *data);

protected:
	ULONG ref;
	HWND hwnd;
	DownloadList downloads;
	CRITICAL_SECTION lock;

};

#endif //NULLSOFT_AUTH_LOGIN_UPDATE_HEADER

