#include "WbmSvcMgr.h"
#include <api/service/waServiceFactory.h>
int Add(HANDLE manifest, GUID service_guid, uint32_t service_type, const char *service_name, const char *service_test_string);
int WbmSvcMgr::service_register(waServiceFactory *svc)
{
	GUID service_guid = svc->getGuid();
	uint32_t service_type = svc->getServiceType();
	const char *service_name = svc->getServiceName();
	const char *service_test_string = (const char *)svc->getTestString();
	printf("Found service: %s\n", service_name);
	Add(manifest, service_guid, service_type, service_name, service_test_string);

	return 1;
}

#define CBCLASS WbmSvcMgr
START_DISPATCH;
CB(API_SERVICE_SERVICE_REGISTER, service_register);
END_DISPATCH;
#undef CBCLASS