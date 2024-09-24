#pragma once

#include <ocidl.h>
#include "JSAPI_DispatchTable.h"
#include "../nu/ConfigCOM.h"
#include "IWasabiDispatchable.h"
#include "JSAPI_Info.h"
#include <dispex.h>

namespace JSAPI2
{
	class ExternalObject : public IDispatchEx, 
		public IWasabiDispatchable,
		public JSAPI::ifc_info
	{
	public:
		ExternalObject(const wchar_t *_key);
		~ExternalObject();
		// *** IUnknown Methods ***
		STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
		STDMETHOD_(ULONG, AddRef)(void);
		STDMETHOD_(ULONG, Release)(void);

	private:
		// *** IDispatch Methods ***
		STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
		STDMETHOD(GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
		STDMETHOD(GetTypeInfoCount)(unsigned int FAR * pctinfo);
		STDMETHOD(Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

		// *** IDispatchEx Methods ***
		STDMETHOD (GetDispID)(BSTR bstrName, DWORD grfdex, DISPID *pid);
		STDMETHOD (InvokeEx)(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller);        
		STDMETHOD (DeleteMemberByName)(BSTR bstrName, DWORD grfdex) ;
		STDMETHOD (DeleteMemberByDispID)(DISPID id);        
		STDMETHOD (GetMemberProperties)(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex);
		STDMETHOD (GetMemberName)(DISPID id, BSTR *pbstrName);        
		STDMETHOD (GetNextDispID)(DWORD grfdex, DISPID id, DISPID *pid);
		STDMETHOD (GetNameSpaceParent)(IUnknown **ppunk);

		// *** IWasabiDispatchable Methods ***
		STDMETHOD(QueryDispatchable)(REFIID riid, Dispatchable **ppDispatchable);

		// *** JSAPI::ifc_info Methods ***
		const wchar_t *GetUserAgent();
		void SetHWND(HWND hwnd);
		HWND GetHWND();
		void SetName(const wchar_t *name);
		const wchar_t *GetName();
		int AddAPI(const wchar_t *name, IDispatch *dispatch);
	private:
		// private helper methods
		DWORD AddDispatch(const wchar_t *name, IDispatch *object);

		// members
		JSAPI::DispatchTable dispatchTable;		
		volatile LONG refCount;
		wchar_t *key;
		HWND hwnd;
	protected:
		RECVS_DISPATCH;
	};
}
