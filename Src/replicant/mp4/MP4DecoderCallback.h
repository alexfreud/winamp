//
//  FLACDecoderPull.h
//  flac
//
//  Created by Ben Allison on 1/12/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//
#pragma once
#include "audio/ifc_audio_decoder_callback.h"
#include "mp4/ifc_mp4audiodecoder.h"
#include "mp4.h"
#include "audio/parameters.h"
#include "MP4FileObject.h"
#include "nswasabi/MetadataChain.h"

class MP4DecoderCallback : public ifc_audio_decoder_callback
{
public:
	MP4DecoderCallback();
	~MP4DecoderCallback();
	int Initialize(MP4FileHandle mp4_file, ifc_mp4audiodecoder *decoder, int flags, nsaudio::Parameters *parameters, MetadataChain<MP4FileObject> *mp4_file_object);
	
private:
	int WASABICALL AudioDecoderCallback_GetMetadata(ifc_metadata **metadata);
	int WASABICALL AudioDecoderCallback_Decode(ifc_audio_decoder_callback::callback *callback);
	int WASABICALL AudioDecoderCallback_DecodeStep(ifc_audio_decoder_callback::callback *callback);
	int WASABICALL AudioDecoderCallback_GetFrameSize(size_t *frame_size);
	MP4FileHandle mp4_file;
	ifc_mp4audiodecoder *audio_decoder;
	MetadataChain<MP4FileObject> *mp4_file_object;
	int flags;
	size_t pregap;
	size_t frame_size;
	unsigned int channels;
};