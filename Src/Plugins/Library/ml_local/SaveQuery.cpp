#include "main.h"
#include "ml_local.h"
#include <shlwapi.h>
#include <assert.h>
#include "../nu/sort.h"

/*
#ifdef _M_IX86
#undef min
static inline int min(int x, int y)
{
	return y+((x-y)>>31)&(x-y);
}
#undef max
static inline int max(int x, int y)
{
	return x-(((x-y)>>(31))&(x-y));
}

#elif defined(_WIN64)
#undef min
static inline int min(int x, int y)
{
	return y+((x-y)>>63)&(x-y);
}
#undef max
static inline int max(int x, int y)
{
	return x-(((x-y)>>(63))&(x-y));
}
#endif
*/

#ifdef _M_IX86
const size_t convert_max_characters = 16; // it's actually 11, but this probably causes less memory fragmentation
#elif defined(_M_X64)
const size_t convert_max_characters = 20;
#endif

static inline int Compare_Int_NegativeIsNull(int v1, int v2)
{
	v1 = max(v1,0);
	v2 = max(v2,0);
	return(v1 - v2);
}

typedef int (__fastcall *SortFunction)(const itemRecordW *a, const itemRecordW *b);
#define SORT(field) SortBy ## field
#define STRING_SORT(field) static int __fastcall SORT(field)(const itemRecordW *a, const itemRecordW *b) { return WCSCMP_NULLOK(a->field, b->field); }
#define EXT_STRING_SORT(field) static int __fastcall SORT(field)(const itemRecordW *a, const itemRecordW *b) { wchar_t *a_field = getRecordExtendedItem_fast(a, extended_fields.field); wchar_t *b_field = getRecordExtendedItem_fast(b, extended_fields.field); return WCSCMP_NULLOK(a_field, b_field);}
#define TIME_SORT(field) static int __fastcall SORT(field)(const itemRecordW *a, const itemRecordW *b) { time_t v1 = (time_t)a->field; time_t v2 = (time_t)b->field; if (v1 == -1) v1 = 0; if (v2 == -1) v2 = 0; return (int)(v1 - v2);}
#define EXT_TIME_SORT(field) static int __fastcall SORT(field)(const itemRecordW *a, const itemRecordW *b) { wchar_t *a_field = getRecordExtendedItem_fast(a, extended_fields.dateadded); wchar_t *b_field = getRecordExtendedItem_fast(b, extended_fields.dateadded); time_t v1 = a_field?_wtoi(a_field):0; time_t v2 = b_field?_wtoi(b_field):0; if (v1 == -1) v1 = 0; if (v2 == -1) v2 = 0; return (int)(v1 - v2);}
#define INT_SORT(field) static int __fastcall SORT(field)(const itemRecordW *a, const itemRecordW *b) { return Compare_Int_NegativeIsNull(a->field, b->field); }
#define EXT_INT_SORT(field) static int __fastcall SORT(field)(const itemRecordW *a, const itemRecordW *b) { wchar_t *a_field = getRecordExtendedItem_fast(a, extended_fields.field); wchar_t *b_field = getRecordExtendedItem_fast(b, extended_fields.field); int v1 = a_field?_wtoi(a_field):0; int v2 = b_field?_wtoi(b_field):0; return (v1-v2);}
#define FLOAT_SORT(field) static int __fastcall SORT(field)(const itemRecordW *a, const itemRecordW *b) { return FLOATWCMP_NULLOK(a->field, b->field); }
STRING_SORT(artist);
STRING_SORT(title);
STRING_SORT(album);
INT_SORT(length);
INT_SORT(track);
STRING_SORT(genre);
INT_SORT(year);
static int __fastcall SORT(filespec)(const itemRecordW *a, const itemRecordW *b)
{
		// remove path before compare...
		wchar_t * af = L"";
		if (a->filename)
			af = PathFindFileNameW(a->filename);

		wchar_t * bf = L"";
		if (b->filename)
			bf = PathFindFileNameW(b->filename);

		return _wcsicmp(af, bf);
}

