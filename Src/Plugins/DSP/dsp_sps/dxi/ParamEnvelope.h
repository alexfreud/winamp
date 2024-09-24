// ParamEnvelope.h: interface for the CParamEnvelope class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARAMENVELOPE_H__F04D0178_4674_45AF_9F7A_5C8206DE4CF6__INCLUDED_)
#define AFX_PARAMENVELOPE_H__F04D0178_4674_45AF_9F7A_5C8206DE4CF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

////////////////////////////////////////////////////////////////////////////////

typedef std::vector<MP_ENVELOPE_SEGMENT> EnvelopeSegs;

////////////////////////////////////////////////////////////////////////////////

struct ParamInfo
{
	MP_PARAMINFO	mppi;				// external parameter info, as presented to the user
	float				fInternalMin;	//	minimum value used by internal processing code
	float				fInternalMax;	//	maximum value used by internal processing code
	const WCHAR*	pwszEnumText;	// text for enumerations

	float MapToInternal( float fValue ) const;
	float MapToExternal( float fValue ) const;
};

////////////////////////////////////////////////////////////////////////////////

class CParamEnvelope : public CCritSec
{
public:
	CParamEnvelope();
	virtual ~CParamEnvelope();

// Attributes
public:

	unsigned GetCount() const
	{
		return m_envSegs.size();
	}

	const MP_ENVELOPE_SEGMENT& GetAt( unsigned ix ) const
	{
		return m_envSegs[ ix ];
	}

	// Tell the envelope about the parameter being controlled
	void SetParamInfo( const ParamInfo& info );

	// These methods are called by CMediaParams to manipulate segments in an envelope
	HRESULT AddEnvelope( DWORD cSegments, MP_ENVELOPE_SEGMENT* pEnvelopeSegments, double dSamplesPerRefTime );
	HRESULT FlushEnvelope( REFERENCE_TIME refTimeStart, REFERENCE_TIME refTimeEnd, double dSamplesPerRefTime );
	HRESULT GetEnvelope( DWORD* cSegments, MP_ENVELOPE_SEGMENT* pEnvelopeSegments );
	HRESULT SetParam( float fValue );
	HRESULT GetParam( float* pfValue );

	// Manage UI capture and release
	void BeginCapture() { m_bCaptured = TRUE; }
	void EndCapture() { m_bCaptured = FALSE; }

	// Set the current position, updating current envelope value and deltas
	HRESULT UpdateValuesForRefTime( REFERENCE_TIME rt, long lSampleRate );

	// Check if automation envelopes have been overriden with a specific value
	BOOL IsOverrideActive() const { return m_bOverride || m_bCaptured; }

	// Get the current automation data point
	float GetCurrentValue() const
	{ 
		if (IsOverrideActive())
			return m_fOverrideValue;
		else
			return m_fEnvelopeValue;
	}

	// Get the current automation deltas (for per-sample rendering)
	HRESULT GetCurrentDeltas( double* pdDelta1, double* pdDelta2 ) const
	{ 
		if (!m_bValidDeltas)
			return E_FAIL;
		*pdDelta1 = m_dEnvelopeDelta1;
		*pdDelta2 = m_dEnvelopeDelta2;
		return S_OK;
	}

	int IndexForRefTime( REFERENCE_TIME rt ) const;

private:

	// Stop overriding data
	void stopOverride()
	{
		if (IsOverrideActive() && GetCount() > 0)
			m_bOverride = FALSE;
	}

	// Make sure the automation track is in a state suitable for playback
	void cleanup();

private:

	ParamInfo				m_info;					// information about this parameter
	EnvelopeSegs			m_envSegs;				// the list of envelope segments
	float						m_fEnvelopeValue;		// our evolving dynamic value
	double					m_dEnvelopeDelta1;	// 1st delta of current envelope (w.r.t. seconds)
	double					m_dEnvelopeDelta2;	// 2nd delta of current envelope (w.r.t. seconds)
	BOOL						m_bValidDeltas;		// TRUE when deltas can be used (e.g. except for sin)
	BOOL						m_bOverride;			// TRUE while automation point value is overridden
	BOOL						m_bCaptured;			// TRUE while the captured by the UI
	float						m_fOverrideValue;		// our override value
	REFERENCE_TIME			m_rtRendered;			// latest time rendered so far

private:

	CParamEnvelope( const CParamEnvelope& );
	CParamEnvelope& operator=( const CParamEnvelope& );
};

////////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_PARAMENVELOPE_H__F04D0178_4674_45AF_9F7A_5C8206DE4CF6__INCLUDED_)
