#pragma once
#include "foundation/dispatch.h"
#include "ifc_mp4file.h"

class ifc_mp4audiodecoder;
class ifc_mp4videodecoder;

// {39A53910-CCFE-465D-A46C-F0B95C7DD257}
static const GUID mp4_decoder_service_type_guid = 
{ 0x39a53910, 0xccfe, 0x465d, { 0xa4, 0x6c, 0xf0, 0xb9, 0x5c, 0x7d, 0xd2, 0x57 } };

class svc_mp4decoder : public Wasabi2::Dispatchable
{
protected:
	svc_mp4decoder() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_mp4decoder() {}
public:
		static GUID GetServiceType() { return mp4_decoder_service_type_guid; }
		int CreateAudioDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **decoder) { return MP4DecoderService_CreateAudioDecoder(mp4_file, mp4_track, decoder); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL MP4DecoderService_CreateAudioDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **decoder)=0;
};