INT_SORT(rating);
INT_SORT(playcount);
TIME_SORT(lastplay);
TIME_SORT(lastupd);
TIME_SORT(filetime);
STRING_SORT(comment);
INT_SORT(filesize);
INT_SORT(bitrate);
INT_SORT(type);
INT_SORT(disc);
STRING_SORT(albumartist);
STRING_SORT(filename);
FLOAT_SORT(replaygain_album_gain);
FLOAT_SORT(replaygain_track_gain);
STRING_SORT(publisher);
STRING_SORT(composer);
static int __fastcall SORT(extension)(const itemRecordW *a, const itemRecordW *b)
{
		wchar_t *extA = PathFindExtensionW(a->filename);
		wchar_t *extB = PathFindExtensionW(b->filename);
		if (extA && *extA)
			extA++;
		if (extB && *extB)
			extB++;
		return WCSCMP_NULLOK(extA, extB);
}
EXT_INT_SORT(ispodcast);
EXT_STRING_SORT(podcastchannel);
EXT_STRING_SORT(podcastpubdate);
INT_SORT(bpm);
STRING_SORT(category);
EXT_STRING_SORT(director);
EXT_STRING_SORT(producer);
EXT_TIME_SORT(dateadded);
EXT_STRING_SORT(cloud);

static int __fastcall SORT(dimension)(const itemRecordW *a, const itemRecordW *b)
{
	wchar_t *a_width = getRecordExtendedItem_fast(a, extended_fields.width);
	wchar_t *b_width = getRecordExtendedItem_fast(b, extended_fields.width);
	wchar_t *a_height = getRecordExtendedItem_fast(a, extended_fields.height);
	wchar_t *b_height = getRecordExtendedItem_fast(b, extended_fields.height);
	int w1 = a_width?_wtoi(a_width):0;
	int w2 = b_width?_wtoi(b_width):0;
	int h1 = a_height?_wtoi(a_height):0;
	int h2 = b_height?_wtoi(b_height):0;
	if (w1 != w2) 
		return (w1-w2);
	else 
		return (h1-h2);
}
// DISABLED FOR 5.62 RELEASE - DRO
//EXT_STRING_SORT(codec);

#define FORCE_ASCENDING ((SortFunction)-2)
// this is where we define sort orders!
static const SortFunction sortOrder[MEDIAVIEW_COL_NUMS][MEDIAVIEW_COL_NUMS+2] =
{
	{SORT(artist), FORCE_ASCENDING, SORT(album), SORT(disc), SORT(track), SORT(title), 0}, // Artist
	{SORT(title), FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(album), FORCE_ASCENDING, SORT(albumartist), SORT(disc), SORT(track), SORT(title), 0},
	{SORT(length), FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(track), FORCE_ASCENDING, SORT(title), SORT(artist), SORT(album), SORT(disc), 0},
	{SORT(genre), FORCE_ASCENDING, SORT(albumartist), SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(year), FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(filespec), SORT(filename),0},
	{SORT(rating), SORT(playcount), SORT(lastplay), FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(playcount), SORT(lastplay), FORCE_ASCENDING,SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(lastplay), FORCE_ASCENDING,SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(lastupd), FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(filetime), FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(comment), FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(filesize),FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(bitrate),  FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(type), FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(disc), FORCE_ASCENDING, SORT(track), SORT(title), SORT(artist), SORT(album), SORT(disc), 0},
	{SORT(albumartist), FORCE_ASCENDING, SORT(album), SORT(disc), SORT(track), 0},
	{SORT(filename), 0},
	{SORT(replaygain_album_gain), FORCE_ASCENDING, SORT(album), SORT(disc), SORT(track), SORT(title), 0},
	{SORT(replaygain_track_gain), FORCE_ASCENDING, SORT(title), 0},
	{SORT(publisher),FORCE_ASCENDING,SORT(artist), SORT(album), SORT(disc), SORT(track), SORT(title), 0},
	{SORT(composer), FORCE_ASCENDING,SORT(album), SORT(disc), SORT(track), 0},
	{SORT(extension), FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(ispodcast),  FORCE_ASCENDING, SORT(podcastchannel), SORT(title), 0},
	{SORT(podcastchannel), SORT(title), 0},
	{SORT(podcastpubdate), 0},
	{SORT(bpm), 0}, // TODO
	{SORT(category),FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), SORT(title), 0},
	{SORT(director),FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), SORT(title), 0},
	{SORT(producer),FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), SORT(title), 0},
	{SORT(dimension), 0},
	// DISABLED FOR 5.62 RELEASE - DRO
	//{SORT(codec), 0},
	{SORT(dateadded), FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
	{SORT(cloud), FORCE_ASCENDING, SORT(artist), SORT(album), SORT(disc), SORT(track), 0},
};

