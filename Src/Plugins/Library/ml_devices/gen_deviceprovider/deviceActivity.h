#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_ACTIVITY_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_ACTIVITY_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef enum DeviceActivityFlags
{
	DeviceActivityFlag_Cancelable = (1 << 0),
	DeviceActivityFlag_SupportProgress = (1 << 0),
} DeviceActivityFlags;
DEFINE_ENUM_FLAG_OPERATORS(DeviceActivityFlags);

typedef void (*DeviceActivityCallback)(DeviceActivity * /*activity*/);
typedef void (*DeviceActivityProgressCallback)(DeviceActivity * /*activity*/, unsigned int /*position*/, unsigned int /*total*/);


class DeviceActivity: public ifc_deviceactivity
{

protected:
	DeviceActivity(DeviceActivityFlags flags, 
				   DeviceActivityCallback startCb, 
				   DeviceActivityCallback finishCb, 
				   DeviceActivityProgressCallback progressCb,
				   void *user);

	~DeviceActivity();
public:
	static HRESULT CreateInstance(DeviceActivityFlags flags,
								  DeviceActivityCallback startCb,
								  DeviceActivityCallback finishCb,
								  DeviceActivityProgressCallback progressCb,
								  void *user,
								  DeviceActivity **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_deviceactivity */
	BOOL GetActive();
	BOOL GetCancelable();
	HRESULT GetProgress(unsigned int *percentCompleted);
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferMax);
	HRESULT GetStatus(wchar_t *buffer, size_t bufferMax);
	HRESULT Cancel(HWND hostWindow);

public:
	void Lock();
	void Unlock();

	HRESULT Start(unsigned int duration, unsigned int interval);
	HRESULT Stop();
	
	HRESULT SetDisplayName(const wchar_t *displayName);
	HRESULT SetStatus(const wchar_t *status);
	
	HRESULT SetUser(void *data);
	HRESULT GetUser(void **data);

protected:
	DWORD ActivityThread(unsigned int duration, unsigned int interval);
	friend static DWORD CALLBACK DeviceActivity_ActivityThreadStarter(void *param);

protected:
	size_t ref;
	DeviceActivityFlags flags;
	DeviceActivityCallback callbackStart;
	DeviceActivityCallback callbackFinish;
	DeviceActivityProgressCallback callbackProgress;
	void *user;
	wchar_t *displayName;
	wchar_t *status;
	HANDLE activityThread;
	HANDLE cancelEvent;
	unsigned int progress;
	CRITICAL_SECTION lock;
	
	
protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_ACTIVITY_HEADER