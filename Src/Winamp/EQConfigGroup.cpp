/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "EqConfigGroup.h"
#include "WinampAttributes.h"


ifc_configitem *EQConfigGroup::GetItem(const wchar_t *name)
{
	if (!wcscmp(name, L"frequencies"))
		return &config_eq_frequencies;
	else if (!wcscmp(name, L"type"))
		return &config_eq_type;
	else if (!wcscmp(name, L"limiter"))
		return &config_eq_limiter;


	return 0;
}



#define CBCLASS EQConfigGroup
START_DISPATCH;
CB(IFC_CONFIGGROUP_GETITEM, GetItem)
CB(IFC_CONFIGGROUP_GETGUID, GetGUID)
END_DISPATCH;
#undef CBCLASS
