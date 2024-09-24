#ifndef NULLSOFT_WINAMP_UPDATE_SERVICE_HEADER
#define NULLSOFT_WINAMP_UPDATE_SERVICE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../omBrowser/ifc_omservice.h"

class obj_ombrowser;

class UpdateService : public ifc_omservice
{
protected:
	UpdateService(obj_ombrowser *browserObject, LPWSTR pszUrl);
	~UpdateService();

public:
	static HRESULT CreateInstance(LPCSTR pszUrl, UpdateService **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omservice */
	unsigned int GetId();
	HRESULT GetName(wchar_t *pszBuffer, int cchBufferMax);
	HRESULT GetUrl(wchar_t *pszBuffer, int cchBufferMax);
	HRESULT GetExternal(IDispatch **ppDispatch);
	HRESULT GetIcon(wchar_t *pszBuffer, int cchBufferMax);

public:
	HRESULT Show();
	HRESULT Finish();

protected:
	ULONG ref;
	LPWSTR url;
	obj_ombrowser *browserManager;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_WINAMP_UPDATE_SERVICE_HEADER