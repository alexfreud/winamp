#include "./setupLog.h"
#include "../common.h"
#include "../api__ml_online.h"
#include "../config.h"

#include "../../nu/trace.h"

#include <ifc_omservice.h>
#include <ifc_omwebstorage.h>
#include <ifc_omstorageasync.h>

#include <shlwapi.h>
#include <strsafe.h>

#define SETUPLOG_SEPARATOR			','
#define SETUPLOG_SECTION			"Setup"
#define SETUPLOG_KEY_SUBSCRIBED		"subscribed"
#define SETUPLOG_KEY_UNSUBSCRIBED	"unsubscribed"

struct LOGPARSERPARAM
{
	LOGPARSERPARAM() : instance(NULL), operation(0) {}

	SetupLog *instance;
	UINT operation;
};

static size_t SetupLog_GetMaxServiceIdCount(SetupLog::ServiceMap *serviceMap)
{
	SetupLog::ServiceMap::iterator it;

	size_t c1 = 0, c2 = 0;
	for (it = serviceMap->begin(); it != serviceMap->end(); it++)
	{
		switch(it->second)
		{
			case SetupLog::opServiceAdded:
				c1++;
				break;
			case SetupLog::opServiceRemoved:
				c2++;
				break;
		}
	}
	return  (c1 > c2) ? c1 : c2;
}

static HRESULT SetupLog_FormatServiceId(SetupLog::ServiceMap *serviceMap, INT operation, LPSTR pszBuffer, size_t cchBufferMax)
{
	if (NULL == pszBuffer) 
		return E_INVALIDARG;

	*pszBuffer = '\0';

	if (NULL == serviceMap)
		return S_OK;

	HRESULT hr = S_OK;
	size_t remaining = cchBufferMax;
	LPSTR cursor = pszBuffer;
	SetupLog::ServiceMap::iterator it;

	const char format[] = { SETUPLOG_SEPARATOR, '%', 'u', '\0'};

	for (it = serviceMap->begin(); it != serviceMap->end() && SUCCEEDED(hr); it++)
	{
		if (it->second == operation)
		{
			hr = StringCchPrintfExA(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, 
					((cursor == pszBuffer) ? (format + 1) : format), it->first);
		}
	}
	return hr;
}

static LPCSTR SetupLog_GetOperationKey(UINT operation)
{
	switch(operation)
	{
		case SetupLog::opServiceAdded: return SETUPLOG_KEY_SUBSCRIBED;
		case SetupLog::opServiceRemoved: return SETUPLOG_KEY_UNSUBSCRIBED;
	}
	return NULL;
}

static LPCWSTR SetupLog_GetOperationAction(UINT operation)
{
	switch(operation)
	{
		case SetupLog::opServiceAdded: return L"add";
		case SetupLog::opServiceRemoved: return L"remove";
	}
	return NULL;
}

static BOOL SetupLog_WriteOperationLog(UINT operation, LPCSTR pszValue)
{
	LPCSTR pszKey = SetupLog_GetOperationKey(operation);
	if (NULL == pszKey) return FALSE;
	return Config_WriteStr(SETUPLOG_SECTION, pszKey, pszValue);
}

static HRESULT SetupLog_ReadOperationLog(UINT operation, LPSTR pszBuffer, UINT cchBufferMax, UINT *cchReaded)
{	
	LPCSTR pszKey = SetupLog_GetOperationKey(operation);
	if (NULL == pszKey) return E_INVALIDARG;

	DWORD readed = Config_ReadStr(SETUPLOG_SECTION, pszKey, NULL, pszBuffer, cchBufferMax);

	if (NULL != cchReaded)
	{
		*cchReaded = readed;
	}
	
	return S_OK;
}

SetupLog::SetupLog() : ref(1)
{
}

SetupLog::~SetupLog()
{

}

SetupLog *SetupLog::Open()
{
	SetupLog *instance = new SetupLog();
	if (NULL == instance) return NULL;

	INT cchBuffer =  32000;
	LPSTR buffer = Plugin_MallocAnsiString(cchBuffer);
	if (NULL == buffer)
	{
		instance->Release();
		return NULL;
	}

	UINT cchReaded = 0;
	const UINT szOperations[] = {	opServiceAdded, 
									opServiceRemoved, };

	UINT serviceId;
	for (INT i = 0; i < ARRAYSIZE(szOperations); i++)
	{
		if (SUCCEEDED(SetupLog_ReadOperationLog(szOperations[i], buffer, cchBuffer, &cchReaded)) && cchReaded > 0)
		{			
			LPSTR cursor = buffer;
			LPSTR block = cursor;

			for(;;)
			{
				if (SETUPLOG_SEPARATOR == *cursor || '\0' == *cursor)
				{
					while (' ' == *block && block < cursor) block++;
					
					if (block < cursor &&
						FALSE != StrToIntExA(block, STIF_SUPPORT_HEX, (INT*)&serviceId) &&
						0 != serviceId)
					{
						instance->LogServiceById(serviceId, szOperations[i]);
					}

					if ('\0' == *cursor)
						break;

					cursor++;
					block = cursor;
				}
				else
				{
					cursor++;
				}
			}
		}
	}

	Plugin_FreeAnsiString(buffer);
	return instance;
}

ULONG SetupLog::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG SetupLog::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

BOOL SetupLog::IsOperationSupported(UINT operation)
{
	switch(operation)
	{
		case SetupLog::opServiceAdded:
		case SetupLog::opServiceRemoved:
			return TRUE;
	}
	return FALSE;
}

HRESULT SetupLog::LogServiceById(UINT serviceUid, UINT operation)
{
	if (0 == serviceUid || FALSE == IsOperationSupported(operation)) 
		return E_INVALIDARG;

	serviceMap[serviceUid] = operation;
	return S_OK;
}

