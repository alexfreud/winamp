#ifndef NULLSOFT_WINAMP_VIDEOCONFIGGROUP_H
#define NULLSOFT_WINAMP_VIDEOCONFIGGROUP_H

#include "main.h"
#include "../Agave/Config/ifc_configgroup.h"

// {2135E318-6919-4bcf-99D2-62BE3FCA8FA6}
static const GUID videoConfigGroupGUID = 
{ 0x2135e318, 0x6919, 0x4bcf, { 0x99, 0xd2, 0x62, 0xbe, 0x3f, 0xca, 0x8f, 0xa6 } };

class VideoConfigGroup : public ifc_configgroup
{
public:
	ifc_configitem *GetItem(const wchar_t *name);
	GUID GetGUID() { return videoConfigGroupGUID; }

protected:
	RECVS_DISPATCH;
};

extern VideoConfigGroup videoConfigGroup;

#endif