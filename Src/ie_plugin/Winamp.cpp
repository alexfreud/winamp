#include "Winamp.h"
#include "../Winamp/JSAPI.h"
#include <mshtmdid.h>
#include <mshtmhst.h>
#include <stdio.h>
#include <shlwapi.h>
#include <malloc.h>

#define BUFFER_LEN 1024

// {294DFA02-5A90-4dc7-872D-1EF54817078D}
static const GUID CLSID_WINAMP_CRAP = 
{ 0x294dfa02, 0x5a90, 0x4dc7, { 0x87, 0x2d, 0x1e, 0xf5, 0x48, 0x17, 0x7, 0x8d } };

Winamp::Winamp()
{
	refCount=1;
	
  webBrowser_=0;
  connectionPointContainer=0;
  cookie_=0;
  document_=0;
	client_site = 0;
}
/* other things being asked for:
{00000008-0000-0000-C000-000000000046} (dunno what interface)
HTMLDocument
IOleControl B196B288-BAB4-101A-B69C-00AA00341D07
IClientSecurity
IQuickActivate
IPersistPropertyBag2
IPersistPropertyBag
IPersistStreamInit
IPersistStorage
IViewObjectEx
IViewObject
IActiveScript BB1A2AE1-A4F9-11CF-8F20-00805F2CD064
IOleCommandTarget B722BCCB-4E68-101B-A2BC-00AA00404770
IDispatchEx A6EF9860-C720-11D0-9337-00A0C90DCAA9
CB5BDC81-93C1-11cf-8F20-00805F2CD064
6D5140D3-7436-11CE-8034-00AA006009FA (dunno what interface)
*/

HRESULT Winamp::QueryInterface(REFIID riid, LPVOID FAR *ppvObj)
{
	//DebugBreak();
	if (!ppvObj)
		return E_POINTER;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObj = /*static_cast<IUnknown *>*/(this);
	else if (IsEqualIID(riid, IID_IObjectWithSite))
		*ppvObj = static_cast<IObjectWithSite *>(this);
	//else if (IsEqualIID(riid, __uuidof(IWinamp)))
	//*ppvObj = static_cast<IWinamp *>(this);
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObj = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IOleObject))
		*ppvObj = (IOleObject *)this;
	else if (IsEqualIID(riid, IID_IPersistStorage))
		*ppvObj = (IPersistStorage *)this;
	else if (IsEqualIID(riid, IID_IDataObject))
		*ppvObj = (IDataObject *)this;
	else if (IsEqualIID(riid, IID_IObjectSafety))
		*ppvObj = (IObjectSafety *)this;
	else
	{
		LPOLESTR guidstr;
		StringFromIID(riid, &guidstr);
		//MessageBox(NULL, guidstr, L"queryinterface", MB_OK);
		*ppvObj = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG Winamp::AddRef(void)
{
	return ++refCount;
}

ULONG Winamp::Release(void)
{
	if (refCount == 0)
		return 0;

	ULONG retCount=--refCount;
	if (refCount == 0)
		delete this;
	return retCount;
}


#define DISP_TABLE \
	CHECK_ID(Test)\
	CHECK_ID(getVersion)\

#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE 
};
#undef CHECK_ID
#define CHECK_ID(str) 		if (wcscmp(rgszNames[i], L## #str) == 0)	{		rgdispid[i] = JSAPI_DISP_ENUMIFY(str); continue; }

HRESULT Winamp::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
		bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		DISP_TABLE

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT Winamp::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT Winamp::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}


#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT Winamp::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
		switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

HRESULT Winamp::GetSite(REFIID riid, void** ppvSite)
{
  return E_NOINTERFACE;
}

HRESULT Winamp::SetSite(IUnknown* iu) 
{/*
  if (! (iu->QueryInterface(IID_IWebBrowser2, (void**) &webBrowser_) == S_OK )) 
	{
    return E_FAIL;
  }

  if (! (iu->QueryInterface(IID_IConnectionPointContainer, (void**) &connectionPointContainer) == S_OK )) 
	{
    return E_FAIL;
    // ::MessageBox(0, "xxx", 0, 0);
  }

  // make sure we\'re getting browser events
  IConnectionPoint* spCP;
  HRESULT hr = connectionPointContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &spCP);
  if (FAILED(hr)) 
	{
     return hr;
  }
  hr = spCP->Advise(static_cast<IDispatch*>(this), &cookie_);
*/
  return S_OK;
}

HRESULT Winamp::Test(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
	
	MessageBoxA(NULL,"test", "IWinamp", MB_OK);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_VARIANT(pvarResult, V_BOOL, VARIANT_TRUE);

	return S_OK;
}


