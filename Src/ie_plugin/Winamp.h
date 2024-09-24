#pragma once
#include <ocidl.h>
#include <exdisp.h>
#include <objsafe.h>

class Winamp : public IObjectWithSite, 
	public IDispatch,
	public IOleObject,
	public IPersistStorage,
	public IDataObject,
	public IObjectSafety
{
public:
	Winamp();
		/* IUnknown */
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);

	// *** IDispatch Methods ***
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

	/* IOleObject */
	HRESULT STDMETHODCALLTYPE SetClientSite(IOleClientSite *pClientSite);
	HRESULT STDMETHODCALLTYPE GetClientSite(IOleClientSite **ppClientSite);
	HRESULT STDMETHODCALLTYPE SetHostNames(LPCOLESTR szContainerApp,LPCOLESTR szContainerObj);
	HRESULT STDMETHODCALLTYPE Close(DWORD dwSaveOption);
	HRESULT STDMETHODCALLTYPE SetMoniker(DWORD dwWhichMoniker,IMoniker *pmk);
	HRESULT STDMETHODCALLTYPE GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker,IMoniker **ppmk);
	HRESULT STDMETHODCALLTYPE InitFromData(IDataObject *pDataObject,BOOL fCreation,DWORD dwReserved);
	HRESULT STDMETHODCALLTYPE GetClipboardData(DWORD dwReserved,IDataObject **ppDataObject);
	HRESULT STDMETHODCALLTYPE DoVerb(LONG iVerb,LPMSG lpmsg,IOleClientSite *pActiveSite,LONG lindex, HWND hwndParent, LPCRECT lprcPosRect);
	HRESULT STDMETHODCALLTYPE EnumVerbs(IEnumOLEVERB **ppEnumOleVerb);
	HRESULT STDMETHODCALLTYPE Update(void);
	HRESULT STDMETHODCALLTYPE IsUpToDate(void);
	HRESULT STDMETHODCALLTYPE GetUserClassID(CLSID *pClsid);
	HRESULT STDMETHODCALLTYPE GetUserType(DWORD dwFormOfType,LPOLESTR *pszUserType);
	HRESULT STDMETHODCALLTYPE SetExtent(DWORD dwDrawAspect,SIZEL *psizel);
	HRESULT STDMETHODCALLTYPE GetExtent(DWORD dwDrawAspect,SIZEL *psizel);
	HRESULT STDMETHODCALLTYPE Advise(IAdviseSink *pAdvSink,DWORD *pdwConnection);
	HRESULT STDMETHODCALLTYPE Unadvise(DWORD dwConnection);
	HRESULT STDMETHODCALLTYPE EnumAdvise(IEnumSTATDATA **ppenumAdvise);
	HRESULT STDMETHODCALLTYPE GetMiscStatus(DWORD dwAspect,DWORD *pdwStatus);
	HRESULT STDMETHODCALLTYPE SetColorScheme(LOGPALETTE *pLogpal);

	/* IPersistStorage */
	HRESULT STDMETHODCALLTYPE GetClassID(CLSID *pClassID);
	HRESULT STDMETHODCALLTYPE IsDirty(void);
	HRESULT STDMETHODCALLTYPE InitNew(IStorage *pStg);
	HRESULT STDMETHODCALLTYPE Load(IStorage *pStg);
	HRESULT STDMETHODCALLTYPE Save(IStorage *pStgSave, BOOL fSameAsLoad);
	HRESULT STDMETHODCALLTYPE SaveCompleted(IStorage *pStgNew);
	HRESULT STDMETHODCALLTYPE HandsOffStorage(void);

	/* IDataStorage */
	 HRESULT STDMETHODCALLTYPE GetData(FORMATETC *pformatetcIn,STGMEDIUM *pmedium);
 HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC *pformatetc,STGMEDIUM *pmedium);
 HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC *pformatetc);
 HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC *pformatectIn,FORMATETC *pformatetcOut);
 HRESULT STDMETHODCALLTYPE SetData(FORMATETC *pformatetc,STGMEDIUM *pmedium,BOOL fRelease);
 HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection,IEnumFORMATETC **ppenumFormatEtc);
 HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC *pformatetc,DWORD advf,IAdviseSink *pAdvSink,DWORD *pdwConnection);
 HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection);
 HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA **ppenumAdvise);

 /* IObjectSafety */
 HRESULT STDMETHODCALLTYPE GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions);
 HRESULT STDMETHODCALLTYPE SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions);

	/* IObjectWithSite */
	HRESULT STDMETHODCALLTYPE GetSite(REFIID riid, void** ppvSite);
	HRESULT STDMETHODCALLTYPE SetSite(IUnknown*);

	STDMETHOD (Test)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
	STDMETHOD (getVersion)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
private:
	LONG refCount;
	IOleClientSite *client_site;

private:
IWebBrowser2 *webBrowser_;
IConnectionPointContainer *connectionPointContainer;
DWORD cookie_;
IDispatch *document_;

};