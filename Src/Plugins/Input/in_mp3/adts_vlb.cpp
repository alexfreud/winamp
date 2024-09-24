#include "adts_vlb.h"
#include "giofile.h"
#include "in2.h"
extern In_Module mod;

ADTS_VLB::ADTS_VLB() : decoder(0), needsync(1)
{}

int ADTS_VLB::Initialize(bool forceMono, bool reverseStereo, bool allowSurround, int maxBits, bool allowRG, bool _useFloat, bool _useCRC)
{
	return 0;
}

bool ADTS_VLB::Open(ifc_mpeg_stream_reader *file)
{
	waServiceFactory *factory = mod.service->service_getServiceByGuid(obj_vlbDecoderGUID);
	if (factory)
		decoder = (obj_vlbDecoder *)factory->getInterface();

	if (decoder)
	{
		int status = decoder->Open((DataIOControl *)(CGioFile *)file);
		if (status == 0)
			return true;
	}

	return false;
}

void ADTS_VLB::Close()
{
	if (decoder)
	{
		waServiceFactory *factory = mod.service->service_getServiceByGuid(obj_vlbDecoderGUID);
		if (factory)
			factory->releaseInterface(decoder);
	}

	decoder = 0;
}

void ADTS_VLB::GetOutputParameters(size_t *numBits, int *numChannels, int *sampleRate)
{
	*sampleRate  = params.sampling_frequency;
	*numChannels = params.num_channels;
	*numBits     = 16;
}

void ADTS_VLB::CalculateFrameSize( int *frameSize )
{
	*frameSize = 576 * 2 * params.num_channels;
	if ( *frameSize > 576 * 2 * 2 )
		*frameSize = 576 * 2 * 2;
}

void ADTS_VLB::Flush(ifc_mpeg_stream_reader *file)
{
	decoder->Flush();
	decoder->Close();
	decoder->Open((DataIOControl *)(CGioFile *)file);

	needsync = 1;
}

size_t ADTS_VLB::GetCurrentBitrate()
{
	return params.bitrate / 1000;
}

size_t ADTS_VLB::GetDecoderDelay()
{
	return 0; // not really sure
}

int ADTS_VLB::Sync(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate)
{
	int status = decoder->Synchronize(&params);
	if (!status)
	{
		needsync = 0;
		return SUCCESS;
	}

	if (file->MPEGStream_EOF())
		return ENDOFFILE;

	return NEEDMOREDATA;
}

int ADTS_VLB::Decode(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate, size_t *endCut)
{
	if (*outputWritten = decoder->Read(output, outputSize))
	{
		// TODO: benski> verify that params is valid here
		*bitrate = params.bitrate / 1000;

		return adts::SUCCESS;
	}

	if (needsync)
	{
		int status = decoder->Synchronize(&params);
		if (!status)
		{
			needsync = 0;
		}
		else if (file->MPEGStream_EOF())
		{
			return adts::ENDOFFILE;
		}
	}

	if (!needsync)
	{
		int status = decoder->DecodeFrame(&params);

		if (status > ERR_NO_ERROR && status != ERR_END_OF_FILE)
		{
			needsync = 1;
			return adts::FAILURE;
		}
		else
		{
			if (status == ERR_END_OF_FILE)
			{
				if (file->MPEGStream_EOF())
				{
					return adts::ENDOFFILE;
				}
				else
				{
					*bitrate = params.bitrate / 1000;
					return adts::NEEDMOREDATA;
				}
			}

			*bitrate = params.bitrate / 1000;
			return adts::SUCCESS;
		}
	}

	return adts::SUCCESS;
}

int ADTS_VLB::GetLayer()
{
	return 4;
}

void ADTS_VLB::Release()
{
	delete this;
}