#pragma once
#include "foundation/dispatch.h"

// {667F8818-AECD-4017-80EE-C43E096E68C1}
static const GUID ifc_component_sync_interface_guid = 
{ 0x667f8818, 0xaecd, 0x4017, { 0x80, 0xee, 0xc4, 0x3e, 0x9, 0x6e, 0x68, 0xc1 } };

class ifc_component_sync : public Wasabi2::Dispatchable
{
protected:
	ifc_component_sync() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_component_sync() {}
public:
	static GUID GetInterfaceGUID() { return ifc_component_sync_interface_guid; }
	int Wait(size_t count) { return ComponentSync_Wait(count); }
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL ComponentSync_Wait(size_t count)=0;
};
