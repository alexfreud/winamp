#pragma once
#include "../replicant/foundation/dispatch.h"
#include "ogg/ogg.h"
#include "../replicant/ogg/ifc_oggaudiodecoder.h"
#include "../replicant/foundation/error.h"

// {EC953F63-DFD3-41E7-B893-92537AE88280}
static const GUID ogg_decoder_service_type_guid = 
{ 0xec953f63, 0xdfd3, 0x41e7, { 0xb8, 0x93, 0x92, 0x53, 0x7a, 0xe8, 0x82, 0x80 } };

class svc_oggdecoder : public Wasabi2::Dispatchable
{
protected:
	svc_oggdecoder() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_oggdecoder() {}
public:
	static GUID GetServiceType() { return ogg_decoder_service_type_guid; }
	ns_error_t CreateAudioDecoder(ogg_packet *packet, ifc_oggaudiodecoder **audio_decoder) { return OggDecoder_CreateAudioDecoder(packet, audio_decoder); }

	enum
	{
		DISPATCHABLE_VERSION,
	};
private:
	virtual ns_error_t WASABICALL OggDecoder_CreateAudioDecoder(ogg_packet *packet, ifc_oggaudiodecoder **audio_decoder)=0;
};
