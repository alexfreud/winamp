#include "Wasabi.h"
#include "api__wasabi-replicant.h"
#include "nswasabi/singleton.h"
#include "foundation/error.h"

SysCallbacks system_callbacks;
ServiceManager service_manager;

static SingletonServiceFactory<SysCallbacks, api_syscb> syscb_factory;

int Wasabi_Init()
{
	syscb_factory.Register(WASABI2_API_SVC, WASABI2_API_SYSCB);
	return NErr_Success;
}
