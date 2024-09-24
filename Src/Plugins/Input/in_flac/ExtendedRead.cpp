/*
** Copyright (C) 2007-2011 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
** Author: Ben Allison benski@winamp.com
** Created: March 1, 2007
**
*/

#include "main.h"
#include <FLAC/all.h>
#include "StreamFileWin32.h"
#include "QuickBuf.h"
#include <bfc/platform/types.h>
#include <assert.h>
#include "FLACFileCallbacks.h"
#include "nswasabi/ReferenceCounted.h"

struct ExtendedRead
{
	int bps, channels, samplerate, truebps;
	uint64_t samples;
	QuickBuf output;
	FLAC__StreamDecoder *decoder;
	size_t used;
	FLACClientData client_data;
};

static void OnError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	//client_data=client_data; // dummy line so i can set a breakpoint
}

static void OnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	ExtendedRead *ext = FLAC_GetObject<ExtendedRead>(client_data);

	switch(metadata->type)
	{
	case FLAC__METADATA_TYPE_STREAMINFO:
		{
			ext->truebps=metadata->data.stream_info.bits_per_sample;
			ext->bps = (ext->truebps +7) & (~7);
			ext->channels=metadata->data.stream_info.channels;
			ext->samplerate=metadata->data.stream_info.sample_rate;
			ext->samples=metadata->data.stream_info.total_samples;
		}
		break;
	}
}

static FLAC__StreamDecoderWriteStatus OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	ExtendedRead *ext = FLAC_GetObject<ExtendedRead>(client_data);

	int byteLength = (ext->bps/8) * ext->channels * frame->header.blocksize;
	ext->output.Reserve(byteLength);
	InterleaveAndTruncate(buffer, ext->output, ext->bps, ext->channels, frame->header.blocksize);
	ext->used = byteLength;
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

extern "C"
{
	__declspec( dllexport ) intptr_t winampGetExtendedRead_openW(const wchar_t *fn, int *size, int *bps, int *nch, int *srate)
	{
		nx_file_t file;
		ReferenceCountedNXString filename_nx;
		ReferenceCountedNXURI filename_uri;
		NXStringCreateWithUTF16(&filename_nx, fn);
		NXURICreateWithNXString(&filename_uri, filename_nx);

		int ret = NXFileOpenFile(&file, filename_uri, nx_file_FILE_read_binary);
		if (ret != NErr_Success)
			return 0;

		ExtendedRead * e = (ExtendedRead *)calloc(sizeof(ExtendedRead), 1);
		e->decoder = FLAC__stream_decoder_new();
		if (e->decoder == 0)
		{
			NXFileRelease(file);
			free(e);
			return 0;
		}

		e->client_data.SetFile(file);
		e->client_data.SetObject(e);

		FLAC__stream_decoder_set_md5_checking(e->decoder, true);
		if(FLAC__stream_decoder_init_stream(
			e->decoder,
			FLAC_NXFile_Read,
			FLAC_NXFile_Seek,
			FLAC_NXFile_Tell,
			FLAC_NXFile_Length,
			FLAC_NXFile_EOF,  
			OnAudio,
			OnMetadata,  
			OnError,
			&e->client_data 
			) != FLAC__STREAM_DECODER_INIT_STATUS_OK) 
		{
			FLAC__stream_decoder_delete(e->decoder);
			NXFileRelease(file);
			free(e);
			return 0;	
		}

		FLAC__stream_decoder_process_until_end_of_metadata(e->decoder);
		*bps = e->truebps;
		*nch = e->channels;
		*srate = e->samplerate;
		*size = (int)(e->samples * (e->bps/8) * e->channels);
		return (intptr_t) e;
	}

	__declspec( dllexport ) intptr_t winampGetExtendedRead_getData(intptr_t handle, char *dest, size_t len, int *killswitch)
	{
		ExtendedRead *ext = (ExtendedRead *)handle;

		while(!ext->used) // loop until we get some data
		{
			if (FLAC__stream_decoder_process_single(ext->decoder) == 0)
				break; // break out if there's an error

			FLAC__StreamDecoderState FLACstate = FLAC__stream_decoder_get_state(ext->decoder);
			if (FLACstate == FLAC__STREAM_DECODER_END_OF_STREAM) // break out if we hit EOF
				break;
		}

		if (ext->used)
		{
			size_t toCopy = min(len, ext->used);
			memcpy(dest, ext->output, toCopy);
			if (toCopy < ext->used)
				ext->output.Move(toCopy);
			ext->used-=toCopy;
			return toCopy;
		}

		return 0;
	}

	__declspec( dllexport ) void winampGetExtendedRead_close(intptr_t handle)
	{
		ExtendedRead *ext = (ExtendedRead *)handle;

		ext->output.Free();
		FLAC__stream_decoder_finish(ext->decoder);
		FLAC__stream_decoder_delete(ext->decoder);
		NXFileRelease(ext->client_data.GetFile());
		free(ext);
	}
}