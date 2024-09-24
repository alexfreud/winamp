#ifndef _FILTER_H_
#define _FILTER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DXi.h"
#include "AudioPlugIn.h"

class CFilter;
class CFilterOutputPin;
class CFilterInputPin;

//--------------------------------------------------------------------------------

class CFilterInputPin : public CBaseInputPin
{
	friend class CFilter;
	friend class CFilterOutputPin;

public:
	// Ctor
	CFilterInputPin( TCHAR* pObjDesc, CFilter* pFilter, HRESULT* phr, LPCWSTR pPinName );

	// Check that we can support this output type
	HRESULT CheckMediaType( const CMediaType* pmtIn );

	// Set the connection media type
	HRESULT SetMediaType( const CMediaType* pmt );

	// Negotiate possible reconnection if types don't match
	HRESULT CompleteConnect( IPin* pRecievePin );

	// What is our media type?
	CMediaType& CurrentMediaType() { return m_mt; };

	// Pass through calls downstream
	STDMETHODIMP EndOfStream();
	STDMETHODIMP BeginFlush();
	STDMETHODIMP EndFlush();
	STDMETHODIMP NewSegment( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate );

	// Handles the next block of data from the stream
	STDMETHODIMP Receive( IMediaSample *pms );

// Implementation
private:
	CFilter*			m_pFilter;						// the filter which owns us
	BOOL				m_bInsideCheckMediaType;	// re-entrancy control
};

//--------------------------------------------------------------------------------

class CFilterOutputPin : public CBaseOutputPin
{
	friend class CFilter;
	friend class CFilterInputPin;

public:

	// Ctors
	CFilterOutputPin( TCHAR* pObjDesc, CFilter* pFilter, HRESULT* phr, LPCWSTR pPinName );
	virtual ~CFilterOutputPin();

	DECLARE_IUNKNOWN;

	// Override to expose IMediaPosition
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	// Override to enumerate media types
	STDMETHODIMP EnumMediaTypes( IEnumMediaTypes** ppEnum );

	// Check that we can support an output type
	HRESULT CheckMediaType( const CMediaType* pmt );
	HRESULT SetMediaType( const CMediaType* pmt );
	HRESULT GetMediaType( int iPosition, CMediaType* pmt );

	// What is our media type?
	CMediaType& CurrentMediaType() { return m_mt; };

	// Negotiation to use our input pins allocator
	HRESULT DecideBufferSize( IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProp );

	// Get the unique identifier for this output pin
	DWORD GetId() const { return m_id; }

// Implementation
private:
	CFilter*		m_pFilter;						// the filter which owns us
	BOOL			m_bInsideCheckMediaType;	// re-entrancy control
	DWORD			m_id;								// unique identifier
	IUnknown*	m_pPosition;

	static DWORD m_idNext;
};

//--------------------------------------------------------------------------------

class CFilter :
	public CAudioPlugIn,

	public CBaseFilter,
	public ISpecifyPropertyPages,
	public IDispatch,
	public CPersistStream
{
	friend class CFilterInputPin;
	friend class CFilterOutputPin;

public:

	// Ctors	
	CFilter( TCHAR *pName, LPUNKNOWN pUnk, HRESULT* phr );
	~CFilter();

	// Function needed for the class factory
	static CUnknown * WINAPI CreateInstance( LPUNKNOWN pUnk, HRESULT* phr );

	DECLARE_IUNKNOWN;

	STDMETHODIMP NonDelegatingQueryInterface( REFIID riid, void** ppv );

	LPAMOVIESETUP_FILTER GetSetupData();

	// CBaseFilter pure virtual overrides
	CBasePin* GetPin( int n );
	int GetPinCount();

	// Overrides to deal with not having input connections
	STDMETHODIMP Run( REFERENCE_TIME tStart );
	STDMETHODIMP Pause();
	STDMETHODIMP Stop();

	// Helpers for media type checking
	HRESULT CheckMediaType( PIN_DIRECTION direction, const CMediaType* pmt );
	HRESULT CheckTransform( const CMediaType* pmtIn, const CMediaType* pmtOut );
	HRESULT GetMediaType( int iPosition, CMediaType* pmt );
	HRESULT SetMediaType( PIN_DIRECTION direction, const CMediaType* pmt );

	// ISpecifyPropertyPages
	STDMETHODIMP GetPages( CAUUID* pPages );

	// CPersistStream
	STDMETHODIMP GetClassID(CLSID* pClsid);
	int SizeMax();
	HRESULT WriteToStream(IStream* pStream);
	HRESULT ReadFromStream(IStream* pStream);

	// IDispatch
	STDMETHODIMP GetTypeInfoCount( UINT* );
	STDMETHODIMP GetTypeInfo( UINT, LCID, ITypeInfo** );
	STDMETHODIMP GetIDsOfNames( REFIID, OLECHAR**, UINT, LCID, DISPID* );
	STDMETHODIMP Invoke( DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT* );

// Implementation
private:

	HRESULT getOutputBuffer( CFilterOutputPin* pPin,
									 AudioBuffer* pbufOut,
									 REFERENCE_TIME* prtStart,
									 BOOL bSyncPoint, BOOL bDiscontuity, BOOL bPreroll );
	HRESULT deliverOutputBuffer( CFilterOutputPin* pPin,
										  AudioBuffer* abufOut,
										  HRESULT hrProcess, BOOL bCleanup );

	HRESULT audioPortsChangeBegin( int nNewPinCount );
	HRESULT audioPortsChangeEnd( int nNewPinCount );

	CFilterInputPin						m_pinInput;
	CFilterOutputPin						m_pinOutput;
	IMemAllocator*							m_pAllocator;
	LONGLONG									m_llSamplePosition;
};

#endif // _FILTER_H_
