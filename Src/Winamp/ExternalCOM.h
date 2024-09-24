#ifndef NULLSOFT_EXTERNALCOMH
#define NULLSOFT_EXTERNALCOMH

#include <ocidl.h>


#if defined(__cplusplus)

#include <vector>
#include "JSAPI_DispatchTable.h"
#include "IWasabiDispatchable.h"
#include "JSAPI_Info.h"

class SkinCOM;
class MediaCoreCOM;
class CurrentSongCOM;
class ExternalCOM;
class ConfigCOM;
namespace JSAPI2 { class ExternalObject; }

HRESULT __cdecl JSAPI1_GetExternal(ExternalCOM **instance);
HRESULT __cdecl JSAPI1_GetSkinCOM(SkinCOM **instance);
HRESULT __cdecl JSAPI1_GetMediaCoreCOM(MediaCoreCOM **instance);
HRESULT __cdecl JSAPI1_GetCurrentSongCOM(CurrentSongCOM **instance);

class ExternalCOM : public IDispatch, 
		public IWasabiDispatchable,
		public JSAPI::ifc_info
{
public:
	enum
	{
		DISP_EXTERNAL_SIDECAR = 777,
		DISP_EXTERNAL_BROWSER,
		DISP_EXTERNAL_CURRENTSONG,
		DISP_EXTERNAL_CURRENTSKIN,
		DISP_EXTERNAL_NEW_ENTRIES_MARKER,
	};

public:
	ExternalCOM();
	~ExternalCOM();
	
	// *** IUnknown ***
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	// *** IDispatch ***
	STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD(GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD(GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD(Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

	JSAPI::DispatchTable dispatchTable;
	DISPID AddDispatch(const wchar_t *name, IDispatch *object);
	BOOL RemoveDispatch(DISPID dispatchId);
	

	// *** IWasabiDispatchable Methods ***
	STDMETHOD(QueryDispatchable)(REFIID riid, Dispatchable **ppDispatchable);

	// *** JSAPI::ifc_info Methods ***
	const wchar_t *GetUserAgent();

	HRESULT FindDispatch(DISPID dispId, IDispatch **instance);
	HRESULT GetSkinCOM(SkinCOM **instance);
	HRESULT GetMediaCoreCOM(MediaCoreCOM **instance);
	HRESULT GetCurrentSongCOM(CurrentSongCOM **instance);
	
	HRESULT GetConfig(LPCWSTR configName, ConfigCOM **config);

protected:
	RECVS_DISPATCH;

private:
	typedef std::vector<ConfigCOM*> ConfigsList;

private:
	ULONG ref;
	CRITICAL_SECTION tableLock;
	wchar_t configFilename[MAX_PATH];
	MediaCoreCOM *mediaCoreCOM;
	SkinCOM *skinCOM;
	CurrentSongCOM *songCOM;
	JSAPI2::ExternalObject *api2;
	ConfigsList configs;
};
#endif // __cplusplus
#endif