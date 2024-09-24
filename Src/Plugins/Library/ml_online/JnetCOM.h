#ifndef NULLSOFT_JNETCOMH
#define NULLSOFT_JNETCOMH

#include <process.h>
#include <ocidl.h>

#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"
#include <objbase.h>
#include "BufferCache.h"
#include <map>
#include <string>


typedef std::map<std::wstring, Buffer_GrowBuf *> BufferMap;
extern BufferMap buffer_map;
#define WORKSIZE 32768

class JnetCOM : public IDispatch
{
public:
  JnetCOM()	: 
	  refCount(1),
	  threadexit(0),
	  active(0),
	  jnetThread(0),
	  jnetbuf(NULL),
	  isBlocking(0),
	  finalSize(0),
	  getter(NULL),
	  callback(NULL),
	  postData(0),
	  url(0),
	  callingThreadId(0),
	  callingThreadHandle(0)
	{
		memset(errorstr, 0, sizeof(errorstr));
	}
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	int refCount;
	// *** IDispatch Methods ***
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

	void DownloadURL(DISPPARAMS FAR *pdispparams );
	void PostURL(DISPPARAMS FAR *pdispparams);
	void callBack();

	int threadexit;
	int isBlocking;

	api_httpreceiver *getter;
	Buffer_GrowBuf *jnetbuf;
	IDispatch *callback;
	BSTR  htmldiv;
	DWORD callingThreadId;
	HANDLE callingThreadHandle;

	static DWORD WINAPI JThreadProc(void *param) 
	{
		CoInitialize(NULL);
		JnetCOM  *th = static_cast<JnetCOM*>(param);
		int ret = th->JthreadProc();
		_endthread();
		return ret;
	}

	static DWORD WINAPI jnetPostProcedure(void *param) 
	{
		CoInitialize(NULL);
		JnetCOM  *th = static_cast<JnetCOM*>(param);
		int ret = th->PostProcedure();
		_endthread();
		return ret;
	}

	size_t finalSize;
	wchar_t *url;
	char *postData;
	int active;
	char errorstr[2048];
	HANDLE jnetThread;
private:
    int JthreadProc();
	int PostProcedure();
};

#endif