/* ---- */
struct SortRules
{
	int by;
	int dir;
};
static int __fastcall sortFuncW(const void *elem1, const void *elem2, const void *context)
{
	assert(sizeof(sortOrder) / sizeof(*sortOrder) == MEDIAVIEW_COL_NUMS);

	const itemRecordW *a = (const itemRecordW*)elem1;
	const itemRecordW *b = (const itemRecordW*)elem2;

	const SortRules *rules = (SortRules *)context;
	int use_by = rules->by;
	int use_dir = !!rules->dir;

	int dir_values[] = {-1, 1};

#define RETIFNZ(v) if ((v)<0) return dir_values[use_dir]; if ((v)>0) return -dir_values[use_dir];

	const SortFunction *order =sortOrder[use_by];
	int i=0;
	while (order[i])
	{
		if (order[i]==FORCE_ASCENDING)
		{
			use_dir=0;
			i++;
			continue;
		}
		int v = order[i](a, b);
		RETIFNZ(v);
		i++;
	}
	return 0;
}

void sortResults(int column, int dir, itemRecordListW *obj) // sorts the results based on the current sort mode
{
	if (obj->Size > 1)
	{
		SortRules rules = {column, dir};
		qsort_itemRecord(obj->Items, obj->Size, &rules, sortFuncW);
	}
}

void sortResults(C_Config *viewconf, itemRecordListW *obj) // sorts the results based on the current sort mode
{
	if (viewconf)
	{
		SortRules rules = {viewconf->ReadInt(L"mv_sort_by", MEDIAVIEW_COL_ARTIST), viewconf->ReadInt(L"mv_sort_dir", 0)};
		if (obj->Size > 1)
			qsort_itemRecord(obj->Items, obj->Size,  &rules, sortFuncW);
	}
}

void setCloudValue(itemRecordW *item, const wchar_t* value)
{
	wchar_t *temp = ndestring_malloc(convert_max_characters*sizeof(wchar_t));
	lstrcpynW(temp, value, convert_max_characters);
	setRecordExtendedItem(item, extended_fields.cloud, temp);
	ndestring_release(temp);
}

