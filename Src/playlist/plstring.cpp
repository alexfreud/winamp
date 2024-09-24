#include "plstring.h"
#include "api__playlist.h"
#include <shlwapi.h>
#include <stdint.h>

static wchar_t *_plstring_wcsdup( const wchar_t *str )
{
	if ( !str )
		return 0;

	size_t  len  = wcslen( str );
	size_t *self = (size_t *)calloc( ( len + 1 ) * sizeof( wchar_t ), sizeof( size_t ) );
	*self = 1;

	wchar_t *new_str = (wchar_t *)( ( (int8_t *)self ) + sizeof( size_t ) );
	memcpy( new_str, str, ( len + 1 ) * sizeof( wchar_t ) );

	return new_str;
}

static wchar_t *_plstring_malloc( size_t str_size )
{
	size_t  *self    = (size_t *)calloc( ( str_size ), sizeof( size_t ) );
	*self = 1;

	wchar_t *new_str = (wchar_t *)( ( (int8_t *)self ) + sizeof( size_t ) );

	return new_str;
}

static void _plstring_release( wchar_t *str )
{
	if ( str )
	{
		size_t *self = (size_t *)( ( (int8_t *)str ) - sizeof( size_t ) );
		( *self )--;

		if ( *self == 0 )
			free( self );
	}
}

static void _plstring_retain( wchar_t *str )
{
	if ( str )
	{
		size_t *self = (size_t *)( ( (int8_t *)str ) - sizeof( size_t ) );
		( *self )++;
	}
}

wchar_t *( *plstring_wcsdup )( const wchar_t *str ) = _plstring_wcsdup;
wchar_t *( *plstring_malloc )( size_t str_size )    = _plstring_malloc;
void     ( *plstring_release )( wchar_t *str )      = _plstring_release;
void     ( *plstring_retain )( wchar_t *str )       = _plstring_retain;

static bool ndestring_tried_load = false;

void plstring_init()
{
	if ( !ndestring_tried_load )
	{
		wchar_t        path[ MAX_PATH ] = { 0 };
		const wchar_t *PROGDIR          = WASABI_API_APP->path_getAppPath();

		PathCombineW( path, PROGDIR, L"winamp.exe" );
		HMODULE ndelib = LoadLibraryW( path );
		if ( ndelib )
		{
			FARPROC ndestring_wcsdup  = GetProcAddress( ndelib, "plstring_wcsdup" );
			FARPROC ndestring_malloc  = GetProcAddress( ndelib, "plstring_malloc" );
			FARPROC ndestring_release = GetProcAddress( ndelib, "plstring_release" );
			FARPROC ndestring_retain  = GetProcAddress( ndelib, "plstring_retain" );

			if ( ndestring_wcsdup && ndestring_malloc && ndestring_release && ndestring_retain )
			{
				*(FARPROC *)&plstring_wcsdup  = *(FARPROC *)ndestring_wcsdup;
				*(FARPROC *)&plstring_malloc  = *(FARPROC *)ndestring_malloc;
				*(FARPROC *)&plstring_release = *(FARPROC *)ndestring_release;
				*(FARPROC *)&plstring_retain  = *(FARPROC *)ndestring_retain;
			}
		}

		ndestring_tried_load = true;
	}
}