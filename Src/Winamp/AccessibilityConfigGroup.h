#ifndef NULLSOFT_WINAMP_ACCESSIBILITYCONFIGGROUP_H
#define NULLSOFT_WINAMP_ACCESSIBILITYCONFIGGROUP_H

//#include "main.h"
#include "../Agave/Config/ifc_configgroup.h"

// {0E2E7F4A-7C51-478f-8774-ABBCF6D5A857}
static const GUID accessibilityConfigGroupGUID = 
{ 0xe2e7f4a, 0x7c51, 0x478f, { 0x87, 0x74, 0xab, 0xbc, 0xf6, 0xd5, 0xa8, 0x57 } };


class AccessibilityConfigGroup : public ifc_configgroup
{
public:
	ifc_configitem *GetItem(const wchar_t *name);
	GUID GetGUID() { return accessibilityConfigGroupGUID; }

protected:
	RECVS_DISPATCH;
};

extern AccessibilityConfigGroup accessibilityConfigGroup;

#endif //NULLSOFT_WINAMP_ACCESSIBILITYCONFIGGROUP_H