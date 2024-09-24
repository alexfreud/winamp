#ifndef NULLSOFT_IN_DSHOW_CWAAUDIORENDERER_H
#define NULLSOFT_IN_DSHOW_CWAAUDIORENDERER_H

#include "audioswitch.h"
#include "CSampleCB.h"

EXTERN_C GUID DECLSPEC_SELECTANY CLSID_WAAudioRend						= 
{ 0x2fa4f053, 0x6d60, 0x4cb0, {0x95, 0x3, 0x8e, 0x89, 0x23, 0x4f, 0xcb, 0xca}};



class CWAAudioRenderer : public CAudioSwitchRenderer//, CBaseReferenceClock
{
public:
  CWAAudioRenderer();
  virtual ~CWAAudioRenderer();
  HRESULT DoRenderSample(IMediaSample *pMediaSample) ;
  HRESULT CheckMediaType(const CMediaType *pmt);
  HRESULT SetMediaType(const CMediaType *pmt);
  CMediaType *GetAcceptedType();
  HRESULT SetCallback(CSampleCB *Callback);
  HRESULT EndOfStream(void);
  HRESULT ShouldDrawSampleNow(IMediaSample *pMediaSample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime);
  HRESULT GetSampleTimes(IMediaSample *pMediaSample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime);

private:
  CSampleCB *m_callback;
  CMediaType m_mt;
};
#endif