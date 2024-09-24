#include "ArtistAlbumLists.h"
#include "Filters.h"
#include "api__ml_pmp.h"
#include "resource1.h"
#include "metadata_utils.h"

extern winampMediaLibraryPlugin plugin;

#define SKIP_THE_AND_WHITESPACE(x) { while (!iswalnum(*x) && *x) x++; if (!_wcsnicmp(x,L"the ",4)) x+=4; while (*x == L' ') x++; }
int STRCMP_NULLOK(const wchar_t *pa, const wchar_t *pb) {
	if (!pa) pa=L"";
	else SKIP_THE_AND_WHITESPACE(pa)
	if (!pb) pb=L"";
	else SKIP_THE_AND_WHITESPACE(pb)
	return lstrcmpi(pa,pb);
}
#undef SKIP_THE_AND_WHITESPACE

Filter * getFilter(wchar_t *name);

#define RETIFNZ(v) if ((v)<0) return use_dir?1:-1; if ((v)>0) return use_dir?-1:1;

typedef struct
{
	songid_t songid;
	Device * dev;
} SortSongItem;

int thread_killed = 0;
static int useby, usedir, usecloud;
static Device * currentDev;

static int sortFunc(const void *elem1, const void *elem2)
{
	int use_by=useby;
	int use_dir=usedir;
	songid_t a=(songid_t)*(void **)elem1;
	songid_t b=(songid_t)*(void **)elem2;
	wchar_t bufa[2048] = {0}, bufb[2048] = {0};

	// this might be too slow, but it'd be nice
	for (int x = 0; x < 5; x ++)
	{
		if (thread_killed) break;

		bufa[0]=bufb[0]=0;
		if (use_by == (7+usecloud)) // year -> artist -> album -> track
		{
			int v1=currentDev->getTrackYear(a);
			int v2=currentDev->getTrackYear(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=0;
		}
		else if (use_by == 4 && usecloud)	 // cloud -> artist -> album -> track
		{
			currentDev->getTrackExtraInfo(a,L"cloud",bufa,ARRAYSIZE(bufa));
			currentDev->getTrackExtraInfo(b,L"cloud",bufb,ARRAYSIZE(bufb));
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=0;
		}
		else if (use_by == 1) // title -> artist -> album -> disc -> track
		{
			currentDev->getTrackTitle(a,bufa,2048);
			currentDev->getTrackTitle(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=0;
		}
		else if (use_by == 0) // artist -> album -> disc -> track -> title
		{
			currentDev->getTrackArtist(a,bufa,2048);
			currentDev->getTrackArtist(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=2;
		}
		else if (use_by == 2) // album -> disc -> track -> title -> artist
		{
			currentDev->getTrackAlbum(a,bufa,2048);
			currentDev->getTrackAlbum(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_dir=0;
			use_by=5;
		}
		else if (use_by == 5+usecloud) // disc -> track -> title -> artist -> album
		{
			int v1=currentDev->getTrackDiscNum(a);
			int v2=currentDev->getTrackDiscNum(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=4;
		}
		else if (use_by == 4+usecloud) // track -> title -> artist -> album -> disc
		{
			int v1=currentDev->getTrackTrackNum(a);
			int v2=currentDev->getTrackTrackNum(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=1;
		}
		else if (use_by == 6+usecloud) // genre -> artist -> album -> disc -> track
		{
			currentDev->getTrackGenre(a,bufa,2048);
			currentDev->getTrackGenre(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=0;
		}
		else if (use_by == 3) // length -> artist -> album -> disc -> track
		{
			int v1=currentDev->getTrackLength(a);
			int v2=currentDev->getTrackLength(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=0;
		}
		else if (use_by == (8+usecloud)) // bitrate -> artist -> album -> disc -> track
		{
			int v1=currentDev->getTrackBitrate(a);
			int v2=currentDev->getTrackBitrate(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=0;
		}
		else if (use_by == (9+usecloud)) // size -> artist -> album -> disc -> track
		{
			__int64 v1=currentDev->getTrackSize(a);
			__int64 v2=currentDev->getTrackSize(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=0;
		}
		else if (use_by == (10+usecloud)) // playcount -> artist -> album -> disc -> track
		{
			int v1=currentDev->getTrackPlayCount(a);
			int v2=currentDev->getTrackPlayCount(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=0;
		}
		else if (use_by == (11+usecloud)) // rating -> artist -> album -> disc -> track
		{
			int v1=currentDev->getTrackRating(a);
			int v2=currentDev->getTrackRating(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=0;
		}
		else if (use_by == (12+usecloud))
		{
			double v = difftime((time_t)currentDev->getTrackLastPlayed(a),(time_t)currentDev->getTrackLastPlayed(b));
			RETIFNZ(v);
			use_by=0;
		}
		else if (use_by == (13+usecloud)) // album artist -> album
		{
			currentDev->getTrackAlbumArtist(a,bufa,ARRAYSIZE(bufa));
			currentDev->getTrackAlbumArtist(b,bufb,ARRAYSIZE(bufb));
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=2;
		}
		else if (use_by == (14+usecloud)) // publisher -> album
		{
			currentDev->getTrackPublisher(a,bufa,ARRAYSIZE(bufa));
			currentDev->getTrackPublisher(b,bufb,ARRAYSIZE(bufb));
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=2;
		}
		else if (use_by == (15+usecloud)) // composer -> album
		{
			currentDev->getTrackComposer(a,bufa,ARRAYSIZE(bufa));
			currentDev->getTrackComposer(b,bufb,ARRAYSIZE(bufb));
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=2;
		}
		else if (use_by == (16+usecloud)) // mime -> artist -> album -> disc -> track
		{
			currentDev->getTrackMimeType(a,bufa,ARRAYSIZE(bufa));
			currentDev->getTrackMimeType(b,bufb,ARRAYSIZE(bufb));
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=2;
		}
		else if (use_by == (17+usecloud)) // date added -> artist -> album -> disc -> track
		{
			double v = difftime((time_t)currentDev->getTrackDateAdded(a),(time_t)currentDev->getTrackDateAdded(b));
			RETIFNZ(v);
			use_by=0;
		}
		else break; // no sort order?

		if (thread_killed) break;
	}
	return 0;
}

static int sortFunc_filteritems(const void *elem1, const void *elem2) {
	FilterItem *a=(FilterItem *)*(void **)elem1;
	FilterItem *b=(FilterItem *)*(void **)elem2;
	return a->compareTo2(b,useby,usedir);
}

class FilterList : public ListContents {
public:
	Filter * filter;
	C_ItemList * items;
	ArtistAlbumLists * aaList;

	wchar_t topString[128];
	int nextFilterNum;
	int tracks;

	int sortcol;
	int sortdir;
	int id;

	FilterList(int id, Device * dev0, C_Config * config, Filter * filter, ArtistAlbumLists * aaList, bool cloud) :
			   id(id), filter(filter), sortcol(0), sortdir(0), aaList(aaList), tracks(0), nextFilterNum(0), items(0) {
		this->topString[0] = 0;
		this->config = config;
		this->dev = dev0;
		this->cloud = cloud;
		this->cloudcol = -1;
		filter->AddColumns(dev, &fields, config, !!this->cloud);

		for(int i = 0; i < fields.GetSize(); i++) {
			if (!lstrcmpi(((ListField *)fields.Get(i))->name, L"cloud"))
			{
				this->cloudcol = ((ListField *)fields.Get(i))->pos;
				break;
			}
		}

		wchar_t temp[16] = {0};
		wsprintf(temp, L"filter%d_sortcol", id);
		this->sortcol = config->ReadInt(temp, 0);
		wsprintf(temp, L"filter%d_sortdir", id);
		this->sortdir = config->ReadInt(temp, 0);

		this->SortColumns();
	}
	virtual ~FilterList() {
		wchar_t temp[16] = {0};
		wsprintf(temp, L"filter%d_sortcol", id);
		config->WriteInt(temp, sortcol);
		wsprintf(temp, L"filter%d_sortdir", id);
		config->WriteInt(temp, sortdir);
	}
	virtual int GetNumColumns() { return fields.GetSize(); }
	virtual int GetNumRows() { return items->GetSize() + (filter->HaveTopItem()?1:0); }
	virtual wchar_t * GetColumnTitle(int num) {
		if(num >=0 && num < fields.GetSize())
			return ((ListField *)fields.Get(num))->name;
		return L"";
	}
	virtual int GetColumnWidth(int num) {
		if(num >=0 && num < fields.GetSize())
			return ((ListField *)fields.Get(num))->width;
		return 0;
	}
	virtual void GetCellText(int row, int col, wchar_t * buf, int buflen) {
		buf[0]=0;
		if(col >= fields.GetSize() || aaList->bgThread_Handle) return;
		int colid = ((ListField*)fields.Get(col))->field;
		if(filter->HaveTopItem()) {
			if(row) ((FilterItem*)items->Get(row-1))->GetCellText(colid,buf,buflen);
			else {
				if(colid%100 == 0) lstrcpyn(buf,topString,buflen);
				else if(colid%100 == 41) wsprintf(buf,L"%d",tracks);
				else if(colid%100 == 40 && filter->nextFilter) wsprintf(buf,L"%d",nextFilterNum);
			}
		} else ((FilterItem*)items->Get(row))->GetCellText(colid,buf,buflen);
	}
	virtual void SortList() {
		if (sortcol > fields.GetSize()) return;
		useby=((ListField*)fields.Get(sortcol))->field;
		usedir=sortdir;
		qsort(items->GetAll(),items->GetSize(),sizeof(void*),sortFunc_filteritems);
	}
	virtual int GetSortDirection() { return sortdir; }
	virtual int GetSortColumn() { return sortcol; }
	virtual void ColumnClicked(int col) {
		if(col == sortcol) 
			sortdir = sortdir?0:1;
		else {
			sortdir=0;
			sortcol=col;
		}
		SortList();
	}
	virtual void ColumnResize(int col, int newWidth) {
		if(col >=0 && col < fields.GetSize()) {
			ListField * lf = (ListField *)fields.Get(col);
			lf->width = newWidth;
			wchar_t buf[100] = {0};
			wsprintf(buf,L"colWidth_%d",lf->field);
			config->WriteInt(buf,newWidth);
		}
	}
	virtual pmpart_t GetArt(int row) { return ((FilterItem*)items->Get(row))->GetArt(); }
	virtual void SetMode(int mode) {
		filter->SetMode(mode);
		int i=fields.GetSize();
		while(i>=0) { i--; delete (ListField*)fields.Get(i); fields.Del(i); }
		filter->AddColumns(dev, &fields, config, !!this->cloud);
		SortColumns();
	}
	virtual songid_t GetTrack(int pos) { return 0; }
};

static void getStars(int stars, wchar_t * buf, int buflen) {
	wchar_t * r=L"";
	switch(stars) {
		case 1: r=L"\u2605"; break;
		case 2: r=L"\u2605\u2605"; break;
		case 3: r=L"\u2605\u2605\u2605"; break;
		case 4: r=L"\u2605\u2605\u2605\u2605"; break;
		case 5: r=L"\u2605\u2605\u2605\u2605\u2605"; break;
	}
	lstrcpyn(buf,r,buflen);
}

extern void timeToString(__time64_t time, wchar_t * buf, int buflen);

static void timeValue(int totalsecs, wchar_t *dest)
{
	int secs=totalsecs%60;
	int mins=(totalsecs/60)%60;
	int hours=(totalsecs/3600)%24;
	int days=(totalsecs/86400);
	if(days==0) {
		wsprintf(dest,L"%d:%02d:%02d",hours,mins,secs);
	} else if(days==1) {
		wsprintf(dest,L"%d day+%d:%02d:%02d",days,hours,mins,secs);
	} else {
	    wsprintf(dest,L"%d days+%d:%02d:%02d",days,hours,mins,secs);
	}
}

void GetInfoString(wchar_t * buf, Device * dev, int numTracks, __int64 totalSize, int totalPlayLength, int cloud) {
	wchar_t lengthStr[100]=L"";
	wchar_t sizeStr[100]=L"";
	wchar_t availStr[100]=L"";
	wchar_t devCapacityStr[100]=L"";
	int usedPercent;

	int fieldsBits = (int)dev->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
	if(!fieldsBits || (fieldsBits & SUPPORTS_LENGTH)) {
		lengthStr[0] = L'[';
		timeValue(totalPlayLength,&lengthStr[1]);
		wcscat_s(lengthStr,100,L"] ");
	}

	__int64 available = dev->getDeviceCapacityAvailable();
	WASABI_API_LNG->FormattedSizeString(sizeStr, ARRAYSIZE(sizeStr), totalSize);
	WASABI_API_LNG->FormattedSizeString(availStr, ARRAYSIZE(availStr), available);
	__int64 capacity = dev->getDeviceCapacityTotal();
	WASABI_API_LNG->FormattedSizeString(devCapacityStr, ARRAYSIZE(devCapacityStr), capacity);

	if(capacity > 0) usedPercent = (int)((((__int64)100)*available) / capacity);
	else usedPercent = 0;
	if (!cloud || cloud && available > 0)
		wsprintf(buf, WASABI_API_LNGSTRINGW(IDS_X_ITEMS_X_AVAILABLE), numTracks, lengthStr, sizeStr, availStr, usedPercent, devCapacityStr);
	else
		wsprintf(buf, WASABI_API_LNGSTRINGW(IDS_X_ITEMS_X_AVAILABLE_SLIM), numTracks, lengthStr, sizeStr);
}

class TracksList : public PrimaryListContents {
public:
	__int64 totalSize;
	int totalPlayLength;
	int sortcol;
	int sortdir;
	C_ItemList * tracks;
	Device * dev;
	ArtistAlbumLists * aaList;
	TracksList(Device * dev,C_Config * config, ArtistAlbumLists * aaList, bool cloud) :
		dev(dev), aaList(aaList), totalSize(0), totalPlayLength(0), tracks(0) {
		this->config = config;
		this->cloud = cloud;
		this->cloudcol = -1;

		this->sortcol = config->ReadInt(L"sortcol", 0);
		this->sortdir = config->ReadInt(L"sortdir", 0);

		int fieldsBits = (int)dev->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
		if (!fieldsBits) fieldsBits = -1;
		if (fieldsBits & SUPPORTS_ARTIST)		fields.Add(new ListField(0, 200, WASABI_API_LNGSTRINGW(IDS_ARTIST), config));
		if (fieldsBits & SUPPORTS_TITLE)		fields.Add(new ListField(1, 200, WASABI_API_LNGSTRINGW(IDS_TITLE), config));
		if (fieldsBits & SUPPORTS_ALBUM)		fields.Add(new ListField(2, 200, WASABI_API_LNGSTRINGW(IDS_ALBUM), config));
		if (fieldsBits & SUPPORTS_LENGTH)		fields.Add(new ListField(3, 64, WASABI_API_LNGSTRINGW(IDS_LENGTH), config));
		if (cloud)								fields.Add(new ListField(3+cloud, 27, WASABI_API_LNGSTRINGW(IDS_CLOUD), config));
		if (fieldsBits & SUPPORTS_TRACKNUM)		fields.Add(new ListField(4+cloud, 50, WASABI_API_LNGSTRINGW(IDS_TRACK_NUMBER), config));
		if (fieldsBits & SUPPORTS_DISCNUM)		fields.Add(new ListField(5+cloud, 38, WASABI_API_LNGSTRINGW(IDS_DISC), config));
		if (fieldsBits & SUPPORTS_GENRE)		fields.Add(new ListField(6+cloud, 100, WASABI_API_LNGSTRINGW(IDS_GENRE), config));
		if (fieldsBits & SUPPORTS_YEAR)			fields.Add(new ListField(7+cloud, 38, WASABI_API_LNGSTRINGW(IDS_YEAR), config));
		if (fieldsBits & SUPPORTS_BITRATE)		fields.Add(new ListField(8+cloud, 45, WASABI_API_LNGSTRINGW(IDS_BITRATE), config));
		if (fieldsBits & SUPPORTS_SIZE)			fields.Add(new ListField(9+cloud, 90, WASABI_API_LNGSTRINGW(IDS_SIZE), config));
		if (fieldsBits & SUPPORTS_PLAYCOUNT)	fields.Add(new ListField(10+cloud, 64, WASABI_API_LNGSTRINGW(IDS_PLAY_COUNT), config));
		if (fieldsBits & SUPPORTS_RATING)		fields.Add(new ListField(11+cloud, 64, WASABI_API_LNGSTRINGW(IDS_RATING), config));
		if (fieldsBits & SUPPORTS_LASTPLAYED)	fields.Add(new ListField(12+cloud, 120, WASABI_API_LNGSTRINGW(IDS_LAST_PLAYED), config));
		if (fieldsBits & SUPPORTS_ALBUMARTIST)	fields.Add(new ListField(13+cloud, 200, WASABI_API_LNGSTRINGW(IDS_ALBUM_ARTIST), config, true));
		if (fieldsBits & SUPPORTS_PUBLISHER)	fields.Add(new ListField(14+cloud, 200, WASABI_API_LNGSTRINGW(IDS_PUBLISHER), config, true));
		if (fieldsBits & SUPPORTS_COMPOSER)		fields.Add(new ListField(15+cloud, 200, WASABI_API_LNGSTRINGW(IDS_COMPOSER), config, true));
		if (fieldsBits & SUPPORTS_MIMETYPE)		fields.Add(new ListField(16+cloud, 100, WASABI_API_LNGSTRINGW(IDS_MIME_TYPE), config, true));
		if (fieldsBits & SUPPORTS_DATEADDED)	fields.Add(new ListField(17+cloud, 120, WASABI_API_LNGSTRINGW(IDS_DATE_ADDED), config, true));
		this->SortColumns();

		if (cloud)
		{
			// not pretty but it'll allow us to know the current
			// position of the cloud column for drawing purposes
			for(int i = 0; i < fields.GetSize(); i++) {
				if (!lstrcmpi(((ListField *)fields.Get(i))->name, L"cloud"))
				{
					this->cloudcol = ((ListField *)fields.Get(i))->pos;
					break;
				}
			}
		}
	}
	virtual ~TracksList() {
		config->WriteInt(L"sortcol", sortcol);
		config->WriteInt(L"sortdir", sortdir);
	}
	virtual int GetNumColumns() { return fields.GetSize(); }
	virtual int GetNumRows() { return (tracks ? tracks->GetSize() : 0); }
	virtual int GetColumnWidth(int num) {
		if(num >=0 && num < fields.GetSize())
			return ((ListField *)fields.Get(num))->width;
		return 0;
	}
	virtual wchar_t * GetColumnTitle(int num) {
		if(num >=0 && num < fields.GetSize())
			return ((ListField *)fields.Get(num))->name;
		return L"";
	}
	virtual void GetCellText(int row, int col, wchar_t * buf, int buflen) {
		buf[0]=0;
		if(row >= tracks->GetSize() || aaList->bgThread_Handle) return;
		songid_t s = (songid_t)tracks->Get(row);
		if(col >=0 && col < fields.GetSize()) {
			if (cloud)
			{
				switch(((ListField *)fields.Get(col))->field) {
					case 0: dev->getTrackArtist(s,buf,buflen); return;
					case 1: dev->getTrackTitle(s,buf,buflen); return;
					case 2: dev->getTrackAlbum(s,buf,buflen); return;
					case 3: { int l=dev->getTrackLength(s); if (l>=0) wsprintf(buf,L"%d:%02d",l/1000/60,(l/1000)%60); return; }
					case 4: { dev->getTrackExtraInfo(s,L"cloud",buf,buflen); return; }
					case 5: { int t=dev->getTrackTrackNum(s); if (t>0) wsprintf(buf,L"%d",t); return; }
					case 6: { int d = dev->getTrackDiscNum(s); if(d>0)  wsprintf(buf,L"%d",d); return; }
					case 7: dev->getTrackGenre(s,buf,buflen); return;
					case 8: { int d = dev->getTrackYear(s); if(d>0) wsprintf(buf,L"%d",d); return; }
					case 9: { int d = dev->getTrackBitrate(s);  if(d>0) wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_KBPS),d); return; }
					case 10: WASABI_API_LNG->FormattedSizeString(buf, buflen, dev->getTrackSize(s)); return;
					case 11: { int d = dev->getTrackPlayCount(s); if(d>=0) wsprintf(buf,L"%d",d); return; }
					case 12: getStars(dev->getTrackRating(s),buf,buflen); return;
					case 13: timeToString(dev->getTrackLastPlayed(s),buf,buflen); return;
					case 14: dev->getTrackAlbumArtist(s,buf,buflen); return;
					case 15: dev->getTrackPublisher(s,buf,buflen); return;
					case 16: dev->getTrackComposer(s,buf,buflen); return;
					case 17: dev->getTrackMimeType(s,buf,buflen); return;
					case 18: timeToString(dev->getTrackDateAdded(s),buf,buflen); return;
				}
			}
			else
			{
				switch(((ListField *)fields.Get(col))->field) {
					case 0: dev->getTrackArtist(s,buf,buflen); return;
					case 1: dev->getTrackTitle(s,buf,buflen); return;
					case 2: dev->getTrackAlbum(s,buf,buflen); return;
					case 3: { int l=dev->getTrackLength(s); wsprintf(buf,L"%d:%02d",l/1000/60,(l/1000)%60); return; }
					case 4: { int t=dev->getTrackTrackNum(s); if (t>0) wsprintf(buf,L"%d",t); return; }
					case 5: { int d = dev->getTrackDiscNum(s); if(d>0)  wsprintf(buf,L"%d",d); return; }
					case 6: dev->getTrackGenre(s,buf,buflen); return;
					case 7: { int d = dev->getTrackYear(s); if(d>0) wsprintf(buf,L"%d",d); return; }
					case 8: { int d = dev->getTrackBitrate(s);  if(d>0) wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_KBPS),d); return; }
					case 9: WASABI_API_LNG->FormattedSizeString(buf, buflen, dev->getTrackSize(s)); return;
					case 10: { int d = dev->getTrackPlayCount(s); if(d>=0) wsprintf(buf,L"%d",d); return; }
					case 11: getStars(dev->getTrackRating(s),buf,buflen); return;
					case 12: timeToString(dev->getTrackLastPlayed(s),buf,buflen); return;
					case 13: dev->getTrackAlbumArtist(s,buf,buflen); return;
					case 14: dev->getTrackPublisher(s,buf,buflen); return;
					case 15: dev->getTrackComposer(s,buf,buflen); return;
					case 16: dev->getTrackMimeType(s,buf,buflen); return;
					case 17: timeToString(dev->getTrackDateAdded(s),buf,buflen); return;
				}
			}
		}
	}
	virtual void SortList() {
		useby = ((ListField*)fields.Get(sortcol))->field;
		usedir = sortdir;
		// if a cloud item then adjust things as needed
		// since we're inserting between genre and year
		usecloud = cloud;
		thread_killed = 0;
		currentDev = dev;
		qsort(tracks->GetAll(),tracks->GetSize(),sizeof(void*),sortFunc);
	}
	virtual void ColumnResize(int col, int newWidth) {
		if(col >=0 && col < fields.GetSize()) {
			ListField * lf = (ListField *)fields.Get(col);
			lf->width = newWidth;
			wchar_t buf[100] = {0};
			wsprintf(buf,L"colWidth_%d",lf->field);
			config->WriteInt(buf,newWidth);
		}
	}
	virtual int GetSortColumn() { return sortcol; }
	virtual int GetSortDirection() { return sortdir; }
	virtual void ColumnClicked(int col) {
		if(col == sortcol)
			sortdir = sortdir?0:1;
		else {
			sortdir=0;
			sortcol=col;
		}
		SortList();
	}
	virtual void GetInfoString(wchar_t * buf) {
		::GetInfoString(buf, dev, tracks->GetSize(), totalSize, totalPlayLength, cloud);
	}
	virtual songid_t GetTrack(int pos) { return (songid_t)tracks->Get(pos); }
	virtual void RemoveTrack(songid_t song) {
		for(int i=0; i<tracks->GetSize(); i++) {
			if((songid_t)tracks->Get(i) == song) tracks->Del(i--);
		}
	}
};

static void FreeFilterItemList(C_ItemList * list) {
	if(!list) return;
	for(int i=0; i < list->GetSize(); i++) {
    FilterItem * a = (FilterItem*)list->Get(i);
		if(a->nextFilter) FreeFilterItemList(a->nextFilter); a->nextFilter=0;
		delete a;
	}
	delete list;
}

static DWORD WINAPI bgThreadSearchProc(void *tmp)
{
	ArtistAlbumLists *aacList = (ArtistAlbumLists *)tmp;
	return (aacList ? aacList->bgSearchThreadProc(tmp) : 0);
}

static DWORD WINAPI bgThreadLoadProc(void *tmp)
{
	ArtistAlbumLists *aacList = (ArtistAlbumLists *)tmp;
	return (aacList ? aacList->bgLoadThreadProc(tmp) : 0);
}

static DWORD WINAPI bgThreadRefineProc(void *tmp)
{
	ArtistAlbumLists *aacList = (ArtistAlbumLists *)tmp;
	return (aacList ? aacList->bgRefineThreadProc(tmp) : 0);
}

extern HWND hwndMediaView;
DWORD WINAPI ArtistAlbumLists::bgLoadThreadProc(void *tmp)
{
	int l = dev->getPlaylistLength(playlistId);

	for(int i=0; i<l; i++) {
		songid_t x=dev->getPlaylistTrack(playlistId,i);
		int t = dev->getTrackType(x);
		if(type != -1 && t != type) continue;
		searchedTracks->Add((void*)x);
		trackList->Add((void*)x);
		unrefinedTracks->Add((void*)x);
	}

	for(int i=0; i<numFilters; i++) {
		if(i==0) {
			if (filters[0]) FreeFilterItemList(filters[0]->items);
			filters[0]->items = CompilePrimaryList(searchedTracks);
		}
		else {
			delete filters[i]->items;
			filters[i]->items = CompileSecondaryList(filters[i-1]->items,i,true);
		}
		filters[i]->SortList();
	}

	SetRefine(L"");

	if (!bgThread_Kill) PostMessage(hwndMediaView, WM_APP + 3, 0x69, 0);
	return 0;
}

DWORD WINAPI ArtistAlbumLists::bgSearchThreadProc(void *tmp)
{
	int l = dev->getPlaylistLength(playlistId);
	C_ItemList allTracks;

	for(int i=0; i<l; i++) {
		songid_t x=dev->getPlaylistTrack(playlistId,i);
		int t = dev->getTrackType(x);
		if(type != -1 && t != type) continue;
		allTracks.Add((void*)x);
	}

	searchedTracks = FilterSongs(lastSearch, &allTracks);
	delete unrefinedTracks;
	unrefinedTracks = new C_ItemList;
	l=searchedTracks->GetSize();
	for(int i=0; i<l; i++) unrefinedTracks->Add(searchedTracks->Get(i));
	for(int i=0; i<numFilters; i++) {
		if(i==0) {
			FreeFilterItemList(filters[0]->items);
			filters[0]->items = CompilePrimaryList(searchedTracks);
		}
		else {
			delete filters[i]->items;
			filters[i]->items = CompileSecondaryList(filters[i-1]->items,i,true);
		}
	}
	SetRefine(L"");

	for(int i=0; i<numFilters; i++) filters[i]->SortList();

	if (!bgThread_Kill) PostMessage(hwndMediaView, WM_APP + 3, 0x69, 0);
	return 0;
}

DWORD WINAPI ArtistAlbumLists::bgRefineThreadProc(void *tmp)
{
	SetRefine(lastRefine);

	if (!bgThread_Kill) PostMessage(hwndMediaView, WM_APP + 3, 0x69, 0);
	return 0;
}

void ArtistAlbumLists::bgQuery_Stop() // exported for other people to call since it is useful (eventually
{
	KillTimer(hwndMediaView, 123);
	if (bgThread_Handle)
	{
		thread_killed = bgThread_Kill = 1;
		WaitForSingleObject(bgThread_Handle, INFINITE);
		CloseHandle(bgThread_Handle);
		bgThread_Handle = 0;
	}
}

void ArtistAlbumLists::bgQuery(int mode) // only internal used
{
	bgQuery_Stop();

	// TODO cache the HWND to avoid confusion
	SetTimer(hwndMediaView, 123, 200, NULL);
	DWORD id;
	bgThread_Kill = 0;
	bgThread_Handle = CreateThread(NULL, 0, (!mode ? bgThreadLoadProc : (mode == 1 ? bgThreadSearchProc : bgThreadRefineProc)), (LPVOID)this, 0, &id);
}

ArtistAlbumLists::ArtistAlbumLists(Device * dev, int playlistId, C_Config * config, wchar_t ** filterNames, int numFilters0, int type, bool async) {
	this->type = type;
	this->dev = dev;
	this->playlistId = playlistId;
	this->numFilters = numFilters0;
	this->bgThread_Handle = 0;
	this->async = async;
	this->lastSearch = 0;
	this->lastRefine = 0;
	ZeroMemory(&filters, sizeof(filters));

	if (config->ReadInt(L"savefilter", 1))
	{
		lastSearch = wcsdup(config->ReadString(L"savedfilter", L""));
		lastRefine = wcsdup(config->ReadString(L"savedrefinefilter", L""));
	}

	Filter * f = NULL;
	for(int i = 0; i < numFilters; i++) {	
		if(!i) { f = firstFilter = getFilter(filterNames[i]); }
		else { f->nextFilter = getFilter(filterNames[i]); f = f->nextFilter; }
	}

	f = firstFilter;	
	for(int i=0; i<numFilters; i++) {
		filters[i] = new FilterList(i,dev,config,f,this,async);
		f = f->nextFilter;
	}

	tracksLC = new TracksList(dev,config,this,async);

	searchedTracks = new C_ItemList;
	trackList = new C_ItemList;
	unrefinedTracks = new C_ItemList;
	if (!async && (!lastSearch || lastSearch && !*lastSearch))
	{
		int l = dev->getPlaylistLength(playlistId);
		for(int i=0; i<l; i++) {
			songid_t x=dev->getPlaylistTrack(playlistId,i);
			int t = dev->getTrackType(x);
			if(type != -1 && t != type) continue;
			searchedTracks->Add((void*)x);
			trackList->Add((void*)x);
			unrefinedTracks->Add((void*)x);
		}
	}

	for(int i=0; i<numFilters; i++) {
		if(i==0) filters[i]->items = CompilePrimaryList(searchedTracks);
		else filters[i]->items = CompileSecondaryList(filters[i-1]->items,i,true);
		filters[i]->SortList();
	}

	tracksLC->tracks = trackList;
	tracksLC->dev=dev;

	if (async) bgQuery();
	else SetRefine((lastRefine ? lastRefine : L""));
}

ArtistAlbumLists::~ArtistAlbumLists() {
	if(numFilters && filters[0]) FreeFilterItemList(filters[0]->items);
	for(int i=0; i<numFilters; i++) {
		delete filters[i]->filter;
		if(i!=0) delete filters[i]->items;
		delete filters[i];
	}
	delete trackList;
	delete searchedTracks;
	delete unrefinedTracks;
	delete tracksLC;

	if (lastSearch) free(lastSearch);
	if (lastRefine) free(lastRefine);
}

static void parsequicksearch(wchar_t *out, wchar_t *in) // parses a list into a list of terms that we are searching for
{
	int inquotes=0, neednull=0;
	while (in && *in)
	{
		wchar_t c=*in++;
		if (c != L' ' && c != L'\t' && c != L'\"')
		{
			neednull=1;
			*out++=c;
		}
		else if (c == L'\"') 
		{
			inquotes=!inquotes;
			if (!inquotes) 
			{
				*out++=0;
				neednull=0;
			}
		}
		else
		{
			if (inquotes) *out++=c;
			else if (neednull)
			{
				*out++=0;
				neednull=0;
			}
		}
	}
	*out++=0;
	*out++=0;
}

static int in_string(wchar_t *string, wchar_t *substring)
{
	if (!string) return 0;
	if (!*substring) return 1;
	int l=lstrlen(substring);
	while (string[0]) if (!_wcsnicmp(string++,substring,l)) return 1; 
	return 0;
}

C_ItemList * FilterSongs(const wchar_t * filter, const C_ItemList * songs, Device * dev, bool cloud)
{
	wchar_t filterstr[256] = {0}, filteritems[300] = {0};
	lstrcpyn(filterstr,filter,256);
	parsequicksearch(filteritems,filterstr);

	C_ItemList * filtered = new C_ItemList;
	int l = songs->GetSize();
	for(int i=0; i<l; i++) {
		songid_t s = (songid_t)songs->Get(i);
		wchar_t *p=filteritems;
		if(p && *p) {
			while(p && *p) {
				bool in=false;
				for(int j=0; j<15; j++) {
					wchar_t buf[2048] = {0};
					int buflen=2048;
					if (cloud)
					{
						switch(j) {
							case 0: dev->getTrackArtist(s,buf,buflen); break;
							case 1: dev->getTrackTitle(s,buf,buflen); break;
							case 2: dev->getTrackAlbum(s,buf,buflen); break;
							case 3: { int l=dev->getTrackLength(s); wsprintf(buf,L"%d:%02d",l/1000/60,(l/1000)%60); break; }
							case 4: { dev->getTrackExtraInfo(s,L"cloud",buf,buflen); break; }
							case 5: wsprintf(buf,L"%d",dev->getTrackTrackNum(s)); break;
							case 6: wsprintf(buf,L"%d",dev->getTrackDiscNum(s)); break;
							case 7: dev->getTrackGenre(s,buf,buflen); break;
							case 8: wsprintf(buf,L"%d",dev->getTrackYear(s)); break;
							case 9: wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_KBPS),dev->getTrackBitrate(s)); break;
							case 10: WASABI_API_LNG->FormattedSizeString(buf, buflen, dev->getTrackSize(s)); break;
							case 11: wsprintf(buf,L"%d",dev->getTrackPlayCount(s)); break;
							case 12: getStars(dev->getTrackRating(s),buf,buflen); break;
							case 13: timeToString(dev->getTrackLastPlayed(s),buf,buflen); break;
							case 14: dev->getTrackAlbumArtist(s,buf,buflen); break;
							case 15: dev->getTrackPublisher(s,buf,buflen); break;
							case 16: dev->getTrackComposer(s,buf,buflen); break;
							case 17: dev->getTrackMimeType(s,buf,buflen); break;
							case 18: timeToString(dev->getTrackDateAdded(s),buf,buflen); break;
							default: lstrcpyn(buf,L"",buflen); break;
						}
					}
					else
					{
						switch(j) {
							case 0: dev->getTrackArtist(s,buf,buflen); break;
							case 1: dev->getTrackTitle(s,buf,buflen); break;
							case 2: dev->getTrackAlbum(s,buf,buflen); break;
							case 3: { int l=dev->getTrackLength(s); wsprintf(buf,L"%d:%02d",l/1000/60,(l/1000)%60); break; }
							case 4: wsprintf(buf,L"%d",dev->getTrackTrackNum(s)); break;
							case 5: wsprintf(buf,L"%d",dev->getTrackDiscNum(s)); break;
							case 6: dev->getTrackGenre(s,buf,buflen); break;
							case 7: wsprintf(buf,L"%d",dev->getTrackYear(s)); break;
							case 8: wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_KBPS),dev->getTrackBitrate(s)); break;
							case 9: WASABI_API_LNG->FormattedSizeString(buf, buflen, dev->getTrackSize(s)); break;
							case 10: wsprintf(buf,L"%d",dev->getTrackPlayCount(s)); break;
							case 11: getStars(dev->getTrackRating(s),buf,buflen); break;
							case 12: timeToString(dev->getTrackLastPlayed(s),buf,buflen); break;
							case 13: dev->getTrackAlbumArtist(s,buf,buflen); break;
							case 14: dev->getTrackPublisher(s,buf,buflen); break;
							case 15: dev->getTrackComposer(s,buf,buflen); break;
							case 16: dev->getTrackMimeType(s,buf,buflen); break;
							case 17: timeToString(dev->getTrackDateAdded(s),buf,buflen); break;
							default: lstrcpyn(buf,L"",buflen); break;
						}
					}
					if(in_string(buf,p)) { in=true; break;}
				}
				if(in) p+=lstrlen(p)+1;
				else break;
			}
		}
		if(p && *p) continue;
		filtered->Add((void*)s);
	}
	return filtered;
}

C_ItemList * ArtistAlbumLists::FilterSongs(const wchar_t * filter, const C_ItemList * songs)
{
	return ::FilterSongs(filter,songs,dev,this->async);
}

static Filter * firstFil;

static int sortFunc_ssi(const void *elem1, const void *elem2)
{
	SortSongItem * a = (SortSongItem *)elem1;
	SortSongItem * b = (SortSongItem *)elem2;
	Filter * f = firstFil;
	while(f) {
		int r = f->sortFunc(a->dev,a->songid,b->songid);
		if(r) return r;
		f = f->nextFilter;
	}
	return 0;
}

C_ItemList * ArtistAlbumLists::CompilePrimaryList(const C_ItemList * songs) {
	C_ItemList * list = new C_ItemList;

	SortSongItem *songList = (SortSongItem*)calloc(songs->GetSize(), sizeof(SortSongItem));
	int l=songs->GetSize();
	for(int i=0; i<l; i++) { songList[i].songid = (songid_t)songs->Get(i); songList[i].dev = dev; }

	firstFil = firstFilter;
	qsort(songList,l,sizeof(SortSongItem),sortFunc_ssi); //sort it

	FilterItem * items[MAX_FILTERS]={0};
	Filter * filters[MAX_FILTERS]={0};

	Filter * f = firstFilter;
	int numFilters=0;
	while(f) {filters[numFilters++] = f; f=f->nextFilter;}

	for(int i=0; i<l; i++) {
		songid_t s = songList[i].songid;
		for(int j=0; j<numFilters; j++) {
			if(items[j] && filters[j]->isInGroup(dev,s,items[j])) filters[j]->addToGroup(dev,s,items[j]);
			else {
				for(int k=j; k<numFilters; k++) {
					items[k] = filters[k]->newGroup(dev,s);
					items[k]->nextFilter = new C_ItemList;
					if(k==0) list->Add(items[k]);
					else {
						items[k-1]->nextFilter->Add(items[k]);
						items[k-1]->numNextFilter++;
					}
				}
				break;
			}
		}
	}
	if (songList)
	{
		free(songList);
		songList = 0;
	}

	if(list->GetSize() && ((FilterItem*)list->Get(0))->isWithoutGroup()) {
		wsprintf(this->filters[0]->topString,WASABI_API_LNGSTRINGW(IDS_ALL_X_WITHOUT_X),
				 list->GetSize()-1,(list->GetSize()==2)?this->filters[0]->filter->name:this->filters[0]->filter->namePlural,((FilterItem*)list->Get(0))->numTracks,this->filters[0]->filter->name);
	}
	else
		wsprintf(this->filters[0]->topString,WASABI_API_LNGSTRINGW(IDS_ALL_X),
				 list->GetSize(),(list->GetSize()==1)?this->filters[0]->filter->name:this->filters[0]->filter->namePlural);
	CharLower(this->filters[0]->topString+3);
	this->filters[0]->tracks = songs->GetSize();
	return list;
}

static int sortFunc_filters(const void *elem1, const void *elem2) {
	FilterItem *a=(FilterItem *)*(void **)elem1;
	FilterItem *b=(FilterItem *)*(void **)elem2;
	return a->compareTo(b);
}

C_ItemList * ArtistAlbumLists::CompileSecondaryList(const C_ItemList * selectedItems, int level, bool updateTopArtist) {
	int totalTracks=0;
	C_ItemList * list = new C_ItemList;
	C_ItemList * collatedlist = new C_ItemList;

	for(int i=0; i < selectedItems->GetSize(); i++) {
		FilterItem * item = (FilterItem*)selectedItems->Get(i);
		if (item) {
			C_ItemList *nf = item->independentNextFilter?item->independentNextFilter:item->nextFilter;
			for(int j=0; j < nf->GetSize(); j++) list->Add(nf->Get(j));
		}
	}
	qsort(list->GetAll(),list->GetSize(),sizeof(void*),sortFunc_filters);

	FilterItem * curItem=0;
	for(int i=0; i < list->GetSize(); i++) {
		FilterItem * item = (FilterItem*)list->Get(i);
		if(curItem && !curItem->compareTo(item)) {
			curItem->independentTracks += item->numTracks;
			curItem->independentSize += item->size;
			curItem->independentLength += item->length;
			curItem->independentCloudState += item->cloudState;
		} else {
			curItem = item;
			collatedlist->Add(item);
			item->independentTracks = item->numTracks;
			curItem->independentSize = item->size;
			curItem->independentLength = item->length;
			curItem->independentCloudState = item->cloudState;
			if(item->independentNextFilter) delete item->independentNextFilter;
			item->independentNextFilter = new C_ItemList;
		}
		totalTracks+=item->numTracks;
		for(int k=0; k<item->nextFilter->GetSize(); k++) curItem->independentNextFilter->Add(item->nextFilter->Get(k));
	}
	delete list;
	if(updateTopArtist) this->filters[level-1]->nextFilterNum = collatedlist->GetSize();

	if(collatedlist->GetSize() && ((FilterItem*)collatedlist->Get(0))->isWithoutGroup()) {
		wsprintf(this->filters[level]->topString,WASABI_API_LNGSTRINGW(IDS_ALL_X_WITHOUT_X),
				 collatedlist->GetSize(),(collatedlist->GetSize()==1)?this->filters[level]->filter->name:this->filters[level]->filter->namePlural,((FilterItem*)collatedlist->Get(0))->numTracks,this->filters[level]->filter->name);
	}
	else {
		wsprintf(this->filters[level]->topString,WASABI_API_LNGSTRINGW(IDS_ALL_X),
				 collatedlist->GetSize(),(collatedlist->GetSize()==1)?this->filters[level]->filter->name:this->filters[level]->filter->namePlural);
	}
	CharLower(this->filters[level]->topString+3);
	this->filters[level]->tracks=totalTracks;
	return collatedlist;
}

// removes song from all relevant lists
void ArtistAlbumLists::RemoveTrack(songid_t song)
{
	if (searchedTracks)
	{
		for (int i=0;i<searchedTracks->GetSize();i++)
		{
			if (searchedTracks->Get(i) == (void *) song)
			{
				searchedTracks->Del(i--);
			}
		}
	}
}

void ArtistAlbumLists::SelectionChanged(int filterNum, SkinnedListView **listview) {
	for(int i=filterNum; i<numFilters-1; i++) {
		int l = listview[i]->listview.GetCount();
		bool all = (i != filterNum || (ListView_GetSelectedCount(listview[i]->listview.getwnd())==0));
		if(listview[i]->listview.GetSelected(0) && filters[i]->filter->HaveTopItem()) all=true;
		C_ItemList selectedItems;
		int j=0,f=0;
		if(filters[i]->filter->HaveTopItem()) { j=1; f=1; }
		for(; j<l; j++) {
			if(all || listview[i]->listview.GetSelected(j)) 
				selectedItems.Add(filters[i]->items->Get(j-f));
		}
		delete filters[i+1]->items;
		filters[i+1]->items = CompileSecondaryList(&selectedItems,i+1,false);
		filters[i+1]->SortList();
		listview[i+1]->UpdateList();
	}

	C_ItemList * tracks = new C_ItemList;

	C_ItemList * selectedItems[MAX_FILTERS]={0};

	for(int i=0; i<=filterNum; i++) {
		bool all = (ListView_GetSelectedCount(listview[i]->listview.getwnd())==0);
		if(listview[i]->listview.GetSelected(0) && filters[i]->filter->HaveTopItem()) all=true;
		if(all) selectedItems[i] = NULL;
		else {
			selectedItems[i] = new C_ItemList;
			int m = filters[i]->items->GetSize();
			int offset = filters[i]->filter->HaveTopItem()?1:0;
			for(int k=0; k<m; k++) if(listview[i]->listview.GetSelected(k+offset)) selectedItems[i]->Add(filters[i]->items->Get(k));
		}
	}

	int l=searchedTracks->GetSize();
	for(int j=0; j<l; j++) {
		songid_t track = (songid_t)searchedTracks->Get(j);
		bool matches=true;
		for(int i=0; i<=filterNum; i++) {
			matches=false;
			if(selectedItems[i]) {
				for(int k=0; k<selectedItems[i]->GetSize(); k++) 
					if(filters[i]->filter->isInGroup(dev,track,(FilterItem*)selectedItems[i]->Get(k))) { matches=true; break; }
			}
			else matches=true;
			if(!matches) break;
		}
		if(matches) //woo hoo, its in!
			tracks->Add((void*)track);
	}
	delete unrefinedTracks;
	unrefinedTracks = tracks;
	SetRefine(L"");
	for(int i=0; i<=filterNum; i++) 
	{
		delete selectedItems[i];
	}
}

void ArtistAlbumLists::SetRefine(const wchar_t * str, bool async) {
	if (!async)
	{
		C_ItemList * refinedTracks = FilterSongs(str,unrefinedTracks);
		C_ItemList * oldTrackList = trackList;
		trackList = refinedTracks;
		tracksLC->tracks = trackList;
		delete oldTrackList;
		tracksLC->SortList();

		// get stats
		__int64 fileSize=0;
		int playLength=0;
		int millis=0;
		for(int i=0; i < trackList->GetSize(); i++) {
			songid_t s = (songid_t)trackList->Get(i);
			fileSize += (__int64)dev->getTrackSize(s);
			playLength += dev->getTrackLength(s)/1000;
			millis += dev->getTrackLength(s)%1000;
		}
		playLength += millis/1000;
		tracksLC->totalPlayLength=playLength;
		tracksLC->totalSize=fileSize;
	}
	else
	{
		if (lastRefine)
		{
			free(lastRefine);
			lastRefine = 0;
		}
		lastRefine = wcsdup(str);

		bgQuery(2);
	}
}

void ArtistAlbumLists::SetSearch(const wchar_t * str, bool async) {
	bgQuery_Stop();

	if (!async)
	{
		delete searchedTracks; searchedTracks = NULL;
		C_ItemList allTracks;

		int l = dev->getPlaylistLength(playlistId);
		for(int i=0; i<l; i++) {
			songid_t x = dev->getPlaylistTrack(playlistId,i);
			if(type != -1 && dev->getTrackType(x) != type) continue;
			allTracks.Add((void*)x);
		}

		searchedTracks = FilterSongs(str,&allTracks);
		delete unrefinedTracks;
		unrefinedTracks = new C_ItemList;
		l=searchedTracks->GetSize();
		for(int i=0; i<l; i++) unrefinedTracks->Add(searchedTracks->Get(i));

		for(int i=0; i<numFilters; i++) {
			if(i==0) { 
				FreeFilterItemList(filters[0]->items);
				filters[0]->items = CompilePrimaryList(searchedTracks);
			}
			else {
				delete filters[i]->items;
				filters[i]->items = CompileSecondaryList(filters[i-1]->items,i,true);
			}	
		}

		SetRefine(L"");

		for(int i=0; i<numFilters; i++) filters[i]->SortList();
	}
	else
	{
		if (lastSearch)
		{
			free(lastSearch);
			lastSearch = 0;
		}

		if (!(str && *str))
		{
			if (unrefinedTracks) delete unrefinedTracks;
			if (searchedTracks) delete searchedTracks;
			if (trackList) delete trackList; 
			searchedTracks = new C_ItemList;
			trackList = new C_ItemList;
			unrefinedTracks = new C_ItemList;
		}
		else
		{
			lastSearch = wcsdup(str);
		}
		bgQuery((str && *str));
	}
}

ListContents * ArtistAlbumLists::GetFilterList(int i) { return filters[i]; }
PrimaryListContents * ArtistAlbumLists::GetTracksList() { return this->tracksLC; }