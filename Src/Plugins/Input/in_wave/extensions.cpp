#include "main.h"
#include "resource.h"
#include "config.h"
#include "assert.h"
#include "../Agave/Language/api_language.h"
#include "../nu/AutoWide.h"

#include <strsafe.h>

char defaultExtensions[1024] = {0};
char fileExtensionsString[1200] = {0};

int SizeAvailableTypes()
{
	int size     = 0;
	int numTypes = 0;

	sf_command( 0, SFC_GET_FORMAT_MAJOR_COUNT, &numTypes, sizeof( numTypes ) );
	
	SF_FORMAT_INFO info;
	for ( int i = 0; i < numTypes; i++ )
	{
		info.format = i;
		sf_command( 0, SFC_GET_FORMAT_MAJOR, &info, sizeof( info ) );
		size += lstrlenA( info.extension ) + 1 /* ; or \0 */;
		//size+=lstrlen(info.name) + 1 /* \0 */;
	}

	return size;
}

int ExtensionExists( const char *ext, const char *extensionList )
{
	size_t len = lstrlenA( ext );
	char temp[ 20 ] = { 0 };
	const char *s = extensionList;
	
	while ( s && *s )
	{
		lstrcpynA( temp, s, 20 );
		
		char *scan = temp;
		while ( scan && *scan && *scan != ';' )
			scan = CharNextA( scan );

		*scan = 0;

		if ( !lstrcmpiA( temp, ext ) )
			return 1;

		s += lstrlenA( temp );
		if ( *s == ';' )
			s = CharNextA( s );
	}

	return 0;
}

// TODO: sort default extension list
void BuildDefaultExtensions()
{
	// if we want a light extension list, here it is: lstrcpyn(defaultExtensions, "WAV;AIFF;VOC;AU;AIF;AIFC;SND");
	int size = SizeAvailableTypes() + 1;
	assert( size < 1024 ); // TODO

	char *extPtr = defaultExtensions;
	size_t extPtrCch = 1023;
	extPtr[ 0 ] = 0;

	int numTypes = 0;
	
	sf_command( 0, SFC_GET_FORMAT_MAJOR_COUNT, &numTypes, sizeof( numTypes ) );
	
	SF_FORMAT_INFO info = { 0 };

	for ( int i = 0; i < numTypes; i++ )
	{
		info.format = i;
		
		sf_command( 0, SFC_GET_FORMAT_MAJOR, &info, sizeof( info ) );
		if ( !_strcmpi( info.extension, "mpc" ) )
			continue;

		if ( ExtensionExists( info.extension, defaultExtensions ) )
			continue;

		if ( *CharPrevA( defaultExtensions, extPtr ) )
			StringCchCatExA( extPtr, extPtrCch, ";", &extPtr, &extPtrCch, 0 );

		StringCchCatExA( extPtr, extPtrCch, info.extension, &extPtr, &extPtrCch, 0 );
	}
}

void SetFileExtensions( const char *extList )
{
	ZeroMemory( &fileExtensionsString, sizeof( fileExtensionsString ) );
	
	char *end = 0;
	
	StringCchCopyExA( fileExtensionsString, 1200, extList, &end, 0, 0 );
	StringCchCopyExA( end + 1, 1200, WASABI_API_LNGSTRING( IDS_SOUND_FILES ), 0, 0, 0 );

	plugin.FileExtensions = fileExtensionsString;
}

static const wchar_t *pExtList[]={L"AIFF",L"AIF",L"AU",L"AVR",L"CAF",L"HTK",L"IFF",L"MAT",L"PAF",L"PVF",L"RAW",L"RF64",L"SD2",L"SDS",L"SF",L"VOC",L"W64",L"WAV",L"WVE",L"XI"};
static const int pExtDescIdList[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
static const int pExtDescList[] = 
{ 
	IDS_FAMILY_STRING_AAIF,
	IDS_FAMILY_STRING_AAIF,
	IDS_FAMILY_STRING_AU,
	IDS_FAMILY_STRING_AVR,
	IDS_FAMILY_STRING_ACA,
	IDS_FAMILY_STRING_HMMTS,
	IDS_FAMILY_STRING_AAIF,
	IDS_FAMILY_STRING_MATLAB,
	IDS_FAMILY_STRING_PARIS,
	IDS_FAMILY_STRING_PVF,
	IDS_FAMILY_STRING_HEADERLESS_RAW,
	IDS_FAMILY_STRING_RF64,
	IDS_FAMILY_STRING_SDII,
	IDS_FAMILY_STRING_RAW_MIDI_SAMPLE_DUMP,
	IDS_FAMILY_STRING_IRCAM,
	IDS_FAMILY_STRING_CVOC,
	IDS_FAMILY_STRING_SFWOW64,
	IDS_FAMILY_STRING_MSWAV,
	IDS_FAMILY_STRING_MSWAV,
	IDS_FAMILY_STRING_FT2WAV,
};

BOOL GetExtensionName( LPCWSTR pszExt, LPWSTR pszDest, INT cchDest )
{
	INT maxCount, index;
	DWORD lcid;
	SF_FORMAT_INFO info;

	lcid = MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), SORT_DEFAULT );
	// first scan our list if there is no entry ask libsnd. we do this cause libsnd format description sucks

	for ( index = sizeof( pExtList ) / sizeof( wchar_t * ) - 1; index >= 0 && CSTR_EQUAL != CompareStringW( lcid, NORM_IGNORECASE, pszExt, -1, pExtList[ index ], -1 ); index-- );
	if ( index >= 0 && S_OK == StringCchCopyW( pszDest, cchDest, WASABI_API_LNGSTRINGW( pExtDescList[ pExtDescIdList[ index ] ] ) ) ) return TRUE;

	sf_command( 0, SFC_GET_FORMAT_MAJOR_COUNT, &maxCount, sizeof( maxCount ) );
	for ( index = 0; index < maxCount; index++ )
	{
		info.format = index;
		sf_command( 0, SFC_GET_FORMAT_MAJOR, &info, sizeof( info ) );
		if ( CSTR_EQUAL == CompareStringW( lcid, NORM_IGNORECASE, pszExt, -1, AutoWide( info.extension ), -1 ) )
		{
			INT len1, len2;
			len1 = lstrlenA( info.extension );
			len2 = lstrlenA( info.name );
			if ( len2 > len1 )
			{
				if ( CSTR_EQUAL == CompareStringA( lcid, NORM_IGNORECASE, info.extension, len1, info.name, len1 ) )
				{
					info.name += len1;
					len2 -= len1;
				}

				if ( len2 > 0 && ' ' == *info.name )
				{
					info.name++;
					len2--;
				}

				if ( len2 > 0 && '(' == *info.name )
				{
					info.name++;
					len2--;
				}

				if ( len2 > 0 && ')' == info.name[ len2 - 1 ] )
					len2--;
			}

			return ( S_OK == StringCchCopyNW( pszDest, cchDest, AutoWide( info.name ), len2 ) );
		}
	}

	return FALSE;
}
