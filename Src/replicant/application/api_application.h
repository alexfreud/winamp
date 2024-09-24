#pragma once
#include "../replicant/foundation/dispatch.h"
#include "../replicant/foundation/error.h"
#include "../replicant/service/types.h"
#include "../replicant/nx/nxuri.h"

// {23B96771-09D7-46d3-9AE2-20DCEA6C86EA}
static const GUID applicationApiServiceGuid =
{
	0x23b96771, 0x9d7, 0x46d3, { 0x9a, 0xe2, 0x20, 0xdc, 0xea, 0x6c, 0x86, 0xea }
};

// ----------------------------------------------------------------------------
class api_application: public Wasabi2::Dispatchable
{
protected:
	api_application()	: Dispatchable(DISPATCHABLE_VERSION) {}
	~api_application()	{}
public:
	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return applicationApiServiceGuid; }
	const char *GetUserAgent() { return Application_GetUserAgent(); }

	/* returns a path where you can store data files, if you need to */
	int GetDataPath(nx_uri_t *path) { return Application_GetDataPath(path); }

	/* checks whether or not a particular feature has permissions to operate.
	returns NErr_True or NErr_False.  see features.h for known GUIDs */
	int GetPermission(GUID feature) { return Application_GetPermission(feature); }

		/* checks whether or not a particular feature is available.
		This only includes some features that might be absent based on OS version, hardware support, or third party dependencies
		It's meant for code that is otherwise unable to easily check directly or via other methods (e.g. WASABI2_API_SVC->GetService) 
	returns NErr_True or NErr_False.  see features.h for known GUIDs */
	int GetFeature(GUID feature) { return Application_GetFeature(feature); }

	/* used by a component to set features that are available.  See notes above for GetFeature 
	   for thread-safety, you should only call this during your RegisterServices() function 
		 (or during application init if you are hosting Wasabi) */
	void SetFeature(GUID feature) { Application_SetFeature(feature); }

	unsigned int GetBuildNumber() { return Application_GetBuildNumber(); }

	int GetVersionString(nx_string_t *version) { return Application_GetVersionString(version); }
	int GetProductShortName(nx_string_t *name) { return Application_GetProductShortName(name); }
	int GetDeviceID(nx_string_t *value) { return Application_GetDeviceID(value); }
	enum
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual const char * Application_GetUserAgent()=0;
	virtual int  Application_GetDataPath(nx_uri_t *path)=0;
	virtual int  Application_GetPermission(GUID feature)=0;
	virtual int  Application_GetFeature(GUID feature)=0;
	virtual void  Application_SetFeature(GUID feature)=0;
	virtual unsigned int  Application_GetBuildNumber() { return 0; }
	virtual int  Application_GetVersionString(nx_string_t *version)=0;
	virtual int  Application_GetProductShortName(nx_string_t *name)=0;
	virtual int  Application_GetDeviceID(nx_string_t *value)=0;
};

