#ifndef NULLSOFT_WINAMP_OMSERVICE_EVENTMANAGER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_EVENTMANAGER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>


// {2ED54062-1442-4dfb-B0AE-C43846BE4FCE}
static const GUID IFC_OmServiceEventMngr =
{ 0x2ed54062, 0x1442, 0x4dfb, { 0xb0, 0xae, 0xc4, 0x38, 0x46, 0xbe, 0x4f, 0xce } };

class ifc_omservice;
class ifc_omserviceevent;

class __declspec(novtable) ifc_omserviceeventmngr : public Dispatchable
{

protected:
	ifc_omserviceeventmngr() {}
	~ifc_omserviceeventmngr() {}

public:
	HRESULT RegisterHandler(ifc_omserviceevent *handler);
	HRESULT UnregisterHandler(ifc_omserviceevent *handler);

	HRESULT Signal_ServiceChange(unsigned int modifiedFlags);
	HRESULT Signal_CommandStateChange(const GUID *commandGroup, unsigned int commandId);

public:
	DISPATCH_CODES
	{
		API_REGISTERHANDLER			= 10,
		API_UNREGISTERHANDLER		= 20,
		API_SIGNAL_SERVICECHANGE	= 30,
		API_SIGNAL_COMMANDSTATECHANGE = 40,
	};
};

inline HRESULT ifc_omserviceeventmngr::RegisterHandler(ifc_omserviceevent *handler)
{
	return _call(API_REGISTERHANDLER, (HRESULT)E_NOTIMPL, handler);
}

inline HRESULT ifc_omserviceeventmngr::UnregisterHandler(ifc_omserviceevent *handler)
{
	return _call(API_UNREGISTERHANDLER, (HRESULT)E_NOTIMPL, handler);
}

inline HRESULT ifc_omserviceeventmngr::Signal_ServiceChange(unsigned int modifiedFlags)
{
	return _call(API_SIGNAL_SERVICECHANGE, (HRESULT)E_NOTIMPL, modifiedFlags);
}

inline HRESULT ifc_omserviceeventmngr::Signal_CommandStateChange( const GUID *commandGroup, unsigned int commandId)
{
	return _call(API_SIGNAL_COMMANDSTATECHANGE, (HRESULT)E_NOTIMPL, commandGroup, commandId);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_EVENTMANAGER_INTERFACE_HEADER