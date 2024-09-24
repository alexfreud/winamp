#ifndef NULLSOFT_AACPLUSDECODER_NSVFACTORY_H
#define NULLSOFT_AACPLUSDECODER_NSVFACTORY_H

#include "../nsv/svc_nsvFactory.h"

// {D121CDF8-8443-4430-8AD0-237FF4AC0163}
static const GUID vp6_nsv_guid = 
{ 0xd121cdf8, 0x8443, 0x4430, { 0x8a, 0xd0, 0x23, 0x7f, 0xf4, 0xac, 0x1, 0x63 } };


class NSVFactory : public svc_nsvFactory
{
public:
	static const char *getServiceName() { return "VP6 NSV Decoder"; }
	static GUID getServiceGuid() { return vp6_nsv_guid; } 
	IVideoDecoder *CreateVideoDecoder(int w, int h, double framerate, unsigned int fmt, int *flip);

protected:
	RECVS_DISPATCH;
};

#endif