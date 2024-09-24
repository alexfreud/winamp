#ifndef NULLSOFT_AGAVE_API_CONFIG_H
#define NULLSOFT_AGAVE_API_CONFIG_H

#include "../../bfc/dispatch.h"
#include "ifc_configgroup.h"

enum
{
	CONFIG_SUCCESS = 0,
	CONFIG_FAILURE = 1,
	CONFIG_GROUPNOTFOUND = 2,
	CONFIG_ITEMNOTFOUND = 3,
};

class api_config : public Dispatchable
{
protected:
	api_config() {}
	~api_config() {}
public:
	ifc_configgroup *GetGroup(GUID groupGUID);
	void RegisterGroup(ifc_configgroup *newGroup);

	/* Shortcut methods */
	bool GetBool(GUID groupGUID, const wchar_t *configItem, bool defaultValue);
	uintptr_t GetUnsigned(GUID groupGUID, const wchar_t *configItem, uintptr_t defaultValue);
  intptr_t GetInt(GUID groupGUID, const wchar_t *configItem, intptr_t defaultValue);
	float GetFloat(GUID groupGUID, const wchar_t *configItem, float defaultValue);
	const wchar_t *GetString(GUID groupGUID, const wchar_t *configItem, const wchar_t *defaultValue);
	ifc_configitem *GetItem(GUID groupGUID, const wchar_t *configItem);
public:
	DISPATCH_CODES
	{
		API_CONFIG_GETGROUP = 10,
		API_CONFIG_REGISTERGROUP = 20,
	};
};

inline ifc_configgroup *api_config::GetGroup(GUID groupGUID)
{
	return _call(API_CONFIG_GETGROUP, (ifc_configgroup *)0, groupGUID);
}

inline void api_config::RegisterGroup(ifc_configgroup *newGroup)
{
	_voidcall(API_CONFIG_REGISTERGROUP, newGroup);
}

inline bool api_config::GetBool(GUID groupGUID, const wchar_t *configItem, bool defaultValue)
{
	ifc_configgroup *group = GetGroup(groupGUID);
	if (group)
	{
		ifc_configitem *item = group->GetItem(configItem);
		if (item)
			return item->GetBool();
	}
	return defaultValue;
}

inline uintptr_t api_config::GetUnsigned(GUID groupGUID, const wchar_t *configItem, uintptr_t defaultValue)
{
	ifc_configgroup *group = GetGroup(groupGUID);
	if (group)
	{
		ifc_configitem *item = group->GetItem(configItem);
		if (item)
			return item->GetUnsigned();
	}
	return defaultValue;
}

inline intptr_t api_config::GetInt(GUID groupGUID, const wchar_t *configItem, intptr_t defaultValue)
{
	ifc_configgroup *group = GetGroup(groupGUID);
	if (group)
	{
		ifc_configitem *item = group->GetItem(configItem);
		if (item)
			return item->GetInt();
	}
	return defaultValue;
}

inline float api_config::GetFloat(GUID groupGUID, const wchar_t *configItem, float defaultValue)
{
	ifc_configgroup *group = GetGroup(groupGUID);
	if (group)
	{
		ifc_configitem *item = group->GetItem(configItem);
		if (item)
			return item->GetFloat();
	}
	return defaultValue;
}

inline const wchar_t *api_config::GetString(GUID groupGUID, const wchar_t *configItem, const wchar_t *defaultValue)
{
	ifc_configgroup *group = GetGroup(groupGUID);
	if (group)
	{
		ifc_configitem *item = group->GetItem(configItem);
		if (item)
			return item->GetString();
	}
	return defaultValue;
}

inline ifc_configitem *api_config::GetItem(GUID groupGUID, const wchar_t *configItem)
{
	ifc_configgroup *group = GetGroup(groupGUID);
	if (group)
	{
		return group->GetItem(configItem);
	}
	return 0;
}

// {AEFBF8BE-E0AA-4318-8CC1-4353410B64DC}
static const GUID AgaveConfigGUID = 
{ 0xaefbf8be, 0xe0aa, 0x4318, { 0x8c, 0xc1, 0x43, 0x53, 0x41, 0xb, 0x64, 0xdc } };

#endif
