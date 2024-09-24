#include "AudioCoderFlac.h"
#include <bfc/platform/types.h>
#include <FLAC/metadata.h>

AudioCoderFlac::AudioCoderFlac(unsigned int nch, unsigned int bps, unsigned int samplerate, unsigned int compression)
{
	/* initialize stuff first so we can clean up safely if things go wrong */
	finished = false;
	finishedBytes = 0;
	padding = 0;
	encoder = 0;
	win32State.bytesWritten = 0;
	win32State.handle = INVALID_HANDLE_VALUE;
	tempFile[0]=0;

	wchar_t tempPath[MAX_PATH-14] = {0};
	GetTempPath(MAX_PATH-14, tempPath);
	GetTempFileName(tempPath, L"wfl", 0, tempFile);
	win32State.handle = CreateFile(tempFile, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);

	if (win32State.handle != INVALID_HANDLE_VALUE)
	{
		this->nch = nch;
		this->bps = bps;
		encoder = FLAC__stream_encoder_new();

		if (!encoder)
			return;

		// set stream info
		if (!FLAC__stream_encoder_set_channels(encoder, nch)
		    || !FLAC__stream_encoder_set_bits_per_sample(encoder, bps)
		    || !FLAC__stream_encoder_set_sample_rate(encoder, samplerate)
		    || !FLAC__stream_encoder_set_total_samples_estimate(encoder, 0)
		    || !FLAC__stream_encoder_set_compression_level(encoder, compression)
		    || !FLAC__stream_encoder_set_blocksize(encoder, 0))
		{
			FLAC__stream_encoder_delete(encoder);
			encoder=0;
			return;
		}
		// TODO: set any more config stuff

		// TODO: seektable?
		//FLAC__StreamMetadata *seektable = FLAC__metadata_object_new(FLAC__METADATA_TYPE_SEEKTABLE);

		padding = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING);
		if (padding)
		{
			padding->length = 16384; // TODO: configurable padding size
			if (!FLAC__stream_encoder_set_metadata(encoder, &padding, 1))
			{
				FLAC__stream_encoder_delete(encoder);
				encoder=0;
				return;
			}
		}

		if (FLAC__stream_encoder_init_stream(encoder, Win32_Write, Win32_Seek, Win32_Tell, NULL, &win32State) != FLAC__STREAM_ENCODER_INIT_STATUS_OK)
		{
			FLAC__stream_encoder_delete(encoder);
			encoder=0;
			return;
		}
	}
}

bool AudioCoderFlac::OK()
{
	if (!encoder)
		return false;

	return FLAC__stream_encoder_get_state(encoder) == FLAC__STREAM_ENCODER_OK;
}

AudioCoderFlac::~AudioCoderFlac()
{
	if (encoder)
		FLAC__stream_encoder_delete(encoder);

	if (padding)
		FLAC__metadata_object_delete(padding);

	if (win32State.handle != INVALID_HANDLE_VALUE)
		CloseHandle(win32State.handle);

}

static void Copy8(FLAC__int32 *buffer, void *inputData, int numSamples)
{
	uint8_t *in = (uint8_t *)inputData;

	for (int i=0;i<numSamples;i++)
	{
		buffer[i] = (FLAC__int32)in[i];
	}
}

static void Copy16(FLAC__int32 *buffer, void *inputData, int numSamples)
{
	int16_t *in = (int16_t *)inputData;

	for (int i=0;i<numSamples;i++)
	{
		buffer[i] = (FLAC__int32)in[i];
	}
}

static void Copy24(FLAC__int32 *buffer, void *inputData, int numSamples)
{
	uint8_t *in = (uint8_t *)inputData;

	for (int i=0;i<numSamples;i++)
	{
		FLAC__int32 val = (((FLAC__int32)in[0]) << 0);
		val = val | (((FLAC__int32)in[1]) << 8);
		val = val | (((FLAC__int32)in[2]) << 16);

		buffer[i] = (FLAC__int32)val;
		in+=3;
	}
}

static void Copy32(FLAC__int32 *buffer, void *inputData, int numSamples)
{
	int32_t *in = (int32_t *)inputData;

	for (int i=0;i<numSamples;i++)
	{
		buffer[i] = (FLAC__int32)in[i];
	}
}

int AudioCoderFlac::Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail)
{
	FLAC__int32 buffer[65536];


	FLAC__uint64 startBytes = win32State.bytesWritten;

	if (!in_avail)
	{
		if (finished)
		{
			int ret = (int)finishedBytes;
			finishedBytes = 0;
			return ret;
		}
		return 0;
	}


	int numSamples = in_avail/(bps/8);

	if (numSamples>65536)
		numSamples = 65536;

	switch (bps)
	{
	case 8:
		Copy8(buffer, in, numSamples);
		break;
	case 16:
		Copy16(buffer, in, numSamples);
		break;
	case 24:
		Copy24(buffer, in, numSamples);
		break;
	case 32:
		Copy32(buffer, in, numSamples);
		break;
	}

	FLAC__bool result = FLAC__stream_encoder_process_interleaved(encoder, buffer, numSamples/nch);

	if (result)
	{
		*in_used = numSamples*(bps/8);
		return (int)(win32State.bytesWritten - startBytes);
	}

	return 0;
}

void AudioCoderFlac::PrepareToFinish()
{
	FLAC__uint64 startBytes = win32State.bytesWritten;
	FLAC__stream_encoder_finish(encoder);
	finishedBytes =  win32State.bytesWritten - startBytes;
}

void AudioCoderFlac::Finish(const wchar_t *destination)
{
	if (win32State.handle != INVALID_HANDLE_VALUE)
		CloseHandle(win32State.handle);

	win32State.handle = INVALID_HANDLE_VALUE;
	if (!MoveFile(tempFile, destination))
	{
		if (CopyFile(tempFile, destination, FALSE))
			DeleteFile(tempFile);
	}
}