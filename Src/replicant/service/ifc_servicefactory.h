#pragma once

#include "../foundation/dispatch.h"
#include "../foundation/guid.h"
#include "../nx/nxstring.h"

// ----------------------------------------------------------------------------

class NOVTABLE ifc_serviceFactory : public Wasabi2::Dispatchable
{
protected:
	ifc_serviceFactory() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_serviceFactory() {}


public:
	GUID GetServiceType() { return ServiceFactory_GetServiceType(); }
	nx_string_t GetServiceName() { return ServiceFactory_GetServiceName(); }
	GUID GetGUID() { return ServiceFactory_GetGUID(); }
	void *GetInterface() { return ServiceFactory_GetInterface(); }
	int ServiceNotify(int msg, intptr_t param1 = 0, intptr_t param2 = 0) { return ServiceFactory_ServiceNotify(msg, param1, param2); }

	// serviceNotify enums
	enum 
	{
		ONREGISTERED=0,	// init yourself here -- not all other services are registered yet
		ONSTARTUP=1,	// everyone is initialized, safe to talk to other services
		ONAPPRUNNING=2,	// app is showing and processing events
		ONBEFORESHUTDOWN=3, // system is about to shutdown, call WASABI2_API_APP->main_cancelShutdown() to cancel
		ONSHUTDOWN=4,	// studio is shutting down, release resources from other services
		ONUNREGISTERED=5,	// bye bye			
	};

	enum
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual GUID WASABICALL ServiceFactory_GetServiceType()=0;
	virtual nx_string_t WASABICALL ServiceFactory_GetServiceName()=0;
	virtual GUID WASABICALL ServiceFactory_GetGUID()=0;
	virtual void *WASABICALL ServiceFactory_GetInterface()=0;
	virtual int WASABICALL ServiceFactory_ServiceNotify(int msg, intptr_t param1 = 0, intptr_t param2 = 0)=0;
};
