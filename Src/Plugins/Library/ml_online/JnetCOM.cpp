#include "JnetCOM.h"
#include "main.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include <algorithm>
#include <map>
#include "config.h"
#include <time.h>
#include "../nu/MediaLibraryInterface.h"
#include "api__ml_online.h"
#include <api/service/waServiceFactory.h>
#include <strsafe.h>

extern C_Config *g_config;

BufferMap buffer_map;

enum
{
	DISP_JNET_INIT = 9323,
	DISP_JNET_DOWNLOAD,
	DISP_JNET_STATUS,
	DISP_JNET_SIZE,
	DISP_JNET_BUFFER,
	DISP_JNET_BUFFER_RAW,
	DISP_JNET_POST,
};

HANDLE DuplicateCurrentThread()
{
	HANDLE fakeHandle = GetCurrentThread();
	HANDLE copiedHandle = 0;
	HANDLE processHandle = GetCurrentProcess();
	DuplicateHandle(processHandle, fakeHandle, processHandle, &copiedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
	return copiedHandle;
}

#define CHECK_ID(str, id) 		if (wcscmp(rgszNames[i], L##str) == 0)	{		rgdispid[i] = id; continue; }
HRESULT JnetCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("Init", DISP_JNET_INIT)
			CHECK_ID("Download", DISP_JNET_DOWNLOAD)
			CHECK_ID("Status", DISP_JNET_STATUS)
			CHECK_ID("Buffer", DISP_JNET_BUFFER)
			CHECK_ID("BufferRaw", DISP_JNET_BUFFER_RAW)
			CHECK_ID("Size", DISP_JNET_SIZE)
			CHECK_ID("Post", DISP_JNET_POST)

			rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JnetCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JnetCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}


HRESULT JnetCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
	case DISP_JNET_INIT:
		{
			if (!active && NULL == callback)
			{
				finalSize = 0;
				isBlocking = true;
				if ( pdispparams->cArgs == 2 )
				{
					isBlocking = false;
					htmldiv = SysAllocString(pdispparams->rgvarg[0].bstrVal);  // HTML DIV
					callback = pdispparams->rgvarg[1].pdispVal; // callback object;
					if (NULL != callback)
						callback->AddRef();
				}
			}
			else
			{
				if (pvarResult)
				{
					VariantInit(pvarResult);
					V_VT(pvarResult) = VT_I4;
					V_I4(pvarResult) = -1;
				}
			}
			return S_OK;
		}
		break;
	case DISP_JNET_DOWNLOAD:
		if (pdispparams->cArgs == 1 || pdispparams->cArgs == 2)
		{
			int result = -1;
			if ( !active )
			{
				time_t ret = 0;
				active = 1;

				DownloadURL(pdispparams);
				result = 1;
			}
			else result = -1;

			if (pvarResult)
			{
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_I4;
				V_I4(pvarResult) = result;
			}

			return S_OK;
		}
		break;
	case DISP_JNET_POST:
		if (pdispparams->cArgs == 2)
		{
			int result = -1;
			if ( !active )
			{
				time_t ret = 0;
				active = 1;

				PostURL(pdispparams);
				result = 1;
			}
			else result = -1;

			if (pvarResult)
			{
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_I4;
				V_I4(pvarResult) = result;
			}

			return S_OK;
		}
		break;
	case DISP_JNET_STATUS:
		{
			if (pvarResult && errorstr && active )
			{
				AutoWide result(errorstr);
				BSTR tag = SysAllocString(result);
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_BSTR;
				V_BSTR(pvarResult) = tag;
			}
			return S_OK;
		}
		break;
	case DISP_JNET_SIZE:
		{
			if (pvarResult && active )
			{
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_I4;
				V_I4(pvarResult) = (INT)finalSize;
			}
			return S_OK;
		}
		break;
	case DISP_JNET_BUFFER_RAW:
		{
			if (pvarResult && jnetbuf->get() && active)
			{
				char dummy[1]={0};
				size_t len = jnetbuf->getlen();
				char *source = (char *)jnetbuf->get();
				if (!len || !source)
				{
					source=dummy;
					len=1;
				}
				else
				{
					while (*(source+len-1) == 0 && len)
						len--;
				}
				SAFEARRAY *bufferArray=SafeArrayCreateVector(VT_UI1, 0, (ULONG)len);
				void *data;
				SafeArrayAccessData(bufferArray, &data);
				memcpy(data, source, len);
				SafeArrayUnaccessData(bufferArray);
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_ARRAY|VT_UI1;
				V_ARRAY(pvarResult) = bufferArray;
			}
			return S_OK;
		}
	case DISP_JNET_BUFFER:
		{
			if (pvarResult && jnetbuf->get() && active )
			{
				AutoWide result((char *)jnetbuf->get());
				BSTR tag = SysAllocString(result);
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_BSTR;
				V_BSTR(pvarResult) = tag;
			}
			return S_OK;
		}
		break;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JnetCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG JnetCOM::AddRef(void)
{
	return ++refCount;
}

