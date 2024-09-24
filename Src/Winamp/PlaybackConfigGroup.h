#ifndef NULLSOFT_WINAMP_PLAYBACKCONFIGGROUP_H
#define NULLSOFT_WINAMP_PLAYBACKCONFIGGROUP_H

#include "main.h"
#include "../Agave/Config/ifc_configgroup.h"

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID = 
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

class PlaybackConfigGroup : public ifc_configgroup
{
public:
	ifc_configitem *GetItem(const wchar_t *name);
	GUID GetGUID() { return playbackConfigGroupGUID; }

protected:
	RECVS_DISPATCH;
};

extern PlaybackConfigGroup playbackConfigGroup;


#endif