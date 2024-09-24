#include "../nde/nde_c.h"
#include "main.h"
#include <shlwapi.h>
#include "../nu/AutoLock.h"
#include "../nu/AutoWide.h"
#include "cddbinterface.h"
#include "cddb.h"

#include "api__in_cdda.h"
#include <api/service/waservicefactory.h>
#include <atlbase.h>
#include <strsafe.h>

using namespace Nullsoft::Utility;
static nde_database_t discDB=0;
static nde_table_t discTable=0, trackTable=0;
static Nullsoft::Utility::LockGuard dbcs;
static int g_dirty;

enum
{
	NDE_CD_SUCCESS=0,
	NDE_CD_FAILURE=1,
};

enum
{
	DISCTABLE_ID_DISCID = 0,
	DISCTABLE_ID_ALBUM=1,
	DISCTABLE_ID_ARTIST=2,
	DISCTABLE_ID_TUID=3,
	DISCTABLE_ID_YEAR=4,
	DISCTABLE_ID_GENRE=5,
	DISCTABLE_ID_COMMENT=6,
	DISCTABLE_ID_DISC=7,
	DISCTABLE_ID_COMPOSER=8,
	DISCTABLE_ID_PUBLISHER=9,
	DISCTABLE_ID_CONDUCTOR=10,
	DISCTABLE_ID_REMIXING=10,
};

enum
{
	TRACKTABLE_ID_DISCID = 0,
	TRACKTABLE_ID_TRACK = 1,
	TRACKTABLE_ID_ARTIST=2,
	TRACKTABLE_ID_TITLE=3,
	TRACKTABLE_ID_TAGID=4,
	TRACKTABLE_ID_COMPOSER=5,
	TRACKTABLE_ID_CONDUCTOR=6,
	TRACKTABLE_ID_EXTENDED_DATA=7,
	TRACKTABLE_ID_REMIXING=8,
	TRACKTABLE_ID_ISRC=9,
};

static void CreateDiscFields(nde_table_t table)
{
	// create defaults
	NDE_Table_NewColumnW(table, DISCTABLE_ID_DISCID, L"discid", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, DISCTABLE_ID_ALBUM, L"title", FIELD_STRING);
	NDE_Table_NewColumnW(table, DISCTABLE_ID_ARTIST, L"artist", FIELD_STRING);
	NDE_Table_NewColumnW(table, DISCTABLE_ID_TUID, L"tuid", FIELD_STRING);
	NDE_Table_NewColumnW(table, DISCTABLE_ID_YEAR, L"year", FIELD_STRING);
	NDE_Table_NewColumnW(table, DISCTABLE_ID_GENRE, L"genre", FIELD_STRING);
	NDE_Table_NewColumnW(table, DISCTABLE_ID_COMMENT, L"comment", FIELD_STRING);
	NDE_Table_NewColumnW(table, DISCTABLE_ID_DISC, L"disc", FIELD_STRING);
	NDE_Table_NewColumnW(table, DISCTABLE_ID_COMPOSER, L"composer", FIELD_STRING);
	NDE_Table_NewColumnW(table, DISCTABLE_ID_PUBLISHER, L"publisher", FIELD_STRING);
	NDE_Table_NewColumnW(table, DISCTABLE_ID_CONDUCTOR, L"conductor", FIELD_STRING);
	NDE_Table_NewColumnW(table, DISCTABLE_ID_REMIXING, L"remixing", FIELD_STRING);

	NDE_Table_PostColumns(table);
	NDE_Table_AddIndexByIDW(table, DISCTABLE_ID_DISCID, L"discid");
}

static void CreateTrackFields(nde_table_t table)
{
	// create defaults
	NDE_Table_NewColumnW(table, TRACKTABLE_ID_DISCID, L"discid", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, TRACKTABLE_ID_TRACK, L"track", FIELD_INTEGER);
	NDE_Table_NewColumnW(table, TRACKTABLE_ID_ARTIST, L"artist", FIELD_STRING);
	NDE_Table_NewColumnW(table, TRACKTABLE_ID_TITLE, L"title", FIELD_STRING);
	NDE_Table_NewColumnW(table, TRACKTABLE_ID_TAGID, L"tagid", FIELD_STRING);
	NDE_Table_NewColumnW(table, TRACKTABLE_ID_COMPOSER, L"composer", FIELD_STRING);
	NDE_Table_NewColumnW(table, TRACKTABLE_ID_CONDUCTOR, L"conductor", FIELD_STRING);
	NDE_Table_NewColumnW(table, TRACKTABLE_ID_EXTENDED_DATA, L"extendeddata", FIELD_STRING);
	NDE_Table_NewColumnW(table, TRACKTABLE_ID_REMIXING, L"remixing", FIELD_STRING);
	NDE_Table_NewColumnW(table, TRACKTABLE_ID_ISRC, L"ISRC", FIELD_STRING);

	NDE_Table_PostColumns(table);
	NDE_Table_AddIndexByIDW(table, TRACKTABLE_ID_DISCID, L"discid");
	NDE_Table_AddIndexByIDW(table, TRACKTABLE_ID_TRACK, L"track");
}

