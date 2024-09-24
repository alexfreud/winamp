#include <shlwapi.h>

#include "api__ml_downloads.h"
#include "Downloaded.h"
#include "../nde/nde_c.h"
#include "../nu/AutoLock.h"

/* DB Schema
Source
Url
Title
DownloadDate
Length
Filename
*/

static Nullsoft::Utility::LockGuard dbcs;
static nde_table_t table = 0;
static nde_database_t db = 0;
using namespace Nullsoft::Utility;

enum
{
	DB_ID_SOURCE       = 0,
	DB_ID_URL          = 1,
	DB_ID_TITLE        = 2,
	DB_ID_DOWNLOADDATE = 3,
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

static void CreateFields( nde_table_t table )
{
	// create defaults
	NDE_Table_NewColumnW( table, DB_ID_SOURCE,       L"source",       FIELD_STRING );
	NDE_Table_NewColumnW( table, DB_ID_URL,          L"url",          FIELD_STRING );
	NDE_Table_NewColumnW( table, DB_ID_TITLE,        L"title",        FIELD_STRING );
	NDE_Table_NewColumnW( table, DB_ID_DOWNLOADDATE, L"downloaddate", FIELD_DATETIME );
	NDE_Table_NewColumnW( table, DB_ID_LENGTH,       L"length",       FIELD_INTEGER );
	NDE_Table_NewColumnW( table, DB_ID_FILENAME,     L"filename",     FIELD_FILENAME );
	NDE_Table_PostColumns( table );
	NDE_Table_AddIndexByIDW( table, DB_ID_URL, L"url" );
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
		PathAppendW( tablePath, L"downloads.dat" );

		PathCombineW( indexPath, inidir, L"plugins" );
		PathAppendW( indexPath, L"downloads.idx" );

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

bool AddDownloadData( const DownloadedFile &data )
{
	AutoLock lock( dbcs );
	if ( !OpenTable() )
		return false;

	nde_scanner_t s = NDE_Table_CreateScanner( table );
	if ( s )
	{
		NDE_Scanner_New( s );
		db_add( s, DB_ID_SOURCE, data.source );
		db_add( s, DB_ID_URL, data.url );
		db_add( s, DB_ID_TITLE, data.title );
		db_add_time( s, DB_ID_DOWNLOADDATE, data.downloadDate );
		db_add_int( s, DB_ID_LENGTH, (int)data.totalSize );
		db_add( s, DB_ID_FILENAME, data.path );
		NDE_Scanner_Post( s );
		NDE_Table_DestroyScanner( table, s );
		NDE_Table_Sync( table );

		return true;
	}
	return false;
}

void CompactDatabase()
{
	AutoLock lock( dbcs );
	if ( OpenTable() )
		NDE_Table_Compact( table );
}
