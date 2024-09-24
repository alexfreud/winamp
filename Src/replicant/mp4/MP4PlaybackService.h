#pragma once
#include "file/svc_fileplayback.h"
#include "nx/nxstring.h"
#include "nswasabi/ServiceName.h"

// {AA80A650-FC61-4011-BB17-E4EBA5C185D9}
static const GUID mp4_file_playback_guid = 
{ 0xaa80a650, 0xfc61, 0x4011, { 0xbb, 0x17, 0xe4, 0xeb, 0xa5, 0xc1, 0x85, 0xd9 } };


class MP4PlaybackService : public svc_fileplayback
{
public:
	WASABI_SERVICE_NAME("MP4 File Playback Service");
	WASABI_SERVICE_GUID(mp4_file_playback_guid);

	ns_error_t WASABICALL FilePlaybackService_CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent);
};
