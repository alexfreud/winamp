#pragma once
#include "foundation/dispatch.h"
#include "audio/parameters.h" 
#include "audio/ifc_audio_decoder_callback.h"
#include "audio/ifc_audio_decoder_pull.h"
#include "nx/nxuri.h"


/* if you return NErr_TryAgain, you will be called again with pass=1 after all other services get a chance at the file */

// DA3BB978-4A85-409F-B67C-10E3E1CF73CB
static const GUID decode_service_type_guid =
{ 0xDA3BB978, 0x4A85, 0x409F, { 0xB6, 0x7C, 0x10, 0xE3, 0xE1, 0xCF, 0x73, 0xCB } };

class svc_decode : public Wasabi2::Dispatchable
{
protected:
	svc_decode() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_decode() {}
public:
	static GUID GetServiceType() { return decode_service_type_guid; }

	/*
	* return values for the CreateAudioDecoder family of functions
	* * NErr_Success: Decoder successfully created
	* * NErr_False: File type not supported by this decoder
	* * NErr_UnsupportedInterface: File type is supported by this decoder, but not the particular style (callback, pull, packet).
	* * Any other code is interpreted as an error and enumeration will stop!
	*/

	/* Flags are defined in ifc_audio_decoder_callback */
	int CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags) { return DecodeService_CreateAudioDecoder_Callback(decoder, filename, parameters, flags); }
	int CreateAudioDecoder_Pull(ifc_audio_decoder_pull **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags) { return DecodeService_CreateAudioDecoder_Pull(decoder, filename, parameters, flags); }

	int CreateAudioDecoder_Callback(unsigned int pass, ifc_audio_decoder_callback **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags)
	{
		if (dispatchable_version == 0)
		{
			if (pass == 0)
				return DecodeService_CreateAudioDecoder_Callback(decoder, filename, parameters, flags); 
			else
				return NErr_False;
		}
		else
			return DecodeService_CreateAudioDecoder_Callback(pass, decoder, filename, parameters, flags); 
	}

	int CreateAudioDecoder_Pull(unsigned int pass, ifc_audio_decoder_pull **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags)
	{
		if (dispatchable_version == 0)
		{
			if (pass == 0)
				return DecodeService_CreateAudioDecoder_Pull(decoder, filename, parameters, flags); 
			else
				return NErr_False;
		}
		else
			return DecodeService_CreateAudioDecoder_Pull(pass, decoder, filename, parameters, flags); 
	}

	enum
	{
		/* Additional flags are defined in the specific decoder interface (e.g. ifc_audio_decoder_pull.h) 
		They should start at 0x80000001 to ensure uniqueness */
		FLAG_NO_METADATA= (1 << 0), // tells the decoder that it doesn't need to parse metadata
		FLAG_VALIDATION = (1 << 1), // turns on codec/file-format specific extra validation checks.  for most implementations this means turning on CRC checking
	};

	enum
	{
		DISPATCHABLE_VERSION=1,
	};
private:
	/* these two no longer have to be implemented */
	virtual int WASABICALL DecodeService_CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags) { return DecodeService_CreateAudioDecoder_Callback(0, decoder, filename, parameters, flags); }
	virtual int WASABICALL DecodeService_CreateAudioDecoder_Pull(ifc_audio_decoder_pull **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags) { return DecodeService_CreateAudioDecoder_Pull(0, decoder, filename, parameters, flags); }

	virtual int WASABICALL DecodeService_CreateAudioDecoder_Callback(unsigned int pass, ifc_audio_decoder_callback **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags) { return NErr_NotImplemented; }
	virtual int WASABICALL DecodeService_CreateAudioDecoder_Pull(unsigned int pass, ifc_audio_decoder_pull **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags) { return NErr_NotImplemented; }
};
