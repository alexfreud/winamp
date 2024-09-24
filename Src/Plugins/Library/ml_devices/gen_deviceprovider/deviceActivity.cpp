#include "main.h"
#include "./deviceActivity.h"

#include <strsafe.h>

typedef struct DeviceActivityThreadParam
{
	DeviceActivity *activity;
	unsigned int duration;
	unsigned int interval;
	HANDLE readyEvent;
}DeviceActivityThreadParam;

DeviceActivity::DeviceActivity(DeviceActivityFlags flags, 
				   DeviceActivityCallback startCb, DeviceActivityCallback finishCb, 
				   DeviceActivityProgressCallback progressCb, void *user)
	: ref(1), displayName(NULL), status(NULL), activityThread(FALSE), cancelEvent(NULL), progress(0)
{
	InitializeCriticalSection(&lock);

	this->flags = flags;
	callbackStart = startCb;
	callbackFinish = finishCb;
	callbackProgress = progressCb;
	this->user = user;
}

DeviceActivity::~DeviceActivity()
{
	Stop();

	String_Free(displayName);
	String_Free(status);

	DeleteCriticalSection(&lock);
}


HRESULT DeviceActivity::CreateInstance(DeviceActivityFlags flags,
								  DeviceActivityCallback startCb, DeviceActivityCallback finishCb,
								  DeviceActivityProgressCallback progressCb, void *user,
								  DeviceActivity **instance)
{
	DeviceActivity *self;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;
	
	self = new DeviceActivity(flags, startCb, finishCb, progressCb, user);
	if (NULL == self) 
		return E_OUTOFMEMORY;
		
	*instance = self;
	return S_OK;

}

size_t DeviceActivity::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceActivity::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceActivity::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceActivity))
		*object = static_cast<ifc_deviceactivity*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

void DeviceActivity::Lock()
{
	EnterCriticalSection(&lock);
}

void DeviceActivity::Unlock()
{
	LeaveCriticalSection(&lock);
}

BOOL DeviceActivity::GetActive()
{
	BOOL running;

	Lock();
	running = (NULL != activityThread);
	Unlock();

	return running;
}

BOOL DeviceActivity::GetCancelable()
{
	BOOL cancelable;

	Lock();
	cancelable = (0 != (DeviceActivityFlag_Cancelable & flags));
	Unlock();

	return cancelable;
}

HRESULT DeviceActivity::GetProgress(unsigned int *percentCompleted)
{

	if (NULL == percentCompleted)
		return E_POINTER;

	Lock();

	*percentCompleted = progress;

	Unlock();

	return S_OK;
}

HRESULT DeviceActivity::GetDisplayName(wchar_t *buffer, size_t bufferMax)
{
	HRESULT hr;

	if (NULL == buffer)
		return E_POINTER;

	Lock();

	if (0 == String_CopyTo(buffer, displayName, bufferMax) &&
		FALSE == IS_STRING_EMPTY(displayName))
	{
		hr = E_FAIL;
	}
	else 
		hr = S_OK;
		
	Unlock();

	return hr;
}

HRESULT DeviceActivity::GetStatus(wchar_t *buffer, size_t bufferMax)
{
	HRESULT hr;

	if (NULL == buffer)
		return E_POINTER;

	Lock();

	if (0 == String_CopyTo(buffer, status, bufferMax) &&
		FALSE == IS_STRING_EMPTY(status))
	{
		hr = E_FAIL;
	}
	else 
		hr = S_OK;
		
	Unlock();

	return hr;
}

HRESULT DeviceActivity::Cancel(HWND hostWindow)
{
	HRESULT hr;

	Lock();
	
	if (0 == (DeviceActivityFlag_Cancelable & flags))
		hr = E_NOTIMPL;
	else 
		hr = E_FAIL;
	
	Unlock();

	return hr;
}


HRESULT DeviceActivity::Start(unsigned int duration, unsigned int interval)
{
	HRESULT hr;

	Lock();

	if (NULL != activityThread)
		hr = E_PENDING;
	else
	{
		hr = S_OK;

		if (NULL == cancelEvent)
		{
			cancelEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (NULL == cancelEvent)
				hr = E_FAIL;
		}

		if (SUCCEEDED(hr))
		{			
			DeviceActivityThreadParam param;
			
			param.activity = this;
			param.duration = duration;
			param.interval = interval;
			param.readyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			if (NULL == param.readyEvent)
				hr = E_FAIL;
			else
			{
				DWORD threadId;
				
				activityThread = CreateThread(NULL, 0, DeviceActivity_ActivityThreadStarter, 
												&param, 0, &threadId);
				if (NULL == activityThread)
					hr = E_FAIL;
				else
					WaitForSingleObject(param.readyEvent, INFINITE);
				
				CloseHandle(param.readyEvent);
			}
		}

		if (FAILED(hr))
			Stop();
	}

	Unlock();

	return hr;
}

