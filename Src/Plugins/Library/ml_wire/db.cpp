#include <shlwapi.h>

#include "api__ml_wire.h"
#include "Downloaded.h"
#include "../nde/nde_c.h"
#include "../nu/AutoLock.h"

/* DB Schema
ChannelTitle
ItemUrl
ItemTitle
PublishDate
Length
Filename
*/

static Nullsoft::Utility::LockGuard dbcs;
static nde_table_t table = 0;
static nde_database_t db = 0;
using namespace Nullsoft::Utility;

enum
{
	DB_ID_CHANNELTITLE = 0,
	DB_ID_ITEMURL      = 1,
	DB_ID_ITEMTITLE    = 2,
	DB_ID_PUBLISHDATE  = 3,
	DB_ID_LENGTH       = 4,
	DB_ID_FILENAME     = 5
};

static bool OpenDatabase()
{
	AutoLock lock(dbcs);
	if (!db)
		db = NDE_CreateDatabase();

	return true;
}

void CloseDatabase()
{
	AutoLock lock( dbcs );
	if ( db )
	{
		if ( table )
			NDE_Database_CloseTable( db, table );

		NDE_DestroyDatabase( db );
	}

	db = 0;
}

static void CreateFields(nde_table_t table)
{
	// create defaults
	NDE_Table_NewColumnW(table, DB_ID_CHANNELTITLE, L"channeltitle", FIELD_STRING);
	NDE_Table_NewColumnW(table, DB_ID_ITEMURL,      L"itemurl",      FIELD_STRING);
	NDE_Table_NewColumnW(table, DB_ID_ITEMTITLE,    L"itemtitle",    FIELD_STRING);
	NDE_Table_NewColumnW(table, DB_ID_PUBLISHDATE,  L"publishdate",  FIELD_DATETIME);
	NDE_Table_NewColumnW(table, DB_ID_LENGTH,       L"length",       FIELD_INTEGER);
	NDE_Table_NewColumnW(table, DB_ID_FILENAME,     L"filename",     FIELD_FILENAME);
	NDE_Table_PostColumns(table);
	NDE_Table_AddIndexByIDW(table, DB_ID_ITEMURL, L"itemurl");
}

static bool OpenTable()
{
	AutoLock lock( dbcs );
	if ( !OpenDatabase() )
		return false;

	if ( !table )
	{
		const wchar_t *inidir = WASABI_API_APP->path_getUserSettingsPath();
		wchar_t tablePath[ MAX_PATH ] = { 0 }, indexPath[ MAX_PATH ] = { 0 };

		PathCombineW( tablePath, inidir, L"plugins" );
		PathAppendW( tablePath, L"podcasts.dat" );
		PathCombineW( indexPath, inidir, L"plugins" );
		PathAppendW( indexPath, L"podcasts.idx" );

		table = NDE_Database_OpenTable( db, tablePath, indexPath, NDE_OPEN_ALWAYS, NDE_CACHE );
		if ( table )
			CreateFields( table );
	}

	return !!table;
}

static void db_add( nde_scanner_t s, unsigned char id, wchar_t *data )
{
	if ( data )
	{
		nde_field_t f = NDE_Scanner_NewFieldByID( s, id );
		NDE_StringField_SetString( f, data );
	}
}

static void db_add_int( nde_scanner_t s, unsigned char id, int data )
{
	if ( data )
	{
		nde_field_t f = NDE_Scanner_NewFieldByID( s, id );
		NDE_IntegerField_SetValue( f, data );
	}
}

static void db_add_time( nde_scanner_t s, unsigned char id, time_t data )
{
	if ( data )
	{
		nde_field_t f = NDE_Scanner_NewFieldByID( s, id );
		NDE_IntegerField_SetValue( f, static_cast<int>( data ) );
	}
}

bool AddPodcastData( const DownloadedFile &data )
{
	AutoLock lock( dbcs );
	if ( !OpenTable() )
		return false;

	nde_scanner_t s = NDE_Table_CreateScanner( table );
	if ( s )
	{
		NDE_Scanner_New( s );
		db_add( s, DB_ID_CHANNELTITLE, data.channel );
		db_add( s, DB_ID_ITEMURL, data.url );
		db_add( s, DB_ID_ITEMTITLE, data.item );
		db_add_time( s, DB_ID_PUBLISHDATE, data.publishDate );
		db_add_int( s, DB_ID_LENGTH, (int)data.totalSize );
		db_add( s, DB_ID_FILENAME, data.path );
		NDE_Scanner_Post( s );
		NDE_Table_DestroyScanner( table, s );
		NDE_Table_Sync( table );

		return true;
	}

	return false;
}

bool IsPodcastDownloaded( const wchar_t *url )
{
	AutoLock lock( dbcs );
	if ( !OpenTable() )
		return false;

	nde_scanner_t s = NDE_Table_CreateScanner( table );
	if ( s )
	{
		if ( NDE_Scanner_LocateString( s, DB_ID_ITEMURL, FIRST_RECORD, url ) )
		{
			NDE_Table_DestroyScanner( table, s );
			return true;
		}

		NDE_Table_DestroyScanner( table, s );
	}

	return false;
}

void CompactDatabase()
{
	AutoLock lock( dbcs );
	if ( OpenTable() )
		NDE_Table_Compact( table );
}
