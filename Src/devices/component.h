#ifndef _NULLSOFT_WINAMP_DEVICES_COMPONENT_HEADER
#define _NULLSOFT_WINAMP_DEVICES_COMPONENT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../Agave/Component/ifc_wa5component.h"

class DevicesComponent : public ifc_wa5component
{
public:
	DevicesComponent();
	~DevicesComponent();

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_wa5component */
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);

protected:
	void ReleaseServices(void);

protected:
	RECVS_DISPATCH;

private:
	CRITICAL_SECTION lock;
};


#endif //_NULLSOFT_WINAMP_DEVICES_COMPONENT_HEADER