static int OpenDiscDatabase()
{
	AutoLock lock(dbcs);
	if (!discDB)
	{
		discDB = NDE_CreateDatabase();
	}
	return NDE_CD_SUCCESS;
}

static int OpenDiscTable()
{
	AutoLock lock(dbcs);
	int ret = OpenDiscDatabase();
	if (ret != NDE_CD_SUCCESS)
	{
		return ret;
	}

	if (!discTable)
	{
		const wchar_t *inidir = WASABI_API_APP->path_getUserSettingsPath();
		wchar_t discTablePath[MAX_PATH] = {0}, discIndexPath[MAX_PATH] = {0};
		PathCombineW(discTablePath, inidir, L"plugins");
		PathAppendW(discTablePath, L"cddiscs.dat");
		PathCombineW(discIndexPath, inidir, L"plugins");
		PathAppendW(discIndexPath, L"cddiscs.idx");
		discTable = NDE_Database_OpenTable(discDB, discTablePath, discIndexPath, NDE_OPEN_ALWAYS, NDE_CACHE);
		if (discTable)
		{
			CreateDiscFields(discTable);
		}
	}
	return (discTable ? NDE_CD_SUCCESS : NDE_CD_FAILURE);
}

static int OpenTrackTable()
{
	AutoLock lock(dbcs);
	int ret = OpenDiscDatabase();
	if (ret != NDE_CD_SUCCESS)
	{
		return ret;
	}

	if (!trackTable)
	{
		const wchar_t *inidir = WASABI_API_APP->path_getUserSettingsPath();
		wchar_t trackTablePath[MAX_PATH] = {0}, trackIndexPath[MAX_PATH] = {0};
		PathCombineW(trackTablePath, inidir, L"plugins");
		PathAppendW(trackTablePath, L"cdtracks.dat");
		PathCombineW(trackIndexPath, inidir, L"plugins");
		PathAppendW(trackIndexPath, L"cdtracks.idx");
		trackTable = NDE_Database_OpenTable(discDB, trackTablePath, trackIndexPath, NDE_OPEN_ALWAYS, NDE_CACHE);
		if (trackTable)
		{
			CreateTrackFields(trackTable);
		}
	}
	return (trackTable ? NDE_CD_SUCCESS : NDE_CD_FAILURE);
}

void CloseTables()
{
	if (discTable)
	{
		if (g_dirty & 1) NDE_Table_Sync(discTable);
		NDE_Database_CloseTable(discDB, discTable);
		discTable = 0;
	}

	if (trackTable)
	{
		if (g_dirty & 2) NDE_Table_Sync(trackTable);
		NDE_Database_CloseTable(discDB, trackTable);
		trackTable = 0;
	}

	if (discDB)
	{
		NDE_DestroyDatabase(discDB);
		discDB = 0;
	}
	g_dirty = 0;
}

static void db_setFieldInt(nde_scanner_t s, unsigned char id, int data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
	NDE_IntegerField_SetValue(f, data);
}

static void db_setFieldString(nde_scanner_t s, unsigned char id, const wchar_t *data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
	NDE_StringField_SetString(f, data);
}

static void db_removeField(nde_scanner_t s, unsigned char id)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
	{
		NDE_Scanner_DeleteField(s, f);
	}
}

static int db_getFieldInt(nde_scanner_t s, unsigned char id, int defaultVal)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
		return NDE_IntegerField_GetValue(f);
	else
		return defaultVal;
}

static wchar_t *db_getFieldString(nde_scanner_t s, unsigned char id)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
		return NDE_StringField_GetString(f);
	else
		return 0;
}

