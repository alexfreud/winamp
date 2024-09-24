#ifndef NULLSOFT_WINAMP_OMCONFIG_INI_HEADER
#define NULLSOFT_WINAMP_OMCONFIG_INI_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omconfig.h"
#include "./ifc_ombrowserconfig.h"
#include "./ifc_omdebugconfig.h"
#include "./ifc_omtoolbarconfig.h"
#include "./ifc_omstatusbarconfig.h"
#include <bfc/multipatch.h>
#include <map>

class api_application;

#define MPIID_OMCONFIG			10
#define MPIID_OMBROWSERCONFIG	20
#define MPIID_OMDEBUGCONFIG		30
#define MPIID_OMTOOLBARCONFIG	40
#define MPIID_OMSTATUSBARCONFIG	50

class OmConfigIni :  public MultiPatch<MPIID_OMCONFIG, ifc_omconfig>,
					 public MultiPatch<MPIID_OMBROWSERCONFIG, ifc_ombrowserconfig>,
					 public MultiPatch<MPIID_OMDEBUGCONFIG, ifc_omdebugconfig>,
					 public MultiPatch<MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig>,
					 public MultiPatch<MPIID_OMSTATUSBARCONFIG, ifc_omstatusbarconfig>
{
protected:
	OmConfigIni(LPCWSTR pszPath);
	~OmConfigIni();

public:
	static HRESULT CreateInstance(LPCWSTR pszName, OmConfigIni **instanceOut);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omconfig */
	HRESULT GetPath(LPWSTR pszBuffer, INT cchBufferMax);
    DWORD ReadStr(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize);
	UINT ReadInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, INT nDefault);
	BOOL ReadBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bDefault);
	HRESULT WriteStr(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString);
	HRESULT WriteInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, INT nValue);
	HRESULT WriteBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bValue);
	HRESULT RegisterCallback(ifc_omconfigcallback *callback, UINT *cookie);
	HRESULT UnregisterCallback(UINT cookie);

	/* ifc_ombrowserconfig */
	HRESULT GetClientId(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT SetClientId(LPWSTR pszClientId);
	UINT GetX(void);
	UINT GetY(void);
	HRESULT SetX(UINT x);
	HRESULT SetY(UINT y);

	/* ifc_omdebugconfig */
	HRESULT GetMenuFilterEnabled(void);
	HRESULT GetScriptErrorEnabled(void);
	HRESULT GetScriptDebuggerEnabled(void);
	HRESULT GetBrowserPath(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT EnableMenuFilter(BOOL fEnable);
	HRESULT EnableScriptError(BOOL fEnable);
	HRESULT EnableScriptDebugger(BOOL fEnable);
	HRESULT SetBrowserPath(LPCWSTR pszPath);

	/* ifc_omtoolbarconfig */
	HRESULT Toolbar_GetBottomDockEnabled(void);
	HRESULT Toolbar_GetAutoHideEnabled(void);
	HRESULT Toolbar_GetTabStopEnabled(void);
	HRESULT Toolbar_GetForceAddressbarEnabled(void);
	HRESULT Toolbar_GetFancyAddressbarEnabled(void);
	HRESULT Toolbar_EnableBottomDock(BOOL fEnable);
	HRESULT Toolbar_EnableAutoHide(BOOL fEnable);
	HRESULT Toolbar_EnableTabStop(BOOL fEnable);
	HRESULT Toolbar_EnableForceAddressbar(BOOL fEnable);
	HRESULT Toolbar_EnableFancyAddressbar(BOOL fEnable);

	/* ifc_omstatusbarconfig */
	HRESULT Statusbar_GetEnabled(void);
	HRESULT Statusbar_EnableStatusbar(BOOL fEnable);

public:
	void NotifyChange(const GUID *configUid, UINT valueId, ULONG_PTR value);

protected:
	typedef std::map<UINT, ifc_omconfigcallback*> CallbackMap;

protected:
	RECVS_MULTIPATCH;

protected:
	ULONG ref;
	LPWSTR configPath;
	UINT lastCookie;
	CallbackMap callbackMap;
	CRITICAL_SECTION lock;
	BOOL pathValidated;
};

#endif //NULLSOFT_WINAMP_OMCONFIG_INI_HEADER