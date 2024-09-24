#include "main.h"
#include "ml_local.h"

#ifdef _M_IX86
const size_t convert_max_characters = 16; // it's actually 11, but this probably causes less memory fragmentation
#elif defined(_M_X64)
const size_t convert_max_characters = 20;
#endif

bool compat_mode = false;

typedef void (__fastcall *FieldFunc)(itemRecordW *obj, nde_field_t f);
#define FIELD_FUNC(field) field ## Func
#define STRINGFIELD_FUNC(item) static void __fastcall FIELD_FUNC(item)(itemRecordW *obj, nde_field_t f) { obj-> ## item = NDE_StringField_GetString(f); ndestring_retain(obj-> ## item); }
#define INTFIELD_FUNC(item) static void __fastcall FIELD_FUNC(item)(itemRecordW *obj, nde_field_t f) { obj-> ## item = NDE_IntegerField_GetValue(f); }
#define EXT_STRINGFIELD_FUNC(field) static void __fastcall FIELD_FUNC(field)(itemRecordW *obj, nde_field_t f) { setRecordExtendedItem_fast(obj, extended_fields.field, NDE_StringField_GetString(f)); }
#define EXT_INTFIELD_FUNC(field) static void __fastcall FIELD_FUNC(field)(itemRecordW *obj, nde_field_t f) { wchar_t *temp = ndestring_malloc(convert_max_characters*sizeof(wchar_t)); unsigned int v = NDE_IntegerField_GetValue(f); wsprintfW(temp, L"%u", v); setRecordExtendedItem_fast(obj, extended_fields.field, temp); ndestring_release(temp); }
#define REALSIZE_INTFIELD_FUNC(field) static void __fastcall FIELD_FUNC(field)(itemRecordW *obj, nde_field_t f) {\
	wchar_t *temp = ndestring_malloc(convert_max_characters*sizeof(wchar_t));\
	__int64 v = NDE_Int64Field_GetValue(f);\
	obj-> ## field = (v > 0 ? ((int)(compat_mode ? v >> 10 : v)) : 0);\
	wsprintfW(temp, L"%ld", v);\
	setRecordExtendedItem_fast(obj, extended_fields.realsize, temp);\
	ndestring_release(temp);\
}
STRINGFIELD_FUNC(title);
STRINGFIELD_FUNC(artist);
STRINGFIELD_FUNC(album);
INTFIELD_FUNC(year);
STRINGFIELD_FUNC(genre);
STRINGFIELD_FUNC(comment);
INTFIELD_FUNC(track);
INTFIELD_FUNC(length);
INTFIELD_FUNC(type);
INTFIELD_FUNC(lastupd);
INTFIELD_FUNC(lastplay);
INTFIELD_FUNC(rating);
INTFIELD_FUNC(playcount);
INTFIELD_FUNC(filetime);
// use a custom version to set 'filesize' and 'realsize' to ensure compatibility
REALSIZE_INTFIELD_FUNC(filesize);
INTFIELD_FUNC(bitrate);
INTFIELD_FUNC(disc);
STRINGFIELD_FUNC(albumartist);
STRINGFIELD_FUNC(replaygain_album_gain);
STRINGFIELD_FUNC(replaygain_track_gain);
STRINGFIELD_FUNC(publisher);
STRINGFIELD_FUNC(composer);
INTFIELD_FUNC(bpm);
INTFIELD_FUNC(discs);
INTFIELD_FUNC(tracks);
EXT_INTFIELD_FUNC(ispodcast);
EXT_STRINGFIELD_FUNC(podcastchannel);
EXT_INTFIELD_FUNC(podcastpubdate);
EXT_STRINGFIELD_FUNC(GracenoteFileID);
EXT_STRINGFIELD_FUNC(GracenoteExtData);
EXT_INTFIELD_FUNC(lossless);
STRINGFIELD_FUNC(category);
EXT_STRINGFIELD_FUNC(codec);
EXT_STRINGFIELD_FUNC(director);
EXT_STRINGFIELD_FUNC(producer);
EXT_INTFIELD_FUNC(width);
EXT_INTFIELD_FUNC(height);
EXT_STRINGFIELD_FUNC(mimetype);
EXT_INTFIELD_FUNC(dateadded);

