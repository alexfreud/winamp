#include "main.h"
#include "./service.h"
#include "./ifc_omserviceevent.h"

HRESULT OmService::RegisterEventHandler(ifc_omserviceevent *handler)
{
	if (NULL == handler) 
		return E_POINTER;

	HRESULT hr;

	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while (index--)
	{
		if (handler == eventList[index])
		{
			break;
		}
	}

	if (((size_t)-1) == index)
	{
		eventList.push_back(handler);
		handler->AddRef();
		hr = S_OK;
	}
	else
	{
		hr = E_UNEXPECTED;
	}

	LeaveCriticalSection(&eventLock);

	return hr;
}

HRESULT OmService::UnregisterEventHandler(ifc_omserviceevent *handler)
{
	if (NULL == handler) 
		return E_POINTER;

	HRESULT hr;

	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while (index--)
	{
		if (handler == eventList[index])
		{
			break;
		}
	}

	if (((size_t)-1) != index)
	{
		ifc_omserviceevent *h = eventList[index];
		eventList.erase(eventList.begin() + index);
		h->Release();
		hr = S_OK;
	}
	else
	{
		hr = S_FALSE;
	}

	LeaveCriticalSection(&eventLock);

	return hr;
}

void OmService::UnregisterAllEventHandlers()
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while(index--)
	{
		ifc_omserviceevent *handler = eventList[index];
		if (NULL != handler) handler->Release();
	}
	eventList.clear();

	LeaveCriticalSection(&eventLock);
}

HRESULT OmService::Signal_ServiceChange(unsigned int modifiedFlags)
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while (index--)
	{
		ifc_omserviceevent *handler = eventList[index];
		if (NULL != handler) 
			handler->ServiceChange(this, modifiedFlags);
	}

	LeaveCriticalSection(&eventLock);
	return S_OK;
}

HRESULT OmService::Signal_CommandStateChange(const GUID *commandGroup, UINT commandId)
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while (index--)
	{
		ifc_omserviceevent *handler = eventList[index];
		if (NULL != handler) 
			handler->CommandStateChange(this, commandGroup, commandId);
	}

	LeaveCriticalSection(&eventLock);
	return S_OK;
}