#if 0

HRESULT Winamp::getVersion(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
	
	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);
	JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(L"7766"));

	return S_OK;
}
#endif

HRESULT Winamp::getVersion(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
	
	wchar_t csVersion[BUFFER_LEN];
	////////////////////////////////
	DWORD BufferSize = BUFFER_LEN;
    DWORD cbData;
	bool keyFound = false;

	wchar_t exeName[] = L"\\winamp.exe";
    wchar_t fileName[BUFFER_LEN]; 
	csVersion[0]=0;
	memset(&fileName[0],'\0',BUFFER_LEN);
    wchar_t fileNameTemp[BUFFER_LEN]; 

	HKEY hKey;
	cbData = BUFFER_LEN;

	// first check the protocol handler registry key, we're looking for
	// the winamp:// protocol handler. If we find this, then this is the
	// "right" exe for winamp we need to get the version number on
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("winamp\\shell\\open\\command"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		if ( RegQueryValueEx( hKey,
								 TEXT(""),
								 NULL,
								 NULL,
								 (LPBYTE) fileNameTemp,
								 &cbData ) != ERROR_SUCCESS) {
			return ERROR_PATH_NOT_FOUND;
		}

		RegCloseKey (hKey);
		if (StrStrW(fileNameTemp,L"winamp.exe")) {
			int indexOfFirstQuote = StrCSpnW(fileNameTemp, L"\"");
			int indexOfSecondQuote = StrCSpnW(&fileNameTemp[indexOfFirstQuote+1], L"\"");

			if (indexOfFirstQuote >= 0) {
				keyFound = true;
				lstrcpynW(fileName,&fileNameTemp[indexOfFirstQuote+1], indexOfSecondQuote+1);
			} 
		} else {
			// some other app (itunes ??) controlling the winamp:// protocol
			// return error
			return ERROR_PATH_NOT_FOUND;
		}
	}

	if (!keyFound) {
		// See if the reg key exists
		if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Winamp"), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
			return ERROR_PATH_NOT_FOUND;
		}

		cbData = BUFFER_LEN;
		if ( RegQueryValueEx( hKey,
								 TEXT(""),
								 NULL,
								 NULL,
								 (LPBYTE) fileName,
								 &cbData ) != ERROR_SUCCESS) {
			return ERROR_PATH_NOT_FOUND;
		}
		RegCloseKey (hKey);
		keyFound = true;
		wcscat(fileName,exeName);
	}

	if (!keyFound) {
		return ERROR_PATH_NOT_FOUND;
	}


	////////////////////////////////
	static TCHAR sBackSlash[] = {'\\','\0'};
	DWORD dwVersionDataLen = GetFileVersionInfoSize(fileName, NULL);

	if (dwVersionDataLen) 
	{
		char* fvBuf = (char *)alloca(dwVersionDataLen);//new char[dwVersionDataLen];
		if (GetFileVersionInfo(fileName, 0, dwVersionDataLen, fvBuf)) {
			
			LPVOID pVal;
			UINT nValLen;
			if (VerQueryValue(fvBuf, sBackSlash, &pVal, &nValLen)) {
				if (nValLen == sizeof(VS_FIXEDFILEINFO)) {
					VS_FIXEDFILEINFO* pFixedFileInfo = (VS_FIXEDFILEINFO*)pVal;
					//wsprintf(csVersion, L"%d.%d.%d.%d",
					//	HIWORD(pFixedFileInfo->dwFileVersionMS), LOWORD(pFixedFileInfo->dwFileVersionMS),
					//	HIWORD(pFixedFileInfo->dwFileVersionLS), LOWORD(pFixedFileInfo->dwFileVersionLS));
					wsprintf(csVersion, L"%d.%d%d",
						HIWORD(pFixedFileInfo->dwFileVersionMS), LOWORD(pFixedFileInfo->dwFileVersionMS),
						HIWORD(pFixedFileInfo->dwFileVersionLS));
				}
			}
		}
	}

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);
	JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(csVersion));

	return S_OK;
}

	HRESULT Winamp::SetClientSite(IOleClientSite *pClientSite)
{ 
	// TODO
	client_site = pClientSite;
	return S_OK;
	}

	HRESULT Winamp::GetClientSite(IOleClientSite **ppClientSite)
{ return E_NOTIMPL; }
	HRESULT Winamp::SetHostNames(LPCOLESTR szContainerApp,LPCOLESTR szContainerObj)
{ return S_OK; }
	HRESULT Winamp::Close(DWORD dwSaveOption)
{ return S_OK; }
	HRESULT Winamp::SetMoniker(DWORD dwWhichMoniker,IMoniker *pmk)
{ return E_NOTIMPL; }
	HRESULT Winamp::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker,IMoniker **ppmk)
{ return E_NOTIMPL; }
	HRESULT Winamp::InitFromData(IDataObject *pDataObject,BOOL fCreation,DWORD dwReserved)
{ return E_NOTIMPL; }
	HRESULT Winamp::GetClipboardData(DWORD dwReserved,IDataObject **ppDataObject)
{ return E_NOTIMPL; }
	HRESULT Winamp::DoVerb(LONG iVerb,LPMSG lpmsg,IOleClientSite *pActiveSite,LONG lindex, HWND hwndParent, LPCRECT lprcPosRect)
{ return S_OK; }
	HRESULT Winamp::EnumVerbs(IEnumOLEVERB **ppEnumOleVerb)
{ return E_NOTIMPL; }
	HRESULT Winamp::Update(void)
{ return E_NOTIMPL; }
	HRESULT Winamp::IsUpToDate(void)
{ return E_NOTIMPL; }
	HRESULT Winamp::GetUserClassID(CLSID *pClsid)
{ return E_NOTIMPL; }
	HRESULT Winamp::GetUserType(DWORD dwFormOfType,LPOLESTR *pszUserType)
{ return E_NOTIMPL; }
	HRESULT Winamp::SetExtent(DWORD dwDrawAspect,SIZEL *psizel)
{ return E_NOTIMPL; }
	
	HRESULT Winamp::GetExtent(DWORD dwDrawAspect,SIZEL *psizel)
{ 
	// TODO
	return E_NOTIMPL; 
	}

	HRESULT Winamp::Advise(IAdviseSink *pAdvSink,DWORD *pdwConnection)
{ return E_NOTIMPL; }
	HRESULT Winamp::Unadvise(DWORD dwConnection)
{ return E_NOTIMPL; }
	HRESULT Winamp::EnumAdvise(IEnumSTATDATA **ppenumAdvise)
{ return E_NOTIMPL; }
	HRESULT Winamp::GetMiscStatus(DWORD dwAspect,DWORD *pdwStatus)
{ 
	*pdwStatus = OLEMISC_INVISIBLEATRUNTIME;
	return S_OK; 
	}
	HRESULT Winamp::SetColorScheme(LOGPALETTE *pLogpal)
{ return E_NOTIMPL; }

	HRESULT Winamp::GetClassID(CLSID *pClassID)
{ *pClassID = CLSID_WINAMP_CRAP; return S_OK; }
	HRESULT Winamp::IsDirty(void)
{ return E_NOTIMPL; }
	HRESULT Winamp::InitNew(IStorage *pStg)
{ return E_NOTIMPL; }
	HRESULT Winamp::Load(IStorage *pStg)
{ return E_NOTIMPL; }
	HRESULT Winamp::Save(IStorage *pStgSave, BOOL fSameAsLoad)
{ return E_NOTIMPL; }
	HRESULT Winamp::SaveCompleted(IStorage *pStgNew)
{ return E_NOTIMPL; }
	HRESULT Winamp::HandsOffStorage(void)
{ return E_NOTIMPL; }

