/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "DeveloperConfigGroup.h"

#include "../Agave/Config/ifc_configitem.h"
#include "WinampAttributes.h"

class MaskBoolConfigItem : public ifc_configitem
{
public:
	MaskBoolConfigItem(int _mask) : mask(_mask) {}
	bool GetBool()
	{
		return !!(config_no_visseh&mask);
	}
protected:
	RECVS_DISPATCH;

private:
	int mask;
};

#define CBCLASS MaskBoolConfigItem
START_DISPATCH;
CB(IFC_CONFIGITEM_GETBOOL, GetBool)
END_DISPATCH;
#undef CBCLASS

static MaskBoolConfigItem sehVisItem(1), sehDSPItem(2), sehGenItem(4), sehIEItem(8);

ifc_configitem *DeveloperConfigGroup::GetItem(const wchar_t *name)
{
	if (!wcscmp(name, L"no_visseh"))
		return &sehVisItem;
	else if (!wcscmp(name, L"no_dspseh"))
		return &sehDSPItem;
	else if (!wcscmp(name, L"no_genseh"))
		return &sehGenItem;
	else if (!wcscmp(name, L"no_ieseh"))
		return &sehIEItem;

	return 0;
}


#define CBCLASS DeveloperConfigGroup
START_DISPATCH;
CB(IFC_CONFIGGROUP_GETITEM, GetItem)
CB(IFC_CONFIGGROUP_GETGUID, GetGUID)
END_DISPATCH;
#undef CBCLASS