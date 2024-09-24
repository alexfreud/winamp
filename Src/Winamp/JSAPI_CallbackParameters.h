#pragma once
#include <dispex.h>
#include <map>
#include <string>

/*
This is just a generic property map
wchar_t * -> VARIANT

Since it stores added properties consecutively (no sorting on property name)
it should be safe to add properties after-the-fact, but it's not recommended

not thread safe, so don't add properties on a different than than whomever is reading them via IDispatch
*/
namespace JSAPI
{
	class CallbackParameters : public IDispatchEx
	{
	public:
		CallbackParameters();
		~CallbackParameters();
		STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
		STDMETHOD_(ULONG, AddRef)(void);
		STDMETHOD_(ULONG, Release)(void);

		/** Call this to add a new property 
		** On object destruction, ~CallbackParameters() will call 
		**** Release() on any VT_DISPATCH variants
		**** SysFreeString() on any VT_BSTR variants
		**** TODO on any VT_ARRAY variants
		** so be sure to prepare your data ahead of time (e.g. call AddRef)
		** if you call the helper functions (below) they will do 
		** SysAllocString, AddRef, etc for you.
		*/
		void AddProperty(const wchar_t *name, const VARIANT &property);

		/** helper functions if you don't want to build your own VARIANT 
		**/
		void AddString(const wchar_t *name, const wchar_t *value);
		void AddDispatch(const wchar_t *name, IDispatch *disp); // note: calls AddRef() on your object;
		void AddLong(const wchar_t *name, LONG value);
		void AddBoolean(const wchar_t *name, bool value);

		typedef enum 
		{
			typeBool = 0,
			typeString = 1,
			typeLong = 2,
			typeDispatch = 3,
		} PtopertyType;

		typedef struct __PropertyTemplate
		{
			unsigned char type;
			const wchar_t *name;
			ULONG_PTR value;
		} PropertyTemplate;


		
		size_t AddPropertyIndirect(const PropertyTemplate *entries, size_t count);

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
		typedef std::map<std::wstring, VARIANT> ParameterList;
		ParameterList params;
		volatile LONG refCount;
	};

	// helper function which calls Invoke(0) with 1 parameter (VT_DISPATCH of your CallbackParameters object)
	HRESULT InvokeEvent(JSAPI::CallbackParameters *parameters, IDispatch *invokee);
}