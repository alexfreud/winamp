#include "main.h"
#include "adts_mp2.h"
#include "../winamp/wa_ipc.h"
#include <math.h>
#include "mpegutil.h"
#include "../nsutil/pcm.h"

extern int g_ds;

<<<<<<< HEAD:in_mp3/adts_mp2.cpp
DecoderHooks hooks={mp3GiveVisData, mp2Equalize, mp3Equalize};

=======
>>>>>>> 5058463... fix old-school vis/eq mp3 stuff:mp3/adts_mp2.cpp
ADTS_MP2::ADTS_MP2() : decoder(0), gain(1.f)
{
	memset(&hooks, 0, sizeof(hooks));
#ifndef NO_MP3SURROUND
	lineFilled=false;
	saDecHandle=0;
	saMode = SA_DEC_OFF;
#endif
	decoderDelay = 529;
	endcut=0;

	outputFrameSize = 0;
	bitsPerSample = 0;
	allowRG = false;
	useFloat = false;
	channels = 0;
	sampleRate = 0;

	memset(&delayline, 0, sizeof(delayline));
	delaylineSize = 0;
}

void ADTS_MP2::SetDecoderHooks(void *layer3_vis, void *layer2_eq, void *layer3_eq)
{
<<<<<<< HEAD:in_mp3/adts_mp2.cpp
	//*(void **)&hooks.layer3_vis = layer3_vis;
=======
	*(void **)&hooks.layer3_vis = layer3_vis;
>>>>>>> 5058463... fix old-school vis/eq mp3 stuff:mp3/adts_mp2.cpp
	*(void **)&hooks.layer2_eq = layer2_eq;
	*(void **)&hooks.layer3_eq = layer3_eq;
}

int ADTS_MP2::Initialize(bool forceMono, bool reverseStereo, bool allowSurround, int maxBits, bool _allowRG, bool _useFloat, bool _useCRC) 
{
	allowRG = _allowRG;
	useFloat = _useFloat;
	int downmix = 0;
	if (reverseStereo)
		downmix = 2;
	else
		downmix = forceMono ? 1 : 0;
	bitsPerSample = maxBits;
<<<<<<< HEAD:in_mp3/adts_mp2.cpp
	decoder = new CMpgaDecoder(&hooks, g_ds, downmix, !!_useCRC);

=======
	decoder = new CMpgaDecoder(hooks.layer3_vis?&hooks:(DecoderHooks *)0, MPEGAUDIO_QUALITY_FULL/*g_ds*/, downmix, _useCRC);
>>>>>>> 5058463... fix old-school vis/eq mp3 stuff:mp3/adts_mp2.cpp
#ifndef NO_MP3SURROUND
	if (allowSurround)
		IIS_SADec_Init(&saDecHandle, 6);
#endif
	return 0;
}

bool ADTS_MP2::Open(ifc_mpeg_stream_reader *file)
{
	decoder->Connect((CGioFile *)file);
	if (allowRG)
		gain = file->MPEGStream_Gain();
	return true;
}

void ADTS_MP2::Close()
{
	if (decoder)
	{
		delete decoder;
		decoder = 0;
	}
#ifndef NO_MP3SURROUND
	if (saDecHandle)
		IIS_SADec_Free(&saDecHandle);
	saDecHandle=0;
#endif
}

void ADTS_MP2::GetOutputParameters(size_t *numBits, int *numChannels, int *sampleRate)
{
	*sampleRate = this->sampleRate;
	*numChannels = channels;
	*numBits = bitsPerSample;
}

void ADTS_MP2::CalculateFrameSize(int *frameSize)
{
	*frameSize = outputFrameSize;
	if (decoder->GetStreamInfo()->GetLayer() == 1)
		*frameSize *= 3;
}

void ADTS_MP2::Flush(ifc_mpeg_stream_reader *file)
{
	decoder->Reset();
#ifndef NO_MP3SURROUND
	if (saDecHandle)
		IIS_SADec_Reset(saDecHandle);
	lineFilled=false;
#endif
}

size_t ADTS_MP2::GetCurrentBitrate()
{
	return decoder->GetStreamInfo()->GetBitrate() / 1000;
}

size_t ADTS_MP2::GetDecoderDelay()
{
#ifndef NO_MP3SURROUND
	if (!saDecHandle || saMode == SA_DEC_OFF || saMode == SA_DEC_BYPASS)
		return decoderDelay;
	else /* bcc adds 576 delay */
		return decoderDelay+576;
#else
	return decoderDelay;
#endif
}

