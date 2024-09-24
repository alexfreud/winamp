#include "ArtistAlbumLists.h"
#include "Filters.h"
#include "api__ml_pmp.h"
#include "resource1.h"
#include "metadata_utils.h"

extern winampMediaLibraryPlugin plugin;

#define RETIFNZ(v) if ((v)<0) return use_dir?1:-1; if ((v)>0) return use_dir?-1:1;

extern int STRCMP_NULLOK(const wchar_t *pa, const wchar_t *pb);

#define SKIP_THE_AND_WHITESPACE(x) { while (!iswalnum(*x) && *x) x++; if (!_wcsnicmp(x,L"the ",4)) x+=4; while (*x == L' ') x++; }
static wchar_t GetIndex(wchar_t *name) {
	SKIP_THE_AND_WHITESPACE(name);
	return towupper(name[0]);
}

void Filter::addToGroup(Device *dev, songid_t song, FilterItem * group) {
	group->length += dev->getTrackLength(song);
	group->size += dev->getTrackSize(song);
	group->numTracks++;

	wchar_t buf[16] = {0};
	dev->getTrackExtraInfo(song, L"cloud", buf, 16);
	int status = _wtoi(buf);
	// local and to be uploaded (4) are the same here
	if (status == 4)
	{
		status = 0;
	}

	group->cloudState += status;
}

void Filter::AddDefaultColumns(Device * dev, C_ItemList * fields, C_Config * config, int columnStart, bool cloud) {
	if(nextFilter) fields->Add(new ListField(columnStart+40, 50 ,nextFilter->namePlural, config));
	fields->Add(new ListField(columnStart+41, 50 , WASABI_API_LNGSTRINGW(IDS_TRACKS), config));
	if (cloud) fields->Add(new ListField(columnStart+52, 27, WASABI_API_LNGSTRINGW(IDS_CLOUD), config));
	int fieldsBits = (int)dev->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
	if (!fieldsBits) fieldsBits = -1;
	if (fieldsBits & SUPPORTS_SIZE) fields->Add(new ListField(columnStart+50, 50, WASABI_API_LNGSTRINGW(IDS_SIZE), config, true));
	if (fieldsBits & SUPPORTS_LENGTH) fields->Add(new ListField(columnStart+51, 50 ,WASABI_API_LNGSTRINGW(IDS_LENGTH), config, true));
}

void FilterItem::GetDefaultColumnsCellText(int col, wchar_t*buf, int buflen) {
	switch (col)
	{
		case 40: wsprintf(buf, L"%d", numNextFilter); break;
		case 41: wsprintf(buf, L"%d", (independentTracks ? independentTracks : numTracks)); break;
		case 50: WASABI_API_LNG->FormattedSizeString(buf, buflen, (independentSize ? independentSize : size)); break;
		case 51:
		{
			__int64 l = (independentLength ? independentLength : length) / 1000;
			int x = (int)l / 60;
			int y = (int)l % 60;
			wsprintf(buf, L"%d:%02d", x, y);
		}
		break;
		case 52:
		{
			int state = (independentCloudState ? independentCloudState : cloudState);
			if (state % (independentTracks ? independentTracks : numTracks))
			{
				wsprintf(buf, L"%d", 2);
			}
			else
			{
				wsprintf(buf, L"%d", state / (independentTracks ? independentTracks : numTracks));
			}
		}
		break;
	}
}

int FilterItem::DefaultSortAction(FilterItem *that, int use_by, int use_dir) {
	int v = 0;
	if(use_by==50) { RETIFNZ(this->size - that->size); }
	if(use_by==51) { RETIFNZ(this->length - that->length); }
	if(use_by==52) {
		if (numTracks > 0)
		{
			int state = (independentCloudState ? independentCloudState : cloudState);
			int thatState = (that->independentCloudState ? that->independentCloudState : that->cloudState);
			if (state % (independentTracks ? independentTracks : numTracks))
			{
				if (thatState % (that->independentTracks ? that->independentTracks : that->numTracks))
				{
					v = 0;
				}
				else
				{
					v = (2) - (thatState / (that->independentTracks ? that->independentTracks : that->numNextFilter));
				}
			}
			else
			{
				if (thatState % (that->independentTracks ? that->independentTracks : that->numTracks))
				{
					v = (state / (independentTracks ? independentTracks : numTracks)) - (2);
				}
				else
				{
					v = (state / (independentTracks ? independentTracks : numTracks)) - (thatState / (that->independentTracks ? that->independentTracks : that->numNextFilter));
				}
			}
		}
		RETIFNZ(v);
	}
	if(use_by==40) {
		v=this->numNextFilter - that->numNextFilter;
		RETIFNZ(v)
	}
	if(use_by==41) {
		if(this->independentTracks || that->independentTracks) v = this->independentTracks - that->independentTracks;
		else v=this->numTracks - that->numTracks;
		RETIFNZ(v)
	}
	return 0;
}

FilterItem::~FilterItem() {
	if(independentNextFilter) delete independentNextFilter; independentNextFilter=0;
	if(nextFilter) delete nextFilter; nextFilter=0;
}

