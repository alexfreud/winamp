#pragma once
#include <bfc/dispatch.h>
#include "../nsavi/avi_header.h"
#include <api/service/services.h>
class ifc_avivideodecoder;
class ifc_aviaudiodecoder;
class ifc_avitextdecoder;
class ifc_avimididecoder;

class NOVTABLE svc_avidecoder : public Dispatchable
{
protected:
	svc_avidecoder() {}
	~svc_avidecoder() {}
public:
	static FOURCC getServiceType() { return WaSvc::AVIDECODER; } 
	enum
	{
		CREATEDECODER_SUCCESS = 0,
		CREATEDECODER_NOT_MINE = -1, // graceful failure
		CREATEDECODER_FAILURE = 1, // generic failure - codec_id is ours but we weren't able to create the decoder (e.g. track_entry_data)
	};
	int CreateAudioDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, 
		unsigned int preferred_bits, unsigned int max_channels, bool floating_point, 
		ifc_aviaudiodecoder **decoder);
	int CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder);

	// for retrieving short codec names, e.g. "MPEG Audio" or "H.264"
	int GetAudioCodec(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, wchar_t *buf, size_t buf_cch);
	int GetVideoCodec(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, wchar_t *buf, size_t buf_cch);

	// for longer description, e.g. "H.264 320x240" or "PCM 16bit stereo 44.1khz"
	int GetAudioDescription(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, wchar_t *buf, size_t buf_cch);
	int GetVideoDescription(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, wchar_t *buf, size_t buf_cch);
	DISPATCH_CODES
	{
		CREATE_AUDIO_DECODER = 0,
		CREATE_VIDEO_DECODER = 1,
		GET_AUDIO_CODEC = 2,
		GET_VIDEO_CODEC = 3,
		GET_AUDIO_DESCRIPTION = 4,
		GET_VIDEO_DESCRIPTION = 5,
	};
};

inline int svc_avidecoder::CreateAudioDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point, ifc_aviaudiodecoder **decoder)
{
	return _call(CREATE_AUDIO_DECODER, (int)CREATEDECODER_NOT_MINE, avi_header, stream_header, stream_format, stream_data, preferred_bits, max_channels, floating_point, decoder);
}

inline int svc_avidecoder::CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder)
{
	return _call(CREATE_VIDEO_DECODER, (int)CREATEDECODER_NOT_MINE, avi_header, stream_header, stream_format, stream_data, decoder);
}

inline int svc_avidecoder::GetAudioCodec(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, wchar_t *buf, size_t buf_cch)
{
	return _call(GET_AUDIO_CODEC, (int)CREATEDECODER_NOT_MINE, avi_header, stream_header, stream_format, stream_data, buf, buf_cch);
}

inline int svc_avidecoder::GetVideoCodec(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, wchar_t *buf, size_t buf_cch)
{
	return _call(GET_VIDEO_CODEC, (int)CREATEDECODER_NOT_MINE, avi_header, stream_header, stream_format, stream_data, buf, buf_cch);
}

inline int svc_avidecoder::GetAudioDescription(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, wchar_t *buf, size_t buf_cch)
{
	return _call(GET_AUDIO_DESCRIPTION, (int)CREATEDECODER_NOT_MINE, avi_header, stream_header, stream_format, stream_data, buf, buf_cch);
}

inline int svc_avidecoder::GetVideoDescription(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, wchar_t *buf, size_t buf_cch)
{
	return _call(GET_VIDEO_DESCRIPTION, (int)CREATEDECODER_NOT_MINE, avi_header, stream_header, stream_format, stream_data, buf, buf_cch);
}