static void SeekToDisc(nde_scanner_t s, unsigned int cddb_id)
{
	if (!NDE_Scanner_LocateInteger(s, DISCTABLE_ID_DISCID, FIRST_RECORD, cddb_id))
	{
		NDE_Scanner_New(s);
		db_setFieldInt(s,DISCTABLE_ID_DISCID,cddb_id);
	}
}

static void SeekToTrack(nde_scanner_t s, unsigned int cddb_id, int track)
{
	nde_field_t f_id = NDE_IntegerField_Create(cddb_id);
	NDE_Scanner_AddFilterByID(s, TRACKTABLE_ID_DISCID, f_id, FILTER_EQUALS);
	if (!NDE_Scanner_LocateInteger(s, TRACKTABLE_ID_TRACK, FIRST_RECORD, track))
	{
		NDE_Scanner_New(s);
		db_setFieldInt(s,TRACKTABLE_ID_DISCID,cddb_id);
		db_setFieldInt(s,TRACKTABLE_ID_TRACK,track);
	}
	NDE_Scanner_RemoveFilters(s);
}

#ifndef IGNORE_API_GRACENOTE
void StoreDisc(unsigned int cddb_id, ICddbDiscPtr pDisc)
{
	AutoLock lock(dbcs);
	CComBSTR str, disc_artist, disc_composer, disc_conductor, disc_remixing;
	BSTR composerRole=L"3", conductorRole=L"12", remixingRole=L"147";

	/*
	for (int i=100;i<300;i++)
	{
	wchar_t id[256] = {0};
	_itow(i, id, 10);
	ICddbRolePtr role;
	pCDDBControl->GetRoleInfo(id, &role);
	if (role)
	{
	BSTR name, description;
	role->get_Name(&name);
	role->get_Description(&description);
	wchar_t str[4096] = {0};
	wsprintf(str, L"ID: %s\r\nName: %s\r\nDescription: %s\r\n", id, name, description);
	MessageBoxW(NULL, str, L"CDDB Role", MB_OK);
	}
	}
	*/
	OpenDiscTable();
	nde_scanner_t s = NDE_Table_CreateScanner(discTable);
	SeekToDisc(s, cddb_id);

	ICddbDisc2Ptr pDisc2;
	pDisc->QueryInterface(&pDisc2);

	ICddbDisc2_5Ptr pDisc2_5;
	pDisc->QueryInterface(&pDisc2_5);

	if (GetRole(pDisc, conductorRole, &disc_conductor) && disc_conductor && disc_conductor.m_str[0])
		db_setFieldString(s, DISCTABLE_ID_CONDUCTOR, disc_conductor);
	else
		db_removeField(s, DISCTABLE_ID_CONDUCTOR);

	if (GetRole(pDisc, composerRole, &disc_composer) && disc_composer && disc_composer.m_str[0])
		db_setFieldString(s, DISCTABLE_ID_COMPOSER, disc_composer);
	else
		db_removeField(s, DISCTABLE_ID_COMPOSER);

	if (GetRole(pDisc, remixingRole, &disc_remixing) && disc_remixing && disc_remixing.m_str[0])
		db_setFieldString(s, DISCTABLE_ID_REMIXING, disc_remixing);
	else
		db_removeField(s, DISCTABLE_ID_REMIXING);

	if (SUCCEEDED(pDisc->get_Artist(&disc_artist)) && disc_artist && disc_artist.m_str[0])
		db_setFieldString(s, DISCTABLE_ID_ARTIST, disc_artist);
	else
		db_removeField(s, DISCTABLE_ID_ARTIST);

	if (SUCCEEDED(pDisc->get_Year(&str)) && str && str.m_str[0])
		db_setFieldString(s, DISCTABLE_ID_YEAR, str);
	else
		db_removeField(s, DISCTABLE_ID_YEAR);

	if (pDisc2_5 == NULL
		|| (FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(3, &str))
		&& FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(2, &str))
		&& FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(1, &str))
		&& FAILED(pDisc2_5->get_V2GenreStringPrimary(&str)))
		)
	{
		pDisc->get_GenreId(&str);
		ICddbGenre *poop = 0;
		if (SUCCEEDED(pCDDBControl->GetGenreInfo(str, &poop)) && poop)
		{
			poop->get_Name(&str);
			poop->Release();
		}
		else
			str.Empty();
	}

	if (str && str.m_str[0])
		db_setFieldString(s, DISCTABLE_ID_GENRE, str);
	else
		db_removeField(s, DISCTABLE_ID_GENRE);

	if (SUCCEEDED(pDisc->get_Title(&str)) && str && str.m_str[0])
		db_setFieldString(s, DISCTABLE_ID_ALBUM, str);
	else
		db_removeField(s, DISCTABLE_ID_ALBUM);

	if (SUCCEEDED(pDisc->get_TitleUId(&str)) && str && str.m_str[0])
		db_setFieldString(s, DISCTABLE_ID_TUID, str);
	else
		db_removeField(s, DISCTABLE_ID_TUID);

	if (SUCCEEDED(pDisc->get_Label(&str)) && str && str.m_str[0])
		db_setFieldString(s, DISCTABLE_ID_PUBLISHER, str);
	else
		db_removeField(s, DISCTABLE_ID_PUBLISHER);

	if (SUCCEEDED(pDisc->get_Notes(&str)) && str && str.m_str[0])
		db_setFieldString(s, DISCTABLE_ID_COMMENT, str);
	else
		db_removeField(s, DISCTABLE_ID_COMMENT);

	/*
	long val;
	pDisc->get_Compilation(&val);
	ps->compilation = !!val;
	*/
	int numdiscs = 0;
	if (SUCCEEDED(pDisc->get_TotalInSet(&str)) && str && str.m_str[0])
		numdiscs = _wtoi(str.m_str);
	int discnum =0;
	if (SUCCEEDED(pDisc->get_NumberInSet(&str)) && str && str.m_str[0])
		discnum = _wtoi(str.m_str);

	if (discnum)
	{
		wchar_t disc_temp[64] = {0};
		if (numdiscs)
			StringCchPrintfW(disc_temp, 64, L"%d/%d", discnum, numdiscs);
		else
			StringCchPrintfW(disc_temp, 64, L"%d", discnum);
		db_setFieldString(s, DISCTABLE_ID_DISC, disc_temp);
	}
	else
		db_removeField(s, DISCTABLE_ID_DISC);

	long tracks=0;
	if (FAILED(pDisc->get_NumTracks(&tracks)))
		tracks=0;

	NDE_Scanner_Post(s);
	NDE_Table_DestroyScanner(discTable, s);
	OpenTrackTable();
	s = NDE_Table_CreateScanner(trackTable);
	for (int x = 0; x < tracks; x ++)
	{
		ICddbTrack *t=0;
		ICddbTrack2_5Ptr track2_5;
		if (FAILED(pDisc->GetTrack(x + 1, &t)) || !t)
			break;

		SeekToTrack(s, cddb_id, x+1);

		// don't store if it's the same as the disc artist
		if (SUCCEEDED(t->get_Artist(&str)) && str && str.m_str[0] && (!disc_artist || !disc_artist.m_str[0] || wcscmp(str.m_str, disc_artist.m_str)))
			db_setFieldString(s, TRACKTABLE_ID_ARTIST, str);
		else
			db_removeField(s, TRACKTABLE_ID_ARTIST);

		if (SUCCEEDED(t->get_Title(&str)) && str && str.m_str[0])
			db_setFieldString(s, TRACKTABLE_ID_TITLE, str);
		else
			db_removeField(s, TRACKTABLE_ID_TITLE);

		if (SUCCEEDED(t->get_ISRC(&str)) && str && str.m_str[0])
			db_setFieldString(s, TRACKTABLE_ID_ISRC, str);
		else
			db_removeField(s, TRACKTABLE_ID_ISRC);

		if (SUCCEEDED(pCDDBControl->GetDiscTagId(pDisc, x + 1, &str)) && str && str.m_str[0])
			db_setFieldString(s, TRACKTABLE_ID_TAGID, str);
		else
			db_removeField(s, TRACKTABLE_ID_TAGID);

		// don't store if it's the same as the disc conductor
		if (GetRole(t, conductorRole, &str) && str && str.m_str[0] && (!disc_conductor || !disc_conductor.m_str[0] || wcscmp(str.m_str, disc_conductor.m_str)))
			db_setFieldString(s, TRACKTABLE_ID_CONDUCTOR, str);
		else
			db_removeField(s, TRACKTABLE_ID_CONDUCTOR);

		// don't store if it's the same as the disc composer
		if (GetRole(t, composerRole, &str) && str && str.m_str[0] && (!disc_composer || !disc_composer.m_str[0] || wcscmp(str.m_str, disc_composer.m_str)))
			db_setFieldString(s, TRACKTABLE_ID_COMPOSER, str);
		else
			db_removeField(s, TRACKTABLE_ID_COMPOSER);

		// don't store if it's the same as the disc remixer
		if (GetRole(t, remixingRole, &str) && str && str.m_str[0] && (!disc_remixing || !disc_remixing.m_str[0] || wcscmp(str.m_str, disc_remixing.m_str)))
			db_setFieldString(s, TRACKTABLE_ID_REMIXING, str);
		else
			db_removeField(s, TRACKTABLE_ID_REMIXING);

		t->QueryInterface(&track2_5);

		if (track2_5 != NULL && (SUCCEEDED(track2_5->get_ExtDataSerialized(&str)) && str && str.m_str[0]) // try track first
			|| (pDisc2_5 != NULL && SUCCEEDED(pDisc2_5->get_ExtDataSerialized(&str)) && str && str.m_str[0])) // then disc
			db_setFieldString(s, TRACKTABLE_ID_EXTENDED_DATA, str);
		else
			db_removeField(s, TRACKTABLE_ID_EXTENDED_DATA);

		NDE_Scanner_Post(s);
		t->Release();
	}

	NDE_Table_DestroyScanner(trackTable, s);
	NDE_Table_Sync(trackTable);
	NDE_Table_Sync(discTable);
}
#endif

