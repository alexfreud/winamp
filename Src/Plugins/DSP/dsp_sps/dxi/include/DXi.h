#ifndef _DXI_H_
#define _DXI_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <DeferZeroFill.h>

// DirectX automation helper
#include "MediaParams.h"

////////////////////////////////////////////////////////////////////////////////

struct AudioBuffer
{
	long				cSamp;	// number of samples in the buffer
	long				lOffset;	// offset into the data to process
	IMediaSample*	pms;		// the raw IMediaSample for this buffer

	AudioBuffer() : cSamp(0), lOffset(0), pms(NULL) {}

	//----------------------------------------------------------------------------
	// Get a pointer to the audio samples, zero-filling if necesssary

	float* GetPointer()
	{
		// Get the raw-pointer
		BYTE* pb = NULL;
		pms->GetPointer( &pb );

		// We cannot defer the zero fill any longer!
		if (bZero)
		{
			IDeferZeroFill* pdzf;
			if (SUCCEEDED( pms->QueryInterface( IID_IDeferZeroFill, (void**)&pdzf ) ))
			{
				// IDeferZeroFill will have taken care of the zero-fill for us, by
				// virtue of our calling IMediaSample::GetPointer.  Nothing more to do.
				pdzf->Release();
			}
			else
			{
				// No IDeferZeroFill is available.  We must zero-fill the hard way.
				memset( pb, 0, cSamp * sizeof(float) );
			}
			bZero = FALSE;
		}

		return reinterpret_cast<float*>( pb + lOffset );
	}

	//----------------------------------------------------------------------------
	// Allow buffers to be tagged as being all zeroes, without actually filling
	// any data until someone asks for the buffer pointer

	BOOL GetZerofill() const { return bZero; }

	void SetZerofill( BOOL bZerofill )
	{
		bZero = bZerofill;
		IDeferZeroFill* pdzf;
		if (SUCCEEDED( pms->QueryInterface( IID_IDeferZeroFill, (void**)&pdzf ) ))
		{
			pdzf->put_NeedsZerofill( bZero );
			pdzf->Release();
		}
	}

private:

	BOOL bZero;
};


////////////////////////////////////////////////////////////////////////////////

class CDXi : public CCritSec
{
public:

	virtual HRESULT Initialize() = 0;

	virtual HRESULT IsValidInputFormat( const WAVEFORMATEX* pwfx ) const = 0;
	virtual HRESULT IsValidOutputFormat( const WAVEFORMATEX* pwfx ) const = 0;
	virtual HRESULT IsValidTransform( const WAVEFORMATEX* pwfxIn, const WAVEFORMATEX* pwfxOut ) const = 0;
	virtual HRESULT SuggestOutputFormat( WAVEFORMATEX* pwfx ) const = 0;

	virtual const WAVEFORMATEX* GetInputFormat() const { return &m_wfxIn; }
	virtual const WAVEFORMATEX* GetOutputFormat() const { return &m_wfxOut; }

	virtual HRESULT	Process( LONGLONG llSampAudioTimestamp,
										AudioBuffer* pbufIn,
										AudioBuffer* pbufOut ) = 0;

	virtual HRESULT	AllocateResources() = 0;
	virtual HRESULT	FreeResources() = 0;

	virtual int			PersistGetSize() const = 0;
	virtual HRESULT	PersistLoad( IStream* pStream ) = 0;
	virtual HRESULT	PersistSave( IStream* pStream ) = 0;

protected:
	WAVEFORMATEX		m_wfxIn;
	WAVEFORMATEX		m_wfxOut;
	CMediaParams*		m_pMediaParams;

	float GetParamValue( DWORD dwParam ) const
	{
		return m_pMediaParams->GetParamEnvelope( dwParam ).GetCurrentValue();
	}

	HRESULT GetParamDeltas( DWORD dwParam, double* pdDelta1, double* pdDelta2 ) const
	{
		return m_pMediaParams->GetParamEnvelope( dwParam ).GetCurrentDeltas( pdDelta1, pdDelta2 );
	}
};

////////////////////////////////////////////////////////////////////////////////

#endif //_DXI_H_
