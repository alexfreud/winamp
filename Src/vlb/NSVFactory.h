#ifndef NULLSOFT_VLB_NSVFACTORY_H
#define NULLSOFT_VLB_NSVFACTORY_H

#include "../nsv/svc_nsvFactory.h"

class NSVFactory : public svc_nsvFactory
{
public:
	IAudioDecoder *CreateAudioDecoder(FOURCC format, IAudioOutput **output) override;

protected:
	RECVS_DISPATCH;
};

#endif