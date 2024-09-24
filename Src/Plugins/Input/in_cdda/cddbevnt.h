#ifndef NULLSOFT_CDDB_EVENT_MANAGER_HEADER
#define NULLSOFT_CDDB_EVENT_MANAGER_HEADER


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

// callbacks
typedef void (CALLBACK *CDDB_CMDCOMPLETED)(LONG /*lCommandCode*/, HRESULT /*hCommandResult*/, VARIANT* /*pCommandData*/, ULONG_PTR /*user*/);
typedef void (CALLBACK *CDDB_LOGMSG)(BSTR /*bstrMessage*/, ULONG_PTR /*user*/);
typedef void (CALLBACK *CDDB_SRVMSG)(LONG /*lMessageCode*/, LONG /*lMessageAction*/, BSTR /*bstrMessageData*/, ULONG_PTR /*user*/);
typedef void (CALLBACK *CDDB_CMDPROGRESS)(LONG /*lCommandCode*/, LONG /*lProgressCode*/, LONG /*lBytesDone*/, LONG /*lBytesTotal*/, ULONG_PTR /*user*/);
// callback types
#define CDDB_CB_CMDCOMPLETED		0
#define CDDB_CB_CMDPROGRESS		1
#define CDDB_CB_LOGMSG			2
#define CDDB_CB_SRVMSG			3

class CDBBEventManager : public IDispatch
{
public:
	CDBBEventManager(void);
	virtual ~CDBBEventManager(void);

public:
	// *** IUnknown Methods ***
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	
	// *** IDispatch Methods ***
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

protected:
	void OnCommandCompleted(LONG lCommandCode, HRESULT hCommandResult, VARIANT *pCommandData);
	void OnLogMessage(BSTR bstrMessage);
	void OnServerMessage(LONG lMessageCode, LONG lMessageAction, BSTR bstrMessageData);
	void OnCommandProgress(LONG lCommandCode, LONG lProgressCode, LONG lBytesDone, LONG lBytesTotal);

public:
	HRESULT Advise(IUnknown *pCDDBCtrl);
	HRESULT Unadvise(IUnknown *pCDDBCtrl);
	HRESULT RegisterCallback(UINT nType, void *fnCallback);
	ULONG_PTR SetUserParam(ULONG_PTR userParam);
protected:
	LONG ref;
	DWORD cookie;
	ULONG_PTR user;
	CDDB_CMDCOMPLETED	fnCmdCompleted;
	CDDB_CMDPROGRESS	fnCmdProgress;
	CDDB_LOGMSG			fnLogMessage;
	CDDB_SRVMSG			fnServerMessage;

};

#endif //NULLSOFT_CDDB_EVENT_MANAGER_HEADER

