// ParamEnvelope.cpp: implementation of the CParamEnvelope class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioPlugIn.h"
#include "ParamEnvelope.h"

#include <math.h>

////////////////////////////////////////////////////////////////////////////////
// ParamInfo
////////////////////////////////////////////////////////////////////////////////

float ParamInfo::MapToInternal( float fValue ) const
{
	// Convert a user-supplied parameter value to one that is for internal
	// use by the plug-in's processing code.

	if (MPT_FLOAT == mppi.mpType)
	{
		// Map floats to the internal range, using a linear mapping
		double dDelta = (fValue - mppi.mpdMinValue) / (mppi.mpdMaxValue - mppi.mpdMinValue);
		return float( fInternalMin + dDelta * (fInternalMax - fInternalMin) );
	}
	else if (MPT_BOOL == mppi.mpType)
	{
		// Map booleans to 0.0 or 1.0
		return float( (fValue < 0.5) ? MPBOOL_FALSE : MPBOOL_TRUE );
	}
	else // (MPT_ENUM == mppi.mpType || MPT_INT == mppi.mpType)
	{
		// Map integers to the internal range, using a linear mapping, and then
		// round to the nearest value.
		double dDelta = (fValue - mppi.mpdMinValue) / (mppi.mpdMaxValue - mppi.mpdMinValue);
		double dMapped = fInternalMin + dDelta * (fInternalMax - fInternalMin);
		return static_cast<float>( floor( dMapped + 0.5 ) );
	}
}

////////////////////////////////////////////////////////////////////////////////

float ParamInfo::MapToExternal( float fValue ) const
{
	// Convert an internal processing value to value in the user's input range

	if (MPT_FLOAT == mppi.mpType)
	{
		// Map floats to the external range, using a linear mapping
		double dDelta = (fValue - fInternalMin) / (fInternalMax - fInternalMin);
		return float( mppi.mpdMinValue + dDelta * (mppi.mpdMaxValue - mppi.mpdMinValue) );
	}
	else if (MPT_BOOL == mppi.mpType)
	{
		// Booleans are already in a suitable range; no mapping required.
		return fValue;
	}
	else // (MPT_ENUM == mppi.mpType || MPT_INT == mppi.mpType)
	{
		// Map integers to the external range, using a linear mapping
		double dDelta = (fValue - fInternalMin) / (fInternalMax - fInternalMin);
		return float( mppi.mpdMinValue + dDelta * (mppi.mpdMaxValue - mppi.mpdMinValue) );
	}
}

////////////////////////////////////////////////////////////////////////////////
// CParamEnvelope
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Ctors

CParamEnvelope::CParamEnvelope() :
	m_bOverride( FALSE ),
	m_bCaptured( FALSE ),
	m_fOverrideValue( 0 ),
	m_fEnvelopeValue( 0 ),
	m_dEnvelopeDelta1( 0 ),
	m_dEnvelopeDelta2( 0 ),
	m_bValidDeltas( TRUE ),
	m_rtRendered( 0 )
{
}

CParamEnvelope::~CParamEnvelope()
{
}


//----------------------------------------------------------------------------
// Phase 2 of construction

void CParamEnvelope::SetParamInfo( const ParamInfo& info )
{
	m_info = info;
	m_fEnvelopeValue = m_info.MapToInternal( m_info.mppi.mpdNeutralValue );
	cleanup();
}


//----------------------------------------------------------------------------
// Find the index for data on or after rt

int CParamEnvelope::IndexForRefTime( REFERENCE_TIME rt ) const
{
	CAutoLock lock( const_cast<CParamEnvelope*>( this ) );

	int const nLength = GetCount();

	// Fail gracefully if the list is empty
	if (0 == nLength)
		return -1;

	// Special case for position after the last segment
	if (rt >= m_envSegs[ nLength - 1 ].rtEnd)
		return nLength - 1;

	int ixMin = 0;
	int ixMax = nLength;
	int ix = ( ixMin + ixMax ) / 2;
	
	// Binary search for the shape which starts on or before the given time
	do
	{
		REFERENCE_TIME rtShape = m_envSegs[ ix ].rtStart;

		// We've made an exact match
		if (rtShape == rt)
			return ix;

		// No match was found, so update search indices
		else if (rt < rtShape)
			ixMax = ix;
		else if (rt > rtShape)
			ixMin = ix;
		ix = (ixMin + ixMax) / 2;
	}
	while (ix != ixMin);

	// The search may have left us at a shape after the desired time, so
	// scan back if necessary
	while (ix >= 0 && m_envSegs[ ix ].rtStart > rt)
		--ix;

	return ix;
}


//------------------------------------------------------------------------------
// Set the current position, updating current envelope value and deltas.  This
// method is called repeatedly by the streaming code, to update parameter values
// as they evolve along the duration of the envelope.

