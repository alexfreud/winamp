#include "main.h"
#include "./component.h"
#include "./deviceManagerFactory.h"

static DeviceManagerFactory deviceManagerFactory;

DevicesComponent::DevicesComponent()
{
	InitializeCriticalSection(&lock);
}

DevicesComponent::~DevicesComponent()
{
	EnterCriticalSection(&lock);
	
	ReleaseServices();

	LeaveCriticalSection(&lock);

	DeleteCriticalSection(&lock);
}

size_t DevicesComponent::AddRef()
{
	return 1;
}

size_t DevicesComponent::Release()
{
	return 1;
}

int DevicesComponent::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;

	*object = NULL;
	return E_NOINTERFACE;
}

void DevicesComponent::RegisterServices(api_service *service)
{	
	EnterCriticalSection(&lock);

	deviceManagerFactory.Register(service);

	LeaveCriticalSection(&lock);

	aTRACE_LINE("Devices Service Registered");
}

int DevicesComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

void DevicesComponent::DeregisterServices(api_service *service)
{	
	EnterCriticalSection(&lock);

	deviceManagerFactory.Unregister(service);

	ReleaseServices();

	LeaveCriticalSection(&lock);

	aTRACE_LINE("Devices Service Unregistered");
}

void DevicesComponent::ReleaseServices()
{
}

#define CBCLASS DevicesComponent
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS