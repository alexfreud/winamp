// AudioPlugIn.h: interface for the CAudioPlugIn class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUDIOPLUGIN_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_)
#define AFX_AUDIOPLUGIN_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct DXiEvent;
struct MfxEvent;

#include "DXi.h"

// TODO: #define PROCESS_IN_PLACE to FALSE if necessary, i.e., for plug-ins
// which convert mono to stereo.
#define PROCESS_IN_PLACE (TRUE)

class CAudioPlugIn :
	public CDXi
{
public:
	CAudioPlugIn( HRESULT* phr );
	virtual ~CAudioPlugIn();

	HRESULT Initialize();

	HRESULT IsValidInputFormat( const WAVEFORMATEX* pwfx ) const;
	HRESULT IsValidOutputFormat( const WAVEFORMATEX* pwfx ) const;
	HRESULT IsValidTransform( const WAVEFORMATEX* pwfxIn, const WAVEFORMATEX* pwfxOut ) const;
	HRESULT SuggestOutputFormat( WAVEFORMATEX* pwfx ) const;

	HRESULT Process( LONGLONG llSampAudioTimestamp,
						  AudioBuffer* pbufIn,
						  AudioBuffer* pbufOut );

	HRESULT AllocateResources();
	HRESULT FreeResources();

	int	  PersistGetSize() const;
	HRESULT PersistLoad( IStream* pStream );
	HRESULT PersistSave( IStream* pStream );
};

#endif // !defined(AFX_AUDIOPLUGIN_H__D9177ACC_DFF4_4C13_8FB9_F949C35BFEF0__INCLUDED_)
