#pragma once

#include "syscb/api_syscb.h"
#include <vector>
#include "nu/AutoLock.h"
#include "service/types.h"
#include "nx/nxstring.h"
#include "nswasabi/ServiceName.h"

class SysCallbacks : public api_syscb
{
public:
	WASABI_SERVICE_NAME("System Callbacks API");

public:
	SysCallbacks();
    int WASABICALL SysCallbacks_RegisterCallback(ifc_sysCallback *cb);
    int WASABICALL SysCallbacks_UnregisterCallback(ifc_sysCallback *cb);
    int WASABICALL SysCallbacks_IssueCallback(GUID eventtype, int msg, intptr_t param1 = 0, intptr_t param2 = 0);
		ifc_sysCallback *WASABICALL SysCallbacks_Enum(GUID eventtype, size_t n); 

private:
	nu::LockGuard callbackGuard;
	std::vector<ifc_sysCallback*> callbacks;
	std::vector<ifc_sysCallback*> deleteMeAfterCallbacks;
	bool inCallback;
	volatile int reentry;
};

extern SysCallbacks system_callbacks;

