#ifndef NULLSOFT_ML_LOCAL_VIEWFILTER_H
#define NULLSOFT_ML_LOCAL_VIEWFILTER_H

#include "main.h"
#include "../nu/listview.h"
#include <vector>
class AlbumArtContainer;

extern wchar_t *emptyQueryListString;
struct  queryListItem
{
	queryListItem() : name(0), albumGain(0), length(0), art(0), artist(0), genre(0),
					  gracenoteFileId(0), rating(0), lastupd(0), size(0) {
		ZeroMemory(&ifields,sizeof(ifields));
	}
	wchar_t *name;
	wchar_t *albumGain;
	int length, ifields[3];
	AlbumArtContainer *art;
	wchar_t *artist;
	wchar_t *genre;
	wchar_t *gracenoteFileId;
	int rating;
	__time64_t lastupd;
	__int64 size;
} ;

struct queryListObject
{
	queryListObject() : Items(0), Size(0), Alloc(0){}
	queryListItem *Items;
	int Size, Alloc;
} ;

class ListField {
public:
	int field;
	int width;
	int pos;
	const wchar_t * name;
	bool hidden, hiddenDefault;
	ListField(int field, int width, const wchar_t * name, C_Config * config, char * av, bool hidden=false, bool hiddenDefault=false, bool readini=true, int pos=0);
	ListField(int field, int width, int name, C_Config * config, char * av, bool hidden=false, bool hiddenDefault=false, bool readini=true, int pos=0);
	~ListField() { free((void*)name); }
	void ResetPos();
};

class ViewFilter
{
public:
	virtual ~ViewFilter(){}
	virtual void SortResults(C_Config *viewconf, int which=0, int isfirst=0)=0;
	virtual void Fill(itemRecordW *items, int numitems, int *killswitch, int numitems2)=0;
	virtual int Size()=0;
	virtual const wchar_t *GetText(int row)=0;
	virtual void CopyText(int row, size_t column, wchar_t *dest, int destCch)=0;
	virtual const wchar_t *CopyText2(int row, size_t column, wchar_t *dest, int destCch)=0;
	virtual void Empty()=0;
	virtual const wchar_t *GetField()=0;
	virtual const wchar_t *GetName()=0;
	virtual const wchar_t *GetNameSingular()=0;
	virtual const wchar_t *GetNameSingularAlt(int mode)=0;
	
	virtual const wchar_t *GroupText(itemRecordW * item, wchar_t * buf, int len)=0;
	virtual void AddColumns2()=0;
	void AddColumns();
	virtual void SaveColumnWidths();
	static int __fastcall BuildSortFunc(const void *elem1, const void *elem2, const void *context);
	virtual void CustomizeColumns(HWND parent, BOOL showmenu);
	void ResetColumns();
	virtual const wchar_t *GetComparisonOperator() {return L"LIKE";}
	virtual char * GetConfigId(){return "av";}
	void ClearColumns();
	virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam){return 0;}
	virtual bool HasTopItem() {return true;}
	virtual bool MakeFilterQuery(int n, GayStringW *str){return false;}

	virtual HMENU GetMenu(bool isFilter, int filterNum, C_Config *c, HMENU themenu);
	virtual void ProcessMenuResult(int r, bool isFilter, int filterNum, C_Config *c, HWND parent);
	virtual void MetaUpdate(int r, const wchar_t *metaItem, const wchar_t *value) {}

	std::vector<ListField*> showncolumns;
	std::vector<ListField*> hiddencolumns;
	ViewFilter *nextFilter;
	W_ListView * list;
	int numGroups;
};

#endif