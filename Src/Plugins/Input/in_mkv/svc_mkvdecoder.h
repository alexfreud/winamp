#pragma once
#include <bfc/dispatch.h>
#include "../nsmkv/Tracks.h"
#include <api/service/services.h>
class ifc_mkvvideodecoder;
class ifc_mkvaudiodecoder;
class NOVTABLE svc_mkvdecoder : public Dispatchable
{
protected:
	svc_mkvdecoder() {}
	~svc_mkvdecoder() {}
public:
	static FOURCC getServiceType() { return WaSvc::MKVDECODER; } 
	enum
	{
		CREATEDECODER_SUCCESS = 0,
		CREATEDECODER_NOT_MINE = -1, // graceful failure
		CREATEDECODER_FAILURE = 1, // generic failure - codec_id is ours but we weren't able to create the decoder (e.g. track_entry_data)
	};
	int CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point, ifc_mkvaudiodecoder **decoder);
	int CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder);
	DISPATCH_CODES
	{
		CREATE_AUDIO_DECODER = 0,
			CREATE_VIDEO_DECODER = 1,
	};
};

inline int svc_mkvdecoder::CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point, ifc_mkvaudiodecoder **decoder)
{
	return _call(CREATE_AUDIO_DECODER, (int)CREATEDECODER_NOT_MINE, codec_id, track_entry_data, audio_data, preferred_bits, max_channels, floating_point, decoder);
}

inline int svc_mkvdecoder::CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder)
{
	return _call(CREATE_VIDEO_DECODER, (int)CREATEDECODER_NOT_MINE, codec_id, track_entry_data, video_data, decoder);
}