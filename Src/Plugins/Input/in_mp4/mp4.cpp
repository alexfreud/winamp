#include "main.h"
#include "mpeg4audio.h"
#include "api__in_mp4.h"
#include <api/service/waservicefactory.h>
#include <api/service/services.h>
//#include "JPEGDecoder.h"
//#include "MPEG4VideoDecoder.h"
//#include "AVCDecoder.h"
#include "../nu/bitbuffer.h"
#include <assert.h>

void ConfigureDecoderASC(MP4FileHandle file, MP4TrackId track, MP4AudioDecoder *decoder)
{
	unsigned char *buffer = NULL;
	unsigned __int32 buffer_size = 0;

	// TODO: HUGE hack
	const char *location = decoder->GetCodecInfoString();
	if (location)
		MP4GetTrackBytesProperty(file, track, location , (unsigned __int8 **)&buffer, &buffer_size);
	else
		MP4GetTrackESConfiguration(file, track, (unsigned __int8 **)&buffer, &buffer_size);

	if (buffer)
	{
		decoder->AudioSpecificConfiguration(buffer, buffer_size);
		MP4Free(buffer);
	}
}

bool CreateDecoder(MP4FileHandle file, MP4TrackId track, MP4AudioDecoder *&decoder, waServiceFactory *&serviceFactory)
{
	const char *media_data_name = MP4GetTrackMediaDataName(file, track);
	u_int8_t audioType = MP4GetTrackEsdsObjectTypeId(file, track);
	u_int8_t mpeg4Type = MP4GetTrackAudioMpeg4Type(file, track);
	if (!media_data_name)
		media_data_name = "mp4a"; // let's assume it's AAC if nothing else is said
	waServiceFactory *sf = 0;

	int n = 0;
	while (sf = mod.service->service_enumService(WaSvc::MP4AUDIODECODER, n++))
	{
		MP4AudioDecoder * dec = static_cast<MP4AudioDecoder *>(sf->getInterface());
		if (dec		    && dec->CanHandleCodec(media_data_name)
		    && (!audioType || dec->CanHandleType(audioType))
		    && (!mpeg4Type || dec->CanHandleMPEG4Type(mpeg4Type))
		    /*&& dec->Open() == MP4_SUCCESS*/)
		{
			//ConfigureDecoderASC(file, track, dec);
			decoder = dec;
			serviceFactory = sf;
			return true;
		}

		sf->releaseInterface(dec);

	}
	return false;
}

bool CreateVideoDecoder(MP4FileHandle file, MP4TrackId track, MP4VideoDecoder *&decoder, waServiceFactory *&serviceFactory)
{
	const char *media_data_name = MP4GetTrackMediaDataName(file, track);
	// TODO check this is ok to disable...
	//u_int8_t audioType = MP4GetTrackEsdsObjectTypeId(file, track);
	//u_int8_t profileLevel = MP4GetVideoProfileLevel(file, track);
	if (!media_data_name)
		return false;

	waServiceFactory *sf = 0;
	int n = 0;
	while (sf = mod.service->service_enumService(WaSvc::MP4VIDEODECODER, n++))
	{
		MP4VideoDecoder *dec = static_cast<MP4VideoDecoder *>(sf->getInterface());
		if (dec		    && dec->CanHandleCodec(media_data_name)
			/*&& dec->Open() == MP4_SUCCESS*/)
		{
			decoder = dec;
			serviceFactory = sf;
			return true;
		}
		sf->releaseInterface(dec);
	}
	/*
	if (!strcmp(media_data_name, "mp4v"))
	{
		// TODO: write a way to get the binary data out of an atom
		uint8_t *buffer;
		uint32_t buffer_size = 0;
    MP4GetTrackESConfiguration(file, track, &buffer, &buffer_size);

		decoder = new MPEG4VideoDecoder(0,0);//buffer, buffer_size);
		return true;
	}
*/

	return false;
}