class ArtistFilter : public Filter {
public:
	class ArtistFI : public FilterItem {
	public:
		ArtistFI(wchar_t *name0) : FilterItem() { name = _wcsdup(name0); }
		virtual ~ArtistFI() { free(name); }
		wchar_t *name;
		virtual bool isWithoutGroup() { return !name[0]; }
		virtual int compareTo(FilterItem *that) { return STRCMP_NULLOK(this->name,((ArtistFI*)that)->name); }
		virtual int compareTo2(FilterItem *that0, int use_by, int use_dir) {
			{int v=DefaultSortAction(that0,use_by-100,use_dir); if(v) return v; }
			ArtistFI *a=this,*b = (ArtistFI*)that0;
			int v;
			v = STRCMP_NULLOK(a->name,b->name); 
			RETIFNZ(v)
			return 0;
		}
		virtual void GetCellText(int col, wchar_t * buf, int buflen) {
			switch(col) {
				case 100: lstrcpyn(buf,name[0]?name:WASABI_API_LNGSTRINGW(IDS_NO_ARTIST),buflen); break;
				default: GetDefaultColumnsCellText(col-100,buf,buflen); break;
			}
		}
	};
	ArtistFilter() : Filter() { name=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ARTIST));
								namePlural=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ARTISTS)); }
	virtual void AddColumns(Device * dev, C_ItemList * fields,C_Config * config, bool cloud) {
		fields->Add(new ListField(100, 170,WASABI_API_LNGSTRINGW(IDS_ARTIST),config));
		AddDefaultColumns(dev,fields,config,100,cloud);
	}
	virtual int sortFunc(Device * dev, songid_t a, songid_t b) {
		wchar_t astr[256]=L"", bstr[256]=L"";
		dev->getTrackArtist(a,astr,256);
		dev->getTrackArtist(b,bstr,256);
		return STRCMP_NULLOK(astr,bstr);
	}
	virtual bool isInGroup(Device *dev, songid_t song, FilterItem * group0) {
		ArtistFI * group = (ArtistFI*)group0;
		wchar_t buf[256]=L"";
		dev->getTrackArtist(song,buf,256);
		return !STRCMP_NULLOK(buf,group->name);
	}
	virtual void addToGroup(Device *dev, songid_t song, FilterItem * group0) {
		Filter::addToGroup(dev,song,group0);
		//ArtistFI * group = (ArtistFI*)group0;
	}
	virtual FilterItem * newGroup(Device *dev, songid_t song) {
		wchar_t buf[256]=L"";
		dev->getTrackArtist(song,buf,256);
		ArtistFI * group = new ArtistFI(buf);
		Filter::addToGroup(dev,song,group);
		return group;
	}
};

class AlbumFilter : public Filter {
public:
	class AlbumFI : public FilterItem {
	public:
		AlbumFI(wchar_t *name0) : FilterItem(), yearHigh(0), yearLow(0) { name = _wcsdup(name0); }
		virtual ~AlbumFI() { free(name); }
		wchar_t *name;
		int yearHigh, yearLow;
		virtual bool isWithoutGroup() { return !name[0]; }
		virtual int compareTo(FilterItem *that) { return STRCMP_NULLOK(this->name,((AlbumFI*)that)->name); }
		virtual int compareTo2(FilterItem *that0, int use_by, int use_dir) {
			{int v=DefaultSortAction(that0,use_by-200,use_dir); if(v) return v; }
			AlbumFI *that = (AlbumFI*)that0;
			int v;
			if(use_by==201) { //year
				v=this->yearHigh - that->yearHigh;
				RETIFNZ(v)
			}
			//by name
			v = STRCMP_NULLOK(this->name,that->name); 
			RETIFNZ(v)
			return 0;
		}
		virtual void GetCellText(int col, wchar_t * buf, int buflen) {
			switch(col) {
				case 200: lstrcpyn(buf,name[0]?name:WASABI_API_LNGSTRINGW(IDS_NO_ALBUM),buflen); break;
				case 201: 
					if(yearHigh>0 && yearLow>0) {
						if(yearHigh == yearLow) wsprintf(buf,L"%d",yearHigh);
						else if(yearHigh/100 == yearLow/100) wsprintf(buf,L"%d-%02d",yearLow,yearHigh%100);
						else wsprintf(buf,L"%d-%d",yearLow,yearHigh);
					}
					break;
				default: GetDefaultColumnsCellText(col-200,buf,buflen); break;
			}
		}
	}; 

