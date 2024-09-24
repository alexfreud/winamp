#include "main.h"
#include "api__ml_local.h"
#include "../nu/sort.h"
#include "AlbumFilter.h"
#include "resource.h"
#include <shlwapi.h>

static size_t m_sort_by, m_sort_dir, m_sort_which;

void emptyQueryListObject(queryListObject *obj);
int reallocQueryListObject(queryListObject *obj);
void freeQueryListObject(queryListObject *obj);


int AlbumFilter::AlbumSortFunc(const void *elem1, const void *elem2)
{
	const queryListItem *a = (const queryListItem*)elem1;
	const queryListItem *b = (const queryListItem*)elem2;
	int use_by = m_sort_by;
	int use_dir = m_sort_dir;
#define RETIFNZ(v) if ((v)<0) return use_dir?1:-1; if ((v)>0) return use_dir?-1:1;
	if (use_by == ALBUMFILTER_COLUMN_NAME)
	{
		int v = WCSCMP_NULLOK(a->name, b->name);
		RETIFNZ(v)
		v = b->ifields[2] - a->ifields[2];
		RETIFNZ(v)
		v = b->ifields[1] - a->ifields[1];
		return v;
	}
	if (use_by == ALBUMFILTER_COLUMN_YEAR)
	{
		int v = a->ifields[2] - b->ifields[2];
		RETIFNZ(v)
		return WCSCMP_NULLOK(a->name, b->name);
	}
	if (use_by == ALBUMFILTER_COLUMN_ALBUMS)
	{
		int v = a->ifields[0] - b->ifields[0];
		RETIFNZ(v)
		return WCSCMP_NULLOK(a->name, b->name);
	}
	if (use_by == ALBUMFILTER_COLUMN_TRACKS)
	{
		int v = a->ifields[1] - b->ifields[1];
		RETIFNZ(v)
		return WCSCMP_NULLOK(a->name, b->name);
	}

	if (use_by == ALBUMFILTER_COLUMN_REPLAYGAIN)
	{
		int v = FLOATWCMP_NULLOK(a->albumGain, b->albumGain);
		RETIFNZ(v)
		return WCSCMP_NULLOK(a->name, b->name);
	}

	if(use_by == ALBUMFILTER_COLUMN_SIZE)
	{
		__int64 v = a->size - b->size;
		RETIFNZ(v)
		return WCSCMP_NULLOK(a->name, b->name);
	}

	if(use_by == ALBUMFILTER_COLUMN_LENGTH)
	{
		int v = a->length - b->length;
		RETIFNZ(v)
		return WCSCMP_NULLOK(a->name, b->name);
	}
	if(use_by == ALBUMFILTER_COLUMN_RATING) {
		int ar = 0;
		if(a->ifields[1]) ar = a->rating/a->ifields[1];
		int br = 0;
		if(b->ifields[1]) br = b->rating/b->ifields[1];
		int v = ar - br;
		RETIFNZ(v)
	}
	if(use_by == ALBUMFILTER_COLUMN_ARTIST || use_by == ALBUMFILTER_COLUMN_RATING) {
		int v = WCSCMP_NULLOK(a->artist, b->artist);
		RETIFNZ(v)
		v = WCSCMP_NULLOK(a->name, b->name);
		RETIFNZ(v)
	}
	else if(use_by == ALBUMFILTER_COLUMN_GENRE) {
		int v = WCSCMP_NULLOK(a->genre, b->genre);
		RETIFNZ(v)
		v = WCSCMP_NULLOK(a->artist, b->artist);
		RETIFNZ(v)
		v = WCSCMP_NULLOK(a->name, b->name);
		RETIFNZ(v)
	}
	if(use_by == ALBUMFILTER_COLUMN_LASTUPD)
	{
		int v = (int)(b->lastupd - a->lastupd);
		RETIFNZ(v)
		return WCSCMP_NULLOK(a->name, b->name);
	}

#undef RETIFNZ
	return 0;
}

void AlbumFilter::SortResults(C_Config *viewconf, int which, int isfirst) // sorts the results based on the current sort mode
{
	if (viewconf)
	{
		wchar_t buf[64] = {0};
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_sort_by_%d", GetConfigId(), which);
		m_sort_by = viewconf->ReadInt(buf, 0);
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_sort_dir_%d", GetConfigId(), which);
		m_sort_dir = viewconf->ReadInt(buf, 0);
		m_sort_which = which;

		if(showncolumns.size()>m_sort_by && m_sort_by>=0) m_sort_by = showncolumns[m_sort_by]->field;

		if (m_sort_dir == 0 && m_sort_by == 0 && isfirst) return;

		if (albumList.Size > 2) qsort(albumList.Items+1,albumList.Size-1,sizeof(queryListItem),AlbumSortFunc);
	}
}

