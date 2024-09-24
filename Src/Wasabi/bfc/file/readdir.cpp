#include "precomp_wasabi_bfc.h"
#include "readdir.h"

#ifdef _WIN32
#include <shlwapi.h>
#endif

#if !defined(WIN32) && !defined(LINUX)
#error port me
#endif

//PORT
ReadDir::ReadDir( const wchar_t *_path, const wchar_t *_match, bool _skipdots ) : skipdots( _skipdots ), first( 1 ), path( _path ), match( _match )
{
    files = INVALID_HANDLE_VALUE;
    if ( match.isempty() )
        match = MATCHALLFILES;
    ZERO( data );

}

ReadDir::~ReadDir()
{
    //PORT
#ifdef WIN32
    if ( files != INVALID_HANDLE_VALUE ) FindClose( files );
#endif
#ifdef LINUX
    if ( d != NULL ) closedir( d );
#endif
}

int ReadDir::next()
{
    //PORT
#ifdef WIN32
    for ( ;;)
    {
        if ( first )
        {
            wchar_t fullpath[ MAX_PATH ];
            PathCombineW( fullpath, path.getValue(), match.getValue() );
            files = FindFirstFileW( fullpath, &data );
        }
        if ( files == INVALID_HANDLE_VALUE ) return 0;
        if ( first )
        {
            first = 0;
            if ( skipdots && ( isDotDir() || isDotDotDir() ) ) continue;
            return 1;
        }
        if ( !FindNextFileW( files, &data ) ) return 0;

        if ( skipdots && ( isDotDir() || isDotDotDir() ) ) continue;
        return 1;
    }
#endif//WIN32
#ifdef LINUX

    path.AddBackslash();
    if ( first || d == NULL )
    {
        if ( !( d = opendir( path ) ) ) return 0;
        first = 0;
    }

    while ( 1 )
    {
        de = readdir( d );
        if ( !de )
        {
            closedir( d );
            d = NULL;
            return 0;
        }

        StringW full;
        full.printf( L"%s%s", path.v(), de->d_name );

        if ( stat( full, &st ) == -1 )
            continue;

        if ( skipdots && ( isDotDir() || isDotDotDir() ) ) continue;

        if ( !Std::match( match, de->d_name ) ) continue;

        return 1;
    }

#endif
}

const wchar_t *ReadDir::getFilename()
{
    if ( first ) if ( !next() ) return NULL;
    //PORT
    return data.cFileName;

}

int ReadDir::isDir()
{
    //PORT
    if ( files == INVALID_HANDLE_VALUE ) return 0;
    return !!( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY );
}

int ReadDir::isReadonly()
{
    //PORT
    if ( files == INVALID_HANDLE_VALUE ) return 0;
    return !!( data.dwFileAttributes & FILE_ATTRIBUTE_READONLY );

}

int ReadDir::isDotDir()
{
    //PORT
    if ( files == INVALID_HANDLE_VALUE ) return 0;
    return ( data.cFileName[ 0 ] == '.' && data.cFileName[ 1 ] == 0 );
}

int ReadDir::isDotDotDir()
{
    //PORT

    if ( files == INVALID_HANDLE_VALUE ) return 0;
    return ( data.cFileName[ 0 ] == '.' && data.cFileName[ 1 ] == '.' && data.cFileName[ 2 ] == 0 );
}
