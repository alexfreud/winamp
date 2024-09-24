#pragma once
#include "application/api_application.h"
#include <set>
#include "syscb/api_syscb.h"

/* implements non-platform-specific methods of api_application.  
You can derive your Application implementation from this to ease your life */
class ApplicationBase : public api_application
{
public:
	ApplicationBase();
    ~ApplicationBase();
    
	/* call this (and check the return value) before doing your own initialization */
	int Initialize();

	/* and call this after doing your own shutdown */
	void Shutdown();

	void SetDataPath(nx_uri_t data_path);
	void SetPermission(GUID feature);
	void RemovePermission(GUID permission);
	void SetDeviceID(nx_string_t device_id);
	void EnableAllPermissions();
	void ClearPermissions();
	void NotifyPermissions(api_syscb *system_callbacks); /* pass in the syscb API to avoid a dependency */
	void DumpPermissions(); /* dumps permissions list to the log file */
protected:
	/* api_application implementation */
	int  Application_GetDataPath(nx_uri_t *path);
	int  Application_GetPermission(GUID feature);
	int  Application_GetFeature(GUID feature);
	void  Application_SetFeature(GUID feature);
	int  Application_GetDeviceID(nx_string_t *value);
	
private:
	typedef std::set<GUID> FeatureList;
	FeatureList features;
	FeatureList permissions;
	bool all_permissions_enabled; /* bypass for developer/QA testing */
	nx_uri_t data_path;
	nx_string_t device_id;
	
};
