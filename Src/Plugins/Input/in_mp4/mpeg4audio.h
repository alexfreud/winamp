#ifndef NULLSOFT_MPEG4AUDIOH
#define NULLSOFT_MPEG4AUDIOH
#include "../external_dependencies/libmp4v2/mp4.h"
#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include <api/service/services.h>
enum
{
    MP4_SUCCESS = 0,
    MP4_FAILURE = 1,

    MP4_GETOUTPUTPROPERTIES_NEED_MORE_INPUT = 2,
		MP4_NEED_MORE_INPUT = 2,

    MP4_GETCURRENTBITRATE_UNKNOWN = 2,  // unable to calculate (e.g. VBR for CT's decoder)

    MP4_OUTPUTFRAMESIZE_VARIABLE = 2,  // don't know if any codecs do this

    MP4_TYPE_MPEG1_AUDIO	= 0x6B,
    MP4_TYPE_MPEG2_AUDIO	= 0x69,
    MP4_TYPE_MPEG2_AAC_MAIN_AUDIO	= 0x66,
    MP4_TYPE_MPEG2_AAC_LC_AUDIO	= 0x67,
    MP4_TYPE_MPEG2_AAC_SSR_AUDIO	= 0x68,
    MP4_TYPE_MPEG4_AUDIO	= 0x40,
    MP4_TYPE_PRIVATE_AUDIO	= 0xC0,
    MP4_TYPE_PCM16_LITTLE_ENDIAN_AUDIO = 0xE0,
    MP4_TYPE_VORBIS_AUDIO	= 0xE1,
    MP4_TYPE_AC3_AUDIO	= 0xE2,
    MP4_TYPE_ALAW_AUDIO	= 0xE3,
    MP4_TYPE_ULAW_AUDIO	= 0xE4,
    MP4_TYPE_G723_AUDIO = 0xE5,
    MP4_TYPE_PCM16_BIG_ENDIAN_AUDIO = 0xE6,

    /* MP4 MPEG-4 Audio types from 14496-3 Table 1.5.1 */
    MP4_MPEG4_TYPE_AAC_MAIN_AUDIO	= 1,
    MP4_MPEG4_TYPE_AAC_LC_AUDIO	= 2,
    MP4_MPEG4_TYPE_AAC_SSR_AUDIO	= 3,
    MP4_MPEG4_TYPE_AAC_LTP_AUDIO	= 4,
    MP4_MPEG4_TYPE_AAC_HE_AUDIO = 5,
    MP4_MPEG4_TYPE_AAC_SCALABLE_AUDIO = 6,
    MP4_MPEG4_TYPE_CELP_AUDIO	= 8,
    MP4_MPEG4_TYPE_HVXC_AUDIO	= 9,
    MP4_MPEG4_TYPE_TTSI_AUDIO	= 12,
    MP4_MPEG4_TYPE_MAIN_SYNTHETIC_AUDIO = 13,
    MP4_MPEG4_TYPE_WAVETABLE_AUDIO	= 14,
    MP4_MPEG4_TYPE_MIDI_AUDIO	= 15,
    MP4_MPEG4_TYPE_ALGORITHMIC_FX_AUDIO	= 16,
		MP4_MPEG4_TYPE_PARAMETRIC_STEREO=29,
		MP4_MPEG4_ALS_AUDIO=31,
		MP4_MPEG4_LAYER1_AUDIO	= 32,
		MP4_MPEG4_LAYER2_AUDIO=	33,
		MP4_MPEG4_LAYER3_AUDIO=	34,
		
		MP4_MPEG4_SLS_AUDIO=37,
};

class MP4AudioDecoder : public Dispatchable
{
protected:
	MP4AudioDecoder() {}
	~MP4AudioDecoder() {}

public:
	static FOURCC getServiceType() { return WaSvc::MP4AUDIODECODER; } 
	int Open();
	int OpenEx(size_t bits, size_t maxChannels, bool useFloat);
	int OpenMP4(MP4FileHandle mp4_file, MP4TrackId mp4_track, size_t output_bits, size_t maxChannels, bool useFloat);
	int AudioSpecificConfiguration(void *buffer, size_t buffer_size); // reads ASC block from mp4 file
	int GetCurrentBitrate(unsigned int *bitrate);
	int OutputFrameSize(size_t *frameSize); // in Frames
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample); // can return an error code for "havn't locked to stream yet"
	int GetOutputPropertiesEx(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat); 
	int DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Flush();
	void Close();
	const char *GetCodecInfoString();
	int CanHandleCodec(const char *codecName);
	int CanHandleType(uint8_t type);
	int CanHandleMPEG4Type(uint8_t type);
	int SetGain(float gain);
	int RequireChunks(); // return 1 if your decoder wants to read whole chunks rather than samples
	void EndOfStream();

