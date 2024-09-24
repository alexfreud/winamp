#pragma once
#include <api/service/api_service.h>
#include <api/syscb/callbacks/svccb.h>
#include <map>
#include <api/syscb/api_syscb.h>

class ServiceWatcher : public SysCallback
{
public:
	ServiceWatcher() : serviceManager(0),systemCallbacks(0) {}
	~ServiceWatcher();

	void WatchWith(api_service *_serviceApi);
	
	template <class T>
	void WatchFor(T **ptr, GUID watchGUID)
	{
		WatchForT((void **)ptr, watchGUID);
	}
	void StopWatching();
	void Clear();
private:
	void WatchForT(void **ptr, GUID watchGUID);
	typedef std::map<GUID, void **> WatchList;
	WatchList watchList;
  FOURCC GetEventType() { return SysCallback::SERVICE; }
  int Notify(int msg, intptr_t param1, intptr_t param2);
	api_service *serviceManager;
	api_syscb *systemCallbacks;
protected:
	RECVS_DISPATCH;
};

class ServiceWatcherSingle : public SysCallback
{
public:
	ServiceWatcherSingle() : serviceManager(0),systemCallbacks(0),service(0) {}
	virtual ~ServiceWatcherSingle();
	void WatchWith(api_service *_serviceApi);
	
	template <class T>
	void WatchFor(T **ptr, GUID watchGUID)
	{
		WatchForT((void **)ptr, watchGUID);
	}

	virtual void OnRegister() {}
	virtual void OnDeregister()=0;
	void StopWatching();
private:
	void WatchForT(void **ptr, GUID watchGUID);
	FOURCC GetEventType() { return SysCallback::SERVICE; }
	int Notify(int msg, intptr_t param1, intptr_t param2);
	api_service *serviceManager;
	api_syscb *systemCallbacks;
	void **service;
	GUID serviceGUID;
protected:
	RECVS_DISPATCH;
};