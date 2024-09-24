#pragma once
#include "foundation/dispatch.h"
#include "audio/parameters.h" 
#include "audio/ifc_audio_decoder_callback.h"
#include "audio/ifc_audio_decoder_pull.h"
#include "nx/nxuri.h"
#include "nx/nxfile.h"
#include "foundation/error.h"
#include "decode/svc_decode.h"

// {5F8FD642-9346-4400-803F-A20F342916FD}
static const GUID filedecode_service_type_guid = 
{ 0x5f8fd642, 0x9346, 0x4400, { 0x80, 0x3f, 0xa2, 0xf, 0x34, 0x29, 0x16, 0xfd } };

class NOVTABLE svc_filedecode : public Wasabi2::Dispatchable
{
protected:
	svc_filedecode() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_filedecode() {}

public:
	static GUID GetServiceType() { return filedecode_service_type_guid; }
	ns_error_t CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, nsaudio::Parameters *parameters, int flags) { return FileDecodeService_CreateAudioDecoder_Callback(decoder, filename, file, parent_metadata, parameters, flags); }
	
enum
	{
		DISPATCHABLE_VERSION=0,
	};
protected:
	virtual ns_error_t WASABICALL FileDecodeService_CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, nsaudio::Parameters *parameters, int flags)=0;
};

