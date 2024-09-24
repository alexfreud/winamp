/* copyright 2006 Ben Allison */
#include "ALACMP4Decoder.h"
#include "alac/ALACBitUtilities.h"
#include "api__alac.h"
#include "decomp.h"
#include <math.h>
#include <string.h>
#include "../external_dependencies/libmp4v2/mp4.h"
#include "../Plugins/Input/in_mp4/mpeg4audio.h"


// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

int ALACMP4Decoder::OpenMP4(MP4FileHandle mp4_file, MP4TrackId mp4_track, size_t output_bits, size_t maxChannels, bool useFloat)
{
	if (useFloat)
		return MP4_FAILURE;
	// get requested output bits
	this->output_bits = (int)output_bits;

	use_rg = false;
	rg = 1.0f;
	uint64_t val;
	if (MP4GetTrackIntegerProperty(mp4_file, mp4_track, "mdia.minf.stbl.stsd.*[0].channels", &val))
		channels = (int)val;
	else
		channels=2;

	if (MP4GetTrackIntegerProperty(mp4_file, mp4_track, "mdia.minf.stbl.stsd.*[0].sampleSize", &val))
		bps= (int)val;
	else
		bps=16;

	return MP4_SUCCESS;
}

int ALACMP4Decoder::GetCurrentBitrate(unsigned int *bitrate)
{
	if (mpAlacDecoder->mConfig.avgBitRate)
	{
		*bitrate = mpAlacDecoder->mConfig.avgBitRate;
		return MP4_SUCCESS;
	}
	return MP4_GETCURRENTBITRATE_UNKNOWN; // TODO
}

 /* 
  * 32 bits  atom size
  * 32 bits  tag                  ("alac")
  * 
  * Following data is passed to the function, ALACSpecificConfig structure (24 Bytes) does not contain
  * "tag version", check and skip if data contains it
  * 
  * 32 bits  tag version          (0)
  * 32 bits  samples per frame    (used when not set explicitly in the frames)  --> frameLength
  *  8 bits  compatible version   (0)			
  *  8 bits  sample size														--> bitDepth
  *  8 bits  history mult         (40)
  *  8 bits  initial history      (10)
  *  8 bits  rice param limit     (14)
  *  8 bits  channels															--> numChannels
  * 16 bits  maxRun               (255)
  * 32 bits  max coded frame size (0 means unknown)								--> maxFrameBytes
  * 32 bits  average bitrate      (0 means unknown)
  * 32 bits  samplerate
  */
int ALACMP4Decoder::AudioSpecificConfiguration(void *buffer, size_t buffer_size)
{
	mpAlacDecoder = new ALACDecoder();
	if (buffer_size > sizeof(ALACSpecificConfig))
	{
		// we have the "tag version" embedded;
		// just skip
		size_t skip = sizeof(uint32_t);
		mpAlacDecoder->Init((void*)((char*)buffer + skip), sizeof(ALACSpecificConfig));
	}
	else
	{
		mpAlacDecoder->Init(buffer, buffer_size);
	}

	/*alac = create_alac(bps, channels);
	alac_set_info(alac, reinterpret_cast<char *>(buffer));*/
	return MP4_SUCCESS;
}

void ALACMP4Decoder::Flush()
{
	// TODO
}

void ALACMP4Decoder::Close()
{
	if (mpAlacDecoder)
	{
		delete mpAlacDecoder;
		mpAlacDecoder = 0;
	}
}

int ALACMP4Decoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample)
{
	*sampleRate = mpAlacDecoder->mConfig.sampleRate; // TODO: verify
	*channels = mpAlacDecoder->mConfig.numChannels;
	*bitsPerSample = mpAlacDecoder->mConfig.bitDepth;
	return MP4_SUCCESS;
}

int ALACMP4Decoder::OutputFrameSize(size_t *frameSize)
{
	*frameSize = mpAlacDecoder->mConfig.frameLength; // TODO: verify
	return MP4_SUCCESS;
}


#define PA_CLIP_( val, min, max )\
{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

inline static void clip(double &x, double a, double b)
{
	double x1 = fabs (x - a);
	double x2 = fabs (x - b);
	x = x1 + (a + b);
	x -= x2;
	x *= 0.5;
}


int ALACMP4Decoder::DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	struct BitBuffer inputChunk;
	BitBufferInit(&inputChunk, (uint8_t*)(inputBuffer), inputBufferBytes);
	uint32_t numFrames = 0;
	mpAlacDecoder->Decode(&inputChunk, (uint8_t*)(outputBuffer), mpAlacDecoder->mConfig.frameLength , mpAlacDecoder->mConfig.numChannels, &numFrames);

	size_t bytesPerSample = (mpAlacDecoder->mConfig.bitDepth / 8) * mpAlacDecoder->mConfig.numChannels;
	*outputBufferBytes = mpAlacDecoder->mConfig.frameLength * bytesPerSample;

	if (use_rg && mpAlacDecoder->mConfig.bitDepth == 16)
	{
		size_t numSamples = *outputBufferBytes / (mpAlacDecoder->mConfig.bitDepth / 8);
		//if (bitsPerSample == 16)
		{
			// TODO: this algorithm assumes ALAC bps is 16!!
			int16_t* audioBuffer = (int16_t*)outputBuffer;
			for (size_t i = 0; i != numSamples; i++)
			{
				float sample = (float)audioBuffer[i];
				int32_t temp = (int32_t)(sample * rg);
				PA_CLIP_(temp, -0x8000, 0x7FFF);
				audioBuffer[i] = (uint16_t)temp;
			}
		}
	}
	return MP4_SUCCESS;
}

const char *ALACMP4Decoder::GetCodecInfoString()
{
	return "mdia.minf.stbl.stsd.alac.alac.decoderConfig";
}

int ALACMP4Decoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "alac");
}

int ALACMP4Decoder::SetGain(float gain)
{
	if (gain != 1.0f)
	{
		use_rg = true;
		rg = gain;
	}
	return MP4_SUCCESS;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS ALACMP4Decoder
START_DISPATCH;
CB(MPEG4_AUDIO_OPENMP4, OpenMP4)
CB(MPEG4_AUDIO_ASC, AudioSpecificConfiguration)
CB(MPEG4_AUDIO_BITRATE, GetCurrentBitrate)
CB(MPEG4_AUDIO_FRAMESIZE, OutputFrameSize)
CB(MPEG4_AUDIO_OUTPUTINFO, GetOutputProperties)
CB(MPEG4_AUDIO_DECODE, DecodeSample)
VCB(MPEG4_AUDIO_FLUSH, Flush)
VCB(MPEG4_AUDIO_CLOSE, Close)
CB(MPEG4_AUDIO_CODEC_INFO_STRING, GetCodecInfoString)
CB(MPEG4_AUDIO_HANDLES_CODEC, CanHandleCodec)
CB(MPEG4_AUDIO_SET_GAIN, SetGain)
END_DISPATCH;