static void Decimate(const float *input, void *output, size_t numSamples, size_t *outputWritten, int bitsPerSample, bool useFloat, float gain)
{
	if (!useFloat)
	{
		// TODO seen a few crashes reported where 'output' is 0
		nsutil_pcm_FloatToInt_Interleaved_Gain(output, input, bitsPerSample, numSamples, gain);
	}
	else if (gain != 1.f)
	{
		float *data = (float *)output;
		for (size_t i=0;i<numSamples;i++)
			data[i]*=gain;
			//data[i]=input[i]*gain;
	}

	*outputWritten = numSamples * (bitsPerSample / 8);
}
/*
notes for mp3 surround implementations
need to check the first two frames for ancillary data
store first valid in temp
store second valid frame in delay line
decimate first valid into output buffer
ancillary data is stored one frame behind, so PCM data decoded from mp3 frame n combines with anc data from frame n+1
*/
int ADTS_MP2::Sync(ifc_mpeg_stream_reader *_file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate)
{
	SSC ssc;
CGioFile *file = (CGioFile *)_file;
	unsigned char ancBytes[8192] = {0};
	int numAncBytes = 0;

	unsigned int delay=0, totalLength=0;

	float floatTemp[1152*2] = {0};
	float *flData=useFloat?(float *)output:floatTemp;
	ssc = decoder->DecodeFrame(flData, sizeof(floatTemp), &outputFrameSize,
<<<<<<< HEAD:in_mp3/adts_mp2.cpp
	                           ancBytes, &numAncBytes, 1, &delay, &totalLength);

	// TODO: benski> we should really have CGioFile try to read this stuff
	if (delay && !file->prepad)
	{
		// validate
		if (delay >= 529)
		{
			decoderDelay = delay;
			endcut = 1152 - ((totalLength + delay) % 1152); // how many 0 samples had to be added?
			endcut += decoderDelay; // also need to cut out the encoder+decoder delay
			file->m_vbr_samples = totalLength;
		}
	}
=======
	                           ancBytes, &numAncBytes);
>>>>>>> fd5c493... have CGioFile read the OFL (from newer fraunhofer encoders):mp3/adts_mp2.cpp

	switch (ssc)
	{
	case SSC_OK:
	{
		channels = decoder->GetStreamInfo()->GetEffectiveChannels();
		sampleRate = decoder->GetStreamInfo()->GetEffectiveSFreq();
#ifndef NO_MP3SURROUND
		if (!numAncBytes && saDecHandle)
		{
			ssc = decoder->DecodeFrame(delayline, sizeof(delayline), &delaylineSize,
			                           ancBytes, &numAncBytes);

			if (SSC_SUCCESS(ssc))
			{
				lineFilled=true;
			}
			else if (ssc == SSC_W_MPGA_SYNCEOF)
				return ENDOFFILE;
			else
				return NEEDMOREDATA;
		}

		if (saDecHandle)
		{
			SA_DEC_ERROR sa_error = IIS_SADec_DecodeAncData(saDecHandle, ancBytes, numAncBytes, 0, 0);
			if (sa_error == SA_DEC_NO_ERROR)
			{
				IIS_SADec_InitInfo(saDecHandle, sampleRate, channels);
				SA_DEC_INFO saInfo = IIS_SADec_GetInfo(saDecHandle);
				sampleRate = saInfo.SampleRate;
				channels = saInfo.nChannelsOut;
				saMode = saInfo.configuredMode;
			}
			else if (saMode == SA_DEC_OFF)
			{
				IIS_SADec_Free(&saDecHandle);
				saDecHandle=0;
			}
			else
			{
				lineFilled=false;
				return NEEDMOREDATA;
			}
		}

		if (saDecHandle)
		{
			float surroundFloatTemp[1152*6] = {0};
			int outputSamples = 0;
			/*SA_DEC_ERROR sa_error = */IIS_SADec_DecodeFrame(saDecHandle,
			                        flData, outputFrameSize/sizeof(float),
			                        (char *)ancBytes, numAncBytes,
			                        surroundFloatTemp, sizeof(surroundFloatTemp),
			                        &outputSamples, saMode, 0, 0,
			                        0, 0);
			if (useFloat)
				memcpy(output, surroundFloatTemp, sizeof(surroundFloatTemp));
			Decimate(surroundFloatTemp, output, outputSamples, outputWritten, bitsPerSample, useFloat, (float)gain);
			outputFrameSize = *outputWritten;
		}
		else
#endif
		{
			Decimate(floatTemp, output, outputFrameSize / sizeof(float), outputWritten, bitsPerSample, useFloat, (float)gain);
			outputFrameSize = *outputWritten;
		}
	}
	return SSC_OK;
	case SSC_W_MPGA_SYNCSEARCHED:
		return NEEDMOREDATA;
	case SSC_W_MPGA_SYNCEOF:
		return ENDOFFILE;
	case SSC_E_MPGA_WRONGLAYER:
		decoder->m_Mbs.Seek(1);	
	default:
		return NEEDMOREDATA;

	}
}