HRESULT SetupLog::LogService(ifc_omservice *service, UINT operation)
{
	if (NULL == service || !IsOperationSupported(operation))
		return E_INVALIDARG;
	
	return LogServiceById(service->GetId(), operation);
}

HRESULT SetupLog::Save()
{
	LPSTR buffer = NULL;
	size_t cchBuffer = SetupLog_GetMaxServiceIdCount(&serviceMap) * 11; 

	if (0 != cchBuffer)
	{
		cchBuffer += 1;
		buffer = Plugin_MallocAnsiString(cchBuffer);
		if (NULL == buffer)
			return E_OUTOFMEMORY;
	}

	const UINT szOperations[] = {	opServiceAdded, 
									opServiceRemoved, };

	for (INT i = 0; i < ARRAYSIZE(szOperations); i++)
	{
		LPCSTR value = (NULL != buffer && 
					SUCCEEDED(SetupLog_FormatServiceId(&serviceMap, szOperations[i], buffer, cchBuffer)) && 
					'\0' != *buffer) ? buffer : NULL;
		SetupLog_WriteOperationLog(szOperations[i], value);
	}

	if (NULL != buffer)
		Plugin_FreeAnsiString(buffer);

	return S_OK;
}

HRESULT SetupLog::Erase()
{
	HRESULT hr = S_OK;
	if (FALSE == Config_WriteStr(SETUPLOG_SECTION, SETUPLOG_KEY_SUBSCRIBED, NULL))
		hr = E_FAIL;
	if (FALSE == Config_WriteStr(SETUPLOG_SECTION, SETUPLOG_KEY_UNSUBSCRIBED, NULL))
		hr = E_FAIL;

	return hr;
}

struct LOGSENDJOBPARAM
{
	LOGSENDJOBPARAM() : totalJobs(0), storage(NULL), completeEvent(NULL) {}

	ULONG totalJobs;
	CRITICAL_SECTION lock;
	ifc_omstorage *storage;
	HANDLE completeEvent;
};

static void CALLBACK SetupLog_SendCompleted(ifc_omstorageasync *async)
{
	if (NULL != async)
	{
		LOGSENDJOBPARAM *param = NULL;
		if (SUCCEEDED(async->GetData((void**)&param)) && NULL != param)
		{
			EnterCriticalSection(&param->lock);

			if (NULL != param->storage)
			{
				param->storage->EndLoad(async, NULL);
				param->storage->Release();
			}
			
			LONG r = InterlockedDecrement((LONG*)&param->totalJobs);
			if (0 == r)
			{
				if (NULL != param->completeEvent)
					SetEvent(param->completeEvent);

				LeaveCriticalSection(&param->lock);
				DeleteCriticalSection(&param->lock);
				free(param);
				param = NULL;
			}
			else
			{
				LeaveCriticalSection(&param->lock);
			}
		}
	}
}

HRESULT SetupLog::Send(HANDLE completeEvent)
{
	size_t cchAlloc = serviceMap.size();
	if (0 == cchAlloc)
	{
		if (NULL != completeEvent) SetEvent(completeEvent);
		return S_OK;
	}

	UINT *buffer = (UINT*)calloc(cchAlloc, sizeof(UINT));
	LOGSENDJOBPARAM *param = (LOGSENDJOBPARAM*)calloc(1, sizeof(LOGSENDJOBPARAM));

	if (NULL == buffer || NULL == param)
	{
		if (NULL != buffer) { free(buffer); buffer = NULL; }
		if (NULL != param) { free(param); param = NULL; }
		if (NULL != completeEvent) { SetEvent(completeEvent); completeEvent = NULL; }
		return E_OUTOFMEMORY;
	}

	ifc_omstorage *storage = NULL;
	HRESULT hr = OMSERVICEMNGR->QueryStorage(&SUID_OmStorageUrl, &storage);
	if (SUCCEEDED(hr) && storage != NULL)
	{
		const UINT szOperations[] = {	opServiceAdded, 
										opServiceRemoved, };
		param->totalJobs = 0;
		param->completeEvent = completeEvent;
		param->storage = storage;

		InitializeCriticalSection(&param->lock);
		EnterCriticalSection(&param->lock);

		for (INT i = 0; i < ARRAYSIZE(szOperations); i++)
		{
			size_t count = 0;
			hr = S_OK;
			for (SetupLog::ServiceMap::iterator it = serviceMap.begin(); it != serviceMap.end() && SUCCEEDED(hr); it++)
			{
				if (it->second == szOperations[i])
				{
					buffer[count] = it->first;
					count++;
				}
			}

			if (0 != count)
			{
				LPWSTR url = NULL;
				LPCWSTR action = SetupLog_GetOperationAction(szOperations[i]);
				if (NULL != action && 
					SUCCEEDED(Plugin_BuildActionUrl(&url, action, buffer, count)))
				{
					ifc_omstorageasync *async = NULL;;
					if (SUCCEEDED(storage->BeginLoad(url, NULL, SetupLog_SendCompleted, param, &async)))
					{
						InterlockedIncrement((LONG*)&param->totalJobs);
						storage->AddRef();
						async->Release();
					}
					Plugin_FreeString(url);
				}
			}
		}

		if (0 == param->totalJobs)
		{
			LeaveCriticalSection(&param->lock);
			DeleteCriticalSection(&param->lock);
			hr = E_FAIL;
			if (param) { free(param); param = NULL; }
		}
		else
		{
			LeaveCriticalSection(&param->lock);
		}

		storage->Release();
	}

	if (buffer) { free(buffer); buffer = NULL; }
	return hr;
}