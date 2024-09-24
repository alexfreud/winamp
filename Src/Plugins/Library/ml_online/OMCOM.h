#ifndef NULLSOFT_OMCOMH
#define NULLSOFT_OMCOMH

#include <ocidl.h>

class ConfigCOM;
class ifc_omservice;

class OMCOM : public IDispatch
{
public:
	OMCOM();
	~OMCOM();
	
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	// *** IDispatch Methods ***
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

private:
	STDMETHOD (IsOmSubscribed)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
	STDMETHOD (AddOmSubscribed)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
	STDMETHOD (Subscribe)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);

	HRESULT FindService(VARIANTARG *pArg, ifc_omservice **service);

public:
	INT GetSerialNumber(BOOL fForceRead);
	HRESULT SetSerialNumber(INT sn);
	HRESULT Publish();

protected:
	ConfigCOM *config;
	INT serialNumber;
	UINT publishCookie;
};


#endif