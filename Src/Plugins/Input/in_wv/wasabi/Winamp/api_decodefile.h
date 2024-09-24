#ifndef NULLSOFT_API_DECODEFILE_H
#define NULLSOFT_API_DECODEFILE_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include "api_audiostream.h"

enum
{
  API_DECODEFILE_SUCCESS = 0,
  API_DECODEFILE_FAILURE = 1,

  API_DECODEFILE_UNSUPPORTED = 2, // type is unsupported
  API_DECODEFILE_NO_INTERFACE = 3, // type is supported, but plugin does provide any interfaces for direct decoding
  API_DECODEFILE_WINAMP_PRO = 4, // user has to pay $$$ to do this
  API_DECODEFILE_NO_RIGHTS = 5, // user is not allowed to decode this file (e.g. DRM)
  API_DECODEFILE_BAD_RESAMPLE = 6, // Winamp is unable to resample this file to CDDA format (stereo 16bit 44.1kHz)
};

enum
{
  AUDIOPARAMETERS_FLOAT = 1,
};
struct AudioParameters
{
public:
	AudioParameters() : bitsPerSample(0), channels(0), sampleRate(0), sampleRateReal(0.f), sizeBytes((size_t) - 1), errorCode(API_DECODEFILE_SUCCESS), flags(0)
	{}
	size_t bitsPerSample;
	size_t channels;
	size_t sampleRate;
	float sampleRateReal; // yes this is duplicate.
	int flags;
	size_t sizeBytes; // total size of decoded file, (size_t)-1 means don't know
	int errorCode;
};

class api_decodefile : public Dispatchable
{
public:
	/* OpenAudioBackground gives you back an api_audiostream that you can use to get decompressed bits
	 * if it returns 0, check parameters->errorCode for the failure reason
	 * fill parameters with desired values (0 if you don't care)
	 * the decoder will _do its best_ to satisfy your passed-in audio parameters
	 * but this API does not guarantee them, so be sure to check the parameters struct after the function returns
	 * it's **UP TO YOU** to do any necessary conversion (sample rate, channels, bits-per-sample) if the decoder can't do it
	 */
	api_audiostream *OpenAudioBackground(const wchar_t *filename, AudioParameters *parameters);
	/* OpenAudio is the same as OpenAudioBackground
	 * but, it will use the input plugin system to decode if necessary
	 * so it's best to use this in a separate winamp.exe
	 * to be honest, it was designed for internal use in the CD burner
	 * so it's best not to use this one at all
	 */
	api_audiostream *OpenAudio(const wchar_t *filename, AudioParameters *parameters);

	void CloseAudio(api_audiostream *audioStream);
	/* verifies that a decoder exists to decompress this filename.
	   this is not a guarantee tgat the file is openable, just that it can be matched to a decoder */
	bool DecoderExists(const wchar_t *filename);
public:
	DISPATCH_CODES
	{
	  API_DECODEFILE_OPENAUDIO = 10,
	  API_DECODEFILE_OPENAUDIO2 = 11,
	  API_DECODEFILE_CLOSEAUDIO = 20,
	  API_DECODEFILE_DECODEREXISTS = 30,
	};
};

inline api_audiostream *api_decodefile::OpenAudio(const wchar_t *filename, AudioParameters *parameters)
{
	return _call(API_DECODEFILE_OPENAUDIO, (api_audiostream *)0, filename, parameters);
}

inline api_audiostream *api_decodefile::OpenAudioBackground(const wchar_t *filename, AudioParameters *parameters)
{
	return _call(API_DECODEFILE_OPENAUDIO2, (api_audiostream *)0, filename, parameters);
}


inline void api_decodefile::CloseAudio(api_audiostream *audioStream)
{
	_voidcall(API_DECODEFILE_CLOSEAUDIO, audioStream);
}

inline bool api_decodefile::DecoderExists(const wchar_t *filename)
{
	return _call(API_DECODEFILE_DECODEREXISTS, (bool)true, filename); // we default to true so that an old implementation doesn't break completely
}

// {9B4188F5-4295-48ab-B50C-F2B0BB56D242}
static const GUID decodeFileGUID =
  {
    0x9b4188f5, 0x4295, 0x48ab, { 0xb5, 0xc, 0xf2, 0xb0, 0xbb, 0x56, 0xd2, 0x42 }
  };


#endif