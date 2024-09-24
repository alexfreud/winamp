#ifndef NULLSOFT_WINAMP_OMBROWSER_UI_HOOK_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_UI_HOOK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_winamphook.h"
#include "./ifc_omserviceevent.h"
#include "./ifc_omconfigcallback.h"
#include <bfc/multipatch.h>

#define MPIID_WINAMPHOOK			10
#define MPIID_SERVICEEVENT			20
#define MPIID_CONFIGCALLBACK		30

class ifc_omservice;
class obj_ombrowser;

class BrowserUiHook :	public MultiPatch<MPIID_WINAMPHOOK, ifc_winamphook>,
						public MultiPatch<MPIID_SERVICEEVENT, ifc_omserviceevent>,
						public MultiPatch<MPIID_CONFIGCALLBACK, ifc_omconfigcallback>
{
protected:
	BrowserUiHook(HWND hTarget, BOOL fPopupMode);
	~BrowserUiHook();

public:
	static HRESULT CreateInstance(HWND hTarget, BOOL fPopupMode, BrowserUiHook **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/*ifc_winamphook (partial) */
	HRESULT SkinChanging(void);
	HRESULT SkinChanged(const wchar_t *skinName);
	HRESULT SkinColorChange(const wchar_t *colorTheme);
	HRESULT ResetFont(void);

	/* ifc_omserviceevent */
	void ServiceChange(ifc_omservice *service, UINT nModified);
	void CommandStateChange(ifc_omservice *service, const GUID *commandGroup, unsigned int commandId);


	/* ifc_omconfigcallback */
	HRESULT ValueChanged(const GUID *configUid, UINT valueId, ULONG_PTR value);

public:
	HRESULT Register(obj_ombrowser *browserManager, ifc_omservice *service);
	HRESULT Unregister(obj_ombrowser *browserManager, ifc_omservice *service);

	HRESULT CheckBlockedState(ifc_omservice *service);

protected:
	ULONG ref;
	BOOL popupMode;
	HWND hwnd;
	UINT winampCookie;
	UINT configCookie;

protected:
	RECVS_MULTIPATCH;

};


#endif //NULLSOFT_WINAMP_OMBROWSER_UI_HOOK_HEADER