static void __fastcall NullFieldFunction(itemRecordW *obj, nde_field_t f) {}
static FieldFunc field_functions[] =
{
	NullFieldFunction, // filename
	FIELD_FUNC(title),
	FIELD_FUNC(artist),
	FIELD_FUNC(album),
	FIELD_FUNC(year),
	FIELD_FUNC(genre),
	FIELD_FUNC(comment),
	FIELD_FUNC(track),
	FIELD_FUNC(length),
	FIELD_FUNC(type),
	FIELD_FUNC(lastupd),
	FIELD_FUNC(lastplay),
	FIELD_FUNC(rating),
	NullFieldFunction, // skip lucky number 13
	NullFieldFunction, // gracenote ID
	FIELD_FUNC(playcount),
	FIELD_FUNC(filetime),
	FIELD_FUNC(filesize),
	FIELD_FUNC(bitrate),
	FIELD_FUNC(disc),
	FIELD_FUNC(albumartist),
	FIELD_FUNC(replaygain_album_gain),
	FIELD_FUNC(replaygain_track_gain),
	FIELD_FUNC(publisher),
	FIELD_FUNC(composer),
	FIELD_FUNC(bpm),
	FIELD_FUNC(discs),
	FIELD_FUNC(tracks),
	FIELD_FUNC(ispodcast),
	FIELD_FUNC(podcastchannel),
	FIELD_FUNC(podcastpubdate),
	FIELD_FUNC(GracenoteFileID),
	FIELD_FUNC(GracenoteExtData),
	FIELD_FUNC(lossless),
	FIELD_FUNC(category),
	FIELD_FUNC(codec),
	FIELD_FUNC(director),
	FIELD_FUNC(producer),
	FIELD_FUNC(width),
	FIELD_FUNC(height),
	FIELD_FUNC(mimetype),
	FIELD_FUNC(dateadded),
};

static int StoreField(void *record, nde_field_t field, void *context)
{
	unsigned char id = NDE_Field_GetID(field);
	if (id < sizeof(field_functions)/sizeof(*field_functions))
	{
		FieldFunc field_function = field_functions[id];
		field_function((itemRecordW *)context, field);
	}
	return 1;
}

static void initRecord(itemRecordW *p)
{
	if (p)
	{
		p->title=0;
		p->album=0;
		p->artist=0;
		p->comment=0;
		p->genre=0;
		p->albumartist=0; 
		p->replaygain_album_gain=0;
		p->replaygain_track_gain=0;
		p->publisher=0;
		p->composer=0;
		p->year=-1;
		p->track=-1;
		p->tracks=-1;
		p->length=-1;
		p->rating=-1;
		p->playcount=-1;
		p->lastplay=-1;
		p->lastupd=-1;
		p->filetime=-1;
		p->filesize=-1;
		p->bitrate=-1;
		p->type=-1;
		p->disc=-1;
		p->discs=-1;
		p->bpm=-1;
		p->extended_info=0; 
		p->category=0;
	}
}

__int64 ScannerRefToObjCacheNFNW(nde_scanner_t s, itemRecordW *obj, bool compat)
{
	initRecord(obj);
	compat_mode = compat;
	NDE_Scanner_WalkFields(s, StoreField, obj);
	return obj->filesize;
}

__int64 ScannerRefToObjCacheNFNW(nde_scanner_t s, itemRecordListW *obj, bool compat)
{
	compat_mode = compat;
	__int64 retval = ScannerRefToObjCacheNFNW(s, &obj->Items[obj->Size], compat);
	obj->Size++;
	return retval;
}