	AlbumFilter() : Filter() { name=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ALBUM));
							   namePlural=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ALBUMS)); }
	virtual void AddColumns(Device * dev, C_ItemList * fields,C_Config * config, bool cloud) {
		int fieldsBits = (int)dev->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
		fields->Add(new ListField(200, 170,WASABI_API_LNGSTRINGW(IDS_ALBUM),config));
		if(!fieldsBits || (fieldsBits & SUPPORTS_YEAR))
		fields->Add(new ListField(201, 50 ,WASABI_API_LNGSTRINGW(IDS_YEAR),config));
		AddDefaultColumns(dev,fields,config,200,cloud);
	}
	virtual int sortFunc(Device * dev, songid_t a, songid_t b) {
		wchar_t astr[256]=L"", bstr[256]=L"";
		dev->getTrackAlbum(a,astr,256);
		dev->getTrackAlbum(b,bstr,256);
		return STRCMP_NULLOK(astr,bstr);
	}
	virtual bool isInGroup(Device *dev, songid_t song, FilterItem * group0) {
		AlbumFI * group = (AlbumFI*)group0;
		wchar_t buf[256]=L"";
		dev->getTrackAlbum(song,buf,256);
		return !STRCMP_NULLOK(buf,group->name);
	}
	virtual void addToGroup(Device *dev, songid_t song, FilterItem * group0) {
		AlbumFI * group = (AlbumFI*)group0;
		Filter::addToGroup(dev,song,group0);
		int y = dev->getTrackYear(song);
		if(y>0) {
			if(y > group->yearHigh || group->yearHigh<=0) group->yearHigh=y;
			if(y < group->yearLow || group->yearLow<=0) group->yearLow=y;
		}
	}
	virtual FilterItem * newGroup(Device *dev, songid_t song) {
		wchar_t buf[256]=L"";
		dev->getTrackAlbum(song,buf,256);
		AlbumFI * group = new AlbumFI(buf);
		group->yearHigh = group->yearLow = dev->getTrackYear(song);
		Filter::addToGroup(dev,song,group);
		return group;
	}
};


class GenreFilter : public Filter {
public:
	class GenreFI : public FilterItem {
	public:
		GenreFI(wchar_t *name0) : FilterItem() { name = _wcsdup(name0); }
		virtual ~GenreFI() { free(name); }
		wchar_t *name;
		virtual bool isWithoutGroup() { return !name[0]; }
		virtual int compareTo(FilterItem *that) { return STRCMP_NULLOK(this->name,((GenreFI*)that)->name); }
		virtual int compareTo2(FilterItem *that0, int use_by, int use_dir) {
			{int v=DefaultSortAction(that0,use_by-300,use_dir); if(v) return v; }
			GenreFI *a=this,*b = (GenreFI*)that0;
			int v;
			v = STRCMP_NULLOK(a->name,b->name); 
			RETIFNZ(v)
			return 0;
		}
		virtual void GetCellText(int col, wchar_t * buf, int buflen) {
			switch(col) {
				case 300: lstrcpyn(buf,name[0]?name:WASABI_API_LNGSTRINGW(IDS_NO_GENRE),buflen); break;
				default: GetDefaultColumnsCellText(col-300,buf,buflen); break;
			}
		}
	};
	GenreFilter() : Filter() { name=_wcsdup(WASABI_API_LNGSTRINGW(IDS_GENRE));
							   namePlural=_wcsdup(WASABI_API_LNGSTRINGW(IDS_GENRES)); }
	virtual void AddColumns(Device * dev, C_ItemList * fields,C_Config * config, bool cloud) {
		fields->Add(new ListField(300, 170,WASABI_API_LNGSTRINGW(IDS_GENRE),config));
		AddDefaultColumns(dev,fields,config,300,cloud);
	}
	virtual int sortFunc(Device * dev, songid_t a, songid_t b) {
		wchar_t astr[256]=L"", bstr[256]=L"";
		dev->getTrackGenre(a,astr,256);
		dev->getTrackGenre(b,bstr,256);
		return STRCMP_NULLOK(astr,bstr);
	}
		virtual bool isInGroup(Device *dev, songid_t song, FilterItem * group0) {
		GenreFI * group = (GenreFI*)group0;
		wchar_t buf[256]=L"";
		dev->getTrackGenre(song,buf,256);
		return !STRCMP_NULLOK(buf,group->name);
	}
	virtual void addToGroup(Device *dev, songid_t song, FilterItem * group0) {
		Filter::addToGroup(dev,song,group0);
		//GenreFI * group = (GenreFI*)group0;
	}
	virtual FilterItem * newGroup(Device *dev, songid_t song) {
		wchar_t buf[256]=L"";
		dev->getTrackGenre(song,buf,256);
		GenreFI * group = new GenreFI(buf);
		Filter::addToGroup(dev,song,group);
		return group;
	}
};

class ArtistIndexFilter : public Filter {
public:
	class ArtistIndexFI : public FilterItem {
	public:
		ArtistIndexFI(wchar_t *name0) : FilterItem() { name = GetIndex(name0); }
		virtual ~ArtistIndexFI() {  }
		wchar_t name;
		virtual bool isWithoutGroup() { return !name; }
		virtual int compareTo(FilterItem *that) { return name - ((ArtistIndexFI*)that)->name; }
		virtual int compareTo2(FilterItem *that0, int use_by, int use_dir) {
			{int v=DefaultSortAction(that0,use_by-400,use_dir); if(v) return v; }
			ArtistIndexFI *that = (ArtistIndexFI*)that0;
			int v;
			//by name
			v = this->name - that->name;
			RETIFNZ(v)
			return 0;
		}
		virtual void GetCellText(int col, wchar_t * buf, int buflen) {
			switch(col) {
				case 400: 
					{
						if(name) { buf[0] = name; buf[1]=0; }
						else WASABI_API_LNGSTRINGW_BUF(IDS_NO_ARTIST,buf,buflen);
					}
					break;
				default: GetDefaultColumnsCellText(col-400,buf,buflen); break;
			}
		}
	}; 

