#ifndef NULLSOFT_CONFIG_CONFIG_H
#define NULLSOFT_CONFIG_CONFIG_H

#include "../Agave/Config/api_config.h"
#include <map>

class Config : public api_config
{
	
public:
	static const char *getServiceName() { return "Agave Config API"; }
	static const GUID getServiceGuid() { return AgaveConfigGUID; }

	ifc_configgroup *GetGroup(GUID groupGUID);
	void RegisterGroup(ifc_configgroup *newGroup);

	typedef std::map<GUID, ifc_configgroup *> GroupList;
	GroupList groups;
protected:
	RECVS_DISPATCH;
};

extern Config config;

#endif