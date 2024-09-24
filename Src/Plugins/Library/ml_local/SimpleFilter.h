#ifndef NULLSOFT_ML_LOCAL_SIMPLEFILTER_H
#define NULLSOFT_ML_LOCAL_SIMPLEFILTER_H

#include "ViewFilter.h"
#include "resource.h"

class SimpleFilter : public ViewFilter
{
public:
	static int SimpleSortFunc(const void *elem1, const void *elem2);
	void SortResults(C_Config *viewconf, int which=0, int isfirst=0);
	void Fill(itemRecordW *items, int numitems, int *killswitch, int numitems2);
	int Size();
	const wchar_t *GetText(int row);
	void CopyText(int row, size_t column, wchar_t *dest, int destCch);
	virtual const wchar_t *CopyText2(int row, size_t column, wchar_t *dest, int destCch);
	void Empty();
	void AddColumns2();
	// override these 4 to make a simple filter
	virtual const wchar_t *GetField()=0;
	virtual const wchar_t *GetName()=0;
	virtual const wchar_t *GetNameSingular()=0;
	virtual const wchar_t *GetNameSingularAlt(int mode)=0;
	virtual const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len)=0;
	wchar_t name[64];
	wchar_t sname[64];
	wchar_t saname[64];
	virtual bool uses_nde_strings() { return true; }
private:
	queryListObject artistList;
};

class AlbumArtistFilter : public SimpleFilter
{
	const wchar_t *GetField(){return DB_FIELDNAME_albumartist;}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_ALBUM_ARTISTS,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_ALBUM_ARTIST,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_ALBUM_ARTIST_ALT:(mode==1?2065:2081)),saname,64);}
	const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len){return item->albumartist;}
	char * GetConfigId(){return "av_aa";}
};

class ArtistFilter : public SimpleFilter
{
	const wchar_t *GetField(){return DB_FIELDNAME_artist;}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_ARTIST_S,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_ARTIST,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_ARTIST_ALT:(mode==1?2066:2082)),saname,64);}
	const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len){return item->artist;}
	char * GetConfigId(){return "av_ar";}
};

class ComposerFilter : public SimpleFilter
{
	const wchar_t *GetField(){return DB_FIELDNAME_composer;}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_COMPOSERS,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_COMPOSER,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_COMPOSER_ALT:(mode==1?2067:2083)),saname,64);}
	const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len){return item->composer;}
	char * GetConfigId(){return "av_c";}
};

class GenreFilter : public SimpleFilter
{
	const wchar_t *GetField(){return DB_FIELDNAME_genre;}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_GENRES,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_GENRE,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_GENRE_ALT:(mode==1?2068:2084)),saname,64);}
	const wchar_t * GroupText(itemRecordW * item, wchar_t * buf, int len){return item->genre;}
	char * GetConfigId(){return "av_g";}
};

class PublisherFilter : public SimpleFilter
{
	const wchar_t *GetField(){return DB_FIELDNAME_publisher;}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_PUBLISHERS,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_PUBLISHER,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_PUBLISHER_ALT:(mode==1?2069:2085)),saname,64);}
	const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len){return item->publisher;}
	char * GetConfigId(){return "av_p";}
};

class YearFilter : public SimpleFilter
{
	const wchar_t *GetComparisonOperator() {return L"==";}
	const wchar_t *GetField(){return DB_FIELDNAME_year;}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_YEARS,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_YEAR,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_YEAR_ALT:(mode==1?2070:2086)),saname,64);}
	const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len){if(item->year>0) wsprintfW(buf,L"%d",item->year); else buf[0]=0; return buf;}
	char * GetConfigId(){return "av_y";}
	virtual bool uses_nde_strings() { return false; }
};

static wchar_t getIndex(const wchar_t* str) {
	if(!str) return 0;
	SKIP_THE_AND_WHITESPACEW(str);
	return towupper(*str);
}

class AlbumArtistIndexFilter : public SimpleFilter
{
	const wchar_t *GetComparisonOperator() {return L"BEGINSLIKE";}
	const wchar_t *GetField(){return DB_FIELDNAME_albumartist;}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_ALBUM_ARTIST_INDEXES,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_ALBUM_ARTIST_INDEX,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_ALBUM_ARTIST_INDEX_ALT:(mode==1?2071:2087)),saname,64);}
	const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len) {buf[0]=getIndex(item->albumartist); buf[1]=0; return buf;}
	char * GetConfigId(){return "av_aai";}
	virtual bool uses_nde_strings() { return false; }
};

class ArtistIndexFilter : public SimpleFilter
{
	const wchar_t *GetComparisonOperator() {return L"BEGINSLIKE";}
	const wchar_t *GetField(){return DB_FIELDNAME_artist;}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_ARTIST_INDEXES,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_ARTIST_INDEX,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_ARTIST_INDEX_ALT:(mode==1?2072:2088)),saname,64);}
	const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len) {buf[0]=getIndex(item->artist); buf[1]=0; return buf;}
	char * GetConfigId(){return "av_ai";}
	virtual bool uses_nde_strings() { return false; }
};

class PodcastChannelFilter : public SimpleFilter
{
	const wchar_t *GetField(){return DB_FIELDNAME_podcastchannel;}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_PODCAST_CHANNELS,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_PODCAST_CHANNEL,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_PODCAST_CHANNEL_ALT:(mode==1?2073:2089)),saname,64);}
	const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len){return getRecordExtendedItem_fast(item,extended_fields.podcastchannel);}
	char * GetConfigId(){return "av_pc";}
	virtual bool uses_nde_strings() { return false; }
};

class CategoryFilter : public SimpleFilter
{
	const wchar_t *GetField(){return L"category";}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_CATEGORIES,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_CATEGORY,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_CATEGORY_ALT:(mode==1?2074:2090)),saname,64);}
	const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len){return item->category;}
	char * GetConfigId(){return "av_cat";}
	virtual bool uses_nde_strings() { return false; }
};

class DirectorFilter : public SimpleFilter
{
	const wchar_t *GetField(){return DB_FIELDNAME_director;}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_DIRECTOR,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_DIRECTOR,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_DIRECTOR_ALT:(mode==1?2075:2091)),saname,64);}
	const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len){return getRecordExtendedItem_fast(item,extended_fields.director);}
	char * GetConfigId(){return "av_drt";}
	virtual bool uses_nde_strings() { return false; }
};

class ProducerFilter : public SimpleFilter
{
	const wchar_t *GetField(){return L"producer";}
	const wchar_t *GetName(){return WASABI_API_LNGSTRINGW_BUF(IDS_PRODUCER,name,64);}
	const wchar_t *GetNameSingular(){return WASABI_API_LNGSTRINGW_BUF(IDS_PRODUCER,sname,64);}
	const wchar_t *GetNameSingularAlt(int mode){return WASABI_API_LNGSTRINGW_BUF((!mode?IDS_PRODUCER_ALT:(mode==1?2076:2092)),saname,64);}
	const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len){return getRecordExtendedItem_fast(item,extended_fields.producer);}
	char * GetConfigId(){return "av_pdc";}
	virtual bool uses_nde_strings() { return false; }
};

#endif