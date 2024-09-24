#pragma once
#include <windows.h>
#include <api/service/api_service.h>

class WbmSvcMgr : public api_service
{
public:
	WbmSvcMgr(HANDLE _manifest)
	{
		manifest=_manifest;
	}
	int service_register(waServiceFactory *svc);
protected:
	RECVS_DISPATCH;
private:
	HANDLE manifest;
};