	ArtistIndexFilter() : Filter() { name=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ARTIST_INDEX));
									 namePlural=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ARTIST_INDEXES)); }
	virtual void AddColumns(Device * dev, C_ItemList * fields,C_Config * config, bool cloud) {
		fields->Add(new ListField(400, 170,WASABI_API_LNGSTRINGW(IDS_ARTIST_INDEX),config));
		AddDefaultColumns(dev,fields,config,400,cloud);
	}
	virtual int sortFunc(Device * dev, songid_t a, songid_t b) {
		wchar_t astr[256]=L"", bstr[256]=L"";
		dev->getTrackArtist(a,astr,256);
		dev->getTrackArtist(b,bstr,256);
		return GetIndex(astr) - GetIndex(bstr);
	}
	virtual bool isInGroup(Device *dev, songid_t song, FilterItem * group0) {
		ArtistIndexFI * group = (ArtistIndexFI*)group0;
		wchar_t astr[256]=L"";
		dev->getTrackArtist(song,astr,256);
		return GetIndex(astr) == group->name;
	}
	virtual void addToGroup(Device *dev, songid_t song, FilterItem * group0) {
		Filter::addToGroup(dev,song,group0);
	}
	virtual FilterItem * newGroup(Device *dev, songid_t song) {
		wchar_t buf[256]=L"";
		dev->getTrackArtist(song,buf,256);
		ArtistIndexFI * group = new ArtistIndexFI(buf);
		Filter::addToGroup(dev,song,group);
		return group;
	}
};

class AlbumArtistIndexFilter : public Filter {
public:
	class ArtistIndexFI : public FilterItem {
	public:
		ArtistIndexFI(wchar_t *name0) : FilterItem() { name = GetIndex(name0); }
		virtual ~ArtistIndexFI() {  }
		wchar_t name;
		virtual bool isWithoutGroup() { return !name; }
		virtual int compareTo(FilterItem *that) { return name - ((ArtistIndexFI*)that)->name; }
		virtual int compareTo2(FilterItem *that0, int use_by, int use_dir) {
			{int v=DefaultSortAction(that0,use_by-900,use_dir); if(v) return v; }
			ArtistIndexFI *that = (ArtistIndexFI*)that0;
			int v;
			//by name
			v = this->name - that->name;
			RETIFNZ(v)
			return 0;
		}
		virtual void GetCellText(int col, wchar_t * buf, int buflen) {
			switch(col) {
				case 900: 
					{
						if(name) { buf[0] = name; buf[1]=0; }
						else WASABI_API_LNGSTRINGW_BUF(IDS_NO_ALBUM_ARTIST,buf,buflen);
					}
					break;
				default: GetDefaultColumnsCellText(col-900,buf,buflen); break;
			}
		}
	}; 

	AlbumArtistIndexFilter() : Filter() { name=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ALBUM_ARTIST_INDEX));
										  namePlural=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ARTIST_INDEXES)); }
	virtual void AddColumns(Device * dev, C_ItemList * fields,C_Config * config, bool cloud) {
		fields->Add(new ListField(900, 170,WASABI_API_LNGSTRINGW(IDS_ALBUM_ARTIST_INDEX),config));
		AddDefaultColumns(dev,fields,config,900,cloud);
	}
	virtual int sortFunc(Device * dev, songid_t a, songid_t b) {
		wchar_t astr[256]=L"", bstr[256]=L"";
		dev->getTrackAlbumArtist(a,astr,256);
		dev->getTrackAlbumArtist(b,bstr,256);
		return GetIndex(astr) - GetIndex(bstr);
	}
	virtual bool isInGroup(Device *dev, songid_t song, FilterItem * group0) {
		ArtistIndexFI * group = (ArtistIndexFI*)group0;
		wchar_t astr[256]=L"";
		dev->getTrackAlbumArtist(song,astr,256);
		return GetIndex(astr) == group->name;
	}
	virtual void addToGroup(Device *dev, songid_t song, FilterItem * group0) {
		Filter::addToGroup(dev,song,group0);
	}
	virtual FilterItem * newGroup(Device *dev, songid_t song) {
		wchar_t buf[256]=L"";
		dev->getTrackAlbumArtist(song,buf,256);
		ArtistIndexFI * group = new ArtistIndexFI(buf);
		Filter::addToGroup(dev,song,group);
		return group;
	}
};

