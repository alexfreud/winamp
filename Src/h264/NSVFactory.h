#pragma once
#include "../nsv/svc_nsvFactory.h"

// {262CCE92-78DC-47a9-AFFB-2471799CA799}
static const GUID h264_nsv_guid = 
{ 0x262cce92, 0x78dc, 0x47a9, { 0xaf, 0xfb, 0x24, 0x71, 0x79, 0x9c, 0xa7, 0x99 } };



class NSVFactory : public svc_nsvFactory
{
public:
	static const char *getServiceName() { return "H.264 NSV Decoder"; }
	static GUID getServiceGuid() { return h264_nsv_guid; } 
	IVideoDecoder *CreateVideoDecoder(int w, int h, double framerate, unsigned int fmt, int *flip);

protected:
	RECVS_DISPATCH;
};

