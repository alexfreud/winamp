#pragma once

#include <api/service/waservicefactory.h>
#include <bfc/multipatch.h>
#include <api/syscb/callbacks/svccb.h>
enum {ServiceFactoryPatch, SysCallbackPatch };
class LazyServiceFactory : public MultiPatch<ServiceFactoryPatch, waServiceFactory>, 	public MultiPatch<SysCallbackPatch, SysCallback>
{
public:
	LazyServiceFactory(FOURCC _service_type, GUID _service_guid, char *_service_name, char *_service_test_string, const wchar_t *_service_filename);
	~LazyServiceFactory();
	FOURCC GetServiceType();
	const char *GetServiceName();
	GUID GetGUID();
	void *GetInterface(int global_lock);
	int SupportNonLockingInterface();
	int ReleaseInterface(void *ifc);
	const char *GetTestString();
	int ServiceNotify(int msg, intptr_t param1, intptr_t param2);
	FOURCC GetEventType() { return SysCallback::SERVICE; }
  int Notify(int msg, intptr_t param1, intptr_t param2);
protected:
	RECVS_MULTIPATCH;

	FOURCC service_type;
	char *service_name;
	GUID service_guid;
	wchar_t service_filename[MAX_PATH];
	char *service_test_string;
};