#include <windows.h>
#include "irctell.h"
#include <api/syscb/api_syscb.h>
#include <api/core/api_core.h>
#include "api__irctell.h"
#include <api/service/waservicefactory.h>
#include "dde.h"
#include <strsafe.h>

static WACIrctell Irctell;
extern "C" __declspec(dllexport) ifc_wa5component *GetWinamp5SystemComponent()
{
	return &Irctell;
}

bool dotell=true;
api_syscb *sysCallbackApi=0;
api_core *core=0;
api_service *WASABI_API_SVC = 0;

void WACIrctell::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(syscbApiServiceGuid);
	if (sf) sysCallbackApi = reinterpret_cast<api_syscb *>(sf->getInterface());

	sysCallbackApi->syscb_registerCallback(static_cast<SysCallback *>(this));
	sf = WASABI_API_SVC->service_getServiceByGuid(coreApiServiceGuid);
	if (sf) core = reinterpret_cast<api_core *>(sf->getInterface());

	if (core)
		core->core_addCallback(0, this);
}

void WACIrctell::DeregisterServices(api_service *service) 
{
	// be sure to delete all your windows etc HERE, not in the destructor
	// because the API pointer might be invalid in the destructor
//	if (core)
	//	core->core_delCallback(0, this);
}

/*
void WACIrctell::onRegisterServices() {
dotell.setName("Enabled");
dotell=0;
registerAttribute(&dotell);

appstr.setName("DDE Target");
appstr="mIRC";
registerAttribute(&appstr);

cmdstr.setName("DDE Command");
cmdstr="/me is listening to %s";
registerAttribute(&cmdstr);
}
*/

int WACIrctell::ccb_notify(int msg, int param1, int param2)
{
	if (msg==TITLECHANGE && dotell)
	{
		const wchar_t *title = (const wchar_t *)param1;
		const wchar_t *cur=core->core_getCurrent(0);

		wchar_t msg[256];
		StringCchPrintfW(msg, 256, L"/describe #winamp is now listening to \"%s\"", title);
		DdeCom::sendCommand(L"mIRC", msg, 1000);
	}
	return 0;
}

FOURCC WACIrctell::getEventType()
{
	return SysCallback::SERVICE;
}

int WACIrctell::notify(int msg, int param1, int param2)
{
	switch(msg)
	{
	case SvcCallback::ONREGISTER:
		{
			waServiceFactory *sf = (waServiceFactory *)param2;
			if (sf->getGuid() == coreApiServiceGuid)
			{
				core = reinterpret_cast<api_core *>(sf->getInterface());
				core->core_addCallback(0, this);
			}
		}
		break;
	
	case SvcNotify::ONDEREGISTERED:
		{
			waServiceFactory *sf = (waServiceFactory *)param2;
			if (sf->getGuid() == coreApiServiceGuid)
			{
				if (core)
					core->core_delCallback(0, this);
				core = 0;
			}
		}
		break;
		
	}
	return 0;

}

#define CBCLASS WACIrctell
START_MULTIPATCH;
START_PATCH(patch_wa5)
M_VCB(patch_wa5, ifc_wa5component, API_WA5COMPONENT_REGISTERSERVICES, RegisterServices);
M_VCB(patch_wa5, ifc_wa5component, API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices);
NEXT_PATCH(patch_core)
M_CB(patch_core, CoreCallback, CCB_NOTIFY, ccb_notify);
NEXT_PATCH(patch_svc)
M_CB(patch_svc, SysCallback, SYSCALLBACK_GETEVENTTYPE, getEventType);
M_CB(patch_svc, SysCallback, SYSCALLBACK_NOTIFY, notify);
END_PATCH
END_MULTIPATCH;
#undef CBCLASS
