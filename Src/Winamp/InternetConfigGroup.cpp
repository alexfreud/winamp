/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "InternetConfigGroup.h"

#include "../Agave/Config/ifc_configitem.h"
#include "WinampAttributes.h"
#include "../nu/ns_wc.h"

class ProxyConfigItem : public ifc_configitem
{
public:
	const wchar_t *GetString() 
	{
		static wchar_t blah[256];
		if (config_proxy[0])
			MultiByteToWideCharSZ(CP_ACP, 0, config_proxy, -1, blah, 256);
		else
			return 0;
		return blah;
	}
protected:
	RECVS_DISPATCH;
};

#define CBCLASS ProxyConfigItem
START_DISPATCH;
CB(IFC_CONFIGITEM_GETSTRING, GetString)
END_DISPATCH;
#undef CBCLASS

static ProxyConfigItem proxyConfigItem;

ifc_configitem *InternetConfigGroup::GetItem(const wchar_t *name)
{
	if (!wcscmp(name, L"proxy"))
		return &proxyConfigItem;
	else if (!wcscmp(name, L"proxy80"))
		return &config_proxy80;

	return 0;
}



#define CBCLASS InternetConfigGroup
START_DISPATCH;
CB(IFC_CONFIGGROUP_GETITEM, GetItem)
CB(IFC_CONFIGGROUP_GETGUID, GetGUID)
END_DISPATCH;
#undef CBCLASS