HRESULT DeviceActivity::Stop()
{
	HRESULT hr;
	HANDLE threadHandle, eventHandle;

	Lock();

	threadHandle = activityThread;
	eventHandle = cancelEvent;

	activityThread = NULL;
	cancelEvent = NULL;

	Unlock();

	if (NULL != threadHandle)
	{
		if (NULL != eventHandle)
			SetEvent(eventHandle);

		WaitForSingleObject(threadHandle, INFINITE);
		CloseHandle(threadHandle);
		hr = S_OK;
	}
	else
		hr = S_FALSE;
	
	if (NULL != eventHandle)
		CloseHandle(eventHandle);

	return hr;
}
	
HRESULT DeviceActivity::SetDisplayName(const wchar_t *name)
{
	HRESULT hr;

	Lock();

	if (NULL == name && NULL == displayName)
		hr = S_FALSE;
	else
	{
		if (NULL != displayName && 
			CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, 0, name, -1, displayName, -1))
		{
			hr = S_FALSE;
		}
		else
		{
			wchar_t *string;

			string = String_Duplicate(name);
			if (NULL == string && NULL != name)
				hr = E_FAIL;
			else
			{
				String_Free(displayName);
				displayName = string;
				hr = S_OK;
			}
		}
	}

	Unlock();

	return hr;
}

HRESULT DeviceActivity::SetStatus(const wchar_t *newStatus)
{
	HRESULT hr;

	if (NULL == newStatus && NULL == status)
		return S_FALSE;

	Lock();

	if (NULL != status && 
		CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, 0, newStatus, -1, status, -1))
	{
		hr = S_FALSE;
	}
	else
	{
		wchar_t *string;

		string = String_Duplicate(newStatus);
		if (NULL == string && NULL != newStatus)
			hr = E_FAIL;
		else
		{
			String_Free(status);
			status = string;
			hr = S_OK;
		}
	}

	Unlock();

	return hr;
}

HRESULT DeviceActivity::SetUser(void *data)
{
	Lock();
	user = data;
	Unlock();

	return S_OK;
	
}
HRESULT DeviceActivity::GetUser(void **data)
{
	if (NULL == data)
		return E_POINTER;

	Lock();

	*data = user;

	Unlock();

	return S_OK;
}

DWORD DeviceActivity::ActivityThread(unsigned int duration, unsigned int interval)
{
	DWORD waitResult, waitTime;
	HANDLE cancelEventCopy;
	unsigned int position;

	if (interval > duration)
		interval = duration;


	position = 0;
	
	Lock();
	
	progress = 0;

	if (NULL == cancelEvent ||
		0 == DuplicateHandle(GetCurrentProcess(), cancelEvent, GetCurrentProcess(), 
								&cancelEventCopy, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		cancelEventCopy = NULL;
	}

	Unlock();

	if(NULL == cancelEventCopy)
		return -3;

	Lock();

	if (NULL != callbackStart)
		callbackStart(this);

	if (NULL != callbackProgress)
		callbackProgress(this, position, duration);

	Unlock();

	for(;;)
	{
		waitTime = interval;
		if ((position + waitTime) > duration)
			waitTime = duration - position;

		waitResult = WaitForSingleObject(cancelEventCopy, waitTime);
		if (WAIT_TIMEOUT == waitResult)
		{
			position += waitTime;
			
			Lock();

			if (duration != 0)
			{
				progress = 100 * position / duration;
				if (progress > 100)
					progress = 100;
			}
			else 
				progress = 100;
		

			if (NULL != callbackProgress)
				callbackProgress(this, position, duration);
			Unlock();

			if (position >= duration)
				break;
		}
		else
			break;
	}

	AddRef();

	Lock();
	
	if (NULL != activityThread)
	{
		CloseHandle(activityThread);
		activityThread = NULL;
	}

	if (NULL != cancelEvent)
	{
		CloseHandle(cancelEvent);
		cancelEvent = NULL;
	}
	
	if (NULL != callbackFinish)
		callbackFinish(this);

	Unlock();

	Release();

	return 0;
}

static DWORD CALLBACK 
DeviceActivity_ActivityThreadStarter(void *user)
{
	DeviceActivityThreadParam *param;
	DeviceActivity *activity;
	unsigned int duration, interval;
	DWORD result;
	
	param = (DeviceActivityThreadParam*)user;
	activity = param->activity;
	duration = param->duration;
	interval = param->interval;
	
	if (NULL != param->readyEvent)
		SetEvent(param->readyEvent);
	
	if (NULL != activity)
		result = activity->ActivityThread(duration, interval);
	else
		result = -2;
	
	return result;
}

#define CBCLASS DeviceActivity
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETACTIVE, GetActive)
CB(API_GETCANCELABLE, GetCancelable)
CB(API_GETPROGRESS, GetProgress)
CB(API_GETDISPLAYNAME, GetDisplayName)
CB(API_GETSTATUS, GetStatus)
CB(API_CANCEL, Cancel)
END_DISPATCH;
#undef CBCLASS