class YearFilter : public Filter {
public:
	class YearFI : public FilterItem {
	public:
		int year;
		YearFI(int _year) : FilterItem(), year(_year) { if(year<0) year=0; }
		virtual ~YearFI() { }
		virtual bool isWithoutGroup() { return year<=0; }
		virtual int compareTo(FilterItem *that) { return year - ((YearFI*)that)->year; }
		virtual int compareTo2(FilterItem *that0, int use_by, int use_dir) {
			{int v=DefaultSortAction(that0,use_by-500,use_dir); if(v) return v; }
			YearFI *a=this,*b = (YearFI*)that0;
			int v;
			v = a->year - b->year;
			RETIFNZ(v)
			return 0;
		}
		virtual void GetCellText(int col, wchar_t * buf, int buflen) {
			switch(col) {
				case 500: if(year>0) wsprintf(buf,L"%d",year); else WASABI_API_LNGSTRINGW_BUF(IDS_NO_YEAR,buf,buflen); break;
				default: GetDefaultColumnsCellText(col-500,buf,buflen); break;
			}
		}
	};
	YearFilter() : Filter() { name=_wcsdup(WASABI_API_LNGSTRINGW(IDS_YEAR));
							  namePlural=_wcsdup(WASABI_API_LNGSTRINGW(IDS_YEARS)); }
	virtual void AddColumns(Device * dev, C_ItemList * fields,C_Config * config, bool cloud) {
		fields->Add(new ListField(500, 170,WASABI_API_LNGSTRINGW(IDS_YEAR),config));
		AddDefaultColumns(dev,fields,config,500,cloud);
	}
	virtual int sortFunc(Device * dev, songid_t a, songid_t b) {
		int w = dev->getTrackYear(a);
		int e = dev->getTrackYear(b);
		if(w<=0) w=0;
		if(e<=0) e=0;
		return w - e;
	}
	virtual bool isInGroup(Device *dev, songid_t song, FilterItem * group0) {
		YearFI * group = (YearFI*)group0;
		int y = dev->getTrackYear(song);
		if(group->isWithoutGroup() && y<=0) return true;
		return group->year == y;
	}
	virtual void addToGroup(Device *dev, songid_t song, FilterItem * group0) {
		Filter::addToGroup(dev,song,group0);
	}
	virtual FilterItem * newGroup(Device *dev, songid_t song) {
		YearFI * group = new YearFI(dev->getTrackYear(song));
		Filter::addToGroup(dev,song,group);
		return group;
	}
};

class AlbumArtistFilter : public Filter {
public:
	class ArtistFI : public FilterItem {
	public:
		ArtistFI(wchar_t *name0) : FilterItem() { name = _wcsdup(name0); }
		virtual ~ArtistFI() { free(name); }
		wchar_t *name;
		virtual bool isWithoutGroup() { return !name[0]; }
		virtual int compareTo(FilterItem *that) { return STRCMP_NULLOK(this->name,((ArtistFI*)that)->name); }
		virtual int compareTo2(FilterItem *that0, int use_by, int use_dir) {
			{int v=DefaultSortAction(that0,use_by-600,use_dir); if(v) return v; }
			ArtistFI *a=this,*b = (ArtistFI*)that0;
			int v;
			v = STRCMP_NULLOK(a->name,b->name); 
			RETIFNZ(v)
			return 0;
		}
		virtual void GetCellText(int col, wchar_t * buf, int buflen) {
			switch(col) {
				case 600: lstrcpyn(buf,name[0]?name:WASABI_API_LNGSTRINGW(IDS_NO_ALBUM_ARTIST),buflen); break;
				default: GetDefaultColumnsCellText(col-600,buf,buflen); break;
			}
		}
	};
	AlbumArtistFilter() : Filter() { name=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ALBUM_ARTIST));
									 namePlural=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ALBUM_ARTISTS)); }
	virtual void AddColumns(Device * dev, C_ItemList * fields,C_Config * config, bool cloud) {
		fields->Add(new ListField(600, 170,WASABI_API_LNGSTRINGW(IDS_ALBUM_ARTIST),config));
		AddDefaultColumns(dev,fields,config,600,cloud);
	}
	virtual int sortFunc(Device * dev, songid_t a, songid_t b) {
		wchar_t astr[256]=L"", bstr[256]=L"";
		dev->getTrackAlbumArtist(a,astr,256);
		dev->getTrackAlbumArtist(b,bstr,256);
		return STRCMP_NULLOK(astr,bstr);
	}
		virtual bool isInGroup(Device *dev, songid_t song, FilterItem * group0) {
		ArtistFI * group = (ArtistFI*)group0;
		wchar_t buf[256]=L"";
		dev->getTrackAlbumArtist(song,buf,256);
		return !STRCMP_NULLOK(buf,group->name);
	}
	virtual void addToGroup(Device *dev, songid_t song, FilterItem * group0) {
		Filter::addToGroup(dev,song,group0);
	}
	virtual FilterItem * newGroup(Device *dev, songid_t song) {
		wchar_t buf[256]=L"";
		dev->getTrackAlbumArtist(song,buf,256);
		ArtistFI * group = new ArtistFI(buf);
		Filter::addToGroup(dev,song,group);
		return group;
	}
};

