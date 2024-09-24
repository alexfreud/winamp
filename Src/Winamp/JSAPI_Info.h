#pragma once
#include <bfc/dispatch.h>

namespace JSAPI
{
	class ifc_info_callback : public Dispatchable
	{
	public:
		/* this is called when an action is denied by the user */
		void OnSecurity(const wchar_t *group, const wchar_t *action, int authorization);
	};

	/*
	JSAPI and JSAPI2's IDispatch object will also have this interface
	(retrieve via IWasabiDispatchable)
	It includes information about the API
	Allows you to associate an HWND with the API (for dialog parenting, etc)
	And register for callbacks to get notifications about things like security denial
	*/

	// this GUID is an interface GUID, NOT a service GUID
	// {64DE905A-8C95-4427-AACE-72A42EB2DE73}
	static const GUID IID_JSAPI_ifc_info = 
	{ 0x64de905a, 0x8c95, 0x4427, { 0xaa, 0xce, 0x72, 0xa4, 0x2e, 0xb2, 0xde, 0x73 } };

	class ifc_info : public Dispatchable
	{
	public:
		static GUID GetIID() { return IID_JSAPI_ifc_info; }

		const wchar_t *GetUserAgent();

		/* associates an HWND with this object 
		mainly used for parenting dialog boxes */
		void SetHWND(HWND hwnd);
		HWND GetHWND();
		
		/* registers for callbacks */
		void RegisterCallback(ifc_info_callback *callback);
		void UnregisterCallback(ifc_info_callback *callback);

		/* associates a name with this object */
		void SetName(const wchar_t *name);
		const wchar_t *GetName();

		/* adds to the API (for this object only for JSAPI2, globally for JSAPI1 because it's a singleton */
		int AddAPI(const wchar_t *name, IDispatch *dispatch);

		enum
		{
			JSAPI_IFC_INFO_GETUSERAGENT = 0,
			JSAPI_IFC_INFO_SETHWND = 1,
			JSAPI_IFC_INFO_GETHWND = 2,
			JSAPI_IFC_INFO_REGISTERCALLBACK = 3,
			JSAPI_IFC_INFO_UNREGISTERCALLBACK = 4,
			JSAPI_IFC_INFO_SETNAME = 5,
			JSAPI_IFC_INFO_GETNAME = 6,
			JSAPI_IFC_INFO_ADDAPI = 7,
		};
	};
}

inline const wchar_t *JSAPI::ifc_info::GetUserAgent()
{
	return _call(JSAPI_IFC_INFO_GETUSERAGENT, (const wchar_t *)0);
}

inline void JSAPI::ifc_info::SetHWND(HWND hwnd)
{
	_voidcall(JSAPI_IFC_INFO_SETHWND, hwnd);
}

inline HWND JSAPI::ifc_info::GetHWND()
{
	return _call(JSAPI_IFC_INFO_GETHWND, (HWND)0);
}

inline void JSAPI::ifc_info::RegisterCallback(ifc_info_callback *callback)
{
	_voidcall(JSAPI_IFC_INFO_REGISTERCALLBACK, callback);
}

inline void JSAPI::ifc_info::UnregisterCallback(ifc_info_callback *callback)
{
	_voidcall(JSAPI_IFC_INFO_UNREGISTERCALLBACK, callback);
}

inline void JSAPI::ifc_info::SetName(const wchar_t *name)
{
	_voidcall(JSAPI_IFC_INFO_SETNAME, name);
}

inline const wchar_t *JSAPI::ifc_info::GetName()
{
	return _call(JSAPI_IFC_INFO_GETNAME, (const wchar_t *)0);
}

inline int JSAPI::ifc_info::AddAPI(const wchar_t *name, IDispatch *dispatch)
{
	return _call(JSAPI_IFC_INFO_ADDAPI, (int)1, name, dispatch);
}