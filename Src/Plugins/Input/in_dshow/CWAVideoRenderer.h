#ifndef NULLSOFT_IN_DSHOW_CWAVIDEORENDERERH
#define NULLSOFT_IN_DSHOW_CWAVIDEORENDERERH

#include "audioswitch.h"
#include "CSampleCB.h"


EXTERN_C GUID DECLSPEC_SELECTANY CLSID_WAVideoRend =
  {0x2fa4f053, 0x6d60, 0x4cb0, {0x95, 0x3, 0x8e, 0x89, 0x23, 0x4f, 0xca, 0xca}};


class CWAVideoRenderer : public CBaseRenderer
{
public:
	CWAVideoRenderer();
	virtual ~CWAVideoRenderer();
	HRESULT DoRenderSample(IMediaSample *pMediaSample);
	HRESULT CheckMediaType(const CMediaType *pmt);
	HRESULT SetMediaType(const CMediaType *pmt);
	CMediaType *GetAcceptedType();
	HRESULT SetCallback(CSampleCB *Callback);
	HRESULT ShouldDrawSampleNow(IMediaSample *pMediaSample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime);
	HRESULT GetSampleTimes(IMediaSample *pMediaSample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime);
	HRESULT EndOfStream(void);

private:
	CSampleCB *m_callback;
	CMediaType m_mt;
	int m_reent;
};

#endif
