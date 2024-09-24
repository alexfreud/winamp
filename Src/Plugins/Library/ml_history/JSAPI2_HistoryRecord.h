#pragma once
#include <dispex.h>
#include "history.h"
namespace JSAPI2
{
	class HistoryRecord : public IDispatchEx
	{
	public:
		HistoryRecord(IUnknown *_parent, const historyRecord *_record);
		~HistoryRecord();
		STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
		STDMETHOD_(ULONG, AddRef)(void);
		STDMETHOD_(ULONG, Release)(void);

		void AddObject(IDispatch *obj);
	private:
		// *** IDispatch Methods ***
		STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
		STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
		STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
		STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

		// *** IDispatchEx Methods ***
		STDMETHOD (GetDispID)(BSTR bstrName, DWORD grfdex, DISPID *pid);
		STDMETHOD (InvokeEx)(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller);        
		STDMETHOD (DeleteMemberByName)(BSTR bstrName, DWORD grfdex) ;
		STDMETHOD (DeleteMemberByDispID)(DISPID id);        
		STDMETHOD (GetMemberProperties)(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex);
		STDMETHOD (GetMemberName)(DISPID id, BSTR *pbstrName);        
		STDMETHOD (GetNextDispID)(DWORD grfdex, DISPID id, DISPID *pid);
		STDMETHOD (GetNameSpaceParent)(IUnknown **ppunk);
	private:
		const historyRecord *record;
		IUnknown *parent;
		volatile LONG refCount;

		STDMETHOD (filename)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult);
		STDMETHOD (title)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult);
		STDMETHOD (length)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult);
		STDMETHOD (playcount)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult);
		STDMETHOD (lastplay)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult);
		STDMETHOD (offset)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult);

	};

}