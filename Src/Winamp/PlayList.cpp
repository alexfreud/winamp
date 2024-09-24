#include "Main.h"
#include <assert.h>
#include "MoreItems.h"
#include "AdData.h"
#include "../nu/AutoLock.h"
#include "strutil.h"
#include "../nu/AutoCharFn.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "../nu/ns_wc.h"
#include "WinampPlaylist.h"
#include <algorithm>
#include "api.h"

#include "../WAT/WAT.h"

using namespace Nullsoft::Utility;

LockGuard playlistGuard(512);

#define MALLOC_CHUNK 512

static int _lastlen = -1;
extern "C"
{
	static wchar_t *_plstring_wcsdup( const wchar_t *str )
	{
		if ( !str )
			return 0;

		size_t  len = wcslen( str );
		size_t *self = (size_t *)malloc( sizeof( size_t ) + ( len + 1 ) * sizeof( wchar_t ) );
		*self = 1;
		wchar_t *new_str = (wchar_t *)( ( (int8_t *)self ) + sizeof( size_t ) );
		memcpy( new_str, str, ( len + 1 ) * sizeof( wchar_t ) );

		return new_str;
	}

	static wchar_t *_plstring_malloc( size_t str_size )
	{
		size_t *self = (size_t *)malloc( sizeof( size_t ) + ( str_size ) );
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
			{
				free( self );
			}
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

	__declspec( dllexport ) wchar_t *( *plstring_wcsdup )( const wchar_t *str ) = _plstring_wcsdup;
	__declspec( dllexport ) wchar_t *( *plstring_malloc )( size_t str_size )    = _plstring_malloc;
	__declspec( dllexport ) void     ( *plstring_release )( wchar_t *str )      = _plstring_release;
	__declspec( dllexport ) void     ( *plstring_retain )( wchar_t *str )       = _plstring_retain;
}

static bool ndestring_tried_load = false;

void plstring_init()
{
	if ( !ndestring_tried_load )
	{
		wchar_t path[ MAX_PATH ] = { 0 };
		PathCombineW( path, SHAREDDIR, L"nde.dll" );

		HMODULE ndelib = LoadLibraryW( path );
		if ( ndelib )
		{
			FARPROC ndestring_wcsdup  = GetProcAddress( ndelib, "ndestring_wcsdup" );
			FARPROC ndestring_malloc  = GetProcAddress( ndelib, "ndestring_malloc" );
			FARPROC ndestring_release = GetProcAddress( ndelib, "ndestring_release" );
			FARPROC ndestring_retain  = GetProcAddress( ndelib, "ndestring_retain" );

			if ( ndestring_wcsdup && ndestring_malloc && ndestring_release && ndestring_retain )
			{
				*(FARPROC *)&plstring_wcsdup  = ndestring_wcsdup;
				*(FARPROC *)&plstring_malloc  = ndestring_malloc;
				*(FARPROC *)&plstring_release = ndestring_release;
				*(FARPROC *)&plstring_retain  = ndestring_retain;
			}
		}

		ndestring_tried_load = true;
	}
}

#define CHECK_STUFF() 
/* { \
if (list_pos >= list_size && list_size>0) { MessageBox(NULL,"SHIT2!","a",0); } \
if (list_size > malloced_size) MessageBox(NULL,"SHIT!","a",0); }
*/
static void Playlist_notifyModified();

struct pl_entry
{
	pl_entry();

	void Free();
	void Create( int length, const wchar_t *filename, const wchar_t *title, const char *curtain, const wchar_t *ext, int is_nde_string = 0 );
	void CreateMore( int length, const wchar_t *filename, const wchar_t *title, const char *curtain/*, const char *browser*/ );
	void SetTitle( const wchar_t *title );
	void SetFile( const wchar_t *file );

	wchar_t       *strFile       = 0;     // file name oct-17-2005-maksim
	wchar_t       *strTitle      = 0;     // title oct-17-2005-maksim
	wchar_t       *strExt        = 0;
	size_t         cbTitle       = 0;     // title length oct-17-2005-maksim
	int            length        = 0;
	int8_t         selected:1;
	int8_t         cached:1;
	int8_t         tmp:1;
	int            more          = 0;     // number of subitems in the playlist (or 0 for none) mar-16-2005-benski
	int            curitems      = 0;     // number of hidden items in this entry
	int            curindex      = 0;     // Current index of item we're playing (0 for 1st/none hidden item)
	int            repeatCount   = 0;     // number of times to repeat
	int            repeatCurrent = 0;     // where we're at...
	unsigned long  starttime     = 0;     // Start time in MS  (0, begin of file)
	unsigned long  endtime       = 0;     // End time in MS  (0, end of file)
	moreitems     *moreStuff     = NULL;  // added for subitems in a playlist mar-16-2005-benski
	ad_data       *ads           = NULL;  // advertisement struct oct-17-2005-maksim
};

void pl_entry::Free()
{
	more = 0;
	if ( moreStuff )
	{
		delete moreStuff;
		moreStuff = NULL;
	}

	if ( ads )
	{
		delete ads;
		ads = 0;
	}

	plstring_release( strFile );
	strFile = 0;

	plstring_release( strExt );
	strExt = 0;

	plstring_release( strTitle );
	strTitle = 0;

	cbTitle = 0;
}

pl_entry::pl_entry()
{}

void pl_entry::Create( int length, const wchar_t *filename, const wchar_t *title, const char *curtain, const wchar_t *ext, int is_nde_string )
{
	assert( filename );
	assert( title );

	if ( is_nde_string & 1 )
	{
		strFile = plstring_wcsdup( filename );
	}
	else
	{
		while ( filename && *filename && IsCharSpaceW( *filename ) )
			filename = CharNextW( filename );

		strFile = plstring_wcsdup( filename );

		wchar_t *end = GetLastCharacterW( strFile );
		while ( end && IsCharSpaceW( *end ) )
		{
			*end = 0;
			end = CharPrevW( strFile, end );
		}
	}

	cbTitle  = wcslen( title ) + 1;
	strTitle = _wcsdup( title );

	if ( ext && *ext )
		strExt = _wcsdup( ext );

	if ( ( curtain && curtain[ 0 ] ) /*|| (browser && browser[0])*/ )
	{
		ads = new ad_data;
		if ( curtain && curtain[ 0 ] != 0 )
		{
			ads->cbCurtain  = (int)strlen( curtain ) + 1;
			ads->strCurtain = new char[ ads->cbCurtain ];
			StringCchCopyA( ads->strCurtain, ads->cbCurtain, curtain );
		}
	}
	else
	{
		ads = NULL;
	}

	this->length  = length;
	selected      = 0;
	cached        = 1;
	more          = 0;
	curindex      = 0;
	curitems      = 0;
	moreStuff     = NULL;
	repeatCount   = 0;
	repeatCurrent = 0;
}

void pl_entry::CreateMore( int length, const wchar_t *filename, const wchar_t *title, const char *curtain/*, const char *browser*/ )
{
	Create( length, filename, title, curtain, L""/*, browser*/);

	more      = 1;
	moreStuff = new moreitems;
}

void pl_entry::SetTitle( const wchar_t *title )
{
	size_t newSize = wcslen( title ) + 1;
	if ( newSize > cbTitle )
	{
		delete[] strTitle;
		strTitle = new wchar_t[ newSize ];
	}

	cbTitle = newSize;
	StringCchCopyW( strTitle, cbTitle, title );
}

void pl_entry::SetFile( const wchar_t *file )
{
	plstring_release( strFile );

	while ( file && *file && IsCharSpaceW( *file ) )
		file = CharNextW( file );

	strFile = plstring_wcsdup( file );

	wchar_t *end = GetLastCharacterW( strFile );
	while ( end && IsCharSpaceW( *end ) )
	{
		*end = 0;
		end = CharPrevW( strFile, end );
	}
}

static pl_entry *list             = 0;
static int       list_size        = 0;
static int       list_pos         = 0;
static int       malloced_size    = 0;

static pl_entry *tlist            = 0;
static int       t_len            = 0;

static int       rnd_listmodified = 1;
static int       rnd_lastls       = -1023;
static int       rnd_i            = 0;
static int      *rnd_rtable;

extern "C"
{
	int PlayList_get_lastlen()
	{
		return _lastlen;
	}

	static void Playlist_rnd_deleteitem(int item);
	static void Playlist_rnd_additem();
	wchar_t *remove_urlcodesW(wchar_t *p)
	{
		assert(p);

		if (!config_fixtitles)
			return p;

		wchar_t buf[FILENAME_SIZE] = {0}, *i = buf, *p2 = p;

		StringCchCopyW(buf, FILENAME_SIZE, p);
		while (i && *i)
		{
			if (*i == L'_' && (config_fixtitles&2))
			{
				*p = L' ';
				i = CharNextW(i);
			}
			else if (!StrCmpNW(i, L"%20", 3) && (config_fixtitles&1))
			{
				*p = L' ';
				i += 3;
			}
			else
			{
				CopyCharW(p, i);
				i = CharNextW(i);
			}

			p = CharNextW(p);
		}

		*p = 0;

		return p2;
	}

	static void PlayList_allocmem(int newsize);

	void PlayList_resetcurrent( void )
	{
		AutoLock lock( playlistGuard );
		if ( list_pos < 0 || list_pos >= list_size )
			return;

		if ( list )
			list[ list_pos ].curindex = 0;
	}

	void PlayList_saveend( int start )
	{
		AutoLock lock( playlistGuard );
		if ( tlist )
		{
			// scan and remove subitems in a playlist mar-16-2005-benski
			for ( int i = 0; i < t_len; i++ )
				tlist[ i ].Free();

			delete[] tlist;
			tlist = 0;
		}

		if ( start < list_size )
		{
			t_len = ( list_size - start );
			assert( t_len > 0 );
			tlist = new pl_entry[ t_len ];
			memcpy( tlist, list + start, sizeof( pl_entry ) * t_len );
			list_size = start;
		}
	}

	void PlayList_restoreend( void )
	{
		AutoLock lock( playlistGuard );
		if ( tlist )
		{
			if ( t_len )
			{
				PlayList_allocmem( list_size + t_len );
				memcpy( list + list_size, tlist, t_len * sizeof( pl_entry ) );
				list_size += t_len;
			}

			delete[] tlist; // note: we *don't* want to clean up the moreStuff linked list here
			tlist = NULL;
			t_len = 0;
		}
	}

	int PlayList_getselect( int x )
	{
		AutoLock lock( playlistGuard );
		if ( x < 0 || x >= list_size )
			return 0;

		CHECK_STUFF();

		return list[ x ].selected;
	}

	int PlayList_getselect2( int x, wchar_t *filename )
	{
		AutoLock lock( playlistGuard );
		if ( x < 0 || x >= list_size )
			return 0;

		CHECK_STUFF();

		if ( filename )
			lstrcpynW( filename, list[ x ].strFile, FILENAME_SIZE );

		return list[ x ].selected;
	}

	int PlayList_GetNextSelected( int start )
	{
		AutoLock lock( playlistGuard );
		start++;
		if ( start < 0 || start >= list_size )
			return -1;

		while ( start < list_size )
		{
			if ( list[ start ].selected )
				return start;

			start++;
		}

		return -1;
	}

	int PlayList_GetSelectedCount()
	{
		AutoLock lock( playlistGuard );
		int num = 0;
		for ( int i = 0; i < list_size; i++ )
		{
			if ( list[ i ].selected )
				num++;
		}

		return num;
	}

	void PlayList_setselect( int x, int sel )
	{
		AutoLock lock( playlistGuard );
		if ( x < 0 || x >= list_size )
			return;

		CHECK_STUFF();

		list[ x ].selected = sel;
	}

	int PlayList_getcached( int x )
	{
		AutoLock lock( playlistGuard );
		if ( x < 0 || x >= list_size )
			return 0;

		CHECK_STUFF();

		return list[ x ].cached;
	}

	void PlayList_setcached( int x, int cached )
	{
		AutoLock lock( playlistGuard );
		if ( x < 0 || x >= list_size )
			return;

		CHECK_STUFF();

		list[ x ].cached = cached;
	}

	int PlayList_getsonglength( int x )
	{
		AutoLock lock( playlistGuard );
		if ( x < 0 || x >= list_size )
			return -1;

		CHECK_STUFF();

		return list[ x ].length;
	}

	int PlayList_gettotallength( void )
	{
		AutoLock lock( playlistGuard );
		int l = 0;

		CHECK_STUFF();

		for ( int x = 0; x < list_size; x++ )
		{
			if ( list[ x ].length >= 0 )
				l += list[ x ].length;
			else
				return -1;
		}

		return l;
	}

	int PlayList_getcurrentlength( void )
	{
		AutoLock lock( playlistGuard );
		return ( PlayList_getsonglength( list_pos ) );
	}

	static void PlayList_allocmem( int newsize )
	{
		assert( newsize );

		int os = malloced_size;

		if ( newsize > malloced_size )
			malloced_size = newsize + MALLOC_CHUNK;
		else if ( newsize < malloced_size - MALLOC_CHUNK )
			malloced_size = newsize;
		else return;

		if ( !malloced_size )
		{
			if ( list )
			{
				delete[] list;
				list = 0;
			}
		}
		else if ( list )
		{
			pl_entry *ol = list;

			assert( malloced_size > 0 );

			list = new pl_entry[ malloced_size ];

			assert( min( os, malloced_size ) > 0 );
			assert( min( os, malloced_size ) <= malloced_size );

			memcpy( list, ol, min( os, malloced_size ) * sizeof( pl_entry ) );
			delete[] ol;
		}
		else
		{
			assert( malloced_size > 0 );
			list = new pl_entry[ malloced_size ];
			memset( list, 0, sizeof( pl_entry ) * ( malloced_size ) );
		}
	}

	int PlayList_gethidden( int pos )
	{
		AutoLock lock( playlistGuard );
		if ( pos < 0 || pos >= list_size )
			return 0;

		if ( list )
			return list[ pos ].curitems;
		else
			return 0;
	}

	int PlayList_ishidden( int pos )
	{
		AutoLock lock( playlistGuard );
		if ( pos < 0 || pos >= list_size )
			return 0;

		if ( list )
			if ( list[ pos ].curitems )
			{
				if ( list[ pos ].curindex != list[ pos ].curitems )
					return 1;
			}

		return 0;
	}

	int PlayList_alldone( int pos )
	{
		AutoLock lock( playlistGuard );
		if ( pos < 0 || pos >= list_size )
			return 1;

		if ( list )
		{
			if ( list[ pos ].curitems )
			{
				if ( list[ pos ].curindex == 0 )
					return 1;
			}
		}

		return 0;
	}

	int PlayList_hasanycurtain( int pos )
	{
		AutoLock lock( playlistGuard );
		if ( pos < 0 || pos >= list_size )
			return 0;

		if ( list )
		{
			if ( list[ pos ].ads && list[ pos ].ads->strCurtain )
				return 1;
		}

		return 0;
	}

	const char *PlayList_getcurtain( int pos )
	{
		AutoLock lock( playlistGuard );
		if ( pos < 0 || pos >= list_size )
			return NULL;

		if ( list )
		{
			if ( list[ pos ].ads && list[ pos ].ads->strCurtain && list[ pos ].curindex == 0 )
				return list[ pos ].ads->strCurtain;

			if ( list[ pos ].curindex && list[ pos ].curitems )
				return list[ pos ].moreStuff->GetHiddenCurtain( list[ pos ].curindex );
		}

		return NULL;
	}

	const char *PlayList_getExtInf( int pos )
	{
		AutoLock lock( playlistGuard );

		if ( pos < 0 || pos >= list_size )
			return NULL;

		if ( list )
		{
			pl_entry l_entry = list[ pos ];

			if ( l_entry.strExt && *l_entry.strExt )
			{
				wa::strings::wa_string l_ext_inf( " ext=\"" );
				l_ext_inf.append( l_entry.strExt );
				l_ext_inf.append( "\"" );

				return _strdup( l_ext_inf.GetA().c_str() );
			}
		}

		return NULL;
	}

	/*
	const char *PlayList_getbrowser(int pos)
	{
		AutoLock lock (playlistGuard);
		if (pos < 0 || pos >= list_size) return NULL;
		if ( list )
		{
			if ( list[pos].ads && list[pos].ads->cbBrowser ) return list[pos].ads->strBrowser;
		}
		return NULL;
	}
	*/

	int PlayList_current_hidden( void )
	{
		AutoLock lock( playlistGuard );
		if ( list_pos < 0 || list_pos >= list_size )
			return 0;

		if ( list && list[ list_pos ].curitems )
		{
			if ( list[ list_pos ].curitems && list[ list_pos ].curindex < list[ list_pos ].curitems )
			{
				list[ list_pos ].curindex++;

				return 1;
			}
		}

		return 0;
	}

	int PlayList_getitem( int position, wchar_t *filename, wchar_t *filetitle )
	{
		AutoLock lock( playlistGuard );
		if ( !list || position < 0 || position >= list_size )
			return 1;

		CHECK_STUFF();

		if ( filename )
			StringCchCopyW( filename, FILENAME_SIZE, list[ position ].strFile );

		if ( filetitle )
			StringCchPrintfW( filetitle, FILETITLE_SIZE, L"%d. %s", position + 1, list[ position ].strTitle );

		if ( list[ position ].length >= 0 )
		{
			wchar_t tmp[ 512 ] = { 0 };
			StringCchPrintfW( tmp, 512, L" - [%d:%02d]", list[ position ].length / 60, list[ position ].length % 60 );
			if ( filetitle )
				StringCchCatW( filetitle, FILETITLE_SIZE, tmp );
		}

		return 0;
	}

	/* this function call specifically has 256 characters max because of the data structure in ipc_pe.h (fileinfo2) */
	int PlayList_getitem3( int position, char *filetitle, char *filelength )
	{
		AutoLock lock( playlistGuard );
		if ( !list || position < 0 || position >= list_size )
			return 1;

		CHECK_STUFF();

		if ( filetitle )
			WideCharToMultiByteSZ( CP_ACP, 0, list[ position ].strTitle, -1, filetitle, 256, 0, 0 );

		if ( filelength && list[ position ].length >= 0 )
			StringCchPrintfA( filelength, 16, "%d:%02d", list[ position ].length / 60, list[ position ].length % 60 );

		return 0;
	}

	/* this function call specifically has 256 characters max because of the data structure in ipc_pe.h (fileinfo2) */
	int PlayList_getitem3W( int position, wchar_t *filetitle, wchar_t *filelength )
	{
		AutoLock lock( playlistGuard );
		if ( !list || position < 0 || position >= list_size )
			return 1;

		CHECK_STUFF();

		if ( filetitle )
			lstrcpynW( filetitle, list[ position ].strTitle, 256 );

		if ( filelength && list[ position ].length >= 0 )
			StringCchPrintfW( filelength, 16, L"%d:%02d", list[ position ].length / 60, list[ position ].length % 60 );

		return 0;
	}

	void PlayList_updateitem( int position )
	{
		AutoLock lock( playlistGuard );
		if ( !list || position < 0 || position >= list_size )
			return;

		_lastlen = -1;

		wchar_t *str = remove_urlcodesW( PlayList_gettitle( list[ position ].strFile, 1 ) );
		list[ position ].SetTitle( str );
		PlayList_setcached( position, 1 );

		if ( _lastlen != -1 )
			list[ position ].length = _lastlen;
	}

	int PlayList_getitem_pl( int position, wchar_t *filetitle )
	{
		AutoLock lock( playlistGuard );
		// static int i;
		CHECK_STUFF();
		if ( !list || position < 0 || position >= list_size )
			return 1;
		/*
		if (!PlayList_getcached(position) && config_riol==0 && !i)
		{
		i=1;
		_lastlen=-1;
		lstrcpy(list[position].filetitle,remove_urlcodes(PlayList_gettitle(list[position].filename,1)));
		PlayList_setcached(position,1);
		if (_lastlen != -1) list[position].length = _lastlen;
		i=0;
		}
		*/

		if ( filetitle && config_shownumsinpl )
		{
			if ( list_size >= 10000 )
				StringCchPrintfW( filetitle, FILETITLE_SIZE, ( config_zeropadplnum ? L"%05u. %s" : L"%5u. %s" ), position + 1, list[ position ].strTitle );
			else if ( list_size >= 1000 )
				StringCchPrintfW( filetitle, FILETITLE_SIZE, ( config_zeropadplnum ? L"%04u. %s" : L"%4u. %s" ), position + 1, list[ position ].strTitle );
			else if ( list_size >= 100 )
				StringCchPrintfW( filetitle, FILETITLE_SIZE, ( config_zeropadplnum ? L"%03u. %s" : L"%3u. %s" ), position + 1, list[ position ].strTitle );
			else if ( list_size >= 10 )
				StringCchPrintfW( filetitle, FILETITLE_SIZE, ( config_zeropadplnum ? L"%02u. %s" : L"%2u. %s" ), position + 1, list[ position ].strTitle );
			else
				StringCchPrintfW( filetitle, FILETITLE_SIZE, L"%1u. %s", position + 1, list[ position ].strTitle );
		}
		else if ( filetitle )
			StringCchCopyW( filetitle, FILETITLE_SIZE, list[ position ].strTitle );

		return 0;
	}

	int PlayList_getitem2( int position, char *filename, char *filetitle )
	{
		AutoLock lock( playlistGuard );
		if ( !list || position < 0 || position >= list_size )
			return 1;

		CHECK_STUFF();

		if ( filename )
			StringCchCopyA( filename, FILENAME_SIZE, AutoCharFn( list[ position ].strFile ) );

		if ( filetitle )
			WideCharToMultiByteSZ( CP_ACP, 0, list[ position ].strTitle, -1, filetitle, FILETITLE_SIZE, 0, 0 );

		return 0;
	}

	int PlayList_getitem2W( int position, wchar_t *filename, wchar_t *filetitle )
	{
		AutoLock lock( playlistGuard );
		if ( !list || position < 0 || position >= list_size )
			return 1;

		CHECK_STUFF();

		if ( filename )
		{
			if ( list[ position ].strFile )
				StringCchCopyW( filename, FILENAME_SIZE, list[ position ].strFile );
			else
				return 1;
		}

		if ( filetitle )
		{
			if ( list[ position ].strTitle )
				StringCchCopyW( filetitle, FILETITLE_SIZE, list[ position ].strTitle );
			else
				return 1;
		}

		return 0;
	}

	int PlayList_getitem_jtfW( int position, wchar_t *str )
	{
		AutoLock lock( playlistGuard );
		if ( !list || position < 0 || position >= list_size )
			return 1;

		CHECK_STUFF();

		StringCchPrintfW( str, FILENAME_SIZE + FILETITLE_SIZE + 1, L"%s %s", list[ position ].strFile, list[ position ].strTitle );

		return 0;
	}

	int PlayList_getlength( void )
	{
		AutoLock lock( playlistGuard );
		return list_size;
	}

	int plmodified = 0, plcleared = 0, plneedsave = 0;

	int g_has_deleted_current;
	// returns:
	// 1, last item in list
	// 0 not
	// -1 no items in list
	int PlayList_deleteitem( int item )
	{
		AutoLock lock( playlistGuard );
		if ( !list ) return -1;
		CHECK_STUFF();
		if ( item < 0 || item >= list_size ) return -1;

		list[ item ].Free();

		list_size--;
		if ( list_size != item )
			memmove( list + item, list + item + 1, sizeof( list[ 0 ] ) * ( list_size - item ) );
		if ( item == list_pos )
			g_has_deleted_current = 1;

		if ( item <= list_pos )
			list_pos--;

		if ( list_pos < 0 ) list_pos = 0;

		PlayList_allocmem( list_size + 1 );
		Playlist_notifyModified();
		Playlist_rnd_deleteitem( item );

		return list ? 0 : 1;
	}

	void PlayList_delete( void )
	{
		AutoLock lock( playlistGuard );

		CHECK_STUFF();

		//if (GetPrivateProfileIntW(L"Winamp", L"rmudhack", 0, INI_FILE) == 666) return ;

		if ( list )
		{
			// code to cleanup subitems in a playlist entry mar-16-2005-benski
			for ( int i = 0; i < list_size; i++ )
				list[ i ].Free();

			//		delete[] list;
		}

		//  list=0;
		list_size = 0;
		list_pos  = 0;
		plcleared = 1;
		//  malloced_size=0;

		Playlist_notifyModified();
	}

	void PlayList_destroy(void)
	{
		AutoLock lock (playlistGuard);
		PlayList_delete();
		delete[] list;
		list = 0;
	}

	void PlayList_setlastlen(int x)
	{
		AutoLock lock (playlistGuard);
		CHECK_STUFF();
		if (x >= 0 && x < list_size) list[x].length = _lastlen;
	}

	static wchar_t *GetFileInfo(const wchar_t *filename, wchar_t *title, size_t titleCch)
	{
		In_Module *mod = in_setmod_noplay(filename, 0);
		_lastlen = -1;
		if (!mod)
		{
			StringCchCopyW(title, GETFILEINFO_TITLE_LENGTH, filename);
			return title;
		}
		InW_GetFileInfo(mod, (filename == FileName) ? 0 : filename, title, &_lastlen);
		if (!title[0])
		{
			StringCchCopyW(title, titleCch, filename);
			if (!PathIsURLW(filename))
			{
				PathStripPathW(title);
				PathRemoveExtensionW(title);
			}
		}

		if (_lastlen != -1) _lastlen /= 1000;
		return 0;
	}

	static wchar_t *GetExtendedTitle( const wchar_t *filename, wchar_t *title, size_t titleCch, bool active = false )
	{
		AutoCharFn narrowFile( filename );
		// make sure the title is inited otherwise it'll be filled/set with rubbish noticeable 
		// on 5.3+ due to nothing internally using this api now and a retval = 1 done for it
		// basically for 3rd party/legacy handling now - 13 May 07
		char narrowTitle[ GETFILEINFO_TITLE_LENGTH ] = { 0 };
		waHookTitleStruct hts = { 0 };
		hts.filename = narrowFile;
		hts.length = -1;
		hts.title = narrowTitle;

		if ( PathIsURLW( filename ) )
		{
			wchar_t buf[ 32 ] = { 0 };
			extendedFileInfoStructW s = { filename, L"streammetadata", buf, sizeof( buf ) / sizeof( wchar_t ) };

			if ( 0 != SendMessageW( hMainWindow, WM_WA_IPC, (WPARAM)&s, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE ) && L'1' == buf[ 0 ] )
			{
				hts.force_useformatting = 1;
			}
		}

		/*
		wchar_t buf[32]=L"";
		if (PathIsURLW(filename) && in_get_extended_fileinfoW(filename, L"streammetadata", buf, sizeof(buf)/sizeof(wchar_t)) && buf[0]=='1')
			hts.force_useformatting = 1;
		*/

		if ( config_useexttitles && SendMessageW( hMainWindow, WM_WA_IPC, (WPARAM)&hts, IPC_HOOK_TITLES ) )
		{
			MultiByteToWideCharSZ( CP_ACP, 0, narrowTitle, -1, title, (int)titleCch );
			if ( hts.length != -1 )
			{
				_lastlen = hts.length;
				return title;
			}
			else
			{
				In_Module *mod = in_setmod_noplay( filename, 0 );
				_lastlen = -1;
				if ( mod )
				{
					wchar_t title2[ GETFILEINFO_TITLE_LENGTH ] = { 0 };
					InW_GetFileInfo( mod, filename, title2, &_lastlen );
				}
				if ( _lastlen != -1 ) _lastlen /= 1000;
			}
			return title;
		}
		return 0;
	}

	static wchar_t *GetExtendedTitleW(const wchar_t *filename, wchar_t *title, size_t titleCch, bool active=false)
	{
		waHookTitleStructW hts = {0};
		hts.filename = filename;
		hts.length = -1;
		hts.title = title;

		if (PathIsURLW(filename))
		{
			wchar_t buf[32] = {0};
			extendedFileInfoStructW s = {filename, L"streammetadata", buf, sizeof(buf)/sizeof(wchar_t)};

			if (0 != SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&s, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE) && 
				L'1' == buf[0])
			{
				hts.force_useformatting = 1;
			}
		}

		/*
		wchar_t buf[32]=L"";
		if (PathIsURLW(filename) && in_get_extended_fileinfoW(filename, L"streammetadata", buf, 32) && buf[0]=='1')
			hts.force_useformatting = 1;

		*/

		if (config_useexttitles && SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&hts, IPC_HOOK_TITLESW))
		{
			if (hts.length != -1)
			{
				_lastlen = hts.length;
				return title;
			}
			else
			{
				In_Module *mod = in_setmod_noplay(filename, 0);
				_lastlen = -1;
				if (mod)
				{
					wchar_t title2[GETFILEINFO_TITLE_LENGTH] = {0};
					InW_GetFileInfo(mod, filename, title2, &_lastlen);
				}
				if (_lastlen != -1) _lastlen /= 1000;
			}
			return title;
		}
		return 0;
	}

	wchar_t *PlayList_gettitle(const wchar_t *filename, int useID3)
	{
		/* OK, this function is going to get ugly */
		AutoLock lock (playlistGuard);
		static wchar_t title[GETFILEINFO_TITLE_LENGTH] = {0};
		CHECK_STUFF();
		if (useID3 || (filename[0] && !StrCmpIW(PathFindExtensionW(filename), L"aip")))
		{
			if (filename[0])
			{
				wchar_t *ret = GetExtendedTitle(filename, title, GETFILEINFO_TITLE_LENGTH);
				if (ret)
					return ret;
				else
				{
					ret = GetExtendedTitleW(filename, title, GETFILEINFO_TITLE_LENGTH);
					if (ret)
						return ret;
					else
					{
						ret = GetFileInfo(filename, title, GETFILEINFO_TITLE_LENGTH);
						if (ret)
							return ret;
					}
				}
			}
			else
			{
				wchar_t *ret = GetExtendedTitle(FileName, title, GETFILEINFO_TITLE_LENGTH, true);
				if (ret)
					return ret;

				else
				{
					ret = GetExtendedTitleW(FileName, title, GETFILEINFO_TITLE_LENGTH, true);
					if (ret)
						return ret;
					else
					{
						ret = GetFileInfo(FileName, title, GETFILEINFO_TITLE_LENGTH);
						if (ret)
							return ret;
					}
				}
			}
		}
		else
		{
			_lastlen = -1;
			if (filename[0] && PathIsURLW(filename))
			{
				if (title) StringCchCopyW(title, GETFILEINFO_TITLE_LENGTH, filename);
			}
			else if (title)
			{
				wchar_t buf[1024] = {0};
				StringCchCopyW(buf, 1024, filename);
				StringCchCopyW(title, GETFILEINFO_TITLE_LENGTH, PathFindFileNameW(buf));
				PathRemoveExtensionW(title);
			}
		}
		return title;
	}

	void PlayList_append_withinfo_curtain(const wchar_t *filename, const wchar_t *title, int length, char *curtain, const wchar_t *ext, int is_nde_string)
	{
		AutoLock lock (playlistGuard);
		bool cached = true;
		wchar_t fn[FILENAME_SIZE] = {0};
		CHECK_STUFF();
		if (!(is_nde_string & 1) && !PathIsURLW(filename))
		{
			if (wcsstr(fn, L"\\"))
			{
				int ret = GetLongPathNameW(filename, fn, FILENAME_SIZE);
				if (ret && ret < FILENAME_SIZE)
					filename=fn;
			} 
		}

		if (!title)
		{
			title = remove_urlcodesW(PlayList_gettitle(filename, (config_riol == 1 ? 1 : 0)));
			// changed 5.64 - if we have a length but no title, use the length and build the title
			if (length < 0)
			{
				_lastlen = -1;
				// added post 5.65 to ensure read on loading will work
				// after some of the other changes made in 5.64 / 5.65
				if (config_riol == 1)
				{
					In_Module *mod = in_setmod_noplay(filename, 0);
					if (mod)
					{
						wchar_t title2[GETFILEINFO_TITLE_LENGTH] = {0};
						InW_GetFileInfo(mod, filename, title2, &_lastlen);
						if (_lastlen != -1) _lastlen /= 1000;
					}
				}
			}
			length = _lastlen;
			cached = config_riol == 1;
		}

		if (list_size && list[list_size - 1].more)
		{
			// We have the final item to add to the last index
			// Add it to the more items list
			// Reset more flag
			// and do not adjust the list_size;
			list[list_size - 1].moreStuff->AddHiddenItem(filename, title, length, list[list_size - 1].more, curtain);
			list[list_size - 1].curitems = list[list_size - 1].more;
			list[list_size - 1].curindex = 0;
			list[list_size - 1].more = 0;
		}
		else
		{
			PlayList_allocmem(list_size + 1);
			if (list_pos == -1)
				list_pos = 0;
			list[list_size].Create(length, filename, title, curtain, ext, is_nde_string);
			list[list_size].cached = cached ? 1 : 0;
			Playlist_rnd_additem();
			list_size++;
		}
		Playlist_notifyModified();
	}

	void PlayList_append_withinfo(const wchar_t *filename, const wchar_t *title, const wchar_t *ext, int length, int is_nde_string)
	{
		AutoLock lock (playlistGuard);
		PlayList_append_withinfo_curtain(filename, title, length, NULL, ext, is_nde_string);
	}

	void PlayList_append_withinfo_hidden(const wchar_t *filename, const wchar_t *title, int length, char *curtain/*, char *browser*/)
	{
		AutoLock lock (playlistGuard);
		bool cached = true;
		wchar_t fn[FILENAME_SIZE] = {0};
		CHECK_STUFF();
		if (!PathIsURLW(filename))
		{
			if (wcsstr(fn, L"\\"))
			{
				int ret = GetLongPathNameW(filename, fn, FILENAME_SIZE);
				if (ret && ret < FILENAME_SIZE)
					filename=fn;
			} 
		}

		if (!title)
		{
			title = remove_urlcodesW(PlayList_gettitle(filename, (config_riol == 1 ? 1 : 0)));
			// changed 5.64 - if we have a length but no title, use the length and build the title
			if (length < 0)
			{
				_lastlen = -1;
				// added post 5.65 to ensure read on loading will work
				// after some of the other changes made in 5.64 / 5.65
				if (config_riol == 1)
				{
					In_Module *mod = in_setmod_noplay(filename, 0);
					if (mod)
					{
						wchar_t title2[GETFILEINFO_TITLE_LENGTH] = {0};
						InW_GetFileInfo(mod, filename, title2, &_lastlen);
						if (_lastlen != -1) _lastlen /= 1000;
					}
				}
			}
			length = _lastlen;
			cached = config_riol == 1;
		}

		if (list_size && list[list_size - 1].more)
		{
			// add item with info to more list
			// increment more flag so another item may be added to the hidden list
			list[list_size - 1].moreStuff->AddHiddenItem(filename, title, length, list[list_size - 1].more, curtain);
			list[list_size - 1].more++;
		}
		else
		{
			// Create a new entry, but set the more flag. so the next item gets added to
			// more list
			PlayList_allocmem(list_size + 1);
			if (list_pos == -1)
				list_pos = 0;
			list[list_size].CreateMore(length, filename, title, curtain/*, browser*/);
			list[list_size].cached = cached ? 1 : 0;
			Playlist_rnd_additem();
			list_size++;
		}
		Playlist_notifyModified();
	}

	static void _appendcd(const wchar_t *url)
	{
		AutoLock lock (playlistGuard);
		wchar_t buf2[32] = {0};
		in_get_extended_fileinfoW(url, L"ntracks", buf2, 32);
		int n = _wtoi(buf2);
		if (n > 0 && n < 256)
		{
			for (int x = 0; x < n; x ++)
			{
				wchar_t s[64] = {0};
				StringCchPrintfW(s, 64, L"%s,%d",url, x + 1);
				PlayList_append(s, 0);
			}
		}
	}

	void PlayList_check_unknown(const wchar_t *url, int is_nde_string)
	{
		if (!_wcsicmp(extensionW(url), L""))  // no extension, check for pls/m3u signatures
		{
			// TODO: move to its own function
			FILE *fp = _wfopen(url,L"r");
			
			char buf[21] = {0};
			if (fp)
			{
				fread(buf, 1, 21, fp);
				fclose(fp);
				buf[20] = 0;
			}
			if (!_strnicmp(buf, "[Reference]", 11))
				LoadPlaylistByExtension(url, L".asx", 1, 0);
			else if (!_strnicmp(buf, "[playlist]", 10))
				LoadPlaylistByExtension(url, L".pls", 1, 0);
			else if (!memcmp(buf, L"[playlist]", 20))
				LoadPlaylistByExtension(url, L".pls", 1, 0);
			else if (!_strnicmp(buf, "#EXTM3U", 7))
				LoadPlaylistByExtension(url, L".m3u", 1, 0);
			else if (!_strnicmp(buf, "<ASX", 4))
				LoadPlaylistByExtension(url, L".asx", 1, 0);
			else if (!_strnicmp(buf, "<?wpl", 5))
				LoadPlaylistByExtension(url, L".wpl", 1, 0);
			else if (!_strnicmp(buf, "<?zpl", 5))
				LoadPlaylistByExtension(url, L".zpl", 1, 0);
			else PlayList_append(url, is_nde_string);
		}
		else
			PlayList_append(url, is_nde_string);
	}

	void PlayList_appendthing(const wchar_t *url, int doMIMEcheck, int is_nde_string)
	{
		wchar_t temp2[MAX_PATH] = {0};
		AutoLock lock (playlistGuard);
		if (!_wcsicmp(extensionW(url), L"lnk"))
		{
			if (ResolveShortCut(hMainWindow, url, temp2))
			{
				url = temp2; //StringCchCopy(url, MAX_PATH, temp2);
				is_nde_string &= ~1;
			}
			else
				return ;
		}

		if (!_wcsnicmp(url, L"cda://", 6))
		{
			if ( wcslen(url) == 7)
				_appendcd(url);
			else
				PlayList_append(url, is_nde_string);
		}
		else if (PathIsURLW(url))
		{
			if (LoadPlaylist(url, 1, doMIMEcheck) == -1) // if it's not a playlist file
				PlayList_append(url, is_nde_string);
		}
		else if (!lstrcmpW(url + 1, L":\\"))
			PlayList_adddir(url, 1);
		else
		{
			WIN32_FIND_DATAW d = {0};
			HANDLE h = FindFirstFileW(url, &d);
			if (h != INVALID_HANDLE_VALUE)
			{
				FindClose(h);
				if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					PlayList_adddir(url, 1);
				else if (LoadPlaylist(url, 1, 0) == -1) // if it's not a playlist file
				{
					PlayList_check_unknown(url, is_nde_string); // no extension, check for pls/m3u signatures
				}
			}
			else
				PlayList_append(url, is_nde_string);
		}
	}

	int PlayList_getrepeatcount(int pos)
	{
		AutoLock lock (playlistGuard);
		if ( list_size )
		{
			return list[pos].repeatCount;
		}
		else return 0;
	}

	void PlayList_SetLastItem_RepeatCount(int count)
	{
		AutoLock lock (playlistGuard);
		if ( list_size )
		{
			list[list_size -1].repeatCount = count;
		}
	}

	void PlayList_SetLastItem_Range(unsigned long start, unsigned long end)
	{
		int index = 0;

		if ( list[list_size - 1].curitems ) index = list[list_size - 1].curitems;
		else if ( list[list_size - 1].more - 1 > 0 ) index = list[list_size - 1].more - 1;

		if ( list_size && index )
		{
			list[list_size - 1].moreStuff->SetRange(index, start, end);
		}
		else if ( list_size )
		{
			list[list_size - 1].starttime = start;
			list[list_size - 1].endtime = end;
		}
		Playlist_notifyModified();
	}

	unsigned long PlayList_GetItem_Start(int pos)
	{
		if (pos >= 0 && pos < list_size)
		{
			if ( !list[pos].curindex )
				return list[pos].starttime;
			else
			{
				return list[pos].moreStuff->GetStart(list[pos].curindex);
			}
		}
		else
			return 0;
	}

	unsigned long PlayList_GetItem_End(int pos)
	{
		if (pos >= 0 && pos < list_size)
		{
			if ( !list[pos].curindex )
				return list[pos].endtime;
			else
			{
				return list[pos].moreStuff->GetEnd(list[pos].curindex);
			}
		}
		else
			return 0;
	}

	void PlayList_terminate_lasthidden(void)
	{
		AutoLock lock (playlistGuard);
		if ( list_size && list[list_size - 1].more )
		{
			// Reset more flag
			// and do not adjust the list_size;

			list[list_size - 1].curitems = list[list_size - 1].more - 1;
			list[list_size - 1].curindex = 0;
			list[list_size - 1].more = 0;
			Playlist_notifyModified();
		}
	}


	void PlayList_append(const wchar_t *filename, int is_nde_string)
	{
		AutoLock lock (playlistGuard);
		PlayList_append_withinfo_curtain(filename, NULL, 0, NULL, L"", 0);
	}

	void PlayList_getcurrent_onstop(wchar_t *filename, wchar_t *filetitle)
	{
		AutoLock lock (playlistGuard);
		CHECK_STUFF();
		if (list_pos >= 0 && list_pos < list_size)
		{
			int position = list_pos;
			if (!PlayList_getcached(position))
			{
				_lastlen = -1;
				wchar_t *str = remove_urlcodesW(PlayList_gettitle(list[position].strFile, 1));
				list[position].SetTitle(str);
				PlayList_setcached(position, 1);
				if (_lastlen != -1)
					list[position].length = _lastlen;
			}

			StringCchCopyW(filename, FILENAME_SIZE, list[list_pos].strFile);
			StringCchCopyW(filetitle, FILETITLE_SIZE, list[list_pos].strTitle);
			//StringCchPrintfW(filetitle, FILETITLE_SIZE, L"%d. %s", list_pos + 1, list[list_pos].strTitle);
		}
		else *filename = *filetitle = 0;
	}

	void PlayList_GetCurrentTitle(wchar_t *filetitle, int cchLen)
	{
		AutoLock lock (playlistGuard);
		CHECK_STUFF();
		if (list_pos >= 0 && list_pos < list_size)
		{
				StringCchCopyW(filetitle, cchLen, list[list_pos].strTitle);
			//StringCchPrintfW(filetitle, cchLen, L"%d. %s", list_pos + 1, list[list_pos].strTitle);
		}
		else *filetitle = 0;
	}

	void PlayList_getcurrent(wchar_t *filename, wchar_t *filetitle, wchar_t *filetitlenum)
	{
		AutoLock lock (playlistGuard);
		CHECK_STUFF();
		if (list_pos >= 0 && list_pos < list_size)
		{
			if ( !list[ list_pos ].curindex )
			{
				wa::strings::wa_string l_filename( L"" );

				wa::strings::wa_string l_original_filename( list[ list_pos ].strFile );

				if ( list[ list_pos ].strExt && *list[ list_pos ].strExt && l_original_filename.contains( "://" ) )
				{
					wa::strings::wa_string l_ext( "." );
					l_ext.append( list[ list_pos ].strExt );
					l_ext.toUpper();

					wa::strings::wa_string l_strFile( list[ list_pos ].strFile );

					l_filename.append( "http://client.winamp.com/fileproxy?destination=" );
					l_filename.append( list[ list_pos ].strFile );

					wa::strings::wa_string l_upper_case_filename( l_strFile.GetW() );
					l_upper_case_filename.toUpper();

					if ( !l_upper_case_filename.contains( l_ext.GetW() ) )
					{
						if ( !l_strFile.contains( "?" ) )
							l_filename.append( "?" );
						else
							l_filename.append( "&" );

						l_filename.append( "ext=" );
						l_filename.append( l_ext );
					}

					StringCchCopyW( filename, FILENAME_SIZE, l_filename.GetW().c_str() );
				}
				else
					StringCchCopyW( filename, FILENAME_SIZE, list[ list_pos ].strFile );
			}
			else
			{
				StringCchCopyW(filename, FILENAME_SIZE, list[list_pos].moreStuff->GetHiddenFilename(list[list_pos].curindex));
			}
			StringCchCopyW(filetitle, FILETITLE_SIZE, list[list_pos].strTitle);
			StringCchPrintfW(FileTitleNum, FILETITLE_SIZE, L"%d. %s", list_pos + 1, list[list_pos].strTitle);
		}
		else *filename = *filetitle = 0;
	}

	void PlayList_setcurrent(const wchar_t *filename, wchar_t *filetitle)
	{
		AutoLock lock (playlistGuard);
		PlayList_setitem(list_pos, filename, filetitle);
	}

	void PlayList_setitem(int x, const wchar_t *filename, wchar_t *filetitle)
	{
		AutoLock lock (playlistGuard);
		CHECK_STUFF();
		if (x >= 0 && x < list_size)
		{
			list[x].SetFile(filename);
			// Dont allow update of the title if there is hidden elements
			if ( !list[x].curitems )
			{
				wchar_t *str = remove_urlcodesW(filetitle);
				list[x].SetTitle(str);
			}
			list[x].cached = 1;
			if (x != list_pos) Playlist_notifyModified();
		}
	}

	void PlayList_swap(int e1, int e2)
	{
		AutoLock lock (playlistGuard);
		pl_entry p;
		if (e1 < 0 || e2 < 0 || e1 >= list_size || e2 >= list_size || e1 == e2) return ;
		p = list[e1];
		list[e1] = list[e2];
		list[e2] = p;
		Playlist_notifyModified();
	}

	int PlayList_setposition(int pos)
	{
		AutoLock lock (playlistGuard);
		if (list_pos >= 0 && list_pos < list_size)
			list[list_pos].curindex = 0;

		list_pos = 0;
		if (pos >= 0 && pos < list_size)
			list[pos].curindex = 0;

		return (PlayList_advance(pos));
	}

	int PlayList_getPosition()
	{
		AutoLock lock (playlistGuard); // to let any pending actions finish
		return list_pos;
	}

	int PlayList_getNextPosition()
	{
		AutoLock lock (playlistGuard); // to let any pending actions finish
		// rnd

		int pl_len = PlayList_getlength();
		if (pl_len > 0)
		{
			// repeat track is enabled so will be current one
			if (!config_pladv && config_repeat)
			{
				// manual playlist advance is on so should be the current track
				// (may change but won't unless user does a prev / next action)
				return list_pos;
			}
			else
			{
				// shuffle is off so can determine next entry
				if (!config_shuffle)
				{
					// if at the end then loop to the start
					// otherwise it will be the next entry
					// butif repeat is off then we don't know
					return ((list_pos + 1 >= pl_len) ? (!config_repeat ? -1 : 0) : list_pos + 1);
				}
				// shuffle is on so need to find out from table
				else
				{
					// keep things in check when at end of table
					if (!rnd_listmodified && rnd_i >= 0 && rnd_i + 1 != rnd_lastls)
					{
						return (rnd_rtable ? rnd_rtable[rnd_i + 1] : 0);
					}
					// otherwise if at the end and repeat on go to start
					else
					{
						if(config_repeat)
						{
							return (rnd_rtable ? rnd_rtable[0] : 0);
						}
					}
				}
			}
		}
		// if nothing or only one item then default to the first entry
		else if(!pl_len)
		{
			return (!config_repeat ? -1 : 0);
		}

		// not sure what is the next so fail nicely
		return -1;
	}

	int PlayList_advance(int byval)
	{
		AutoLock lock (playlistGuard);
		CHECK_STUFF();
		if ( byval == HIDDEN_TRAP )
		{
			if ( list_size )
			{
				// Our hidden trap
				if ( list[list_pos].curitems && list[list_pos].curindex < list[list_pos].curitems )
				{
					list[list_pos].curindex++;
					return list_pos;
				}
				byval = 1;
				list[list_pos].curindex = 0;
				if ( list[list_pos].repeatCount && list[list_pos].repeatCurrent++ != list[list_pos].repeatCount )
				{
					return list_pos;
				}
				list[list_pos].repeatCurrent = 0;
			}
		}
		if (!byval) return list_pos;
		//if ( list_size && byval != 1 ) for ( x = 0; x < list_size; x++ ) list[x].curindex = 0;
		//if ( list_size && byval == 1 ) list[list_pos].curindex = 0;
		if (list_size > 1 && list_pos >= 0 && list_pos < list_size)
		{
			list[list_pos].curindex = 0;
			list[list_pos].repeatCurrent = 0;
		}

		list_pos += byval;
		if (list_pos < 0) list_pos = 0;
		else if (list_pos >= list_size) list_pos = list_size - 1;
		else return list_pos;
		return -1;
	}

	void PlayList_addfromdlg(const wchar_t *fns)
	{
		AutoLock lock (playlistGuard);
		WinampPlaylist playlist;
		playlistManager->LoadFromDialog(fns, &playlist);
	}

	void PlayList_getcurrent_tupdate(wchar_t *FileName, wchar_t *FileTitle)
	{
		AutoLock lock (playlistGuard);
		if (!g_has_deleted_current)
		{
			PlayList_refreshtitle(); // removed for evil vbr fix. ? ?
			PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
		}
		else
		{
			_lastlen = -1;
			StringCchCopyW(FileTitle, FILETITLE_SIZE, remove_urlcodesW(PlayList_gettitle(L"", 1)));
		}
	}

	void PlayList_refreshtitle(void)
	{
		AutoLock lock (playlistGuard);
		CHECK_STUFF();
		if (list_pos >= 0 && list_pos < list_size)
		{
			_lastlen = -1;

			// TODO: benski> tag probably needs this, but MN radio can't have it:  if ( !list[list_pos].curitems )
			{
				wchar_t *title = remove_urlcodesW(PlayList_gettitle(L"", 1));
				list[list_pos].SetTitle(title);
			}
			PlayList_setcached(list_pos, 1);
			if (_lastlen != -1) list[list_pos].length = _lastlen;
			//JF100500		SendMessageW(hPLWindow,WM_USER+1,0,0);
		}
	}

	const wchar_t *PlayList_GetCachedTitle(const wchar_t *filename)
	{
		AutoLock lock (playlistGuard);
		CHECK_STUFF();
		for (int i = 0;i < list_size;i++)
		{
			if (!lstrcmpiW(filename, list[i].strFile))
			{
				if (list[i].cached)
				{
					_lastlen = list[i].length;
					return list[i].strTitle;
				}
				else
					return 0;
			}
		}
		return 0;
	}

	void PlayList_UpdateTitle(const wchar_t *filename)
	{
		AutoLock lock (playlistGuard);
		CHECK_STUFF();
		for (int i = 0;i < list_size;i++)
		{
			if (!lstrcmpiW(filename, list[i].strFile))
				PlayList_updateitem(i);
		}

		Playlist_notifyModified();
	}

	void PlayList_updaterandpos(void)
	{
		AutoLock lock (playlistGuard);
		//MessageBox(NULL,"updaterandpos","b",0);
		rnd_listmodified = 1;
	}

	void Playlist_rnd_additem()
	{
		AutoLock lock (playlistGuard);
		if (!rnd_listmodified && rnd_rtable && rnd_lastls > 0)
		{
			int x, newpos;
			rnd_lastls++;
			int *newtable = (int*)realloc(rnd_rtable, sizeof(int) * (rnd_lastls));
			if (!newtable)
			{
				newtable = (int*)malloc(sizeof(int) * (rnd_lastls));
				if (newtable)
				{
					memcpy(newtable, rnd_rtable, sizeof(int)*(rnd_lastls - 1));
					free(rnd_rtable);
					rnd_rtable = newtable;
				}
				else return ;
			}
			else rnd_rtable = newtable;

			newpos = warand() % rnd_lastls;

			for (x = rnd_lastls - 1; x > newpos; x --)
			{
				rnd_rtable[x] = rnd_rtable[x - 1];
				if (rnd_rtable[x] >= list_size) rnd_rtable[x]++;
			}
			rnd_rtable[x] = list_size;
			while (x > 0)
			{
				x--;
				if (rnd_rtable[x] >= list_size) rnd_rtable[x]++;
			}
		}
		else rnd_listmodified = 1;
	}

	void Playlist_rnd_deleteitem(int item)
	{
		AutoLock lock (playlistGuard);
		if (!rnd_listmodified && rnd_rtable && rnd_lastls > 0)
		{
			for (int x = 0; x < rnd_lastls; x ++)
			{
				if (rnd_rtable[x] > item)
				{
					rnd_rtable[x]--;
				}
				else if (rnd_rtable[x] == item)
				{
					std::rotate(&rnd_rtable[x], &rnd_rtable[x + 1], &rnd_rtable[rnd_lastls]);
					if (rnd_rtable[x] > item) rnd_rtable[x]--;
					if (rnd_i >= x) rnd_i--;
					if (rnd_i < 0) rnd_i = 0;
					rnd_lastls--;
				}
			}
		}
		else rnd_listmodified = 1;
	}

	static int Rand(int n)
	{
		return warand() % n ;
	}

	int PlayList_randpos(int x)
	{
		AutoLock lock (playlistGuard);
		int ls;

		if (x == -666) // exiting winamp
		{
			rnd_i = 0;
			free(rnd_rtable);
			rnd_rtable = 0;
			return 0;
		}

		if (ls = PlayList_getlength())
		{
			if (ls != rnd_lastls || rnd_listmodified)
			{
				int first_run = (rnd_lastls < 0);
				//init table;
				rnd_lastls = ls;
				rnd_listmodified = 0;
				if (!ls) return 0;
				free(rnd_rtable);
				rnd_rtable = (int *) malloc(ls * sizeof(int));
				for (x = 0; x < ls; x ++)
				{
					rnd_rtable[x] = x;
				}
				x = 0;
				if (first_run && list_pos >= 0 && list_pos < ls)
				{
					// at startup, go back to current song from last session:
					rnd_rtable[list_pos] = 0;
					rnd_rtable[0] = list_pos;
					x = 1; // (don't re-randomize this entry)
				}
				std::random_shuffle(&rnd_rtable[x], &rnd_rtable[ls], Rand);
				rnd_i = 0;
			}
			else rnd_i += x;

			if (rnd_i >= ls)
			{
				if (config_repeat)
				{
					rnd_i = 0;
					if (ls > 2)
					{
						// note: config_shuffle_morph_rate==0 means slow morph, and minimal changes to playlist each time through [window_size==2]
						//       config_shuffle_morph_rate==50 means fast morph, and guarantees that HALF of the other songs will play before any one song repeats [ideal]
						int window_size = max(MulDiv(ls, config_shuffle_morph_rate, 100), 2);
						for (x = 0; x < ls - 1; x ++)
						{
							std::random_shuffle(&rnd_rtable[x], &rnd_rtable[min(x + window_size, ls)], Rand);
						}
					}
				}
				else rnd_i = ls - 1;
				PlayList_setposition(rnd_rtable[rnd_i]);
				return 1;
			}
			if (rnd_i < 0)
			{
				rnd_i = 0;
				PlayList_setposition(rnd_rtable[rnd_i]);
				return 1;
			}
			PlayList_setposition(rnd_rtable[rnd_i]);
		}
		else rnd_lastls = 0;
		return 0;
	}

	static bool PlayList_sortByFile(pl_entry &a, pl_entry &b) //const void *a, const void *b)
	{
		const wchar_t *file1 = PathFindFileNameW(a.strFile);
		const wchar_t *file2 = PathFindFileNameW(b.strFile);

		//		int comp = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE | /*NORM_IGNOREKANATYPE |*/ NORM_IGNOREWIDTH, file1, -1, file2 , -1);
		//return comp == CSTR_LESS_THAN;
		return FileCompareLogical(file1, file2) < 0;
	}

	static bool PlayList_sortByTitle(pl_entry &a, pl_entry &b)
	{
		//	int comp = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE/*|NORM_IGNOREKANATYPE*/|NORM_IGNOREWIDTH, a.strTitle, -1, b.strTitle, -1);
		//		return comp == CSTR_LESS_THAN;
		return CompareStringLogical(a.strTitle, b.strTitle) < 0;
	}

	static bool PlayList_sortByDirectory(pl_entry &a, pl_entry &b) // by dir, then by title
	{
		const wchar_t *directory1 = a.strFile;
		const wchar_t *directory2 = b.strFile;
		const wchar_t *directoryEnd1 = scanstr_backcW(directory1, L"\\", 0);
		const wchar_t *directoryEnd2 = scanstr_backcW(directory2 , L"\\", 0);
		size_t dirLen1 = directoryEnd1 - directory1;
		size_t dirLen2 = directoryEnd2 - directory2;

		if (!dirLen1 && !dirLen2) // both in the current directory?
			return PlayList_sortByFile(a, b); // not optimized, because the function does another scanstr_back, but easy for now :)

		if (!dirLen1) // only the first dir is empty?
			return true; // sort it first

		if (!dirLen2) // only the second dir empty?
			return false; // empty dirs go first

		int comp = FileCompareLogicalN(directory1, dirLen1, directory2, dirLen2);
		if (comp == 0)
			return PlayList_sortByFile(a, b);
		else
			return comp < 0;
#if 0 // old way
		int comp = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE |         /*NORM_IGNOREKANATYPE | */NORM_IGNOREWIDTH, directory1, dirLen1, directory2 , dirLen2);
		if (comp == CSTR_EQUAL) // same dir
			return PlayList_sortByFile(a, b); // do second sort
		else // different dirs
			return comp == CSTR_LESS_THAN;