void AlbumFilter::Fill(itemRecordW *items, int numitems, int *killswitch, int numitems2)
{
	if (numitems > 1) 
		qsort_itemRecord(items,numitems, this, BuildSortFunc);

	if (killswitch && *killswitch) return;

	emptyQueryListObject(&albumList);
	reallocQueryListObject(&albumList);

	itemRecordW *p = items;
	int n = numitems;
	int numalbumstotal = 0;
	wchar_t *lastname = 0;
	const wchar_t *lastalb=0;
	wchar_t albbuf[100] = {0}, albbuf2[100] = {0};
	ZeroMemory(&albumList.Items[0],sizeof(queryListItem));
	int isbl = 0;
	while (n--)
	{
		if (killswitch && *killswitch) return;
		if (!lastname || WCSCMP_NULLOK(lastname, p->album))
		{
			albumList.Size++;
			if (reallocQueryListObject(&albumList)) break;
			wchar_t *albumGain = p->replaygain_album_gain;
			ndestring_retain(albumGain);
			wchar_t *gracenoteFileId = getRecordExtendedItem_fast(p, extended_fields.GracenoteFileID);
			ZeroMemory(&albumList.Items[albumList.Size],sizeof(queryListItem));
			albumList.Items[albumList.Size].albumGain = albumGain;
			albumList.Items[albumList.Size].gracenoteFileId = gracenoteFileId;
			ndestring_retain(gracenoteFileId);
			
			if (p->album)
			{
				ndestring_retain(p->album);
				lastname = albumList.Items[albumList.Size].name = p->album;
			}
			else
				lastname = albumList.Items[albumList.Size].name = emptyQueryListString;

			lastalb=0;
			SKIP_THE_AND_WHITESPACEW(lastname) // optimization technique
			if(*lastname) numalbumstotal++;
		}
		if(p->year>0) {
			int y = albumList.Items[albumList.Size].ifields[2];
			if(y == 0) y = MAKELONG((short)p->year,(short)p->year);
			else if(p->year > (short)LOWORD(y)) y = MAKELONG((short)p->year,(short)HIWORD(y));
			else if(p->year < (short)HIWORD(y)) y = MAKELONG((short)LOWORD(y),(short)p->year);
			albumList.Items[albumList.Size].ifields[2] = y;
		}

		if (!p->album || !*p->album) isbl++;
		if (albumList.Size) {
			albumList.Items[albumList.Size].ifields[1]++;
			if(p->length>0) albumList.Items[albumList.Size].length += p->length;
			if(p->filesize>0) albumList.Items[albumList.Size].size += p->filesize;
		}
		if (nextFilter && (!lastalb ||  WCSCMP_NULLOK(lastalb,nextFilter->GroupText(p,albbuf2,100))))
		{
			lastalb = nextFilter->GroupText(p,albbuf,100);
			if(lastalb && *lastalb) albumList.Items[albumList.Size].ifields[0]++;
			if (lastalb) SKIP_THE_AND_WHITESPACEW(lastalb) // optimization technique
		}
		p++;
	}
	wchar_t buf[64] = {0}, sStr[16] = {0}, langBuf[64] = {0};
	if (isbl)
	{
		wsprintfW(buf, WASABI_API_LNGSTRINGW_BUF(IDS_ALL_X_ALBUMS_X_WITHOUT_ALBUM, langBuf, 64), albumList.Size - 1, WASABI_API_LNGSTRINGW_BUF(albumList.Size == 2 ? IDS_ALBUM : IDS_ALBUMS,sStr,16), isbl);
	}
	else
	{
		wsprintfW(buf, WASABI_API_LNGSTRINGW_BUF(IDS_ALL_X_ALBUMS, langBuf, 64), albumList.Size, WASABI_API_LNGSTRINGW_BUF(albumList.Size == 1 ? IDS_ALBUM : IDS_ALBUMS,sStr,16));
	}

	// for some languages a lowercased first word is invalid so need to prevent this from happening
	process_substantives(buf);

	albumList.Items[0].name = ndestring_wcsdup(buf);
	albumList.Items[0].ifields[1] = numitems;
	albumList.Items[0].ifields[0] = numitems2;
	albumList.Size++;
	numGroups = numalbumstotal;
}

