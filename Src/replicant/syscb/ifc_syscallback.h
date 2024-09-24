#pragma once
#include "foundation/dispatch.h"
#include "foundation/types.h"

// ----------------------------------------------------------------------------

class ifc_sysCallback : public Wasabi2::Dispatchable
{
protected:
	ifc_sysCallback() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_sysCallback() {}
public:

public:
	GUID GetEventType() { return SysCallback_GetEventType(); }
	int Notify(int msg, intptr_t param1 = 0, intptr_t param2 = 0) { return SysCallback_Notify(msg, param1, param2); }

	enum 
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual GUID WASABICALL SysCallback_GetEventType()=0;
	virtual int WASABICALL SysCallback_Notify(int msg, intptr_t param1, intptr_t param2)=0;
};

