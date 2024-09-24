#ifndef NULLSOFT_FACTORY_HANDLER_H
#define NULLSOFT_FACTORY_HANDLER_H

#include <api/service/waservicefactory.h>
#include <api/service/services.h>

class CommonHandlerFactory : public waServiceFactory
{
public:
	
	FOURCC GetServiceType();
	int SupportNonLockingInterface();
	int ReleaseInterface(void *ifc);
	const char *GetTestString();
	int ServiceNotify(int msg, int param1, int param2);
	
};

#define DECLARE_HANDLER_FACTORY(CLASSNAME) class CLASSNAME  : public CommonHandlerFactory {\
public:\
	const char *GetServiceName();\
	GUID GetGUID();\
	void *GetInterface(int global_lock);\
protected:\
	RECVS_DISPATCH;}

DECLARE_HANDLER_FACTORY(ASXHandlerFactory);
DECLARE_HANDLER_FACTORY(WPLHandlerFactory);

#endif