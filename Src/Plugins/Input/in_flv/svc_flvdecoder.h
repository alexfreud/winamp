#pragma once

#include <bfc/dispatch.h>
#include <api/service/services.h>

class ifc_flvaudiodecoder;
class ifc_flvvideodecoder;
class svc_flvdecoder : public Dispatchable
{
protected:
	svc_flvdecoder() {}
	~svc_flvdecoder() {}
public:
	static FOURCC getServiceType() { return WaSvc::FLVDECODER; } 
	enum
	{
		CREATEDECODER_SUCCESS = 0,
		CREATEDECODER_NOT_MINE = -1, // graceful failure
		CREATEDECODER_FAILURE = 1, // generic failure - format_type is ours but we weren't able to create the decoder
	};
	int CreateAudioDecoder(int stereo, int bits, int sample_rate, int format, ifc_flvaudiodecoder **decoder);
	int CreateVideoDecoder(int format_type, int width, int height, ifc_flvvideodecoder **decoder);
	int HandlesAudio(int format_type);
	int HandlesVideo(int format_type);
	DISPATCH_CODES
	{
		CREATE_AUDIO_DECODER = 0,
			CREATE_VIDEO_DECODER = 1,
			HANDLES_AUDIO=2,
			HANDLES_VIDEO=3,
	};
};

inline int svc_flvdecoder::CreateAudioDecoder(int stereo, int bits, int sample_rate, int format, ifc_flvaudiodecoder **decoder)
{
	return _call(CREATE_AUDIO_DECODER, (int)CREATEDECODER_NOT_MINE, stereo, bits, sample_rate, format, decoder);
}

inline int svc_flvdecoder::CreateVideoDecoder(int format_type, int width, int height, ifc_flvvideodecoder **decoder)
{
	return _call(CREATE_VIDEO_DECODER, (int)CREATEDECODER_NOT_MINE, format_type,width, height, decoder);
}

inline int svc_flvdecoder::HandlesAudio(int format_type)
{
	return _call(HANDLES_AUDIO, (int)CREATEDECODER_NOT_MINE, format_type);
}

inline int svc_flvdecoder::HandlesVideo(int format_type)
{
	return _call(HANDLES_VIDEO, (int)CREATEDECODER_NOT_MINE, format_type);
}