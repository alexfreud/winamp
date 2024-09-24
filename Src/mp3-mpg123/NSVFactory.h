#pragma once
#include "../nsv/svc_nsvFactory.h"

// {66AFC6B1-BFFD-49e2-BA71-62009F5266B0}
static const GUID mp3_nsv_guid = 
{ 0x66afc6b1, 0xbffd, 0x49e2, { 0xba, 0x71, 0x62, 0x0, 0x9f, 0x52, 0x66, 0xb0 } };


class NSVFactory : public svc_nsvFactory
{
public:
	static const char *getServiceName() { return "MP3 NSV Decoder"; }
	static GUID getServiceGuid() { return mp3_nsv_guid; } 
	IAudioDecoder *CreateAudioDecoder(FOURCC format, IAudioOutput **output) override;

protected:
	RECVS_DISPATCH;
};

