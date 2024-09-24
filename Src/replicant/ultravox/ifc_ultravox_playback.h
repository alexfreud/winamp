#pragma once
#include "jnetlib/jnetlib_defines.h"
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "player/svc_output.h"
#include "player/ifc_player.h"
#include "http/ifc_http.h"
#include "ultravox/ifc_ultravox_reader.h"
#include "player/ifc_playback_parameters.h"

class ifc_ultravox_playback: public Wasabi2::Dispatchable
{
protected:
	ifc_ultravox_playback() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_ultravox_playback() {}
public:
	enum
	{
		DISPATCHABLE_VERSION,
	};

	int Run(ifc_http *http_parent, ifc_player *player, ifc_ultravox_reader *reader) { return UltravoxPlayback_Run(http_parent, player, reader); }

protected:
	virtual int WASABICALL UltravoxPlayback_Run(ifc_http *http_parent, ifc_player *player, ifc_ultravox_reader *reader)=0;
};
