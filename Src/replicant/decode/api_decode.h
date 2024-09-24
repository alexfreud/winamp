#pragma once
#include "foundation/dispatch.h"
#include "audio/ifc_audio_decoder_callback.h"
#include "audio/parameters.h"
#include "service/types.h"
#include "nx/nxuri.h"

// {AA4404BC-69E7-4898-9296-420F774C3331}
static const GUID decode_api_service_guid = 
{ 0xaa4404bc, 0x69e7, 0x4898, { 0x92, 0x96, 0x42, 0xf, 0x77, 0x4c, 0x33, 0x31 } };

/* benski> some of this is TODO as of 25-Jan-2012
We want to have several ways to create a decoder
1) Simple method.  Create a decoder of the desired style (callback, pull, packet).  api_decode will convert between styles if necessary
2) Constraint method: Non-zero values for nsaudio::Parameters members are treated as requirements and api_decode will convert.  Flags might indicate that sample rate or channels is a "maximum" rather than a strict requirement
3) Multiple nsaudio::Parameters values.  Used if you are a little more flexible in the data format, e.g. if you can handle 16bit audio or floating point
*/
class api_decode : public Wasabi2::Dispatchable
{
protected:
	api_decode() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~api_decode() {}
public:
	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return decode_api_service_guid; }

	int CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags) { return DecodeAPI_CreateAudioDecoder_Callback(decoder, filename, parameters, flags); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL DecodeAPI_CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags)=0;
};
