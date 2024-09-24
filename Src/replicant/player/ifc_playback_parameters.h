#pragma once
#include "foundation/dispatch.h"

/* ifc_output_parameters abstracts output parameters that are passed to an input plugin
  it is things that an input plugin wouldn't necessary know about
	for example, is a playback object is being used for track preview,
	it might be configured to play out of a different output device 
	and there's no way an input plugin would know that 
	*/
class NOVTABLE ifc_playback_parameters : public Wasabi2::Dispatchable
{
protected:
	ifc_playback_parameters() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_playback_parameters() {}
	
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
};