int ADTS_MP2::Decode(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate, size_t *endCut)
{
	if (endcut)
	{
		*endCut = endcut;
		endcut=0;
	}
#ifndef NO_MP3SURROUND
	if (!saDecHandle && lineFilled) // if we don't have surround info, go ahead and flush out the delayline buffer
	{
		Decimate(delayline, output, delaylineSize / sizeof(float), outputWritten, bitsPerSample, useFloat, (float)gain);
		*bitrate = decoder->GetStreamInfo()->GetBitrate() / 1000;
		lineFilled=false;
		return adts::SUCCESS;
	}

	if (saDecHandle && !lineFilled && !decoder->IsEof()) // have surround info, but don't have a previously decoded frame
	{
		// resync
		int ret = Sync(file, output, outputSize, outputWritten, bitrate);
		if (ret == SSC_OK && saDecHandle)
		{
			*bitrate = decoder->GetStreamInfo()->GetBitrate() / 1000;
			return adts::SUCCESS;
		}
		else if (saDecHandle)
			return ret;
		else
			return adts::FAILURE;
	}
#endif

	unsigned char ancBytes[8192] = {0};
	int numAncBytes   = 0;

	int newl;
	
	float floatTemp[1152*2] = {0};
	float *flData=useFloat?(float *)output:floatTemp;
	SSC ssc = decoder->DecodeFrame(flData, sizeof(floatTemp), &newl, ancBytes, &numAncBytes);

	if (SSC_SUCCESS(ssc))
	{
#ifndef NO_MP3SURROUND
		if (saDecHandle && lineFilled)
		{
			float surroundFloatTemp[1152*6] = {0};
			int outputSamples;
			/*SA_DEC_ERROR sa_error = */IIS_SADec_DecodeFrame(saDecHandle,
			                        delayline, delaylineSize/sizeof(float),
			                        (char *)ancBytes, numAncBytes,
			                        surroundFloatTemp, sizeof(surroundFloatTemp),
			                        &outputSamples, saMode, 0, 0,
			                        0, 0);

			if (useFloat)
				memcpy(output, surroundFloatTemp, sizeof(surroundFloatTemp));
			Decimate(surroundFloatTemp, output, outputSamples, outputWritten, bitsPerSample, useFloat, (float)gain);
			memcpy(delayline, flData, delaylineSize);
		}
		else
#endif
		{
			Decimate(floatTemp, output, newl / sizeof(float), outputWritten, bitsPerSample, useFloat, (float)gain);
		}
		*bitrate = decoder->GetStreamInfo()->GetBitrate() / 1000;
		return adts::SUCCESS;
	}
	else if (decoder->IsEof())
	{
		#ifndef NO_MP3SURROUND
		/* In case of SA processing one ancillary data
		package and maybe fill samples left in the dynamic
		buffer of the mp3 decoder. Take care, that the
		ancillary data buffer is greater then the dynamic buffer of
		the mp3 decoder. */
		if (saDecHandle && lineFilled)
		{
			decoder->GetLastAncData(ancBytes, &numAncBytes);
			float surroundFloatTemp[1152*6];
			int outputSamples = 0;
			/*SA_DEC_ERROR sa_error = */IIS_SADec_DecodeFrame(saDecHandle,
			                        delayline, delaylineSize/sizeof(float),
			                        (char *)ancBytes, numAncBytes,
			                        surroundFloatTemp, sizeof(surroundFloatTemp),
			                        &outputSamples, saMode, 0, 0,
			                        0, 0);

			if (useFloat)
				memcpy(output, surroundFloatTemp, sizeof(surroundFloatTemp));
			Decimate(surroundFloatTemp, output, outputSamples, outputWritten, bitsPerSample, useFloat, (float)gain);
			lineFilled=false;
			*bitrate = decoder->GetStreamInfo()->GetBitrate() / 1000;
			return adts::SUCCESS;
		}
		else
#endif
			return adts::ENDOFFILE;
	}
	else if (ssc == SSC_W_MPGA_SYNCNEEDDATA)
	{
		*bitrate = decoder->GetStreamInfo()->GetBitrate() / 1000;
		return adts::NEEDMOREDATA;
	}
	else if (ssc==SSC_W_MPGA_SYNCLOST || ssc==SSC_W_MPGA_SYNCSEARCHED)
	{
		*bitrate = decoder->GetStreamInfo()->GetBitrate() / 1000;
		return adts::NEEDSYNC;
	}
	else
	{
		if (ssc == SSC_E_MPGA_WRONGLAYER)
			decoder->m_Mbs.Seek(1);

		return adts::FAILURE;
	}

}

int ADTS_MP2::GetLayer()
{
	if (decoder)
		return decoder->GetStreamInfo()->GetLayer();
	else
		return 0;
}

void ADTS_MP2::Release()
{
	delete this;
}