#pragma once
#include "foundation/dispatch.h"
#include "nx/nxuri.h"
#include "metadata/ifc_metadata.h"

class ifc_audio_decoder_packet : public Wasabi2::Dispatchable
{
protected:
	ifc_audio_decoder_packet() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_audio_decoder_packet() {}
public:

	int GetMetadata(ifc_metadata **metadata) { return AudioDecoderPacket_GetMetadata(metadata); }
	int Decode(void **out_packet, size_t *frames_available) { return AudioDecoderPacket_Decode(out_packet, frames_available); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL AudioDecoderPacket_GetMetadata(ifc_metadata **metadata)=0;
	virtual int WASABICALL AudioDecoderPacket_Decode(void **out_packet, size_t *frames_available)=0;
};