static void CDText_Process(unsigned int cddb_id, const char *title, const char *performers, const char *composers, int codepage)
{
	AutoLock lock(dbcs);
	OpenDiscTable();
	nde_scanner_t s = NDE_Table_CreateScanner(discTable);
	SeekToDisc(s, cddb_id);

	char thisTitle[1024] = {0};

	const char *titles = title;
	// first, get disc title
	thisTitle[0] = 0;
	titles = ReadLine(titles, thisTitle, 1024, codepage);

	db_setFieldString(s, DISCTABLE_ID_ALBUM, AutoWide(thisTitle, codepage));

	// now get track titles
	OpenTrackTable();
	nde_scanner_t tableScanner = NDE_Table_CreateScanner(trackTable);
	int trackNum = 1;
	while (titles && *titles)
	{
		SeekToTrack(tableScanner, cddb_id, trackNum++);
		thisTitle[0] = 0;
		titles = ReadLine(titles, thisTitle, 1024, codepage);
		db_setFieldString(tableScanner, TRACKTABLE_ID_TITLE, AutoWide(thisTitle, codepage));
		NDE_Scanner_Post(tableScanner);
	}

	titles = performers;
	// now get disc artist
	thisTitle[0] = 0;
	titles = ReadLine(titles, thisTitle, 1024, codepage);
	db_setFieldString(s, DISCTABLE_ID_ARTIST, AutoWide(thisTitle, codepage));

	// now get track artists
	trackNum = 1;
	while (titles && *titles)
	{
		SeekToTrack(tableScanner, cddb_id, trackNum++);

		thisTitle[0] = 0;
		titles = ReadLine(titles, thisTitle, 1024, codepage);
		db_setFieldString(tableScanner, TRACKTABLE_ID_ARTIST, AutoWide(thisTitle, codepage));
		NDE_Scanner_Post(tableScanner);
	}

	titles = composers;
	// now get disc composer
	thisTitle[0] = 0;
	titles = ReadLine(titles, thisTitle, 1024, codepage);
	db_setFieldString(s, DISCTABLE_ID_COMPOSER, AutoWide(thisTitle, codepage));

	// now get track composers
	trackNum = 1;
	while (titles && *titles)
	{
		SeekToTrack(tableScanner, cddb_id, trackNum++);

		thisTitle[0] = 0;
		titles = ReadLine(titles, thisTitle, 1024, codepage);
		db_setFieldString(tableScanner, TRACKTABLE_ID_COMPOSER, AutoWide(thisTitle, codepage));
		NDE_Scanner_Post(tableScanner);
	}

	NDE_Table_DestroyScanner(trackTable, tableScanner);
	NDE_Scanner_Post(s);
	NDE_Table_DestroyScanner(discTable, s);
}

