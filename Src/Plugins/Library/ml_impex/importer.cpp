#include "../plist/types.h"
#include "importer.h"
#include "../../General/gen_ml/ml.h"
#include <bfc/string/stringdict.h>
#include <bfc/string/url.h>

BEGIN_STRINGDICTIONARY(_itunesprops)
SDI(L"Track ID", IT_TRACKID);
SDI(L"Name", IT_NAME);
SDI(L"Artist", IT_ARTIST);
SDI(L"Album Artist", IT_ALBUMARTIST);
SDI(L"Album", IT_ALBUM);
SDI(L"Genre", IT_GENRE);
SDI(L"Comments", IT_COMMENTS);
SDI(L"Kind", IT_KIND);
SDI(L"Size", IT_SIZE);
SDI(L"Total Time", IT_TOTALTIME);
SDI(L"Track Number", IT_TRACKNUM);
SDI(L"Track Count", IT_TRACKCOUNT);
SDI(L"Year", IT_YEAR);
SDI(L"Date Modified", IT_DATEMODIFIED);
SDI(L"Date Added", IT_DATEADDED);
SDI(L"Bit Rate", IT_BITRATE);
SDI(L"Bitrate", IT_BITRATE);
SDI(L"Sample Rate", IT_SAMPLERATE);
SDI(L"Rating", IT_RATING);
SDI(L"Location", IT_LOCATION);
SDI(L"File Folder Count", IT_FOLDERCOUNT);
SDI(L"Library Folder Count", IT_LIBFOLDERCOUNT);
SDI(L"Play Count", IT_PLAYCOUNT);
SDI(L"Play Date", IT_PLAYDATE);
SDI(L"Play Date UTC", IT_PLAYDATE_UTC);
SDI(L"Composer", IT_COMPOSER);
SDI(L"Publisher", IT_PUBLISHER);
SDI(L"Disc Number", IT_DISCNUMBER);
SDI(L"Disc Count", IT_DISCCOUNT);
SDI(L"BPM", IT_BPM);
SDI(L"Has Video", IT_HAS_VIDEO);
SDI(L"Grouping", IT_GROUPING);
SDI(L"Producer", IT_PRODUCER);
SDI(L"Director", IT_DIRECTOR);
SDI(L"Artwork Count", IT_ARTWORK_COUNT);
SDI(L"Persistent ID", IT_PERSISTENT_ID);
SDI(L"Track Type", IT_TRACK_TYPE);
SDI(L"HD", IT_HD);
SDI(L"Video Width", IT_VIDEO_WIDTH);
SDI(L"Video Height", IT_VIDEO_HEIGHT);
SDI(L"Movie", IT_MOVIE);
SDI(L"Release Date", IT_RELEASE_DATE);
SDI(L"Normalization", IT_NORMALIZATION);
SDI(L"Sort Name", IT_SORTNAME);
SDI(L"Purchased", IT_PURCHASED);
SDI(L"iTunesU", IT_ITUNESU);
SDI(L"Skip Count", IT_SKIPCOUNT);
SDI(L"Skip Date", IT_SKIPDATE);
SDI(L"Sort Album", IT_SORTALBUM);
SDI(L"Sort Composer", IT_SORTCOMPOSER);
SDI(L"Part Of Gapless Album", IT_PART_OF_GAPLESS_ALBUM);
SDI(L"Compilation", IT_COMPILATION);
SDI(L"Sort Album Artist", IT_SORT_ALBUM_ARTIST);
SDI(L"Sort Artist", IT_SORT_ARTIST);
END_STRINGDICTIONARY(_itunesprops, itunesprops)

void FixPath(const wchar_t *strdata, StringW &f)
{
	f = strdata;
	// if the file starts with the local filename header, strip it
	if (!_wcsnicmp(f, ITUNES_FILENAME_HEADER, wcslen(ITUNES_FILENAME_HEADER))) {
		if (f[wcslen(ITUNES_FILENAME_HEADER)] == '/')
			f = StringW(f.getValue()+wcslen(ITUNES_FILENAME_HEADER)-1);
		else
			f = StringW(f.getValue()+wcslen(ITUNES_FILENAME_HEADER));
		// and then convert the slashes to backslashes
		wchar_t *p = f.getNonConstVal();
		while (p && *p) { if (*p == '/') *p = '\\'; p++; }
	}
	// oddly enough, iTunes XML library filenames have a trailing slash, go figure... and strip it!
	if (f.lastChar() == '\\') f.trunc((int)f.len()-1);
	else if (f.lastChar() == '/') f.trunc((int)f.len()-1); // if this is a url, there was no / to \ conversion
	// decode %XX 
	Url::decode(f);
}