HRESULT CParamEnvelope::UpdateValuesForRefTime( REFERENCE_TIME rt, long lSampleRate )
{
	CAutoLock lock( this );

	int const nLength = GetCount();
	if (0 == nLength)
		return S_OK; // nothing to do

	int const ix = IndexForRefTime( rt );
	const MP_ENVELOPE_SEGMENT* pmpseg = (ix < 0 || ix >= nLength) ? NULL : &m_envSegs[ ix ];

	// Assume deltas are valid.  We'll make them invalid if we encounter a SIN curve.
	m_bValidDeltas = TRUE;

	if (NULL == pmpseg || rt < pmpseg->rtStart || rt > pmpseg->rtEnd)
	{
		// The seek position is between 2 segments, so do not modify the current envelope
		// value.  The envelope will either latch the previous value, or continue to obey
		// any intervening override value, until we hit the next segment boundary.
		if (NULL != pmpseg)
			m_fEnvelopeValue = m_info.MapToInternal( pmpseg->valEnd );
		m_dEnvelopeDelta1 = m_dEnvelopeDelta2 = 0;
	}
	else
	{
		// We're dealing with point directly over a shape.  Stop any override value.
		stopOverride();
		
		// Compute the time delta between this vector and the next, as value
		// between 0..1.  We use to interpolate between points.
		double dx = double(pmpseg->rtEnd - pmpseg->rtStart) / UNITS;
		double y0 = m_info.MapToInternal( pmpseg->valStart );
		double y1 = m_info.MapToInternal( pmpseg->valEnd );
		double dy = y1 - y0;
		double x = (double(rt - pmpseg->rtStart) / UNITS) / dx;
		
		// Convert dx to units per sample, before computing deltas
		dx = dx * lSampleRate;

		// Interpolate between times
		if (MP_CURVE_JUMP == pmpseg->iCurve)
		{
			m_dEnvelopeDelta2	= 0;
			m_dEnvelopeDelta1	= 0;
			m_fEnvelopeValue	= static_cast<float>( y0 );
		}
		else if (MP_CURVE_LINEAR == pmpseg->iCurve)
		{
			m_dEnvelopeDelta2	= 0;
			m_dEnvelopeDelta1	= dy / dx;
			m_fEnvelopeValue	= static_cast<float>( y0 + dy * x );
		}
		else if (MP_CURVE_SQUARE == pmpseg->iCurve || MP_CURVE_INVSQUARE == pmpseg->iCurve)
		{
			double A;
			double B;
			if (MP_CURVE_SQUARE == pmpseg->iCurve)
			{
				A = y0;
				B = dy;
			}
			else
			{
				x = x - 1;
				A = y1;
				B = -dy;
			}
			
			m_dEnvelopeDelta2	= 2.0 * B / (dx * dx);
			m_dEnvelopeDelta1	= (B / dx) * (2.0 * x + (1.0 / dx));
			m_fEnvelopeValue	= static_cast<float>( A + B * x * x );
		}
		else if (MP_CURVE_SINE)
		{
			static const double dPI = 3.14159265358979323846264338327950288419716939937510;
			double dTheta = dPI * (x - 0.5);
			m_bValidDeltas = FALSE;
			m_fEnvelopeValue = float( dy * ( sin( dTheta ) + 0.5 ) );
		}
	}

	// Keep track of the latest time rendered so far
	m_rtRendered = max( m_rtRendered, rt );

	return S_OK;
}


//------------------------------------------------------------------------------
// If the list is empty, make sure we get an override value.

void CParamEnvelope::cleanup()
{
	if (0 == GetCount() && !IsOverrideActive())
	{
		m_fOverrideValue = m_fEnvelopeValue;
		m_bOverride = TRUE;
	}
}


//---------------------------------------------------------------------------
// Set the value of our parameter, overriding any segment in effect.

HRESULT CParamEnvelope::SetParam( float fValue )
{
	m_bOverride = TRUE;
	m_fOverrideValue = m_info.MapToInternal( fValue );
	m_fEnvelopeValue = m_fOverrideValue;
	m_dEnvelopeDelta1 = m_dEnvelopeDelta2 = 0;
	return S_OK;
}


//------------------------------------------------------------------------------
// Get the value of the parameter, either overriden on an a segment

HRESULT CParamEnvelope::GetParam( float* pfValue )
{
	if (NULL == pfValue)
		return E_POINTER;
	*pfValue = m_info.MapToExternal( GetCurrentValue() );
	return S_OK;
}


//------------------------------------------------------------------------------
// Add segments to this envelope

static bool compareEnvSeg( const MP_ENVELOPE_SEGMENT& a, const MP_ENVELOPE_SEGMENT& b )
{
	return a.rtStart < b.rtStart;
}

