#pragma once

#include <bfc/dispatch.h>
#include <api/service/services.h>

#include "..\..\General\gen_ml/ml.h"

// {70B27610-D1C9-4442-ABF2-763AD041E458}
static const GUID playlistGeneratorGUID = 
{ 0x70b27610, 0xd1c9, 0x4442, { 0xab, 0xf2, 0x76, 0x3a, 0xd0, 0x41, 0xe4, 0x58 } };

class api_playlist_generator : public Dispatchable
{
protected:
	api_playlist_generator() {}
	~api_playlist_generator() {}
public:
	static FOURCC getServiceType() { return WaSvc::UNIQUE; }
	static const char *getServiceName() { return "Playlist Generator Service"; }
	static GUID getServiceGuid() { return playlistGeneratorGUID; }

	int GeneratePlaylist(HWND parent, const itemRecordListW *selectedSeedRecordList);
	
	enum
	{
		API_PLAYLIST_GENERATOR_GENERATEPLAYLIST = 0,
	};

};

inline int api_playlist_generator::GeneratePlaylist(HWND parent, const itemRecordListW *selectedSeedRecordList)
{
	return _call(API_PLAYLIST_GENERATOR_GENERATEPLAYLIST, (int)DISPATCH_FAILURE, parent, selectedSeedRecordList);
}


