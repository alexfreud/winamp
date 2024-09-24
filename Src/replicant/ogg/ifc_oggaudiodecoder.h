#pragma once
#include "../replicant/foundation/dispatch.h"

class ifc_oggaudiodecoder : public Wasabi2::Dispatchable
{
protected:
	ifc_oggaudiodecoder() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_oggaudiodecoder() {}
public:

	enum
	{
		DISPATCHABLE_VERSION,
		GET_BPS
	};
};