class PublisherFilter : public Filter {
public:
	class ArtistFI : public FilterItem {
	public:
		ArtistFI(wchar_t *name0) : FilterItem() { name = _wcsdup(name0); }
		virtual ~ArtistFI() { free(name); }
		wchar_t *name;
		virtual bool isWithoutGroup() { return !name[0]; }
		virtual int compareTo(FilterItem *that) { return STRCMP_NULLOK(this->name,((ArtistFI*)that)->name); }
		virtual int compareTo2(FilterItem *that0, int use_by, int use_dir) {
		{int v=DefaultSortAction(that0,use_by-700,use_dir); if(v) return v; }
			ArtistFI *a=this,*b = (ArtistFI*)that0;
			int v;
			v = STRCMP_NULLOK(a->name,b->name); 
			RETIFNZ(v)
			return 0;
		}
		virtual void GetCellText(int col, wchar_t * buf, int buflen) {
			switch(col) {
				case 700: lstrcpyn(buf,name[0]?name:WASABI_API_LNGSTRINGW(IDS_NO_PUBLISHER),buflen); break;
				default: GetDefaultColumnsCellText(col-700,buf,buflen); break;
			}
		}
	};
	PublisherFilter() : Filter() { name=_wcsdup(WASABI_API_LNGSTRINGW(IDS_PUBLISHER));
								   namePlural=_wcsdup(WASABI_API_LNGSTRINGW(IDS_PUBLISHERS)); }
	virtual void AddColumns(Device * dev, C_ItemList * fields,C_Config * config, bool cloud) {
		fields->Add(new ListField(700, 170,WASABI_API_LNGSTRINGW(IDS_PUBLISHER),config));
		AddDefaultColumns(dev,fields,config,700,cloud);
	}
	virtual int sortFunc(Device * dev, songid_t a, songid_t b) {
		wchar_t astr[256]=L"", bstr[256]=L"";
		dev->getTrackPublisher(a,astr,256);
		dev->getTrackPublisher(b,bstr,256);
		return STRCMP_NULLOK(astr,bstr);
	}
		virtual bool isInGroup(Device *dev, songid_t song, FilterItem * group0) {
		ArtistFI * group = (ArtistFI*)group0;
		wchar_t buf[256]=L"";
		dev->getTrackPublisher(song,buf,256);
		return !STRCMP_NULLOK(buf,group->name);
	}
	virtual void addToGroup(Device *dev, songid_t song, FilterItem * group0) {
		Filter::addToGroup(dev,song,group0);
	}
	virtual FilterItem * newGroup(Device *dev, songid_t song) {
		wchar_t buf[256]=L"";
		dev->getTrackPublisher(song,buf,256);
		ArtistFI * group = new ArtistFI(buf);
		Filter::addToGroup(dev,song,group);
		return group;
	}
};

class ComposerFilter : public Filter {
public:
	class ArtistFI : public FilterItem {
	public:
		ArtistFI(wchar_t *name0) : FilterItem() { name = _wcsdup(name0); }
		virtual ~ArtistFI() { free(name); }
		wchar_t *name;
		virtual bool isWithoutGroup() { return !name[0]; }
		virtual int compareTo(FilterItem *that) { return STRCMP_NULLOK(this->name,((ArtistFI*)that)->name); }
		virtual int compareTo2(FilterItem *that0, int use_by, int use_dir) {
		{int v=DefaultSortAction(that0,use_by-800,use_dir); if(v) return v; }
			ArtistFI *a=this,*b = (ArtistFI*)that0;
			int v;
			v = STRCMP_NULLOK(a->name,b->name); 
			RETIFNZ(v)
			return 0;
		}
		virtual void GetCellText(int col, wchar_t * buf, int buflen) {
			switch(col) {
				case 800: lstrcpyn(buf,name[0]?name:WASABI_API_LNGSTRINGW(IDS_NO_COMPOSER),buflen); break;
				default: GetDefaultColumnsCellText(col-800,buf,buflen); break;
			}
		}
	};
	ComposerFilter() : Filter() { name=_wcsdup(WASABI_API_LNGSTRINGW(IDS_COMPOSER));
								  namePlural=_wcsdup(WASABI_API_LNGSTRINGW(IDS_COMPOSERS)); }
	virtual void AddColumns(Device * dev, C_ItemList * fields,C_Config * config, bool cloud) {
		fields->Add(new ListField(800, 170,WASABI_API_LNGSTRINGW(IDS_COMPOSER),config));
		AddDefaultColumns(dev,fields,config,800,cloud);
	}
	virtual int sortFunc(Device * dev, songid_t a, songid_t b) {
		wchar_t astr[256]=L"", bstr[256]=L"";
		dev->getTrackComposer(a,astr,256);
		dev->getTrackComposer(b,bstr,256);
		return STRCMP_NULLOK(astr,bstr);
	}
		virtual bool isInGroup(Device *dev, songid_t song, FilterItem * group0) {
		ArtistFI * group = (ArtistFI*)group0;
		wchar_t buf[256]=L"";
		dev->getTrackComposer(song,buf,256);
		return !STRCMP_NULLOK(buf,group->name);
	}
	virtual void addToGroup(Device *dev, songid_t song, FilterItem * group0) {
		Filter::addToGroup(dev,song,group0);
	}
	virtual FilterItem * newGroup(Device *dev, songid_t song) {
		wchar_t buf[256]=L"";
		dev->getTrackComposer(song,buf,256);
		ArtistFI * group = new ArtistFI(buf);
		Filter::addToGroup(dev,song,group);
		return group;
	}
};