#endif
	}

	void PlayList_mark()
	{
		for (int x = 0; x < list_size; x++)
			list[x].tmp = 0;
		list[list_pos].tmp = 1;
	}

	void PlayList_restore()
	{
		for (int x = 0; x < list_size; x ++)
		{
			if (list[x].tmp)
			{
				list_pos = x;
				break;
			}
		}
	}

	void PlayList_sort(int bytitle, int start_p)
	{
		AutoLock lock (playlistGuard);
		if (start_p >= list_size || list_size < 2 || !list) return ;
		PlayList_mark();
		if (bytitle == 0)
			std::sort(&list[start_p], &list[list_size], PlayList_sortByFile);
		else if (bytitle == 1)
			std::sort(&list[start_p], &list[list_size], PlayList_sortByTitle);
		else
			std::sort(&list[start_p], &list[list_size], PlayList_sortByDirectory);
		PlayList_restore();
		Playlist_notifyModified();
	}

	void PlayList_reverse(void)
	{
		AutoLock lock (playlistGuard);
		if (!list || list_size <= 1)
			return ;
		PlayList_mark();
		std::reverse(list, &list[list_size]);
		PlayList_restore();
		Playlist_notifyModified();
	}

	void PlayList_randomize(void)
	{
		AutoLock lock (playlistGuard);
		if (!list || list_size <= 1)
			return ;
		PlayList_mark();
		std::random_shuffle(list, &list[list_size], Rand);
		PlayList_restore();
		Playlist_notifyModified();
	}

	void PlayList_adddir(const wchar_t *path, int recurse)
	{
		if (playlistManager)
		{
			AutoLock lock (playlistGuard);
			WinampDirectoryLoad dir(recurse!=0, 0);
			WinampPlaylist playlist(0);
			playlistManager->LoadDirectory(path, &playlist, &dir);
		}
	}

	void PlayList_makerelative(const wchar_t *listfile, wchar_t *filename, int useBase)
	{
		if (useBase)
			MakeRelativePathName(filename, filename, M3UBASE);
		else
		{
			wchar_t path[MAX_PATH] = {0};
			StringCchCopyW(path, MAX_PATH, listfile);
			PathRemoveFileSpecW(path);
			PathRemoveBackslashW(path);
			MakeRelativePathName(filename, filename, path);
		}
	}
}

