//
//  MP4Decoder.h
//
//  Created by Ben Allison on 1/17/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//
#pragma once

#include "file/svc_filedecode.h"
#include "nswasabi/ServiceName.h"

// {7EDB2571-AA99-4DF0-8247-A7846B338B04}
static const GUID mp4_file_decoder_guid  = 
{ 0x7edb2571, 0xaa99, 0x4df0, { 0x82, 0x47, 0xa7, 0x84, 0x6b, 0x33, 0x8b, 0x4 } };

class MP4Decoder : public svc_filedecode
{
public:
	WASABI_SERVICE_NAME("MP4 File Decoder");
	WASABI_SERVICE_GUID(mp4_file_decoder_guid);
	
private:
	ns_error_t WASABICALL FileDecodeService_CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, nsaudio::Parameters *parameters, int flags);
};