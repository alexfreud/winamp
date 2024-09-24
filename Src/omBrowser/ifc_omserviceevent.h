#ifndef NULLSOFT_WINAMP_OMSERVICE_EVENTHANDLER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_EVENTHANDLER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {82B3DD14-A2B3-4f80-A4E1-C8702E0AF70E}
static const GUID IFC_OmServiceEvent = 
{ 0x82b3dd14, 0xa2b3, 0x4f80, { 0xa4, 0xe1, 0xc8, 0x70, 0x2e, 0xa, 0xf7, 0xe } };

class ifc_omservice;

class __declspec(novtable) ifc_omserviceevent : public Dispatchable
{

protected:
	ifc_omserviceevent() {}
	~ifc_omserviceevent() {}

public:
	void ServiceChange(ifc_omservice *service, unsigned int modifiedFlags);
	void CommandStateChange(ifc_omservice *service, const GUID *commandGroup, unsigned int commandId);

public:
	DISPATCH_CODES
	{		
		API_SERVICECHANGE = 10,
		API_COMMANDSTATECHANGE = 20,
	};
};

inline void ifc_omserviceevent::ServiceChange(ifc_omservice *service, unsigned int modifiedFlags)
{
	_voidcall(API_SERVICECHANGE, service, modifiedFlags);
}

inline void ifc_omserviceevent::CommandStateChange(ifc_omservice *service, const GUID *commandGroup, unsigned int commandId)
{
	_voidcall(API_COMMANDSTATECHANGE, service, commandGroup, commandId);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_EVENTHANDLER_INTERFACE_HEADER