static void Playlist_notifyModified()
{
	plneedsave = plmodified = 1;
	SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_PLAYLIST_MODIFIED);
}

int IsPlaylistExtension(const wchar_t *ext)
{
	wchar_t temp[32] = {0};
	StringCchPrintfW(temp, 32, L"hi.%s", ext);

	if (playlistManager && playlistManager->CanLoad(temp)) // TODO: make a "valid extension" method
		return 1;
	else
		return 0;
}

int IsPlaylistExtensionA(const char *ext)
{
	return IsPlaylistExtension(AutoWide(ext));
}

void GetMIMEType(const char *url, char *mimeType, int mimeTypeCch);

int LoadPlaylistByExtension(const wchar_t *fn, const wchar_t *ext, int whattodo, int useBase)
{
	if (!playlistManager || !playlistManager->CanLoad(ext))
		return -1;

	//if (useBase)
	//	WASABI_API_APP->path_setWorkingPath(M3UBASE);
		//cut: SetCurrentDirectoryW(M3UBASE);

	int i = 0;

	if (PathIsURLW(fn))
	{
		int rval = 1;
		if (!httpRetrieveFileW(hMainWindow, AutoChar(fn), TEMP_FILE, getStringW(IDS_RETRPL, NULL, 0)))
		{
			rval = LoadPlaylistByExtension(TEMP_FILE, ext, whattodo, useBase);
		}
		DeleteFileW(TEMP_FILE);
		return rval;
	}

	if (PlayList_getlength())
	{
		if (whattodo < 1)
		{
			PlayList_delete();
			i = 1;
		}
	}

	WinampPlaylist playlist(useBase?M3UBASE:0, !useBase);
	playlistManager->LoadAs(fn, ext, &playlist);

	if (i)
	{
		StopPlaying(0);
		if (config_shuffle) PlayList_randpos( -BIGINT);
		PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
		if (whattodo == 0) StartPlaying();
	}
	return 0;
}

int LoadPlaylist(const wchar_t *filename, int whattodo, int doMIMEcheck)
{
	wchar_t ext[65]=L".";
	extension_exW(filename, ext+1, 64); // TODO: make api_playlistmanager use this function
	int x = LoadPlaylistByExtension(filename, ext, whattodo, 0);
	if (doMIMEcheck && x == -1 && PathIsURLW(filename))
	{
		/*
		benski> disabled for now
		GetMIMEType(AutoChar(filename), ext, 64);
		if (*ext)
		{
			if (!_stricmp(ext, "video/x-ms-asf"))
				return LoadPlaylistByExtension(filename, L".asx", whattodo, 0); // TODO: make "load playlist by MIME type" API
			else if (!_stricmp(ext, "video/asx"))
				return LoadPlaylistByExtension(filename, L".asx", whattodo, 0);
		}
		*/
	}
	return x;
}

void PlayList_insert(int position, const wchar_t *filename)
{
	AutoLock lock (playlistGuard);
	PlayList_saveend(position);
	PlayList_appendthing(filename, 0, 0);
	PlayList_restoreend();
}