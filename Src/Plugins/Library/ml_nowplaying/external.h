#ifndef NULLSOFT_NOWPLAYING_PLUGIN_EXTERNAL_HEADER
#define NULLSOFT_NOWPLAYING_PLUGIN_EXTERNAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class ExternalDispatch : public IDispatch
{

public:
	typedef enum
	{
		DISPATCH_HIDDEN = 777,
	} DispatchCodes;

protected:
	ExternalDispatch();
	~ExternalDispatch();

public:
	static HRESULT CreateInstance(ExternalDispatch **instance);
	static LPCWSTR GetName();

public:
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	// *** IDispatch Methods ***
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

protected:
	ULONG ref;

};


#endif //NULLSOFT_NOWPLAYING_PLUGIN_EXTERNAL_HEADER