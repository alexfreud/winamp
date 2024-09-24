#pragma once

#ifndef _API_SERVICE_H
#define _API_SERVICE_H

#include "../foundation/dispatch.h"
#include "../foundation/types.h"
#include "../foundation/guid.h"
#include "../foundation/error.h"
#include "../service/ifc_servicefactory.h"

// ----------------------------------------------------------------------------
// {6DD93387-6636-4479-B982-9FF5B91CF4B4}
static const GUID service_api_service_guid = 
{ 0x6dd93387, 0x6636, 0x4479, { 0xb9, 0x82, 0x9f, 0xf5, 0xb9, 0x1c, 0xf4, 0xb4 } };

class NOVTABLE api_service : public Wasabi2::Dispatchable
{
protected:
	api_service() : Dispatchable(DISPATCHABLE_VERSION) {}
	~api_service() {}
public:
	static GUID GetServiceGUID() { return service_api_service_guid; }
	int Register(ifc_serviceFactory *svc) { return Service_Register(svc); }
	int Unregister(ifc_serviceFactory *svc) { return Service_Unregister(svc); }
	size_t GetServiceCount(GUID svc_type) { return Service_GetServiceCount(svc_type); }
	ifc_serviceFactory *EnumService(GUID svc_type, size_t n) { return Service_EnumService(svc_type, n); }
	ifc_serviceFactory *EnumService(size_t n) { return Service_EnumService(n); }
	ifc_serviceFactory *GetServiceByGUID(GUID guid) { return Service_GetServiceByGUID(guid); }

	/* called by a component when it returns NErr_TryAgain from one of the quit-phase functions */
	void ComponentDone() { Service_ComponentDone(); }

	template <class _t>
	int GetService(_t **out_service) 
	{
		ifc_serviceFactory *sf = Service_GetServiceByGUID(_t::GetServiceGUID());
		if (!sf)
			return NErr_ServiceUnavailable;

		_t *service = reinterpret_cast<_t *>(sf->GetInterface());
		if (!service)
			return NErr_FailedCreate;

		*out_service = service;
		return NErr_Success;
	}

	enum 
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual int WASABICALL Service_Register(ifc_serviceFactory *svc)=0;
	virtual int WASABICALL Service_Unregister(ifc_serviceFactory *svc)=0;
	virtual size_t WASABICALL Service_GetServiceCount(GUID svc_type)=0;
	virtual ifc_serviceFactory * WASABICALL Service_EnumService(GUID svc_type, size_t n)=0;
	virtual ifc_serviceFactory * WASABICALL Service_EnumService(size_t n)=0;
	virtual ifc_serviceFactory * WASABICALL Service_GetServiceByGUID(GUID guid)=0;
	virtual void WASABICALL Service_ComponentDone()=0;
};

#endif // !_API_SERVICE_H