#include "main.h"
#include "AccessibilityConfigGroup.h"
#include "WinampAttributes.h"


ifc_configitem *AccessibilityConfigGroup::GetItem(const wchar_t *name)
{
	if (!wcscmp(name, L"modalbeep"))
		return &config_accessibility_modalbeep;
	else if (!wcscmp(name, L"modalflash"))
		return &config_accessibility_modalflash;
	return 0;
}


#define CBCLASS AccessibilityConfigGroup
START_DISPATCH;
CB(IFC_CONFIGGROUP_GETITEM, GetItem)
CB(IFC_CONFIGGROUP_GETGUID, GetGUID)
END_DISPATCH;
#undef CBCLASS