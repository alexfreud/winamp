// MediaParams.cpp: implementation of the CMediaParams class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioPlugIn.h"
#include "MediaParams.h"
#include "ParamEnvelope.h"

#include "CakeMedParam_i.c"

#define DEFINE_PARAM_INFO
#include "Parameters.h"

//////////////////////////////////////////////////////////////////////
// Ctors

CMediaParams::CMediaParams( IUnknown* pUnkOuter ) : m_pUnkOuter(pUnkOuter)
{
	m_pCallback = NULL;
	m_cRef = 0;
	m_aEnv = NULL;
	m_dDecimationInterval = 20.0 / 1000.0; // 20 msec
	m_lFs = 44100;
}

CMediaParams::~CMediaParams()
{
	ASSERT( 0 == m_cRef );
	if (m_pCallback)
		m_pCallback->Release();
	m_pCallback = NULL;
	m_pUnkOuter = NULL;

	delete [] m_aEnv;
	m_aEnv = NULL;
}


//////////////////////////////////////////////////////////////////////
// Factory-style construction

HRESULT CMediaParams::Create( CMediaParams** ppObj, IUnknown* pUnkOuter )
{
	if (NULL == ppObj)
		return E_POINTER;
	if (NULL == pUnkOuter)
		return E_POINTER;

	// Construct the CMediaParams object
	CMediaParams* pNew = new CMediaParams( pUnkOuter );
	if (NULL == pNew)
		return E_OUTOFMEMORY;

	// Construct and initialize its parameters
	pNew->m_aEnv = new CParamEnvelope [ NUM_PARAMS ];
	if (NULL == pNew->m_aEnv)
		return E_OUTOFMEMORY;
	for (ULONG ix = 0; ix < NUM_PARAMS; ++ix)
		pNew->m_aEnv[ ix ].SetParamInfo( m_aParamInfo[ ix ] );

	pNew->AddRef();
	*ppObj = pNew;
	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// Given a sample range, fills pTimes with sample positions where we need
// to recompute one or more automated parameter values.  Positions are always
// added periodically at the decimation interval; positions are also added
// for every segment boundary among all of the parameters.

HRESULT CMediaParams::GetDecimationTimes( LONGLONG llSampStart,
													   LONGLONG llSampEnd,
														std::vector<LONGLONG>* pTimes )
{
	LONGLONG const			llInterval = static_cast<LONGLONG>( GetDecimationInterval() * GetSampleRate() );
	double const			dSamplesPerRefTime = static_cast<double>( GetSampleRate() ) / UNITS;
	REFERENCE_TIME const	rtStart = REFERENCE_TIME( llSampStart / dSamplesPerRefTime + 0.5 );

	// Make an worst-case guess at how many decimation points we'll need
	ULONG uPoints = 0;
	for (DWORD dwParam = 0; dwParam < NUM_AUTOMATED_PARAMS; dwParam++)
	{
		const CParamEnvelope& env = m_aEnv[ dwParam ];
		uPoints += env.GetCount() * 2;
	}

	// If there is no automation, then there is no need to decimate
	if (0 == uPoints)
		return S_OK;

	// Account for points that are added due to periodic decimation
	uPoints += ULONG( ((llSampEnd - llSampStart) / llInterval) + 1 );

	// Reserve some memory for landmark points
	pTimes->reserve( uPoints );

	// Add periodic landmarks at the decimation interval
	LONGLONG llSamp = (llSampStart / llInterval) * llInterval;
	if (llSamp < llSampStart)
		llSamp += llInterval;
	while (llSamp < llSampEnd)
	{
		pTimes->push_back( llSamp );
		llSamp += llInterval;
	}

	// Add landmarks for each shape boundary
	for (dwParam = 0; dwParam < NUM_AUTOMATED_PARAMS; dwParam++)
	{
		const CParamEnvelope& env = m_aEnv[ dwParam ];
		unsigned const nCount = env.GetCount();

		// Add each shape endpoint that falls in our time range
		for (unsigned ix = 0; ix < nCount; ix++)
		{
			const MP_ENVELOPE_SEGMENT& seg = env.GetAt( ix );
			LONGLONG const llEnvStart = static_cast<LONGLONG>( seg.rtStart * dSamplesPerRefTime + 0.5 );
			LONGLONG const llEnvEnd = static_cast<LONGLONG>( seg.rtEnd * dSamplesPerRefTime + 0.5 );
			if (llSampStart <= llEnvStart && llEnvStart < llSampEnd)
				pTimes->push_back( llEnvStart );
			if (llSampStart <= llEnvEnd && llEnvEnd < llSampEnd)
				pTimes->push_back( llEnvEnd );
		}
	}

	// Sort result
	std::sort( pTimes->begin(), pTimes->end() );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// Set the current position among all parameters, updating current envelope
// value and deltas.  This method is called repeatedly by the streaming code,
// to update parameter values as they evolve along the duration of the envelope.

HRESULT CMediaParams::UpdateValuesForSample( LONGLONG llSamp )
{
	double const			dSamplesPerRefTime = static_cast<double>( GetSampleRate() ) / UNITS;
	REFERENCE_TIME const	rt = REFERENCE_TIME( llSamp / dSamplesPerRefTime + 0.5 );

	HRESULT hr = S_OK;
	for (DWORD dwParam = 0; dwParam < NUM_AUTOMATED_PARAMS; dwParam++)
	{
		hr = m_aEnv[ dwParam ].UpdateValuesForRefTime( rt, GetSampleRate() );
		if (FAILED( hr ))
			break;
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CMediaParams::QueryInterface( REFIID riid, void** ppv )
{
	if (NULL == ppv)
		return E_POINTER;

	if (riid == IID_IUnknown)
	{
		*ppv = static_cast<IUnknown*>( static_cast<IMediaParams*>( this ) );
		m_pUnkOuter->AddRef();
		return S_OK;
	}
	else
	{
		return m_pUnkOuter->QueryInterface( riid, ppv );
	}
}

ULONG CMediaParams::AddRef()
{
	return InterlockedIncrement( &m_cRef );
}

ULONG CMediaParams::Release()
{
	ASSERT( m_cRef > 0 );
	ULONG ul = InterlockedDecrement( &m_cRef );
	if (0 == ul)
	{
		delete this;
		return 0;
	}
	else
		return ul;
}


////////////////////////////////////////////////////////////////////////////////
// IMediaParams

HRESULT CMediaParams::GetParam(ULONG dwParamIndex, FLOAT* pValue)
{
	if (dwParamIndex >= NUM_PARAMS)
		return E_INVALIDARG;
	if (NULL == pValue)
		return E_POINTER;

	return m_aEnv[ dwParamIndex ].GetParam( pValue );
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMediaParams::SetParam(ULONG dwParamIndex, FLOAT value)
{
	if (dwParamIndex >= NUM_PARAMS)
		return E_INVALIDARG;

	return m_aEnv[ dwParamIndex ].SetParam( value );
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMediaParams::AddEnvelope(ULONG dwParamIndex, ULONG cSegments, MP_ENVELOPE_SEGMENT* pmpes)
{
	if (dwParamIndex >= NUM_AUTOMATED_PARAMS && dwParamIndex != DWORD_ALLPARAMS)
		return E_INVALIDARG;
	if (0 == cSegments)
		return S_OK;
	if (IsBadReadPtr( pmpes, cSegments * sizeof(MP_ENVELOPE_SEGMENT) ))
		return E_POINTER;

	double const dSamplesPerRefTime = static_cast<double>( GetSampleRate() ) / UNITS;

	if (dwParamIndex == DWORD_ALLPARAMS)
	{
		for (ULONG ix = 0; ix < NUM_AUTOMATED_PARAMS; ix++)
		{
			HRESULT hr = m_aEnv[ ix ].AddEnvelope( cSegments, pmpes, dSamplesPerRefTime );
			if (FAILED( hr ))
				return hr;
		}
		return S_OK;
	}
	else
	{
		return m_aEnv[ dwParamIndex ].AddEnvelope( cSegments, pmpes, dSamplesPerRefTime );
	}
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMediaParams::FlushEnvelope(ULONG dwParamIndex, REFERENCE_TIME rtStart, REFERENCE_TIME rtEnd)
{
	if (dwParamIndex >= NUM_AUTOMATED_PARAMS && dwParamIndex != DWORD_ALLPARAMS)
		return E_INVALIDARG;
	if (rtStart > rtEnd)
		return E_INVALIDARG;

	double const dSamplesPerRefTime = static_cast<double>( GetSampleRate() ) / UNITS;

	if (dwParamIndex == DWORD_ALLPARAMS)
	{
		for (ULONG ix = 0; ix < NUM_AUTOMATED_PARAMS; ix++)
		{
			HRESULT hr = m_aEnv[ ix ].FlushEnvelope( rtStart, rtEnd, dSamplesPerRefTime );
			if (FAILED( hr ))
				return hr;
		}
		return S_OK;
	}
	else
	{
		HRESULT hr = m_aEnv[ dwParamIndex ].FlushEnvelope( rtStart, rtEnd, dSamplesPerRefTime );
		if (FAILED( hr ))
			return hr;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMediaParams::SetTimeFormat(GUID guidTimeFormat, ULONG mpTimeData)
{
	if (guidTimeFormat != GUID_TIME_REFERENCE)
		return E_INVALIDARG;
	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// IMediaParamInfo

HRESULT CMediaParams::GetParamCount(ULONG* pdwParams)
{
	if (NULL == pdwParams)
		return E_POINTER;

	*pdwParams = NUM_AUTOMATED_PARAMS;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMediaParams::GetParamInfo(ULONG dwParamIndex, MP_PARAMINFO* pInfo)
{
	if (dwParamIndex >= NUM_AUTOMATED_PARAMS)
		return E_INVALIDARG;
	if (IsBadWritePtr( pInfo, sizeof(MP_PARAMINFO) ))
		return E_POINTER;

	*pInfo = m_aParamInfo[ dwParamIndex ].mppi;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMediaParams::GetParamText(ULONG dwParamIndex, WCHAR** ppwchText)
{
	if (dwParamIndex >= NUM_AUTOMATED_PARAMS)
		return E_INVALIDARG;
	if (NULL == ppwchText)
		return E_POINTER;

	const ParamInfo&		info = m_aParamInfo[ dwParamIndex ];
	const MP_PARAMINFO&	mppi = info.mppi;

	// Count up lengths of label and unit strings, plus null terminators
	int cch = wcslen(mppi.szLabel) + wcslen(mppi.szUnitText) + 3;

	// Add in length of the enum. text if any was supplied
	if (NULL != info.pwszEnumText)
		cch += wcslen(info.pwszEnumText) + 1;

	// Allocate memory for the returned string
	*ppwchText = (WCHAR*)CoTaskMemAlloc( sizeof(WCHAR) * cch );
	if (NULL == *ppwchText)
		return E_OUTOFMEMORY;
	
	// Text format is "Name\0Units\0Enum1\0Enum2\0...EnumN\0\0"
	WCHAR* pwsz = *ppwchText;

	// [1] Copy in the name
	wcscpy( pwsz, mppi.szLabel );
	pwsz += wcslen(mppi.szLabel) + 1;

	// [2] Copy in the units
	wcscpy( pwsz, mppi.szUnitText );
	pwsz += wcslen(mppi.szUnitText) + 1;

	// [3] Copy in the enum. text, if any was supplied
	if (NULL != info.pwszEnumText)
	{
		wcscpy( pwsz, info.pwszEnumText );

		// Replace commas with nulls, to conform to DX8 string format spec
		while (*pwsz)
		{
			if (*pwsz == L',')
				*pwsz = 0;
			pwsz++;
		}
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMediaParams::GetNumTimeFormats(ULONG* pdwNumTimeFormats)
{
	if (NULL == pdwNumTimeFormats)
		return E_POINTER;
	*pdwNumTimeFormats = 1;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMediaParams::GetSupportedTimeFormat(ULONG dwFormatIndex, GUID* pguidTimeFormat)
{
	if (NULL == pguidTimeFormat)
		return E_POINTER;
	if (0 != dwFormatIndex)
		return E_INVALIDARG;
	*pguidTimeFormat = GUID_TIME_REFERENCE;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMediaParams::GetCurrentTimeFormat(GUID* pguidTimeFormat, ULONG*)
{
	if (NULL == pguidTimeFormat)
		return E_POINTER;
	*pguidTimeFormat = GUID_TIME_REFERENCE;
	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// IMediaParamsSetUICallback

HRESULT CMediaParams::SetUICallback(IMediaParamsUICallback* pICallback)
{
	if (pICallback)
		pICallback->AddRef();
	if (m_pCallback)
		m_pCallback->Release();
	m_pCallback = pICallback;
	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// IMediaParamsUICallback

HRESULT CMediaParams::ParamsBeginCapture(DWORD *aIndex, DWORD cPoints)
{
	HRESULT hr = S_OK;

	// Inform each parameter that capture has begun
	for (DWORD ix = 0; ix < cPoints; ix++)
		m_aEnv[ aIndex[ ix ] ].BeginCapture();

	if (m_pCallback)
		hr = m_pCallback->ParamsBeginCapture( aIndex, cPoints );

	return hr;
}

HRESULT CMediaParams::ParamsChanged(DWORD *aIndex, DWORD cPoints, MP_DATA *paData)
{
	HRESULT hr = S_OK;

	// Send the parameter change to each parameter
	for (DWORD ix = 0; ix < cPoints; ix++)
	{
		hr = SetParam( aIndex[ ix ], paData[ ix ] );
		if (FAILED( hr ))
			return hr;
	}

	// Send the parameter change to our callback
	if (m_pCallback)
		hr = m_pCallback->ParamsChanged( aIndex, cPoints, paData );

	return hr;
}

HRESULT CMediaParams::ParamsEndCapture(DWORD *aIndex, DWORD cPoints)
{
	HRESULT hr = S_OK;

	// Inform each parameter that capture has ended
	for (DWORD ix = 0; ix < cPoints; ix++)
		m_aEnv[ aIndex[ ix ] ].EndCapture();

	if (m_pCallback)
		hr = m_pCallback->ParamsEndCapture( aIndex, cPoints );

	return hr;
}

////////////////////////////////////////////////////////////////////////////////
