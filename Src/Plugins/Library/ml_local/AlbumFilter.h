#ifndef NULLSOFT_ML_LOCAL_ALBUMFILTER_H
#define NULLSOFT_ML_LOCAL_ALBUMFILTER_H

#include "ViewFilter.h"

class AlbumFilter : public ViewFilter
{
public:
	static int AlbumSortFunc(const void *elem1, const void *elem2);
	void SortResults(C_Config *viewconf, int which=0, int isfirst=0);
	void Fill(itemRecordW *items, int numitems, int *killswitch, int numitems2);
	virtual int Size();
	virtual const wchar_t *GetText(int row);
	virtual void CopyText(int row, size_t column, wchar_t *dest, int destCch);
	virtual const wchar_t *CopyText2(int row, size_t column, wchar_t *dest, int destCch);
	void Empty();
	const wchar_t *GetField();
	const wchar_t *GetName();
	const wchar_t *GetNameSingular();
	const wchar_t *GetNameSingularAlt(int mode);
	void AddColumns2();
	virtual const wchar_t * GroupText(itemRecordW * item, wchar_t * buf, int len);
	virtual void SaveColumnWidths();
	virtual void CustomizeColumns(HWND parent, BOOL showmenu);
	virtual char * GetConfigId(){return "av2";}
	static char * getColConfig(int i);
	wchar_t name[64];
	wchar_t sing_name[64];
	wchar_t sing_name_alt[64];
protected:
	queryListObject albumList;

	enum
	{
		ALBUMFILTER_COLUMN_NAME = 0,
		ALBUMFILTER_COLUMN_YEAR = 1,
		ALBUMFILTER_COLUMN_ALBUMS = 2,
		ALBUMFILTER_COLUMN_TRACKS = 3,
		ALBUMFILTER_COLUMN_REPLAYGAIN = 4,
		ALBUMFILTER_COLUMN_SIZE = 5,
		ALBUMFILTER_COLUMN_LENGTH = 6,
		ALBUMFILTER_COLUMN_ARTIST = 7,
		ALBUMFILTER_COLUMN_GENRE = 8,
		ALBUMFILTER_COLUMN_RATING = 9,
		ALBUMFILTER_COLUMN_LASTUPD = 10,
	};
};

#endif