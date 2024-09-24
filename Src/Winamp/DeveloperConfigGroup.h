#pragma once

#include "main.h"
#include "../Agave/Config/ifc_configgroup.h"


// {113D413A-5D1F-4f4c-8AB7-5BDED46033A4}
static const GUID developerConfigGroupGUID= 
{ 0x113d413a, 0x5d1f, 0x4f4c, { 0x8a, 0xb7, 0x5b, 0xde, 0xd4, 0x60, 0x33, 0xa4 } };


class DeveloperConfigGroup : public ifc_configgroup
{
public:
	ifc_configitem *GetItem(const wchar_t *name);
	GUID GetGUID() { return developerConfigGroupGUID; }

protected:
	RECVS_DISPATCH;
};

extern DeveloperConfigGroup developerConfigGroup;
