#ifndef NULLSOFT_WINAMP_OMBROWSER_WINDOW_ENUMERATOR_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_WINDOW_ENUMERATOR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_ombrowserwndenum.h"

class OmBrowserWndRecord;

class OmBrowserWndEnumerator : public ifc_ombrowserwndenum
{

protected:
	OmBrowserWndEnumerator(const GUID *windowTypeFilter, const UINT *serviceIdFilter, OmBrowserWndRecord * const *windowList, size_t windowListSize);
	~OmBrowserWndEnumerator();

public:
	static HRESULT CreateInstance(const GUID *windowTypeFilter, const UINT *serviceIdFilter, OmBrowserWndRecord * const *windowList, size_t windowListSize, OmBrowserWndEnumerator **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_ombrowserwndenum */
	HRESULT Next(ULONG listSize, HWND *elementList, ULONG *elementCount);
	HRESULT Reset(void);
	HRESULT Skip(ULONG elementCount);
	
protected:
	ULONG ref;
	size_t index;
	OmBrowserWndRecord **list;
	size_t size;
	UINT serviceId;
	GUID windowType;
	BOOL filterId;
	BOOL filterType;

protected:
	RECVS_DISPATCH;

};

#endif //NULLSOFT_WINAMP_OMBROWSER_WINDOW_ENUMERATOR_HEADER