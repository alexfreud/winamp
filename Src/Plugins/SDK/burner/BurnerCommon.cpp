#include "BurnerCommon.h"
#include "api.h"
#include <api/service/waservicefactory.h>

BurnerCommon::BurnerCommon(obj_primo *_primo)
{
	primo = _primo;
	triggerEvent=0;
	triggerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

BurnerCommon::~BurnerCommon()
{
	if (triggerEvent)
		CloseHandle(triggerEvent);
	
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_primo::getServiceGuid());
	if (sf) sf->releaseInterface(primo);
}

void BurnerCommon::TriggerCallback()
{
	if (triggerEvent)
		SetEvent(triggerEvent);
}