HRESULT Winamp::GetData(FORMATETC *pformatetcIn,STGMEDIUM *pmedium)
{ return E_NOTIMPL; }
 HRESULT Winamp::GetDataHere(FORMATETC *pformatetc,STGMEDIUM *pmedium)
{ return E_NOTIMPL; }
 HRESULT Winamp::QueryGetData(FORMATETC *pformatetc)
{ return E_NOTIMPL; }
 HRESULT Winamp::GetCanonicalFormatEtc(FORMATETC *pformatectIn,FORMATETC *pformatetcOut)
{ return E_NOTIMPL; }
 HRESULT Winamp::SetData(FORMATETC *pformatetc,STGMEDIUM *pmedium,BOOL fRelease)
{ return E_NOTIMPL; }
 HRESULT Winamp::EnumFormatEtc(DWORD dwDirection,IEnumFORMATETC **ppenumFormatEtc)
{ return E_NOTIMPL; }
 HRESULT Winamp::DAdvise(FORMATETC *pformatetc,DWORD advf,IAdviseSink *pAdvSink,DWORD *pdwConnection)
{ return E_NOTIMPL; }
 HRESULT Winamp::DUnadvise(DWORD dwConnection)
{ return E_NOTIMPL; }
 HRESULT Winamp::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{ return E_NOTIMPL; }

 HRESULT Winamp::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
 { 
	 *pdwSupportedOptions =  INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;
	 *pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;

	 return S_OK; 
 }
 HRESULT Winamp::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
	 { 

		 return S_OK; 
 }