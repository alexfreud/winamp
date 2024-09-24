#pragma once
#include <ocidl.h>
#include <api/skin/widgets/mb/iebrowser.h>

class MinibrowserCOM : IDispatch
{
public:
	MinibrowserCOM(BrowserWnd* brw);
	/** IUnknown */
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	/** IDispatch */
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR*  pctinfo);
	STDMETHOD (GetTypeInfo)(unsigned int  iTInfo, LCID  lcid, ITypeInfo FAR* FAR*  ppTInfo);
	STDMETHOD (GetIDsOfNames)(REFIID riid,LPOLESTR __RPC_FAR *rgszNames,UINT cNames,LCID lcid,DISPID __RPC_FAR *rgDispId);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

	enum
	{
		MINIBROWSERCOM_MAKI_MESSAGETOMAKI = 666,
	};
private:
	BrowserWnd* brw;
};
