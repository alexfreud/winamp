#ifndef NULLSOFT_API_PLAYLISTMANAGER_H
#define NULLSOFT_API_PLAYLISTMANAGER_H

#include <bfc/dispatch.h>

class ifc_playlistloadercallback;
class ifc_playlist;
class ifc_playlistdirectorycallback;
enum
{
	PLAYLISTMANAGER_SUCCESS                 = 0,

	PLAYLISTMANAGER_FAILED                  = 1,
	PLAYLISTMANAGER_LOAD_NO_LOADER          = 1,
	PLAYLISTMANAGER_LOAD_LOADER_OPEN_FAILED = 2,
};

class api_playlistmanager : public Dispatchable
{
protected:
	api_playlistmanager()                                             {}
	~api_playlistmanager()                                            {}

public:
	int            Load( const wchar_t *filename, ifc_playlistloadercallback *playlist );
	int            LoadAs( const wchar_t *filename, const wchar_t *ext, ifc_playlistloadercallback *playlist ); // call with ext in the format ".pls"
	int            LoadFromDialog( const wchar_t *fns, ifc_playlistloadercallback *playlist );
	int            LoadFromANSIDialog( const char *fns, ifc_playlistloadercallback *playlist );
	
	int            Save( const wchar_t *filename, ifc_playlist *playlist );
	
	size_t         Copy( const wchar_t *destFn, const wchar_t *srcFn ); // returns number of items copied
	
	size_t         CountItems( const wchar_t *filename );
	
	int            GetLengthMilliseconds( const wchar_t *filename );
	uint64_t       GetLongLengthMilliseconds( const wchar_t *filename );
	
	void           Randomize( ifc_playlist *playlist );
	void           Reverse( ifc_playlist *playlist );
	
	void           LoadDirectory( const wchar_t *directory, ifc_playlistloadercallback *callback, ifc_playlistdirectorycallback *dirCallback );
	
	bool           CanLoad( const wchar_t *filename );
	
	void           GetExtensionList( wchar_t *extensionList, size_t extensionListCch );	
	void           GetFilterList( wchar_t *extensionList, size_t extensionListCch );

	const wchar_t *EnumExtension( size_t num );

public:
	DISPATCH_CODES
	{
	  API_PLAYLISTMANAGER_LOAD                   =  10,
	  API_PLAYLISTMANAGER_LOADNULLDELIMITED      =  11,
	  API_PLAYLISTMANAGER_LOADNULLDELIMITED_ANSI =  12,
	  API_PLAYLISTMANAGER_LOADAS                 =  13,
	  API_PLAYLISTMANAGER_SAVE                   =  20,
	  API_PLAYLISTMANAGER_COPY                   =  30,
	  API_PLAYLISTMANAGER_COUNT                  =  40,
	  API_PLAYLISTMANAGER_GETLENGTH              =  50,
	  API_PLAYLISTMANAGER_GETLONGLENGTH          =  51,
	  API_PLAYLISTMANAGER_LOADDIRECTORY          =  60,
	  API_PLAYLISTMANAGER_RANDOMIZE              = 100,
	  API_PLAYLISTMANAGER_REVERSE                = 110,
	  API_PLAYLISTMANAGER_CANLOAD                = 120,
	  API_PLAYLISTMANAGER_GETEXTENSIONLIST       = 130,
	  API_PLAYLISTMANAGER_GETFILTERLIST          = 140,
	  API_PLAYLISTMANAGER_ENUMEXTENSION          = 150,
	};
};

inline void api_playlistmanager::GetFilterList( wchar_t *extensionList, size_t extensionListCch )
{
	extensionList[ 0 ] = 0; // just in case no one implements it
	extensionList[ 1 ] = 0;

	_voidcall( API_PLAYLISTMANAGER_GETFILTERLIST, extensionList, extensionListCch );
}

inline int api_playlistmanager::LoadAs( const wchar_t *filename, const wchar_t *ext, ifc_playlistloadercallback *playlist )
{
	return _call( API_PLAYLISTMANAGER_LOADAS, (int)PLAYLISTMANAGER_FAILED, filename, ext, playlist );
}

inline void api_playlistmanager::GetExtensionList( wchar_t *extensionList, size_t extensionListCch )
{
	extensionList[ 0 ] = 0; // just in case no one implements it
	_voidcall( API_PLAYLISTMANAGER_GETEXTENSIONLIST, extensionList, extensionListCch );
}

inline int api_playlistmanager::Load( const wchar_t *filename, ifc_playlistloadercallback *playlist )
{
	return _call( API_PLAYLISTMANAGER_LOAD, (int)PLAYLISTMANAGER_FAILED, filename, playlist );
}

inline int api_playlistmanager::LoadFromDialog( const wchar_t *filename, ifc_playlistloadercallback *playlist )
{
	return _call( API_PLAYLISTMANAGER_LOADNULLDELIMITED, (int)PLAYLISTMANAGER_FAILED, filename, playlist );
}

inline int api_playlistmanager::LoadFromANSIDialog( const char *filename, ifc_playlistloadercallback *playlist )
{
	return _call( API_PLAYLISTMANAGER_LOADNULLDELIMITED_ANSI, (int)PLAYLISTMANAGER_FAILED, filename, playlist );
}

inline int api_playlistmanager::Save( const wchar_t *filename, ifc_playlist *playlist )
{
	return _call( API_PLAYLISTMANAGER_SAVE, (int)PLAYLISTMANAGER_FAILED, filename, playlist );
}

inline size_t api_playlistmanager::Copy( const wchar_t *destFn, const wchar_t *srcFn )
{
	return _call( API_PLAYLISTMANAGER_COPY, (size_t)0, destFn, srcFn );
}

inline size_t api_playlistmanager::CountItems( const wchar_t *filename )
{
	return _call( API_PLAYLISTMANAGER_COUNT, (size_t)0, filename );
}

inline int api_playlistmanager::GetLengthMilliseconds( const wchar_t *filename )
{
	return _call( API_PLAYLISTMANAGER_GETLENGTH, (int)0, filename );
}

inline uint64_t api_playlistmanager::GetLongLengthMilliseconds( const wchar_t *filename )
{
	return _call( API_PLAYLISTMANAGER_GETLONGLENGTH, (uint64_t)0, filename );
}

inline void api_playlistmanager::Randomize( ifc_playlist *playlist )
{
	_voidcall( API_PLAYLISTMANAGER_RANDOMIZE, playlist );
}

inline void api_playlistmanager::Reverse( ifc_playlist *playlist )
{
	_voidcall( API_PLAYLISTMANAGER_REVERSE, playlist );
}

inline void api_playlistmanager::LoadDirectory( const wchar_t *directory, ifc_playlistloadercallback *callback, ifc_playlistdirectorycallback *dirCallback )
{
	_voidcall( API_PLAYLISTMANAGER_LOADDIRECTORY, directory, callback, dirCallback );
}

inline bool api_playlistmanager::CanLoad( const wchar_t *filename )
{
	return _call( API_PLAYLISTMANAGER_CANLOAD, (bool)true, filename );
}

inline const wchar_t *api_playlistmanager::EnumExtension( size_t num )
{
	return _call( API_PLAYLISTMANAGER_ENUMEXTENSION, (const wchar_t *)0, num );
}

// {C5618774-7177-43aa-9906-933C9F40EBDC}
static const GUID api_playlistmanagerGUID =
{ 0xc5618774, 0x7177, 0x43aa, { 0x99, 0x6, 0x93, 0x3c, 0x9f, 0x40, 0xeb, 0xdc } };

#endif