ULONG JnetCOM::Release(void)
{
	if (--refCount)
		return refCount;

	threadexit = 1; // exit the thread if active
	if (!isBlocking)
	{ // Only if a callback was associated would the thread be running
		while (getter) // If there no getter, the thread is gone
			Sleep(10);
	}

	if (!finalSize)
	{
		BufferMap::iterator buffer_it = buffer_map.find(url);
		if (buffer_it != buffer_map.end())
		{
			jnetbuf = buffer_it->second;
			buffer_map.erase(buffer_it->first);
			delete jnetbuf;
			jnetbuf = NULL;
		}
	}
	Plugin_FreeString(url);
	url=0;
	if (postData) 
	{
		free(postData);
		postData=0;
	}
	
	if (NULL != callback)
	{
		callback->Release();
		callback = NULL;
	}
	
	delete this;
	return 0;
}

static VOID CALLBACK CallbackAPC(ULONG_PTR param)
{
	VARIANT arguments[2];
	DISPPARAMS params;
	unsigned int ret;

	JnetCOM *jnet = (JnetCOM *)param;
	if (NULL == jnet || NULL == jnet->callback)
		return;

	VariantInit(&arguments[0]);
	VariantInit(&arguments[1]); 

	V_VT(&arguments[0]) = VT_DISPATCH;
	V_DISPATCH(&arguments[0]) = jnet;

	V_VT(&arguments[1]) = VT_BSTR;
	V_BSTR(&arguments[1]) = jnet->htmldiv;

	params.cArgs = ARRAYSIZE(arguments);
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = NULL;
	params.rgvarg = arguments;

	jnet->callback->Invoke(0, GUID_NULL, 0, DISPATCH_METHOD, &params, NULL, NULL, &ret);


	V_DISPATCH(&arguments[0]) = NULL;
	V_BSTR(&arguments[1]) = NULL;

	VariantClear(&arguments[0]);
	VariantClear(&arguments[1]);

	jnet->Release();


}

void JnetCOM::callBack()
{
	AddRef();
	if (GetCurrentThreadId() == callingThreadId)
		CallbackAPC((ULONG_PTR)this);
	else
	{
		if (NULL == callingThreadHandle || 
			0 == QueueUserAPC(CallbackAPC, callingThreadHandle, (ULONG_PTR)this))
		{
			Release();
		}
	}
}

#define USER_AGENT_SIZE (10 /*User-Agent*/ + 2 /*: */ + 6 /*Winamp*/ + 1 /*/*/ + 1 /*5*/ + 3/*.21*/ + 1 /*Null*/)
void SetUserAgent(api_httpreceiver *http)
{
	char user_agent[USER_AGENT_SIZE] = {0};
	int bigVer = ((winampVersion & 0x0000FF00) >> 12);
	int smallVer = ((winampVersion & 0x000000FF));
	StringCchPrintfA(user_agent, USER_AGENT_SIZE, "User-Agent: Winamp/%01x.%02x", bigVer, smallVer);
	http->addheader(user_agent);
}

