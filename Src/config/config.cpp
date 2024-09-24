/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "config.h"

void Config::RegisterGroup(ifc_configgroup *newGroup)
{
	if (newGroup)
	{
		groups[newGroup->GetGUID()]=newGroup;
	}
}

ifc_configgroup *Config::GetGroup(GUID groupGUID)
{
	return groups[groupGUID];
}


#define CBCLASS Config
START_DISPATCH;
CB(API_CONFIG_GETGROUP, GetGroup)
VCB(API_CONFIG_REGISTERGROUP, RegisterGroup)
END_DISPATCH;
