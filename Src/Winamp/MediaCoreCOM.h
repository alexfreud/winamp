#ifndef NULLSOFT_WINAMP_MEDIACORECOM_H
#define NULLSOFT_WINAMP_MEDIACORECOM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./dispatchCallback.h"

class MediaCoreCOM : public IDispatch
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

public:
	void OnPlay();
	void OnStop(int last_time, int fullstop);
	void OnPause();
	void OnResume();

private:
	DispatchCallbackStore coreCallbacks;
};


#endif