class AlbumArtFilter : public Filter {
public:
	class AlbumFI : public FilterItem {
	public:
		AlbumFI(Device* dev, wchar_t *artist0, wchar_t *album0, int year, wchar_t *genre0, int rating0, pmpart_t art) : 
				FilterItem(), dev(dev), yearHigh(year), yearLow(year), art(art)
				{ 
					artist = _wcsdup(artist0); album = _wcsdup(album0); genre = _wcsdup(genre0); 
					if(rating0 > 0) rating = rating0;
				}
		virtual ~AlbumFI() { free(artist); free(album); free(genre); if(art) dev->releaseArt(art); }
		Device *dev;
		wchar_t *artist;
		wchar_t *album;
		int yearHigh, yearLow;
		wchar_t *genre;
		int rating;
		pmpart_t art;
		virtual bool isWithoutGroup() { return !artist[0] && !album[0]; }
		virtual int compareTo(FilterItem *that) {
			int x = STRCMP_NULLOK(this->artist,((AlbumFI*)that)->artist);
			if(x) return x;
			return STRCMP_NULLOK(this->album,((AlbumFI*)that)->album);
		}
		virtual int compareTo2(FilterItem *that0, int use_by, int use_dir) {
			{int v=DefaultSortAction(that0,use_by-1000,use_dir); if(v) return v; }
			AlbumFI *that = (AlbumFI*)that0;
			__int64 v;
			if(use_by==1086) {
				v=this->size - that->size;
				RETIFNZ(v)
			}
			if(use_by==1085) {
				v=this->length - that->length;
				RETIFNZ(v)
			}
			if(use_by==1084) {
				v=this->rating - that->rating;
				RETIFNZ(v)
			}
			if(use_by==1083) {
				v=STRCMP_NULLOK(this->genre,that->genre); 
				RETIFNZ(v)
			}
			if(use_by==1082) { //year
				v=this->yearHigh - that->yearHigh;
				RETIFNZ(v)
			}
			if(use_by==1001) {
				v = STRCMP_NULLOK(this->album,that->album); 
				RETIFNZ(v)
			}
			//by name
			v = STRCMP_NULLOK(this->artist,that->artist); 
			RETIFNZ(v)
			return 0;
		}
		virtual void GetCellText(int col, wchar_t * buf, int buflen) {
			switch(col) {
				case 1000: lstrcpyn(buf,artist[0]?artist:WASABI_API_LNGSTRINGW(IDS_NO_ARTIST),buflen); break;
				case 1001: lstrcpyn(buf,album[0]?album:WASABI_API_LNGSTRINGW(IDS_NO_ALBUM),buflen); break;
				case 1082: 
					if(yearHigh>0 && yearLow>0) {
						if(yearHigh == yearLow) wsprintf(buf,L"%d",yearHigh);
						else if(yearHigh/100 == yearLow/100) wsprintf(buf,L"%d-%02d",yearLow,yearHigh%100);
						else wsprintf(buf,L"%d-%d",yearLow,yearHigh);
					}
					break;
				case 1083: lstrcpyn(buf,genre[0]?genre:WASABI_API_LNGSTRINGW(IDS_NO_GENRE),buflen); break;
				case 1084:
					if(numTracks > 0) {
						wchar_t r[] = L"\u2605\u2605\u2605\u2605\u2605";
						double d = double(rating) / double(numTracks);
						int rat = int(d);
						if(d - double(rat) >= 0.5 && rat<5) rat++;
						if(rat>0 && rat<=5) r[rat]=0;
						else r[0]=0;
						lstrcpyn(buf,r,buflen);
					}
					break;
				case 1085: {__int64 l=length/1000; int x = (int)l/60; int y = (int)l%60; wsprintf(buf,L"%d:%02d",x,y); } break;
				case 1086: WASABI_API_LNG->FormattedSizeString(buf, buflen, size); break;
				default: GetDefaultColumnsCellText(col-1000,buf,buflen); break;
			}
		}
		virtual pmpart_t GetArt() {return art;}
	};
	int mode;
	AlbumArtFilter() : Filter(),mode(0) { name=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ALBUM));
										  namePlural=_wcsdup(WASABI_API_LNGSTRINGW(IDS_ALBUMS)); }
	virtual void AddColumns(Device * dev, C_ItemList * fields,C_Config * config, bool cloud) {
		int fieldsBits = (int)dev->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
		if(!fieldsBits) fieldsBits=-1;
		fields->Add(new ListField(1000, 90,WASABI_API_LNGSTRINGW(IDS_ARTIST),config,0));
		fields->Add(new ListField(1001, 90,WASABI_API_LNGSTRINGW(IDS_ALBUM),config,0));
		if(fieldsBits & SUPPORTS_YEAR) fields->Add(new ListField(1082, 50 ,WASABI_API_LNGSTRINGW(IDS_YEAR),config,0));
		if(fieldsBits & SUPPORTS_GENRE) fields->Add(new ListField(1083, 50 ,WASABI_API_LNGSTRINGW(IDS_GENRE),config,!(mode==1 || mode==2)));
		if(fieldsBits & SUPPORTS_RATING) fields->Add(new ListField(1084, 50 ,WASABI_API_LNGSTRINGW(IDS_RATING),config,!(mode==1 || mode==2)));
		if(fieldsBits & SUPPORTS_LENGTH) fields->Add(new ListField(1085, 50 ,WASABI_API_LNGSTRINGW(IDS_LENGTH),config,mode!=2));
		if(fieldsBits & SUPPORTS_SIZE) fields->Add(new ListField(1086, 50 ,WASABI_API_LNGSTRINGW(IDS_SIZE),config,mode!=2));
		fields->Add(new ListField(1041, 50 ,WASABI_API_LNGSTRINGW(IDS_TRACKS),config));
		//AddDefaultColumns(dev,fields,config,1000);
	}
	virtual int sortFunc(Device * dev, songid_t a, songid_t b) {
		wchar_t astr[256]=L"", bstr[256]=L"";
		dev->getTrackAlbumArtist(a,astr,256);
		if(!astr[0]) dev->getTrackArtist(a,astr,256);
		dev->getTrackAlbumArtist(b,bstr,256);
		if(!bstr[0]) dev->getTrackArtist(b,bstr,256);
		int x = STRCMP_NULLOK(astr,bstr);
		if(x) return x;
		dev->getTrackAlbum(a,astr,256);
		dev->getTrackAlbum(b,bstr,256);
		return STRCMP_NULLOK(astr,bstr);
	}
	virtual bool isInGroup(Device *dev, songid_t song, FilterItem * group0) {
		AlbumFI * group = (AlbumFI*)group0;
		wchar_t bufa[256]=L"",bufb[256]=L"";
		dev->getTrackAlbumArtist(song,bufa,256);
		if(!bufa[0]) dev->getTrackArtist(song,bufa,256);
		dev->getTrackAlbum(song,bufb,256);
		return !STRCMP_NULLOK(bufa,group->artist) && !STRCMP_NULLOK(bufb,group->album);
	}
	virtual void addToGroup(Device *dev, songid_t song, FilterItem * group0) {
		AlbumFI * group = (AlbumFI*)group0;
		Filter::addToGroup(dev,song,group0);
		int y = dev->getTrackYear(song);
		if(y>0) {
			if(y > group->yearHigh || group->yearHigh<=0) group->yearHigh=y;
			if(y < group->yearLow || group->yearLow<=0) group->yearLow=y;
		}
		if(!group->art) group->art = dev->getArt(song);
		else {
			int w=0,h=0;
			dev->getArtNaturalSize(group->art,&w,&h);
			pmpart_t newart = dev->getArt(song);
			int nw=0,nh=0;
			dev->getArtNaturalSize(newart,&nw,&nh);
			if(nw > w && nh > h) {
				dev->releaseArt(group->art);
				group->art = newart;
			} else dev->releaseArt(newart);
		}
		int r = dev->getTrackRating(song);
		if(r > 0) group->rating+=r;
	}
	virtual FilterItem * newGroup(Device *dev, songid_t song) {
		wchar_t bufa[256]=L"";
		wchar_t bufb[256]=L"";
		wchar_t bufc[256]=L"";
		dev->getTrackAlbumArtist(song,bufa,256);
		if(!bufa[0]) dev->getTrackArtist(song,bufa,256);
		dev->getTrackAlbum(song,bufb,256);
		dev->getTrackGenre(song,bufc,256);
		AlbumFI * group = new AlbumFI(dev,bufa,bufb,dev->getTrackYear(song),bufc,dev->getTrackRating(song),dev->getArt(song));
		Filter::addToGroup(dev,song,group);
		return group;
	}
	virtual bool HaveTopItem() {return false;}
	virtual void SetMode(int m) {mode=m;}
};

Filter * getFilter(wchar_t *name) {
	if(name) {
		if(!_wcsicmp(L"Artist",name)) return new ArtistFilter(); // 100
		if(!_wcsicmp(L"Album",name)) return new AlbumFilter(); //200
		if(!_wcsicmp(L"Genre",name)) return new GenreFilter(); //300
		if(!_wcsicmp(L"Artist Index",name)) return new ArtistIndexFilter(); //400
		if(!_wcsicmp(L"Year",name)) return new YearFilter(); //500
		if(!_wcsicmp(L"Album Artist",name)) return new AlbumArtistFilter(); //600
		if(!_wcsicmp(L"Publisher",name)) return new PublisherFilter(); //700
		if(!_wcsicmp(L"Composer",name)) return new ComposerFilter(); //800
		if(!_wcsicmp(L"Album Artist Index",name)) return new AlbumArtistIndexFilter(); //900
		if(!_wcsicmp(L"Album Art",name)) return new AlbumArtFilter(); //1000
	}
	return new ArtistFilter();
}