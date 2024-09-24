#ifndef NULLSOFT_AUTH_LOGINBOX_EXTERNAL_MANAGER_HEADER
#define NULLSOFT_AUTH_LOGINBOX_EXTERNAL_MANAGER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <dispex.h>
#include "../nu/Vectors.h"


class ExternalManager : public IDispatchEx
{
protected:
	ExternalManager();
	~ExternalManager();

public:
	static HRESULT CreateInstance(ExternalManager **instance);

public:
	/* IUnknown */
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/* IDispatch*/
	STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD(GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD(GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD(Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

	/* IDispatchEx*/
	STDMETHOD (GetDispID)(BSTR bstrName, DWORD grfdex, DISPID *pid);
	STDMETHOD (InvokeEx)(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller);
	STDMETHOD (DeleteMemberByName)(BSTR bstrName, DWORD grfdex);
	STDMETHOD (DeleteMemberByDispID)(DISPID id);
	STDMETHOD (GetMemberProperties)(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex);
	STDMETHOD (GetMemberName)(DISPID id, BSTR *pbstrName);
	STDMETHOD (GetNextDispID)(DWORD grfdex, DISPID id, DISPID *pid);
	STDMETHOD (GetNameSpaceParent)(IUnknown **ppunk);

public:
	HRESULT AddDispatch(LPCWSTR pszName, IDispatch *pDispatch, DISPID *pid);

protected:
	typedef struct __DispatchRecord
	{
		DISPID		id;
		LPWSTR		name;
		IDispatch	*object;
	} DispatchRecord;

	typedef Vector<DispatchRecord> DispatchList;

protected:
	ULONG ref;
	DispatchList	list;
	CRITICAL_SECTION lock;
	DISPID lastDispId;
};


#endif //NULLSOFT_AUTH_LOGINBOX_EXTERNAL_MANAGER_HEADER