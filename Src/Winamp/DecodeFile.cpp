/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include <stddef.h>
#include "DecodeFile.h"
#include "ExtendedReader.h"
#include "OutputPluginAudioStream.h"
#include "ResamplingReader.h"
#include "CommonReader.h"

// "input" means input FROM the decoder, "output" means output TO the person using the ifc_audiostream
static bool RequiresResampling(const AudioParameters *input, size_t outputChannels, size_t outputBits, size_t outputSampleRate)
{
	if (outputChannels != input->channels)
		return true; // need to up or downmix

	if (outputBits != input->bitsPerSample)
		return true;

	if (outputSampleRate != input->sampleRate)
		return true;

	return false;
}

static void SetResamplingParameters(const AudioParameters *input, size_t &outputChannels, size_t &outputBits, size_t &outputSampleRate)
{
	// channels
		if (!outputChannels) 
			outputChannels=input->channels;
		else if ((input->flags & AUDIOPARAMETERS_MAXCHANNELS) && input->channels <= outputChannels)
			outputChannels = input->channels;

	// bits
	if (!outputBits) outputBits = input->bitsPerSample;
		
	// samplerate
	if (!outputSampleRate)
		outputSampleRate = input->sampleRate;
	else if ((input->flags & AUDIOPARAMETERS_MAXSAMPLERATE) && input->sampleRate <= outputSampleRate)
		outputSampleRate = input->sampleRate;

}

ifc_audiostream *DecodeFile::OpenAudio(const wchar_t *filename, AudioParameters *parameters)
{
	// save some info
	size_t outputChannels = parameters->channels;
	size_t outputBits = parameters->bitsPerSample;
	size_t outputSampleRate = parameters->sampleRate;

	CommonReader *reader = MakeReader(filename, parameters, true);
	if (!reader)
		return 0;

	// check if they've requested certain output parameters
	if (outputChannels || outputBits || outputSampleRate)
	{
		SetResamplingParameters(parameters, outputChannels, outputBits, outputSampleRate);

		// check if we need any resampling/conversion
		if (RequiresResampling(parameters, outputChannels, outputBits, outputSampleRate))
		{
			Resampler *resampler = new Resampler(parameters->bitsPerSample, parameters->channels, parameters->sampleRate,
			                                     outputBits, outputChannels, outputSampleRate, parameters->flags & AUDIOPARAMETERS_FLOAT);

			if (!resampler->OK())
			{
				parameters->errorCode = API_DECODEFILE_BAD_RESAMPLE;
				delete resampler;
				return 0;
			}

			parameters->bitsPerSample = (uint32_t)outputBits;
			parameters->channels = (uint32_t)outputChannels;
			parameters->sampleRate = (uint32_t)outputSampleRate;
			parameters->sizeBytes =(size_t)((double)parameters->sizeBytes * resampler->sizeFactor);
			ResamplingReader *resampleReader = new ResamplingReader(resampler, reader, outputChannels * outputBits / 8);

			return resampleReader;
		}

	}
	return reader;
}

ifc_audiostream *DecodeFile::OpenAudioBackground(const wchar_t *filename, AudioParameters *parameters)
{
	// save some info
	size_t outputChannels = parameters->channels;
	size_t outputBits = parameters->bitsPerSample;
	size_t outputSampleRate = parameters->sampleRate;

	CommonReader *reader = MakeReader(filename, parameters, false);
	if (!reader)
		return 0;

	// check if they've requested certain output parameters
	if (outputChannels || outputBits || outputSampleRate)
	{
		SetResamplingParameters(parameters, outputChannels, outputBits, outputSampleRate);

		// check if we need any resampling/conversion
		if (RequiresResampling(parameters, outputChannels, outputBits, outputSampleRate))
		{
			Resampler *resampler = new Resampler(parameters->bitsPerSample, parameters->channels, parameters->sampleRate,
			                                     outputBits, outputChannels, outputSampleRate, parameters->flags & AUDIOPARAMETERS_FLOAT);

			if (!resampler->OK())
			{
				parameters->errorCode = API_DECODEFILE_BAD_RESAMPLE;
				delete resampler;
				delete reader;
				return 0;
			}

			parameters->bitsPerSample = (uint32_t)outputBits;
			parameters->channels = (uint32_t)outputChannels;
			parameters->sampleRate = (uint32_t)outputSampleRate;

			ResamplingReader *resampleReader = new ResamplingReader(resampler, reader, outputChannels * outputBits / 8);
			
			return resampleReader;
		}

	}
	return reader;
}

CommonReader *DecodeFile::MakeReader(const wchar_t *filename, AudioParameters *parameters, bool useUnagi)
{
	In_Module *in;
	int a = 0;
	bool found = false;

	while (a >= 0)
	{
		OpenFunc open = 0, openFloat=0;
		OpenWFunc openW=0, openWFloat=0;
		GetDataFunc getData = 0;
		CloseFunc close = 0;
		SetTimeFunc setTime = 0;

		in = in_setmod_noplay(filename, &a);
		if (!in) break;
		if (a >= 0) a++;

		found = true;

		open = (OpenFunc)GetProcAddress(in->hDllInstance, "winampGetExtendedRead_open");
		openFloat = (OpenFunc)GetProcAddress(in->hDllInstance, "winampGetExtendedRead_open_float");
		openW = (OpenWFunc)GetProcAddress(in->hDllInstance, "winampGetExtendedRead_openW");
		openWFloat = (OpenWFunc)GetProcAddress(in->hDllInstance, "winampGetExtendedRead_openW_float");
		getData = (GetDataFunc)GetProcAddress(in->hDllInstance, "winampGetExtendedRead_getData");
		close = (CloseFunc)GetProcAddress(in->hDllInstance, "winampGetExtendedRead_close");
		setTime = (SetTimeFunc)GetProcAddress(in->hDllInstance, "winampGetExtendedRead_setTime"); //optional!
		if ((open || openW) && getData && close)
		{
			ExtendedReader *reader = new ExtendedReader(open, openW, openFloat, openWFloat, getData, close, setTime);
			if (reader->Open(filename, parameters))
				return reader;
			else
				delete reader;
		}

	}

	if (!found || !useUnagi)
	{
		parameters->errorCode = API_DECODEFILE_UNSUPPORTED;
	}
	else
	{
		OutputPluginAudioStream *reader = new OutputPluginAudioStream;
		in = in_setmod_noplay(filename, 0);
		if (reader->Open(in, filename, parameters))
			return reader;
		delete reader;
	}

	return 0;
}

void DecodeFile::CloseAudio(ifc_audiostream *audioStream)
{
	CommonReader *reader = static_cast<CommonReader *>(audioStream);
	delete reader;
	//audioStream=0;
}


#define CBCLASS DecodeFile
START_DISPATCH;
CB(API_DECODEFILE_OPENAUDIO, OpenAudio)
CB(API_DECODEFILE_OPENAUDIO2, OpenAudioBackground)
VCB(API_DECODEFILE_CLOSEAUDIO, CloseAudio)
END_DISPATCH;
