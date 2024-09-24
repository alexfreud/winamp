#pragma once
#include "decode/svc_decode.h"
#include "nswasabi/ServiceName.h"

// {F07160ED-2820-4DA5-9CE9-857A4CD9DFA0}
static const GUID file_decoder_guid = 
{ 0xf07160ed, 0x2820, 0x4da5, { 0x9c, 0xe9, 0x85, 0x7a, 0x4c, 0xd9, 0xdf, 0xa0 } };

class FileDecoderService : public svc_decode
{
public:
	WASABI_SERVICE_NAME("File Decoder");
	WASABI_SERVICE_GUID(file_decoder_guid);
	
private:
	int WASABICALL DecodeService_CreateAudioDecoder_Callback(unsigned int pass, ifc_audio_decoder_callback **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags);
	int WASABICALL DecodeService_CreateAudioDecoder_Pull(unsigned int pass, ifc_audio_decoder_pull **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags);
};