static void Importer_AddKeyToItemRecord(int t, const plistString *data, itemRecordW &ir)
{
	const wchar_t *strdata = data->getString();

	// load this property into the appropriate gen_ml field
	switch (t) 
	{
		case IT_TRACKID:
			// ignored
			break;
		case IT_NAME:
			ir.title = _wcsdup(strdata);
			break;
		case IT_ARTIST:
			ir.artist = _wcsdup(strdata);
			break;
		case IT_ALBUMARTIST:
			ir.albumartist = _wcsdup(strdata);
			break;
		case IT_ALBUM:
			ir.album = _wcsdup(strdata);
			break;
		case IT_GENRE:
			ir.genre = _wcsdup(strdata);
			break;
		case IT_COMMENTS:
			ir.comment = _wcsdup(strdata);
			break;
		case IT_KIND:
			// ignored
			break;
		case IT_LOCATION:
		{
			StringW f;
			FixPath(strdata, f);
			// done
			ir.filename = _wcsdup(f);
			break;
		}
		case IT_COMPOSER:
			ir.composer = _wcsdup(strdata);
			break;
		case IT_PUBLISHER:
			ir.publisher = _wcsdup(strdata);
			break;
		case IT_GROUPING:
			setRecordExtendedItem(&ir, L"category", strdata);
			break;
		case IT_PRODUCER:
			setRecordExtendedItem(&ir, L"producer", strdata);
			break;
		case IT_DIRECTOR:
			setRecordExtendedItem(&ir, L"director", strdata);
			break;
		case IT_PERSISTENT_ID:
			break;
		case IT_TRACK_TYPE:
			break;
		case IT_SORTNAME:
			break;
		case IT_SORTALBUM:
			break;
		case IT_SORTCOMPOSER:
			break;
		case IT_SORT_ALBUM_ARTIST:
			break;
		case IT_SORT_ARTIST:
			break;
		default:
			//DebugStringW(L"Unknown property: %s\n", prop->getName());
			break;
	}
}

static void Importer_AddKeyToItemRecord(int t, const plistInteger *data, itemRecordW &ir)
{
	int64_t value = data->getValue();

	/* benski> we need to keep the ones that were changed to plistBoolean,
	because old exported libraries will still be written with integers */

	// load this property into the appropriate gen_ml field
	switch (t) 
	{
		case IT_TRACKID:
			// ignore
			break;
		case IT_SIZE:
			ir.filesize = (int)(value >> 10);
			setRecordExtendedItem(&ir, L"realsize", data->getString());
			break;
		case IT_TOTALTIME:
			if (value)
				ir.length = (int)(value / 1000);
			break;
		case IT_TRACKNUM:
			ir.track = (int)value;
			break;
		case IT_TRACKCOUNT:
			if (value)
				ir.tracks = (int)value;
			break;
		case IT_YEAR:
			if (value)
				ir.year = (int)value;
			break;
		case IT_BITRATE:
			ir.bitrate = (int)value;
			break;
		case IT_SAMPLERATE:
			// ignored
			break;
		case IT_RATING:
			ir.rating = (int)(((double)value / 100.0) * 5.0);
			break;
		case IT_FOLDERCOUNT:
			// ignored
			break;
		case IT_LIBFOLDERCOUNT:
			// ignored
			break;
		case IT_PLAYCOUNT:
			if (value > 0)
				ir.playcount = (int)value;
			break;
		case IT_PLAYDATE:
			if (value)
				ir.lastplay = value;
			break;
		case IT_DISCNUMBER:
			if (value)
				ir.disc = (int)value;
			break;
		case IT_DISCCOUNT:
			if (value)
				ir.discs = (int)value;
			break;
		case IT_BPM:
			if (value)
				ir.bpm = (int)value;
			break;
		case IT_HAS_VIDEO:
			if (value == 1)
				ir.type = 1;
			break;
		case IT_ARTWORK_COUNT:
			break;
		case IT_VIDEO_WIDTH:
			setRecordExtendedItem(&ir, L"width", data->getString());
			break;
		case IT_VIDEO_HEIGHT:
			setRecordExtendedItem(&ir, L"height", data->getString());
			break;
		case IT_NORMALIZATION:
			// TODO: can we convert this to replay gain?
			break;
		case IT_SKIPCOUNT:
			break;
		default:
			break;
	}
}

static void Importer_AddKeyToItemRecord(int t, const plistBoolean *data, itemRecordW &ir)
{
	int value = !!data->getValue();

	// load this property into the appropriate gen_ml field
	switch (t) 
	{
		case IT_HAS_VIDEO:
			ir.type = value;
			break;
		case IT_HD:
			break;
		case IT_MOVIE:
			break;
		case IT_PURCHASED:
			break;
		case IT_ITUNESU:
			break;
		case IT_PART_OF_GAPLESS_ALBUM:
			break;
		case IT_COMPILATION:
			break;
		default:
			break;
	}
}

static void Importer_AddKeyToItemRecord(int t, const plistDate *data, itemRecordW &ir)
{
	time_t date_value= data->getDate();

	// load this property into the appropriate gen_ml field
	switch (t) 
	{
		case IT_DATEMODIFIED:
			if (date_value != -1)
				ir.filetime = date_value;
			break;
		case IT_DATEADDED:
			if (date_value != -1)
				ir.lastupd = date_value;
			break;
		case IT_PLAYDATE_UTC:
			if (date_value != -1)
				ir.lastplay = date_value;
			break;
		case IT_RELEASE_DATE:
			break;
		case IT_SKIPDATE:
			break;
		default:
			break;
	}
}

void Importer_AddKeyToItemRecord(const plistKey *prop, itemRecordW &ir)
{
	const plistData *data = prop->getData();

	if (data)
	{
		int t = itunesprops.getId(prop->getName());
		switch(data->getType())
		{
			case PLISTDATA_STRING:
				Importer_AddKeyToItemRecord(t, (const plistString *)data, ir);
				break;
			case PLISTDATA_INTEGER:
				Importer_AddKeyToItemRecord(t, (const plistInteger *)data, ir);
				break;
			case PLISTDATA_DATE:
				Importer_AddKeyToItemRecord(t, (const plistDate *)data, ir);
				break;
			case PLISTDATA_BOOLEAN:
				Importer_AddKeyToItemRecord(t, (const plistBoolean *)data, ir);
				break;
			default:
				break;
		}
	}
}