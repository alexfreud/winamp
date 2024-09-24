#pragma once
#include "../nx/nxstring.h"

/*
this is a helper class to implement GetServiceName 
just put WASABI_SERVICE_NAME("Your Service Name"); in the public section of your class declaration.
this implementation does leak memory, but I don't think it's that big of a deal (services can't be unloaded at run-time anyway)
if we ever implement the NXSTR() macro, we can eliminate this leak

e.g.
class MyAPI : public api_whatever
{
public:
	WASABI_SERVICE_NAME("My API");
};
 */

#define WASABI_SERVICE_NAME(x) static nx_string_t GetServiceName(){	static nx_string_t service_name=0; if (!service_name) NXStringCreateWithUTF8(&service_name, x); return NXStringRetain(service_name); }
#define WASABI_SERVICE_GUID(x) static GUID GetServiceGUID() { return x; }


