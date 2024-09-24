#ifndef NULLSOFT_WINAMP_INTERNETCONFIGGROUP_H
#define NULLSOFT_WINAMP_INTERNETCONFIGGROUP_H

#include "main.h"
#include "../Agave/Config/ifc_configgroup.h"

// {C0A565DC-0CFE-405a-A27C-468B0C8A3A5C}
static const GUID internetConfigGroupGUID = 
{ 0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c } };

class InternetConfigGroup : public ifc_configgroup
{
public:
	ifc_configitem *GetItem(const wchar_t *name);
	GUID GetGUID() { return internetConfigGroupGUID; }

protected:
	RECVS_DISPATCH;
};

extern InternetConfigGroup internetConfigGroup;
#endif