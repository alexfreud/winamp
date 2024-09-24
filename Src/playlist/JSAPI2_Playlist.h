#pragma once

#include <ocidl.h>
#include <atomic>

#include "Playlist.h"

namespace JSAPI2
{
	// {8535DB01-7630-45df-9429-2640A29B9468}
	static const GUID IID_PlaylistObject =
	{ 0x8535db01, 0x7630, 0x45df, { 0x94, 0x29, 0x26, 0x40, 0xa2, 0x9b, 0x94, 0x68 } };


	class PlaylistObject : public IDispatch
	{
		public:
		PlaylistObject( const wchar_t *_key );
		STDMETHOD( QueryInterface )( REFIID riid, PVOID *ppvObject );
		STDMETHOD_( ULONG, AddRef )( void );
		STDMETHOD_( ULONG, Release )( void );
		// *** IDispatch Methods ***
		STDMETHOD( GetIDsOfNames )( REFIID riid, OLECHAR FAR *FAR *rgszNames, unsigned int cNames, LCID lcid, DISPID FAR *rgdispid );
		STDMETHOD( GetTypeInfo )( unsigned int itinfo, LCID lcid, ITypeInfo FAR *FAR *pptinfo );
		STDMETHOD( GetTypeInfoCount )( unsigned int FAR *pctinfo );
		STDMETHOD( Invoke )( DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR *pexecinfo, unsigned int FAR *puArgErr );

		Playlist playlist;
		private:
		const wchar_t *key;
		volatile std::atomic<std::size_t> _refCount = 1;

		STDMETHOD( Clear )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( AppendURL )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( GetItemFilename )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( GetItemTitle )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( GetItemLength )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( GetItemExtendedInfo )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( Reverse )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( SwapItems )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( Randomize )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( RemoveItem )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( SortByTitle )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( SortByFilename )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( SetItemFilename )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( SetItemTitle )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( SetItemLength )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( InsertURL )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
		STDMETHOD( numitems )( WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr );
	};
}
