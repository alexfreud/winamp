#pragma once

#include <ocidl.h>
#include "JSAPI_Info.h"

namespace JSAPI2
{
	class SkinAPI : public IDispatch
	{
	public:
		SkinAPI(const wchar_t *_key, JSAPI::ifc_info *info);
		~SkinAPI();
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
		JSAPI::ifc_info *info;
		volatile LONG refCount;

		STDMETHOD (GetClassicColor)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (GetPlaylistColor)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (GetSkinColor)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);

		STDMETHOD (name)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (font)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (fontsize)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
	
	};
}