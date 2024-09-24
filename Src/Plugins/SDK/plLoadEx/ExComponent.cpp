#include "ExComponent.h"
#include "api/service/api_service.h" // Service Manager is central to Wasabi
#include "SimpleHandlerFactory.h"    // the Service Factory we're going to regsister

// the service factory we're going to register
static SimpleHandlerFactory simpleHandlerFactory;

void ExComponent::RegisterServices(api_service *service)
{
	// If we need any services, we can retrieve them here
	// however, you have no guarantee that a service you want will be active yet
	// so it's best to "lazy load" and get it the first time you need it

	// Register any services we provide here
	service->service_register(&simpleHandlerFactory);
}

void ExComponent::DeregisterServices(api_service *service)
{
	// Unregister our services
	service->service_deregister(&simpleHandlerFactory);

	// And release any services we retrieved
}

// Define the dispatch table
#define CBCLASS ExComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