int AlbumFilter::Size()
{
	return albumList.Size;
}

const wchar_t *AlbumFilter::GetText(int row) // gets main text (first column)
{
	return (row < albumList.Size) ? albumList.Items[row].name : NULL;
}

void AlbumFilter::Empty()
{
	freeQueryListObject(&albumList);
}


void AlbumFilter::CopyText(int row, size_t column, wchar_t *dest, int destCch)
{
	if(column >= showncolumns.size()) return;
	column = showncolumns[column]->field;
	if (row>=albumList.Size)
		return ;
	switch(column)
	{
	case ALBUMFILTER_COLUMN_NAME: // name
		if (albumList.Items[row].name && *albumList.Items[row].name)
			lstrcpynW(dest,albumList.Items[row].name,destCch);
		else 
			WASABI_API_LNGSTRINGW_BUF(IDS_NO_ALBUM,dest,destCch);
		break;
	case ALBUMFILTER_COLUMN_YEAR: // year
		if (HIWORD(albumList.Items[row].ifields[2]) < 1)
			dest[0] = 0;
		else {
			int y = albumList.Items[row].ifields[2];
			if(HIWORD(y) == LOWORD(y)) StringCchPrintfW(dest, destCch, L"%d", HIWORD(y));
			else if(HIWORD(y)/100 == LOWORD(y)/100) StringCchPrintfW(dest, destCch, L"%d-%02d", HIWORD(y),LOWORD(y)%100);
			else StringCchPrintfW(dest, destCch, L"%d-%d", HIWORD(y),LOWORD(y));
		}
		break;
	case ALBUMFILTER_COLUMN_ALBUMS: // albums
		StringCchPrintfW(dest, destCch,L"%d",albumList.Items[row].ifields[0]);
		break;
	case ALBUMFILTER_COLUMN_TRACKS: // tracks
		StringCchPrintfW(dest, destCch,L"%d",albumList.Items[row].ifields[1]);
		break;
	case ALBUMFILTER_COLUMN_REPLAYGAIN: // album replay gain
		if (albumList.Items[row].albumGain)
			lstrcpynW(dest, albumList.Items[row].albumGain, destCch);
		else
			dest[0] = 0;
		break;
	case ALBUMFILTER_COLUMN_SIZE: // size
		if(row && albumList.Items[row].size) {
			WASABI_API_LNG->FormattedSizeString(dest, destCch, albumList.Items[row].size);
		} else dest[0]=0;
		break;
	case ALBUMFILTER_COLUMN_LENGTH: // length
		if(row && albumList.Items[row].length)
			StringCchPrintfW(dest, destCch,L"%d:%02d", albumList.Items[row].length / 60, albumList.Items[row].length % 60);
		else dest[0]=0;
		break;
	case ALBUMFILTER_COLUMN_ARTIST: // artist
		if(row && albumList.Items[row].artist)
			lstrcpynW(dest,albumList.Items[row].artist,destCch);
		else
			WASABI_API_LNGSTRINGW_BUF(IDS_NO_ARTIST,dest,destCch);
		break;
	case ALBUMFILTER_COLUMN_GENRE: // genre
		if(row && albumList.Items[row].genre)
			lstrcpynW(dest,albumList.Items[row].genre,destCch);
		else
			WASABI_API_LNGSTRINGW_BUF(IDS_NO_GENRE,dest,destCch);
		break;
	case ALBUMFILTER_COLUMN_RATING: // rating
		dest[0]=0;
		if(row && albumList.Items[row].rating > 0 && albumList.Items[row].ifields[1]) {
			int r = albumList.Items[row].rating / albumList.Items[row].ifields[1];
			if(r >=0 && r<=5) {
				wchar_t ra[]=L"*****";
				ra[r]=0;
				StringCchPrintfW(dest, destCch,L"%d",ra);
			}
		}
		break;
	case ALBUMFILTER_COLUMN_LASTUPD: // last updated
		dest[0]=0;
		if(row && albumList.Items[row].lastupd)
		{
			StringCchPrintfW(dest, destCch,L"%d", albumList.Items[row].lastupd);
		}
		else dest[0]=0;
		break;
	}
}
const wchar_t *AlbumFilter::CopyText2(int row, size_t column, wchar_t *dest, int destCch)
{
	const wchar_t *cdest=dest;
	if(column >= showncolumns.size()) return NULL;
	column = showncolumns[column]->field;
	if (row>=albumList.Size)
		return NULL;
	switch(column)
	{
	case ALBUMFILTER_COLUMN_NAME: // name
		if (albumList.Items[row].name && *albumList.Items[row].name)
			cdest = albumList.Items[row].name;
		else 
			WASABI_API_LNGSTRINGW_BUF(IDS_NO_ALBUM,dest,destCch);
		break;
	case ALBUMFILTER_COLUMN_YEAR: // year
		if (HIWORD(albumList.Items[row].ifields[2]) < 1)
			dest[0] = 0;
		else {
			int y = albumList.Items[row].ifields[2];
			if(HIWORD(y) == LOWORD(y)) StringCchPrintfW(dest, destCch, L"%d", HIWORD(y));
			else if(HIWORD(y)/100 == LOWORD(y)/100) StringCchPrintfW(dest, destCch, L"%d-%02d", HIWORD(y),LOWORD(y)%100);
			else StringCchPrintfW(dest, destCch, L"%d-%d", HIWORD(y),LOWORD(y));
		}
		break;
	case ALBUMFILTER_COLUMN_ALBUMS: // albums
		StringCchPrintfW(dest, destCch,L"%d",albumList.Items[row].ifields[0]);
		break;
	case ALBUMFILTER_COLUMN_TRACKS: // tracks
		StringCchPrintfW(dest, destCch,L"%d",albumList.Items[row].ifields[1]);
		break;
	case ALBUMFILTER_COLUMN_REPLAYGAIN: // album replay gain
		if (albumList.Items[row].albumGain)
			cdest = albumList.Items[row].albumGain;
		else
			dest[0] = 0;
		break;
	case ALBUMFILTER_COLUMN_SIZE: // size
		if(row && albumList.Items[row].size) {
			WASABI_API_LNG->FormattedSizeString(dest, destCch, albumList.Items[row].size);
		} else dest[0]=0;
		break;
	case ALBUMFILTER_COLUMN_LENGTH: // length
		if(row && albumList.Items[row].length)
			StringCchPrintfW(dest, destCch,L"%d:%02d", albumList.Items[row].length / 60, albumList.Items[row].length % 60);
		else dest[0]=0;
		break;
	case ALBUMFILTER_COLUMN_ARTIST: // artist
		if(row && albumList.Items[row].artist)
			cdest = albumList.Items[row].artist;
		else
			WASABI_API_LNGSTRINGW_BUF(IDS_NO_ARTIST,dest,destCch);
		break;
	case ALBUMFILTER_COLUMN_GENRE: // genre
		if(row && albumList.Items[row].genre)
			cdest = albumList.Items[row].genre;
		else
			WASABI_API_LNGSTRINGW_BUF(IDS_NO_GENRE,dest,destCch);
		break;
	case ALBUMFILTER_COLUMN_RATING: // rating
		dest[0]=0;
		if(row && albumList.Items[row].rating > 0 && albumList.Items[row].ifields[1]) {
			double d = double(albumList.Items[row].rating) / double(albumList.Items[row].ifields[1]);
			int r = int(d);
			if(d - double(r) >= 0.5 && r<5) r++;
			if(r >=0 && r<=5) {
				wchar_t ra[]=L"*****";
				ra[r]=0;
				StringCchPrintfW(dest, destCch,L"%s",ra);
			}
		}
		break;
	case ALBUMFILTER_COLUMN_LASTUPD: // last updated
		dest[0]=0;
		if(row && albumList.Items[row].lastupd)
		{
			StringCchPrintfW(dest, destCch,L"%d", albumList.Items[row].lastupd);
		}
		else dest[0]=0;
		break;
	}
	return cdest;
}
const wchar_t *AlbumFilter::GetField()
{
	return DB_FIELDNAME_album;
}