public:
	DISPATCH_CODES
	{
	    MPEG4_AUDIO_OPEN = 10,
			MPEG4_AUDIO_OPEN_EX = 11,
			MPEG4_AUDIO_OPENMP4 = 12,
	    MPEG4_AUDIO_ASC = 20,	
	    MPEG4_AUDIO_BITRATE = 30,
	    MPEG4_AUDIO_FRAMESIZE = 40,
	    MPEG4_AUDIO_OUTPUTINFO = 50,
			MPEG4_AUDIO_OUTPUTINFO_EX = 51,
	    MPEG4_AUDIO_DECODE = 60,
	    MPEG4_AUDIO_FLUSH = 70,
	    MPEG4_AUDIO_CLOSE = 80,
	    MPEG4_AUDIO_CODEC_INFO_STRING = 90,
	    MPEG4_AUDIO_HANDLES_CODEC = 100,
	    MPEG4_AUDIO_HANDLES_TYPE = 110,
	    MPEG4_AUDIO_HANDLES_MPEG4_TYPE = 120,
			MPEG4_AUDIO_SET_GAIN=130,
		MPEG4_AUDIO_REQUIRE_CHUNKS = 140,
		MPEG4_END_OF_STREAM = 150,
	};
};

inline int MP4AudioDecoder::Open()
{
	return _call(MPEG4_AUDIO_OPEN, (int)MP4_FAILURE);
}

inline int MP4AudioDecoder::OpenEx(size_t bits, size_t maxChannels, bool useFloat)
{
	void *params[3] = { &bits, &maxChannels, &useFloat};
	int retval;
	if (_dispatch(MPEG4_AUDIO_OPEN_EX, &retval, params, 3)) 
		return retval;
	else
		return Open();
}

inline int MP4AudioDecoder::OpenMP4(MP4FileHandle mp4_file, MP4TrackId mp4_track, size_t output_bits, size_t maxChannels, bool useFloat)
{
	void *params[5] = { &mp4_file, &mp4_track, &output_bits, &maxChannels, &useFloat};
	int retval;
	if (_dispatch(MPEG4_AUDIO_OPENMP4, &retval, params, 5)) 
		return retval;
	else
		return OpenEx(output_bits, maxChannels, useFloat);
}

inline int MP4AudioDecoder::AudioSpecificConfiguration(void *buffer, size_t buffer_size)
{
	return _call(MPEG4_AUDIO_ASC, (int)MP4_FAILURE, buffer, buffer_size);
}
inline int MP4AudioDecoder::GetCurrentBitrate(unsigned int *bitrate)
{
	return _call(MPEG4_AUDIO_BITRATE, (int)MP4_FAILURE, bitrate);
}
inline int MP4AudioDecoder::OutputFrameSize(size_t *frameSize)
{
	return _call(MPEG4_AUDIO_FRAMESIZE, (int)MP4_FAILURE, frameSize);
}
inline int MP4AudioDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample)
{
	return _call(MPEG4_AUDIO_OUTPUTINFO, (int)MP4_FAILURE, sampleRate, channels, bitsPerSample);
}
inline int MP4AudioDecoder::GetOutputPropertiesEx(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *useFloat)
{
	void *params[4] = { &sampleRate, &channels, &bitsPerSample, &useFloat};
    int retval;
    if (_dispatch(MPEG4_AUDIO_OUTPUTINFO_EX, &retval, params, 4)) 
			return retval;
		else
		{
			*useFloat=false;
			return GetOutputProperties(sampleRate, channels, bitsPerSample);
		}
//	return _call(MPEG4_AUDIO_OUTPUTINFO_EX, (int)MP4_FAILURE, sampleRate, channels, bitsPerSample);
}
inline int MP4AudioDecoder::DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	return _call(MPEG4_AUDIO_DECODE, (int)MP4_FAILURE, inputBuffer, inputBufferBytes, outputBuffer, outputBufferBytes);
}

inline void MP4AudioDecoder::Flush()
{
	_voidcall(MPEG4_AUDIO_FLUSH);
}
inline void MP4AudioDecoder::Close()
{
	_voidcall(MPEG4_AUDIO_CLOSE);
}
inline const char *MP4AudioDecoder::GetCodecInfoString()
{
	return _call(MPEG4_AUDIO_CODEC_INFO_STRING, (const char *)0);
}

inline int MP4AudioDecoder::CanHandleCodec(const char *codecName)
{
	return _call(MPEG4_AUDIO_HANDLES_CODEC, (int)0, codecName);
}

inline int MP4AudioDecoder::CanHandleType(uint8_t type)
{
	return _call(MPEG4_AUDIO_HANDLES_TYPE, (int)0, type);
}
inline int MP4AudioDecoder::CanHandleMPEG4Type(uint8_t type)
{
	return _call(MPEG4_AUDIO_HANDLES_MPEG4_TYPE, (int)0, type);
}

inline int MP4AudioDecoder::SetGain(float gain)
{
	return _call(MPEG4_AUDIO_SET_GAIN, (int)MP4_FAILURE, gain);
}

inline int MP4AudioDecoder::RequireChunks()
{
	return _call(MPEG4_AUDIO_REQUIRE_CHUNKS, (int)0);
}

inline void MP4AudioDecoder::EndOfStream()
{
	_voidcall(MPEG4_END_OF_STREAM);
}
#endif