/* returns true if there was CD text */
bool StoreCDText(unsigned int cddb_id, wchar_t device)
{
	if (!device)
		return false;

	#ifndef IGNORE_API_GRACENOTE
	if (config_use_veritas)
	{
		obj_primo *primo=0;
		waServiceFactory *sf = line.service->service_getServiceByGuid(obj_primo::getServiceGuid());
		if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
		if (primo)
		{
			DWORD unit = device;
			if (primo->DiscInfoEx(&unit, 0, NULL, NULL, NULL, NULL, NULL, NULL) != PRIMOSDK_OK) // CDTextInfoEJ suggest that this needs to be called first
			{
				sf->releaseInterface(primo);
				return false;
			}
			char titleE[8192] = "", performerE[8192] = "", composerE[8192] = "", titleJ[2000] = "", performerJ[2000] = "", composerJ[2000] = "";
			if (primo->CDTextInfoEJ(&unit, (PBYTE)titleE, (PBYTE)performerE, (PBYTE)composerE, (PBYTE)titleJ, (PBYTE)performerJ, (PBYTE)composerJ) == PRIMOSDK_OK)
			{
				sf->releaseInterface(primo);
				// read titles
				if (titleE[0])
				{
					CDText_Process(cddb_id, titleE, performerE, composerE, 28591 /* Latin1 */);
					return true;
				}
				else if (titleJ[0])
				{
					CDText_Process(cddb_id, titleJ, performerJ, composerJ, 932 /* SHIFT-JIS */);
					return true;
				}
			}
		}
	}
	#endif

	return false;
}