const wchar_t *AlbumFilter::GetName()
{
	return WASABI_API_LNGSTRINGW_BUF(IDS_ALBUMS,name,64);
}

const wchar_t *AlbumFilter::GetNameSingular()
{
	return WASABI_API_LNGSTRINGW_BUF(IDS_ALBUM,sing_name,64);
}

const wchar_t *AlbumFilter::GetNameSingularAlt(int mode)
{
	return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_ALBUM_ALT:(mode==1?2064:2080)),sing_name_alt,64);
}

char * AlbumFilter::getColConfig(int i) {
	switch (i) {
		case 0: return "album";
		case 1: return "year";
		case 2: return "nf";
		case 3: return "tracks";
		case 4: return "albumgain";
		case 5: return "size";
		case 6: return "length";
		case 7: return "artist";
		case 8: return "genre";
		case 9: return "rating";
		case 10: return "lastupd";
	}
	return "";
}

static void readConf(C_Config * c, int col, int * width, int * pos, bool * hidden, int defwidth, int defpos, bool defhidden, char * confid) {
	char * name = AlbumFilter::getColConfig(col);
	wchar_t buf[100] = {0};
	StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%hs", confid, name);
	*width = c->ReadInt(buf, defwidth);
	StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%hs_pos", confid, name);
	*pos = c->ReadInt(buf, defpos);
	StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%hs_hidden", confid, name);
	*hidden = !!c->ReadInt(buf, defhidden);
}

