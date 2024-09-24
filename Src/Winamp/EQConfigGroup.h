#ifndef NULLSOFT_WINAMP_EQCONFIGGROUP_H
#define NULLSOFT_WINAMP_EQCONFIGGROUP_H

#include "main.h"
#include "../Agave/Config/ifc_configgroup.h"

// {72409F84-BAF1-4448-8211-D84A30A1591A}
static const GUID eqConfigGroupGUID = 
{ 0x72409f84, 0xbaf1, 0x4448, { 0x82, 0x11, 0xd8, 0x4a, 0x30, 0xa1, 0x59, 0x1a } };


class EQConfigGroup : public ifc_configgroup
{
public:
	ifc_configitem *GetItem(const wchar_t *name);
	GUID GetGUID() { return eqConfigGroupGUID; }

protected:
	RECVS_DISPATCH;
};

extern EQConfigGroup eqConfigGroup;

#endif