static bool operator==( const MP_ENVELOPE_SEGMENT& a, const MP_ENVELOPE_SEGMENT& b )
{
	return 0 == memicmp( &a, &b, sizeof(MP_ENVELOPE_SEGMENT) );
}

HRESULT CParamEnvelope::AddEnvelope( DWORD cSegments, MP_ENVELOPE_SEGMENT* pmpes, double dSamplesPerRefTime )
{
	CAutoLock lock( this );

	// Make room for what we are going to add
	m_envSegs.reserve( m_envSegs.size() + cSegments );

	// Add each segment, noting which one is earliest in time
	REFERENCE_TIME rtMin = _I64_MAX;
	for (int ix = 0; ix < cSegments; ix++)
	{
		// Round reference times to sample boundaries
		MP_ENVELOPE_SEGMENT mpes = pmpes[ ix ];
		mpes.rtStart = REFERENCE_TIME(mpes.rtStart * dSamplesPerRefTime) / dSamplesPerRefTime + 0.5;
		mpes.rtEnd = REFERENCE_TIME(mpes.rtEnd * dSamplesPerRefTime) / dSamplesPerRefTime + 0.5;
		m_envSegs.push_back( pmpes[ ix ] );
		if (mpes.rtStart < rtMin)
			rtMin = mpes.rtStart;
	}

	// Flush all segments prior to the first newly added one
	ix = IndexForRefTime( rtMin );
	if (ix > 0 && rtMin < m_rtRendered)
		m_envSegs.erase( m_envSegs.begin(), m_envSegs.begin() + ix );

	// Sort them
	std::sort( m_envSegs.begin(), m_envSegs.end(), compareEnvSeg );

	// Remove duplicates
	EnvelopeSegs::iterator it = m_envSegs.begin();
	while (it != m_envSegs.end())
	{
		EnvelopeSegs::iterator itBegin = it + 1;
		EnvelopeSegs::iterator itEnd = itBegin;
		while (itEnd != m_envSegs.end() && *itEnd == *it)
		  itEnd++;
		if (itEnd != itBegin)
			it = m_envSegs.erase( itBegin, itEnd );
		else
			it++;
		
	}

	return S_OK;
}


//------------------------------------------------------------------------------
// Flush segments within the specified time range.  The rules for flushing are
// described as follows in the documentation for IMediaParams:
//
// If the time span specified by refTimeStart and refTimeEnd overlaps an envelope
// segment, the entire segment is flushed. On the other hand, if it falls on
// the boundary of an envelope segment, the entire segment is retained. Thus: 
//
// [] If the start time falls inside an envelope segment, the segment is flushed. 
// [] If the end time falls inside an envelope segment, the segment is flushed. 
// [] If the start time equals the end time of an envelope segment, the segment is retained. 
// [] If the end time equals the start time of an envelope segment, the segment is retained. 

HRESULT CParamEnvelope::FlushEnvelope( REFERENCE_TIME rtStart, REFERENCE_TIME rtEnd, double dSamplesPerRefTime )
{
	CAutoLock lock( this );

	// Round reference times to sample boundaries
	if (rtStart != _I64_MIN && rtStart != _I64_MAX)
		rtStart = REFERENCE_TIME( REFERENCE_TIME(rtStart / dSamplesPerRefTime) * dSamplesPerRefTime + 0.5 );
	if (rtEnd != _I64_MIN && rtEnd != _I64_MAX)
		rtEnd = REFERENCE_TIME( REFERENCE_TIME(rtEnd / dSamplesPerRefTime) * dSamplesPerRefTime + 0.5 );

	EnvelopeSegs::iterator it = m_envSegs.begin();
	while (it != m_envSegs.end())
	{
		if (!(rtStart >= it->rtEnd || rtEnd <= it->rtStart))
			it = m_envSegs.erase( it );
		else
			it++;
	}

	// Once envelopes get thrown away, we need to redetermine our max render time
	m_rtRendered = 0;

	cleanup();

	return S_OK;
}


//------------------------------------------------------------------------------
// The parameter pcSegments passes values both ways.  The caller needs to pass in the
// size of the segment array.  GetEnvelope() then uses pcSegments to return the number
// of segments it has placed in the array.

HRESULT CParamEnvelope::GetEnvelope( DWORD *pcSegments, MP_ENVELOPE_SEGMENT *pmpes )
{
	CAutoLock lock( this );

	ASSERT( pcSegments );

	DWORD ix = 0;
	for (EnvelopeSegs::iterator it = m_envSegs.begin();
		  it != m_envSegs.end() && ix < *pcSegments;
		  it++, ix++)
	{
		pmpes[ ix ] = *it;
	}

	*pcSegments = ix;

	return S_OK;
}
