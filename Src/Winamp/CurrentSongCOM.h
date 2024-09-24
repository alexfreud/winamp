#ifndef NULLSOFT_CURRENTSONGCOM
#define NULLSOFT_CURRENTSONGCOM

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./dispatchCallback.h"
//#include <ocidl.h>

class CurrentSongCOM : public IDispatch
{
public:
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	// *** IDispatch Methods ***
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

	void MetadataChanged(char *metadataString); 
	void TitleChanged();

	DispatchCallbackStore metadataCallbacks;
	DispatchCallbackStore titleChangeCallbacks;
};


#endif