#pragma once

#include "foundation/dispatch.h"
#include "foundation/types.h"
#include "service/types.h"

class ifc_sysCallback;

// {57B7A1B6-700E-44ff-9CB0-70B92BAF3959}
static const GUID syscbApiServiceGuid = 
{ 0x57b7a1b6, 0x700e, 0x44ff, { 0x9c, 0xb0, 0x70, 0xb9, 0x2b, 0xaf, 0x39, 0x59 } };

// ----------------------------------------------------------------------------

class NOVTABLE api_syscb : public Wasabi2::Dispatchable
{
protected:
	api_syscb() : Dispatchable(DISPATCHABLE_VERSION) {}
	~api_syscb() {}
public:
	static const GUID GetServiceGUID() { return syscbApiServiceGuid; }
	static const GUID GetServiceType() { return SVC_TYPE_UNIQUE; }

	int RegisterCallback(ifc_sysCallback *cb) { return SysCallbacks_RegisterCallback(cb); }
	int UnregisterCallback(ifc_sysCallback *cb) { return SysCallbacks_UnregisterCallback(cb); }
	int IssueCallback(GUID eventtype, int msg, intptr_t param1 = 0, intptr_t param2 = 0) { return SysCallbacks_IssueCallback(eventtype, msg, param1, param2); }

	/** pass eventtype == 0 to enumerate all syscallbacks 
	** call Release() on the returned ifc_sysCallback when you are done
	** although very few wasabi objects support this at this time (2 June 2008)
	**/
	ifc_sysCallback *syscb_enum(GUID eventtype, size_t n) { return SysCallbacks_Enum(eventtype, n); }

	enum 
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual int WASABICALL SysCallbacks_RegisterCallback(ifc_sysCallback *cb)=0;
	virtual int WASABICALL SysCallbacks_UnregisterCallback(ifc_sysCallback *cb)=0;
	virtual int WASABICALL SysCallbacks_IssueCallback(GUID eventtype, int msg, intptr_t param1 = 0, intptr_t param2 = 0)=0;

	/** pass eventtype == 0 to enumerate all syscallbacks 
	** call Release() on the returned ifc_sysCallback when you are done
	** although very few wasabi objects support this at this time (2 June 2008)
	**/
	virtual ifc_sysCallback *WASABICALL SysCallbacks_Enum(GUID eventtype, size_t n)=0;
  
  
};

