#include "api.h"
#include "../Agave/Component/ifc_wa5component.h"
#include "factory_isocreator.h"
#include "factory_isoburner.h"
#include "factory_databurner.h"
#include <bfc/platform/export.h>

class WA5_Burner : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};

ISOCreatorFactory creatorFactory;
ISOBurnerFactory isoBurnerFactory;
DataBurnerFactory dataBurnerFactory;
api_service *WASABI_API_SVC = 0;

void WA5_Burner::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	WASABI_API_SVC->service_register(&creatorFactory);
	WASABI_API_SVC->service_register(&isoBurnerFactory);	
	WASABI_API_SVC->service_register(&dataBurnerFactory);	
}

int WA5_Burner::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_Burner::DeregisterServices(api_service *service)
{
	service->service_deregister(&creatorFactory);
	service->service_deregister(&isoBurnerFactory);
	service->service_deregister(&dataBurnerFactory);
}

static WA5_Burner wa5_burner;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_burner;
}

#define CBCLASS WA5_Burner
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS