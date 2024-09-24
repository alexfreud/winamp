#pragma once
/* 
Ben Allison
April 30, 2008
*/
#include <oaidl.h>
#include <vector>
#include "FLVMetadata.h"

struct DispatchCallbackInfo
{
	DispatchCallbackInfo() : dispatch(0), threadId(0), threadHandle(0) {}
	DispatchCallbackInfo(IDispatch *d, DWORD id, HANDLE _handle) : dispatch(d), threadId(id), threadHandle(_handle) {}
	IDispatch *dispatch;
	DWORD threadId;
	HANDLE threadHandle;
};

class FLVCOM : public IDispatch
{
public:
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	
	void MetadataCallback(FLVMetadata::Tag *tag);
private:
	// *** IDispatch Methods ***
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);
	
	std::vector<DispatchCallbackInfo> callbacks;
};

extern FLVCOM flvCOM;