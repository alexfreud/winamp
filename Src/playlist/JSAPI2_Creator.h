#pragma once

#include "../Winamp/JSAPI2_svc_apicreator.h"

#include <api/service/waservicefactory.h>
#include <api/service/services.h>

class JSAPI2Factory : public waServiceFactory
{
public:
	FOURCC GetServiceType();
	const char *GetServiceName();
	GUID GetGUID();
	void *GetInterface(int global_lock);
	int SupportNonLockingInterface();
	int ReleaseInterface(void *ifc);
	const char *GetTestString();
	int ServiceNotify(int msg, int param1, int param2);
	
protected:
	RECVS_DISPATCH;
};


class JSAPI2_Creator : public JSAPI2::svc_apicreator
{
	IDispatch *CreateAPI(const wchar_t *name, const wchar_t *key, JSAPI::ifc_info *info);
	int PromptForAuthorization(HWND parent, const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI2::api_security::AuthorizationData *data);
protected:
	RECVS_DISPATCH;
};