#include "HTTPPlaybackService.h"
#include "player/ifc_player.h"
#include "player/ifc_playback.h"
#include "HTTPPlayback.h"
#include "nx/nxpath.h"
#include "nswasabi/ReferenceCounted.h"

int HTTPPlaybackService::PlaybackService_CreatePlayback(unsigned int pass, nx_uri_t filename, ifc_player *player, ifc_playback **out_playback_object)
{
	if (NXPathProtocol(filename, "http") == NErr_Success)
	{
		HTTPPlayback *http_playback = new ReferenceCounted<HTTPPlayback>;
		if (!http_playback)
			return NErr_OutOfMemory;

		int ret = http_playback->Initialize(filename, player);
		if (ret != NErr_Success)
		{
			http_playback->ifc_playback::Release();
			return ret;
		}

		*out_playback_object  = http_playback;
		return NErr_Success;
	}
	return NErr_False;
}