int JnetCOM::JthreadProc()
{
	int ret;
	char temp[WORKSIZE] = {0};

	char *proxy = mediaLibrary.GetProxy();

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
	if (sf) getter = (api_httpreceiver *)sf->getInterface();

	// we're keying off of 'getter' to know if the thread is running or not, so we can release now
	Release();

	if (!getter)
		return -1;

	getter->AllowCompression();
	getter->open(API_DNS_AUTODNS, WORKSIZE, proxy);

	SetUserAgent(getter);

	getter->connect(AutoChar(url));

	BOOL bDone = FALSE;
	while (!bDone && !threadexit)
	{
		Sleep(10);
		ret = getter->run();
		if (ret == -1 || ret == 1)
			bDone = TRUE;

		int bytesReceived = getter->get_bytes(temp, WORKSIZE);
		if (bytesReceived)
			jnetbuf->add(temp, bytesReceived);
	}

	if (!threadexit)
	{

		if (ret == -1) StringCchCopyA(errorstr, 2048, getter->geterrorstr());
		else
		{
			int bytesReceived;
			do // flush out the socket
			{
				bytesReceived = getter->get_bytes(temp, WORKSIZE);
				if (bytesReceived)
					jnetbuf->add(temp, bytesReceived);
			}
			while (bytesReceived);

			temp[0] = 0;
			jnetbuf->add(temp, 1);
		}

		finalSize = jnetbuf->getlen();

		callBack();
	}

	sf->releaseInterface(getter);
	threadexit = 0;
	if (callingThreadHandle)
		CloseHandle(callingThreadHandle);
	getter = NULL;
	return ret;
}

int JnetCOM::PostProcedure()
{
	int ret;
	char temp[WORKSIZE] = {0};

	char *proxy = mediaLibrary.GetProxy();

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
	if (sf) getter = (api_httpreceiver *)sf->getInterface();

	// we're keying off of 'getter' to know if the thread is running or not, so we can release now
	Release();
	if (!getter)
		return -1;

	getter->AllowCompression();
	getter->open(API_DNS_AUTODNS, WORKSIZE, proxy);

	SetUserAgent(getter);
	getter->addheader("Content-Type: application/x-www-form-urlencoded");

	size_t contentLength = strlen(postData);
	char contentLengthHeader[256] = {0};
	StringCchPrintfA(contentLengthHeader, 256, "Content-Length: %u", contentLength);
	getter->addheader(contentLengthHeader);

	char *dataIndex = postData;
	bool done=false;

	getter->connect(AutoChar(url), 0, "POST");

	// time to post data!
	api_connection *connection = getter->GetConnection();

	while (contentLength && !threadexit)
	{
		Sleep(1);
		getter->run();				

		size_t lengthToSend = min(contentLength, connection->GetSendBytesAvailable());
		if (lengthToSend)
		{
			connection->send(dataIndex, (int)lengthToSend);
			dataIndex+=lengthToSend;
			contentLength-=lengthToSend;
		}
	}

	while (!threadexit && !done)
	{
		Sleep(10);
		ret = getter->run();
		if (ret == -1)
			break;

		// ---- check our reply code ----
		int replycode = getter->getreplycode();
		switch (replycode)
		{
		case 0:
			break;
		case 100:
			break;
		case 200:
			{
				int bytesReceived = getter->get_bytes(temp, WORKSIZE);
				if (bytesReceived)
					jnetbuf->add(temp, bytesReceived);		
				do
				{
					Sleep(10);
					ret = getter->run();
					bytesReceived = getter->get_bytes(temp, WORKSIZE);
					if (bytesReceived)
						jnetbuf->add(temp, bytesReceived);
				}
				while (ret == HTTPRECEIVER_RUN_OK);
				done=true; // just in case
			}
			break;
		default:
			done=true;
			break;
		}
		if (ret != HTTPRECEIVER_RUN_OK)
			break;		
	}
	if (!threadexit)
	{
		if (ret == -1) 
			StringCchCopyA(errorstr, 2048, getter->geterrorstr());
		else
		{
			int bytesReceived;
			do // flush out the socket
			{
				bytesReceived = getter->get_bytes(temp, WORKSIZE);
				if (bytesReceived)
					jnetbuf->add(temp, bytesReceived);
			}
			while (bytesReceived && !threadexit);

			temp[0] = 0;
			jnetbuf->add(temp, 1);
		}

		if ( !threadexit )
		{
			finalSize = jnetbuf->getlen();
			callBack();
		}

	}
	sf->releaseInterface(getter);

	threadexit = 0;
	if (callingThreadHandle)
		CloseHandle(callingThreadHandle);
	getter = NULL;
	return ret;
}

