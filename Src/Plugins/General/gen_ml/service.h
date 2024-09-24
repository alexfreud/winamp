#pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../ombrowser/ifc_omservice.h"

#define SERVICE_SIGN_IN		750
#define SERVICE_SOURCES		751
#define SERVICE_LABS		752

class OmService : public ifc_omservice
{
protected:
	OmService(UINT nId);
	~OmService();

public:
	static HRESULT CreateInstance(UINT nId, LPCWSTR pszName, OmService **instance);

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
	HRESULT SetName(LPCWSTR pszName);
	HRESULT SetUrl(LPCWSTR pszUrl);
	HRESULT SetIcon(LPCWSTR pszIcon);

protected:
	RECVS_DISPATCH;

protected:
	ULONG ref;
	UINT id;
	LPWSTR name;
	LPWSTR url;
};