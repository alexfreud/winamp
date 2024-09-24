#pragma once
#include "jnetlib/jnetlib_defines.h"
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "player/svc_output.h"
#include "player/ifc_player.h"
#include "http/ifc_http.h"
#include "player/ifc_playback_parameters.h"

class ifc_http_demuxer: public Wasabi2::Dispatchable
{
protected:
	ifc_http_demuxer() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_http_demuxer() {}
public:
	enum
	{
		DISPATCHABLE_VERSION,
	};

	int Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters) { return HTTPDemuxer_Run(http_parent, player, secondary_parameters); }

protected:
	virtual int WASABICALL HTTPDemuxer_Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters)=0;
};