void JnetCOM::DownloadURL(DISPPARAMS FAR *pdispparams)
{
	Plugin_FreeString(url);
	url = Plugin_CopyString(pdispparams->rgvarg[pdispparams->cArgs - 1].bstrVal);

	callingThreadId = GetCurrentThreadId();
	callingThreadHandle = DuplicateCurrentThread();

	BufferMap::iterator buffer_it = buffer_map.find(url);
	if (buffer_it != buffer_map.end())
	{
		time_t check = time(NULL);
		jnetbuf = buffer_it->second;
		if ( check >= jnetbuf->expire_time)
		{
			buffer_map.erase(buffer_it->first);
			delete jnetbuf;
			jnetbuf = NULL;
		}
		else
		{
			finalSize = jnetbuf->getlen();
			callBack();
		}

	}
	if (!jnetbuf)
	{
		time_t now = 0;

		jnetbuf = buffer_map[url] = new Buffer_GrowBuf;

		if ( pdispparams->cArgs == 2 ) // passed in a time from Javascript, or 0 to not cache
		{
			if ( pdispparams->rgvarg[0].iVal )
			{
				now = time(NULL);
				jnetbuf->expire_time = now + pdispparams->rgvarg[0].iVal;
			}
		}
		else // Use winamp config cache time
		{
			int when = 0, x = 0;

			x = g_config->ReadInt("radio_upd_freq", 0);
			switch ( x )
			{
			case 0:
				{
					when = 3600;    // One Hour
				}
				break;
			case 1:
				{
					when = 3600 * 24; // One Day aka 24 hours
				}
				break;
			case 2:
				{
					when = 3600 * 24 * 7;   // One week (weak)
				}
				break;
			default:
				break;
			}
			if (when)
				now = time(NULL);

			jnetbuf->expire_time = now + when;

		}
		if (isBlocking)
		{
			AddRef(); // need to call this because JthreadProc does a Release
			JthreadProc();  // Call it directly, block until done.
		}
		else
		{
			// Launch the thread
			DWORD threadId;
			AddRef(); // make sure jnetcom object doesn't die while the thread is launching.
			jnetThread = CreateThread(NULL, NULL, JThreadProc, (void *)this, NULL, &threadId);
		}

	}
}

void JnetCOM::PostURL(DISPPARAMS FAR *pdispparams)
{
	postData = AutoCharDup(pdispparams->rgvarg[0].bstrVal);
	Plugin_FreeString(url);
	url = Plugin_CopyString(pdispparams->rgvarg[1].bstrVal);

	callingThreadId = GetCurrentThreadId();
	callingThreadHandle = DuplicateCurrentThread();

	if (!jnetbuf)
	{
		time_t now = 0;

		jnetbuf = buffer_map[url] = new Buffer_GrowBuf;

		if (isBlocking)
		{
			AddRef(); // need to do this beacuse PostProcedure calls Release
			PostProcedure();  // Call it directly, block until done.
		}
		else
		{
			// Launch the thread
			DWORD threadId;
			AddRef(); // make sure the jnetcom doesn't get deleted before the thread launches
			jnetThread = CreateThread(NULL, NULL, jnetPostProcedure, (void *)this, NULL, &threadId);
		}

	}
}

