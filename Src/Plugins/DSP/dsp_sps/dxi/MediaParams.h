// MediaParams.h: interface for the CMediaParams class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEDIAPARAMS_H__3DB99F00_3887_4C35_BBA8_C47835777A69__INCLUDED_)
#define AFX_MEDIAPARAMS_H__3DB99F00_3887_4C35_BBA8_C47835777A69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MedParam.h>		// DX8 automation
#include "CakeMedParam.h"	// DX8 automation

#include "ParamEnvelope.h"
#include "Parameters.h"

////////////////////////////////////////////////////////////////////////////////

class CMediaParams :
	public IMediaParams,
	public IMediaParamInfo,
	public IMediaParamsSetUICallback,
	public IMediaParamsUICallback
{
public:
	
	static HRESULT Create( CMediaParams** ppObj, IUnknown* pUnkOuter );

public:

	// IUnknown
	STDMETHOD(QueryInterface)( REFIID riid, void** ppv );
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMediaParams
	STDMETHOD(GetParam)(ULONG dwParamIndex, FLOAT* pValue);
	STDMETHOD(SetParam)(ULONG dwParamIndex, FLOAT value);
	STDMETHOD(AddEnvelope)(ULONG dwParamIndex, ULONG cSegments, MP_ENVELOPE_SEGMENT* pEnvelopeSegments);
	STDMETHOD(FlushEnvelope)(ULONG dwParamIndex, REFERENCE_TIME refTimeStart, REFERENCE_TIME refTimeEnd);
	STDMETHOD(SetTimeFormat)(GUID guidTimeFormat, ULONG mpTimeData);

	// IMediaParamInfo
	STDMETHOD(GetParamCount)(ULONG* pdwParams);
	STDMETHOD(GetParamInfo)(ULONG dwParamIndex, MP_PARAMINFO* pInfo);
	STDMETHOD(GetParamText)(ULONG dwParamIndex, WCHAR** ppwchText);
	STDMETHOD(GetNumTimeFormats)(ULONG* pdwNumTimeFormats);
	STDMETHOD(GetSupportedTimeFormat)(ULONG dwFormatIndex, GUID* pguidTimeFormat);
	STDMETHOD(GetCurrentTimeFormat)(GUID* pguidTimeFormat, ULONG* pTimeData);

	// IMediaParamsSetUICallback
	STDMETHOD(SetUICallback)(IMediaParamsUICallback* pICallback);

	// IMediaParamsUICallback
	STDMETHOD(ParamsBeginCapture)(DWORD *aIndex, DWORD cPoints);
	STDMETHOD(ParamsChanged)(DWORD *aIndex, DWORD cPoints, MP_DATA *paData);
	STDMETHOD(ParamsEndCapture)(DWORD *aIndex, DWORD cPoints);

	// Helpers for setting the current sample rate
	void SetSampleRate( long lFs ) { m_lFs = lFs; }
	long GetSampleRate() const { return m_lFs; }

	// Helpers to decimate shapes into smaller chunks
	HRESULT GetDecimationTimes( LONGLONG llSampStart, LONGLONG llSampEnd, std::vector<LONGLONG>* pTimes );
	void SetDecimationInterval( double d ) { m_dDecimationInterval = d; }
	double GetDecimationInterval() const { return m_dDecimationInterval; }

	// Set our position among all parameter segments, updating current values, and
	// flushing any out-of-date segments.
	HRESULT UpdateValuesForSample( LONGLONG llSamp );

	// Get the envelope for a given parameter
	const CParamEnvelope& GetParamEnvelope( DWORD ix )
	{
		ASSERT( ix >= 0 && ix < NUM_PARAMS );
		return m_aEnv[ ix ];
	}

private:

	IUnknown*						m_pUnkOuter;
	IMediaParamsUICallback*		m_pCallback;
	LONG								m_cRef;
	CParamEnvelope*				m_aEnv;
	double							m_dDecimationInterval;
	long								m_lFs;

private:

	static const ParamInfo m_aParamInfo[ NUM_PARAMS ];

private:

	CMediaParams( IUnknown* pUnkOuter );
	virtual ~CMediaParams();
};

#endif // !defined(AFX_MEDIAPARAMS_H__3DB99F00_3887_4C35_BBA8_C47835777A69__INCLUDED_)