void StoreCDNoInfo(unsigned int cddb_id)
{
	AutoLock lock(dbcs);
	OpenDiscTable();
	nde_scanner_t s = NDE_Table_CreateScanner(discTable);
	SeekToDisc(s, cddb_id);
	NDE_Scanner_Post(s);
	NDE_Table_DestroyScanner(discTable, s);
	g_dirty |= 1;
}

// sets part and parts to 0 on fail/missing
void ParseIntSlashInt(const wchar_t *string, int *part, int *parts)
{
	*part = 0;
	*parts = 0;

	if (string && string[0])
	{
		*part = _wtoi(string);
		while (string && *string && *string != '/')
		{
			string++;
		}
		if (string && *string == '/')
		{
			string++;
			*parts = _wtoi(string);
		}
	}
}

bool QueryDINFO(unsigned int cddb_id, DINFO *info)
{
	info->Reset();
	AutoLock lock(dbcs);
	if (OpenDiscTable() != NDE_CD_SUCCESS)
		return false;

	nde_scanner_t s = NDE_Table_CreateScanner(discTable);
	if (NDE_Scanner_LocateInteger(s, DISCTABLE_ID_DISCID, FIRST_RECORD, cddb_id))
	{
		ndestring_retain(info->title     = db_getFieldString(s, DISCTABLE_ID_ALBUM));
		ndestring_retain(info->artist    = db_getFieldString(s, DISCTABLE_ID_ARTIST));
		ndestring_retain(info->tuid      = db_getFieldString(s, DISCTABLE_ID_TUID));
		ndestring_retain(info->year      = db_getFieldString(s, DISCTABLE_ID_YEAR));
		ndestring_retain(info->genre     = db_getFieldString(s, DISCTABLE_ID_GENRE));
		ndestring_retain(info->notes     = db_getFieldString(s, DISCTABLE_ID_COMMENT));
		const wchar_t *disc_str          = db_getFieldString(s, DISCTABLE_ID_DISC);
		ParseIntSlashInt(disc_str , &info->discnum, &info->numdiscs);
		ndestring_retain(info->composer  = db_getFieldString(s, DISCTABLE_ID_COMPOSER));
		ndestring_retain(info->label     = db_getFieldString(s, DISCTABLE_ID_PUBLISHER));
		ndestring_retain(info->conductor = db_getFieldString(s, DISCTABLE_ID_CONDUCTOR));

		/* Read tracks */
		OpenTrackTable();
		nde_scanner_t trackScanner = NDE_Table_CreateScanner(trackTable);

		nde_field_t f_id = NDE_IntegerField_Create(cddb_id);
		NDE_Scanner_AddFilterByID(trackScanner, TRACKTABLE_ID_DISCID, f_id, FILTER_EQUALS);

		int trackNum=1;
		while (trackNum < 100)
		{
			TRACKINFO &trackInfo = info->tracks[trackNum-1];
			if (!NDE_Scanner_LocateInteger(trackScanner, TRACKTABLE_ID_TRACK, FIRST_RECORD, trackNum))
				break;
			trackNum++;
			ndestring_retain(trackInfo.artist    = db_getFieldString(trackScanner, TRACKTABLE_ID_ARTIST));
			ndestring_retain(trackInfo.title     = db_getFieldString(trackScanner, TRACKTABLE_ID_TITLE));
			ndestring_retain(trackInfo.tagID     = db_getFieldString(trackScanner, TRACKTABLE_ID_TAGID));
			ndestring_retain(trackInfo.composer  = db_getFieldString(trackScanner, TRACKTABLE_ID_COMPOSER));
			ndestring_retain(trackInfo.conductor = db_getFieldString(trackScanner, TRACKTABLE_ID_CONDUCTOR));
			ndestring_retain(trackInfo.extData   = db_getFieldString(trackScanner, TRACKTABLE_ID_EXTENDED_DATA));
		}
		NDE_Table_DestroyScanner(trackTable, trackScanner);
		NDE_Table_DestroyScanner(discTable, s);
		info->populated = true;
		return true;
	}
	NDE_Table_DestroyScanner(discTable, s);
	return false;
}

