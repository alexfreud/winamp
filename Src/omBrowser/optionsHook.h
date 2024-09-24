#ifndef NULLSOFT_WINAMP_OMBROWSER_OPTIONS_HOOK_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_OPTIONS_HOOK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omconfigcallback.h"

class ifc_omservice;

class OptionsConfigHook : public ifc_omconfigcallback
{
protected:
	OptionsConfigHook(HWND hTarget);
	~OptionsConfigHook();

public:
	static HRESULT CreateInstance(HWND hTarget, OptionsConfigHook **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omconfigcallback */
	HRESULT ValueChanged(const GUID *configUid, UINT valueId, ULONG_PTR value);

protected:
	RECVS_DISPATCH;

protected:
	ULONG ref;
	HWND hwnd;
};


#endif //NULLSOFT_WINAMP_OMBROWSER_OPTIONS_HOOK_HEADER