static void saveConf(C_Config * c, int col, int defwidth, int defpos, bool defhidden, bool exists, char * confid) {
	char * name = AlbumFilter::getColConfig(col);
	wchar_t buf[100] = {0};
	if(exists) {
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%hs", confid, name);
		c->WriteInt(buf, defwidth);
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%hs_pos", confid, name);
		c->WriteInt(buf, defpos);
	}
	StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_col_%hs_hidden", confid, name);
	c->WriteInt(buf, defhidden);
}

void AlbumFilter::AddColumns2() {
	int width, pos;
	bool hidden;
	char * config = GetConfigId();
	readConf(g_view_metaconf,0,&width,&pos,&hidden,155,0,false,config);
	showncolumns.push_back(new ListField(0,width,IDS_ALBUM,g_view_metaconf,"",hidden,false,false,pos));
	readConf(g_view_metaconf,1,&width,&pos,&hidden,47,1,false,config);
	showncolumns.push_back(new ListField(1,width,IDS_YEAR,g_view_metaconf,"",hidden,false,false,pos));
	readConf(g_view_metaconf,2,&width,&pos,&hidden,48,2,false,config);
	if(nextFilter)
		showncolumns.push_back(new ListField(2,width,nextFilter->GetName(),g_view_metaconf,"",hidden,false,false,pos));
	readConf(g_view_metaconf,3,&width,&pos,&hidden,47,3,false,config);
	showncolumns.push_back(new ListField(3,width,IDS_TRACKS_MENU,g_view_metaconf,"",hidden,false,false,pos));
	
	readConf(g_view_metaconf,4,&width,&pos,&hidden,77,4,!g_config->ReadInt(L"show_albumgain", 0),config);
	showncolumns.push_back(new ListField(4,width,IDS_ALBUM_GAIN,g_view_metaconf,"",hidden,true,false,pos));
	
	readConf(g_view_metaconf,5,&width,&pos,&hidden,45,5,true,config);
	showncolumns.push_back(new ListField(5,width,IDS_SIZE,g_view_metaconf,"",hidden,true,false,pos));
	readConf(g_view_metaconf,6,&width,&pos,&hidden,45,6,true,config);
	showncolumns.push_back(new ListField(6,width,IDS_LENGTH,g_view_metaconf,"",hidden,true,false,pos));
}


void AlbumFilter::SaveColumnWidths()
{
	char *config = GetConfigId();
	for ( size_t i = 0; i < showncolumns.size(); i++ )
	{
		int field = showncolumns[ i ]->field;
		saveConf( g_view_metaconf, field, list->GetColumnWidth( i ), i, 0, list->ColumnExists( i ), config );
	}
	for ( size_t i = 0; i < hiddencolumns.size(); i++ )
	{
		int field = hiddencolumns[ i ]->field;
		saveConf( g_view_metaconf, field, 0, 0, true, false, config );
	}
}

const wchar_t * AlbumFilter::GroupText(itemRecordW * item, wchar_t * buf, int len) {
	return item->album;
}

void AlbumFilter::CustomizeColumns(HWND parent, BOOL showmenu) {
	ViewFilter::CustomizeColumns(parent,showmenu);
	int v = 0;
	for (size_t i = 0;i < showncolumns.size();i++) if(showncolumns[i]->field == 4) { v=1; break; }
	g_config->WriteInt(L"show_albumgain", v);
}