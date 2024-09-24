#include "CWAVideoRenderer.h"
#include "Main.h"

char *toChar(const GUID &guid, char *target)
{
	// {1B3CA60C-DA98-4826-B4A9-D79748A5FD73}
	StringCchPrintfA(target, 39, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
	          (int)guid.Data1, (int)guid.Data2, (int)guid.Data3,
	          (int)guid.Data4[0], (int)guid.Data4[1],
	          (int)guid.Data4[2], (int)guid.Data4[3],
	          (int)guid.Data4[4], (int)guid.Data4[5],
	          (int)guid.Data4[6], (int)guid.Data4[7] );

	return target;
}

CWAVideoRenderer::CWAVideoRenderer()
		: CBaseRenderer(CLSID_WAVideoRend, TEXT("WAVideoRenderer"), NULL, NULL)
{
	m_callback = NULL;
	m_reent = 0;
}

CWAVideoRenderer::~CWAVideoRenderer()
{
	delete m_callback;
}

HRESULT CWAVideoRenderer::DoRenderSample(IMediaSample *pMediaSample)
{
	if (m_callback)
	{
		REFERENCE_TIME StartTime, StopTime;
		pMediaSample->GetTime( &StartTime, &StopTime);
		StartTime += m_pInputPin->CurrentStartTime( );
		StopTime += m_pInputPin->CurrentStartTime( );
		m_callback->sample_cb(StartTime, StopTime, pMediaSample);
	}
	return NOERROR;
}

HRESULT CWAVideoRenderer::CheckMediaType(const CMediaType *pmt)
{
	if (pmt->majortype != MEDIATYPE_Video) return E_INVALIDARG;
	if (pmt->formattype != FORMAT_VideoInfo && pmt->formattype != FORMAT_VideoInfo2) return E_INVALIDARG;
#if 0
	{
		char blah[512] = {0};
		toChar(pmt->subtype, blah);
		OutputDebugString(blah);
	}
#endif
	if (pmt->subtype != MEDIASUBTYPE_YUY2 &&
	    pmt->subtype != MEDIASUBTYPE_YV12 &&
	    pmt->subtype != MEDIASUBTYPE_RGB32 &&
	    pmt->subtype != MEDIASUBTYPE_RGB24 &&
	    pmt->subtype != MEDIASUBTYPE_RGB8 /* &&
	    																	 pmt->subtype!=MEDIASUBTYPE_YVYU*/) return E_INVALIDARG; 
	return NOERROR;
}

HRESULT CWAVideoRenderer::SetMediaType(const CMediaType *pmt)
{
	m_mt = *pmt;

	//fix for upside down videos
	if (!m_reent && (pmt->subtype == MEDIASUBTYPE_YUY2 || pmt->subtype == MEDIASUBTYPE_YV12) )
	{
		m_reent = 1;
		IPin *p = m_pInputPin->GetConnected();
		p->Disconnect();
		m_pInputPin->Disconnect();
		if (pmt->formattype == FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER *pHeader = (VIDEOINFOHEADER*)pmt->pbFormat;
			pHeader->bmiHeader.biHeight = -pHeader->bmiHeader.biHeight;
		}
		else
		{
			VIDEOINFOHEADER2 *pHeader = (VIDEOINFOHEADER2*)pmt->pbFormat;
			pHeader->bmiHeader.biHeight = -pHeader->bmiHeader.biHeight;
		}
		if (p->Connect(m_pInputPin, pmt))
		{
			//oops it failed (like with MJPEG decompressor) so lets get it back to normal
			if (pmt->formattype == FORMAT_VideoInfo)
			{
				VIDEOINFOHEADER *pHeader = (VIDEOINFOHEADER*)pmt->pbFormat;
				pHeader->bmiHeader.biHeight = -pHeader->bmiHeader.biHeight;
			}
			else
			{
				VIDEOINFOHEADER2 *pHeader = (VIDEOINFOHEADER2*)pmt->pbFormat;
				pHeader->bmiHeader.biHeight = -pHeader->bmiHeader.biHeight;
			}
			p->Connect(m_pInputPin, pmt);
		}
		m_reent = 0;
	}
	return NOERROR;
}
//  GUID GetAcceptedType() {
CMediaType *CWAVideoRenderer::GetAcceptedType()
{
	return &m_mt;
	//    return m_mt.subtype;
}

HRESULT CWAVideoRenderer::SetCallback(CSampleCB *Callback)
{
	m_callback = Callback;
	return NOERROR;
}

HRESULT CWAVideoRenderer::ShouldDrawSampleNow(IMediaSample *pMediaSample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime)
{
	return S_OK;
}

HRESULT CWAVideoRenderer::GetSampleTimes(IMediaSample *pMediaSample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime)
{
	return S_OK;
}

HRESULT CWAVideoRenderer::EndOfStream(void)
{
	if (m_callback)
		m_callback->endofstream();
	return CBaseRenderer::EndOfStream();
}

