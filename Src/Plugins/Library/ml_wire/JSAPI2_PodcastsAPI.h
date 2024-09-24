#pragma once

#include <ocidl.h>
#include <atomic>

#include "../Winamp/JSAPI_Info.h"

namespace JSAPI2
{
	class PodcastsAPI : public IDispatch
	{
	public:
		PodcastsAPI(const wchar_t *_key, JSAPI::ifc_info *info);
		STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
		STDMETHOD_(ULONG, AddRef)(void);
		STDMETHOD_(ULONG, Release)(void);
		// *** IDispatch Methods ***
		STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
		STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
		STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
		STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);
	private:
		const wchar_t *key;
		volatile std::atomic<std::size_t> _refCount = 1;
		JSAPI::ifc_info *info;

		STDMETHOD (Subscribe)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);

	};
}
