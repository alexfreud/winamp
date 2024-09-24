#ifndef NULLSOFT_ORB_PLUGIN_DISPATCHTABLE_HEADER
#define NULLSOFT_ORB_PLUGIN_DISPATCHTABLE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define DISPTABLE_INCLUDE() public:\
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);\
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);\
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);\
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);\
	private:\
	typedef struct __DISPATCHENTRY DISPATCHENTRY;\
	const static DISPATCHENTRY szDispatchTable[];

#define DISPTABLE_BEGIN()\
	typedef HRESULT (DISPTABLE_CLASS::*DISPATCHHANDLER)(WORD /*wFlags*/, DISPPARAMS FAR* /*pdispparams*/, VARIANT FAR* /*pvarResult*/, UINT FAR* /*puArgErr*/);\
	typedef struct __DISPATCHENTRY {\
		UINT id;\
		LPCWSTR name;\
		DISPATCHHANDLER handler;\
	} DISPATCHENTRY;\
	const DISPTABLE_CLASS::DISPATCHENTRY DISPTABLE_CLASS::szDispatchTable[] = {

#define DISPTABLE_END\
	};\
	HRESULT DISPTABLE_CLASS::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid) {\
		UINT unknowns = 0;\
		for (unsigned int i = 0; i != cNames; i++) {\
			rgdispid[i] = DISPID_UNKNOWN;\
			for (INT j =0; j < ARRAYSIZE(szDispatchTable); j++) 	{\
				if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, 0, rgszNames[i], -1, szDispatchTable[j].name, -1)) {\
					rgdispid[i] = szDispatchTable[j].id; break; }\
			}\
			if (DISPID_UNKNOWN == rgdispid[i]) { unknowns++; }\
		}\
		return (0 != unknowns) ? DISP_E_UNKNOWNNAME : S_OK;\
	}\
	HRESULT DISPTABLE_CLASS::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo) { return E_NOTIMPL; }\
	HRESULT DISPTABLE_CLASS::GetTypeInfoCount(unsigned int FAR * pctinfo) { return E_NOTIMPL; }\
	HRESULT DISPTABLE_CLASS::Invoke(DISPID dispId, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr) {\
		for (INT i = 0; i < ARRAYSIZE(szDispatchTable); i++) {\
			if (dispId == szDispatchTable[i].id) { return (this->*szDispatchTable[i].handler)(wFlags, pdispparams, pvarResult, puArgErr); }\
		}\
		return DISP_E_MEMBERNOTFOUND;\
	}
	
#define DISPENTRY_ADD(__id, __name, __handler)\
	{ DISPTABLE_CLASS::__id, __name, &DISPTABLE_CLASS::__handler },

#define DISPHANDLER_REGISTER(__handler)\
	HRESULT __handler(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);

#endif //NULLSOFT_ORB_PLUGIN_DISPATCHTABLE_HEADER