static void db_add_or_set(nde_scanner_t s, unsigned char id, wchar_t *data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (data)
	{
		if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
		NDE_StringField_SetNDEString(f, data);
	}
	else if (f)
	{
		NDE_Scanner_DeleteField(s, f);
	}
}

static void db_compare_add_or_set(nde_scanner_t s, unsigned char id, const wchar_t *disc_data, wchar_t *data)
{
	if (disc_data && data && !wcscmp(disc_data, data))
		db_removeField(s, id);
	else
		db_add_or_set(s, id, data);
}

//#ifndef IGNORE_API_GRACENOTE
bool StoreDINFO(unsigned cddb_id, DINFO *info)
{
	AutoLock lock(dbcs);
	if (OpenDiscTable() != NDE_CD_SUCCESS)
		return false;

	nde_scanner_t s = NDE_Table_CreateScanner(discTable);
	SeekToDisc(s, cddb_id);

	db_add_or_set(s, DISCTABLE_ID_ALBUM, info->title);
	db_add_or_set(s, DISCTABLE_ID_ARTIST, info->artist);
	db_add_or_set(s, DISCTABLE_ID_TUID, info->tuid);
	db_add_or_set(s, DISCTABLE_ID_YEAR, info->year);
	db_add_or_set(s, DISCTABLE_ID_GENRE, info->genre);
	db_add_or_set(s, DISCTABLE_ID_PUBLISHER, info->label);
	db_add_or_set(s, DISCTABLE_ID_COMMENT, info->notes);

	if (info->discnum)
	{
		wchar_t disc_temp[64] = {0};
		if (info->numdiscs)
			StringCchPrintfW(disc_temp, 64, L"%d/%d", info->discnum, info->numdiscs);
		else
			StringCchPrintfW(disc_temp, 64, L"%d", info->discnum);
		db_setFieldString(s, DISCTABLE_ID_DISC, disc_temp);
	}
	else
		db_removeField(s, DISCTABLE_ID_DISC);

	db_add_or_set(s, DISCTABLE_ID_CONDUCTOR, info->conductor);
	db_add_or_set(s, DISCTABLE_ID_COMPOSER, info->composer);

	NDE_Scanner_Post(s);
	NDE_Table_DestroyScanner(discTable, s);

	OpenTrackTable();
	s = NDE_Table_CreateScanner(trackTable);
	for (int x=0;x<info->ntracks;x++)
	{
		SeekToTrack(s, cddb_id, x+1);
		TRACKINFO &trackInfo = info->tracks[x];
		db_compare_add_or_set(s, TRACKTABLE_ID_ARTIST, info->artist, trackInfo.artist);
		db_add_or_set(s, TRACKTABLE_ID_TITLE, trackInfo.title);
		db_add_or_set(s, TRACKTABLE_ID_TAGID, trackInfo.tagID);
		db_compare_add_or_set(s, TRACKTABLE_ID_COMPOSER, info->composer, trackInfo.composer);
		db_compare_add_or_set(s, TRACKTABLE_ID_CONDUCTOR, info->conductor, trackInfo.conductor);
		db_add_or_set(s, TRACKTABLE_ID_EXTENDED_DATA, trackInfo.extData);
		NDE_Scanner_Post(s);
	}
	NDE_Table_DestroyScanner(trackTable, s);
	NDE_Table_Sync(trackTable);
	NDE_Table_Sync(discTable);
	g_dirty = 0;
	return true;
}
//#endif