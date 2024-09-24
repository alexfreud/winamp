#pragma once
#include "jnetlib/jnetlib_defines.h"
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "player/svc_output.h"
#include "player/ifc_player.h"
#include "http/ifc_http.h"
#include "icy/ifc_icy_reader.h"
#include "player/ifc_playback_parameters.h"

class ifc_icy_playback: public Wasabi2::Dispatchable
{
protected:
	ifc_icy_playback() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_icy_playback() {}
public:
	enum
	{
		DISPATCHABLE_VERSION,
	};

	int Run(ifc_http *http_parent, ifc_player *player, ifc_icy_reader *reader) { return ICYPlayback_Run(http_parent,  player, reader); }

protected:
	virtual int WASABICALL ICYPlayback_Run(ifc_http *http_parent, ifc_player *player, ifc_icy_reader *reader)=0;
};
