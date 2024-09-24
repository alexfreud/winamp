#ifndef NULLSOFT_NSV_SVC_NSVFACTORY_H
#define NULLSOFT_NSV_SVC_NSVFACTORY_H

#include <bfc/Dispatch.h>
#include <bfc/platform/types.h>
#include <api/service/services.h>
class IAudioDecoder;
class IVideoDecoder;
class IAudioOutput;

/*
 **********************************************************************
 *  Important Note!                                                   *
 *                                                                    *
 *                                                                    *
 *  Objects created by this class are allocated using api_memmgr      *
 *  You MUST call api_memmgr::Delete() to delete these objects.       *
 *  Do not use the C++ delete operator                                *
 *  as the memory was allocated with a different heap / malloc arena  *
 *                                                                    *
 **********************************************************************
 */
class svc_nsvFactory : public Dispatchable
{
protected:
  svc_nsvFactory(){}
  ~svc_nsvFactory(){}
public:
  virtual IAudioDecoder *CreateAudioDecoder(FOURCC format, IAudioOutput **output);
  IVideoDecoder *CreateVideoDecoder(int w, int h, double framerate, unsigned int fmt, int *flip);
  
public:
    DISPATCH_CODES
  {
    SVC_NSVFACTORY_CREATEAUDIODECODER=10,
			 SVC_NSVFACTORY_CREATEVIDEODECODER=20,
  };

	static FOURCC getServiceType() { return WaSvc::NSVFACTORY; }
};

inline IAudioDecoder *svc_nsvFactory::CreateAudioDecoder(FOURCC format, IAudioOutput **output)
{
  return _call(SVC_NSVFACTORY_CREATEAUDIODECODER, (IAudioDecoder *)0, format, output);
}

inline IVideoDecoder *svc_nsvFactory::CreateVideoDecoder(int w, int h, double framerate, unsigned int fmt, int *flip)
{
	return _call(SVC_NSVFACTORY_CREATEVIDEODECODER, (IVideoDecoder *)0, w, h, framerate, fmt, flip);
}

#endif