int saveQueryToListW(C_Config *viewconf, nde_scanner_t s, itemRecordListW *obj,
					 CloudFiles *uploaded, CloudFiles *uploading,
					 resultsniff_funcW cb, int user32, int *killswitch, __int64 *total_bytes)
{
	__int64 total_kb = 0;

	emptyRecordList(obj);

	EnterCriticalSection(&g_db_cs);
	NDE_Scanner_First(s, killswitch);
	if (killswitch && *killswitch)
	{
		LeaveCriticalSection(&g_db_cs);
		return 0;
	}

	int r = 0;
	unsigned int total_length_s = 0;
	unsigned int uncert_cnt = 0;
	unsigned int cert_cnt = 0;
	int recordCount = NDE_Scanner_GetRecordsCount(s);
	allocRecordList(obj, recordCount);
	do
	{
		nde_field_t f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_FILENAME);
		if (f)
		{
			allocRecordList(obj, obj->Size + 1);
			if (!obj->Alloc) break;

			obj->Items[obj->Size].filename = NDE_StringField_GetString(f);
			ndestring_retain(obj->Items[obj->Size].filename);
			total_kb += ScannerRefToObjCacheNFNW(s, obj, (cb == (resultsniff_funcW)-1 ? true : false));

			// look for any files known to the cloud  so the icon can be set accordingly
			// TODO possibly need to do more here to better cope with transient states??
			if (uploaded && uploaded->size() > 0)
			{
				bool found = false;
				for(CloudFiles::iterator iter = uploaded->begin(); iter != uploaded->end(); iter++)
				{
					if (nde_wcsicmp_fn(obj->Items[obj->Size - 1].filename,(*iter)) == 0)
					{
						bool pending = false;
						found = true;

						// catches files being uploaded but not fully known to be in the cloud, etc
						if (uploading && uploading->size() > 0)
						{
							for(CloudFiles::iterator iter2 = uploading->begin(); iter2 != uploading->end(); iter2++)
							{
								if (nde_wcsicmp_fn(obj->Items[obj->Size - 1].filename,(*iter2)) == 0)
								{
									pending = true;
									setCloudValue(&obj->Items[obj->Size - 1], L"5");
									break;
								}
							}
						}

						if (!pending)
						{
							setCloudValue(&obj->Items[obj->Size - 1], L"0");
						}
						break;
					}
				}

				if (!found)
				{
					// catches files being uploaded but not fully known to be in the cloud, etc
					if (uploading && uploading->size() > 0)
					{
						for(CloudFiles::iterator iter = uploading->begin(); iter != uploading->end(); iter++)
						{
							if (nde_wcsicmp_fn(obj->Items[obj->Size - 1].filename,(*iter)) == 0)
							{
								found = true;
								setCloudValue(&obj->Items[obj->Size - 1], L"5");
								break;
							}
						}
					}

					if (!found)
					{
						setCloudValue(&obj->Items[obj->Size - 1], L"4");
					}
				}
			}

			int thisl = obj->Items[obj->Size - 1].length;

			if (thisl > 0)
			{
				total_length_s += thisl;
				cert_cnt++;
			}
			else
			{
				uncert_cnt++;
			}
		}

		r = NDE_Scanner_Next(s, killswitch);
		if (killswitch && *killswitch)
		{
			LeaveCriticalSection(&g_db_cs);
			return 0;
		}
	}
	while (r && !NDE_Scanner_EOF(s));

	if (((Table *)g_table)->HasErrors()) // TODO: don't use C++ NDE API
	{
		wchar_t *last_query = NULL;
		if (m_media_scanner)
		{
			const wchar_t *lq = NDE_Scanner_GetLastQuery(m_media_scanner);
			if (lq) last_query = _wcsdup(lq);
			NDE_Table_DestroyScanner(g_table, m_media_scanner);
		}
		NDE_Table_Compact(g_table);
		if (m_media_scanner)
		{
			m_media_scanner = NDE_Table_CreateScanner(g_table);
			if (last_query != NULL)
			{
				NDE_Scanner_Query(m_media_scanner, last_query);
				free(last_query);
			}
		}
	}
	LeaveCriticalSection(&g_db_cs);

	if (total_bytes)
	{
		// maintain compatibility as needed
		if (cb == (resultsniff_funcW)-1)
			*total_bytes = total_kb * 1024;
		else
			*total_bytes = total_kb;
	}
	compactRecordList(obj);

	if (cb && cb != (resultsniff_funcW)-1)
	{
		cb(obj->Items, obj->Size, user32, killswitch);
	}

	if (killswitch && *killswitch) return 0;

	sortResults(viewconf, obj);

	if (killswitch && *killswitch) return 0;

	if (uncert_cnt)
	{
		if (cert_cnt)
		{
			__int64 avg_len = (__int64)total_length_s / cert_cnt;
			total_length_s += (DWORD)(avg_len * uncert_cnt);
		}
		total_length_s |= (1 << 31);
	}

	return total_length_s;
}