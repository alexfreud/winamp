#ifndef NULLSOFT_AGAVE_IFC_CONFIGGROUP_H
#define NULLSOFT_AGAVE_IFC_CONFIGGROUP_H

#include "../../bfc/dispatch.h"
#include "../../bfc/platform/types.h"
#include "../../bfc/platform/guid.h"
#include "ifc_configitem.h"

class ifc_configgroup : public Dispatchable
{
protected:
	ifc_configgroup() {}
	~ifc_configgroup() {}
public:
	ifc_configitem *GetItem(const wchar_t *name);
	GUID GetGUID();
public:
	DISPATCH_CODES
	{
		IFC_CONFIGGROUP_GETITEM = 10,
		IFC_CONFIGGROUP_GETGUID = 20,
	};
	
};

inline ifc_configitem *ifc_configgroup::GetItem(const wchar_t *name)
{
	return _call(IFC_CONFIGGROUP_GETITEM, (ifc_configitem *)0, name);
}

inline GUID ifc_configgroup::GetGUID()
{
	return _call(IFC_CONFIGGROUP_GETGUID, (GUID)INVALID_GUID);
}
#endif
