#include "CWAAudioRenderer.h"


CWAAudioRenderer::CWAAudioRenderer()
		: CAudioSwitchRenderer(CLSID_WAAudioRend, TEXT("WAAudioRenderer"), NULL, NULL)//,
		//CBaseReferenceClock(TEXT("WARefClock"), NULL, NULL, NULL)
{
	m_callback = NULL;
}

CWAAudioRenderer::~CWAAudioRenderer()
{
	delete(m_callback);
}

HRESULT CWAAudioRenderer::DoRenderSample(IMediaSample *pMediaSample)
{
	if (m_callback)
	{
		REFERENCE_TIME StartTime, StopTime;
		pMediaSample->GetTime( &StartTime, &StopTime);
		StartTime += m_pInputPin[m_inputSelected]->CurrentStartTime( );
		StopTime += m_pInputPin[m_inputSelected]->CurrentStartTime( );
		m_callback->sample_cb(StartTime, StopTime, pMediaSample);
	}
	return NOERROR;
}

HRESULT CWAAudioRenderer::CheckMediaType(const CMediaType *pmt)
{
	if (pmt->majortype != MEDIATYPE_Audio) return E_INVALIDARG;
	if (pmt->formattype != FORMAT_WaveFormatEx) return E_INVALIDARG;
	if (pmt->subtype != MEDIASUBTYPE_PCM && pmt->subtype != MEDIASUBTYPE_IEEE_FLOAT) return E_INVALIDARG;
	return NOERROR;
}

HRESULT CWAAudioRenderer::SetMediaType(const CMediaType *pmt)
{
	m_mt = *pmt;
	return NOERROR;
}

CMediaType *CWAAudioRenderer::GetAcceptedType()
{
	return &m_mt;
}

HRESULT CWAAudioRenderer::SetCallback(CSampleCB *Callback)
{
	m_callback = Callback;
	return NOERROR;
}


HRESULT CWAAudioRenderer::EndOfStream(void)
{
	if (m_callback)
		m_callback->endofstream();
	return CAudioSwitchRenderer::EndOfStream();
}

HRESULT CWAAudioRenderer::ShouldDrawSampleNow(IMediaSample *pMediaSample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime)
{
	return S_OK;
}

HRESULT CWAAudioRenderer::GetSampleTimes(IMediaSample *pMediaSample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime)
{
	return S_OK;
}
