#ifndef _SKINNEDLISTVIEW_H_
#define _SKINNEDLISTVIEW_H_

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <shlobj.h>
#include "..\..\General\gen_ml/ml.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include "pmp.h"
#include "../nu/listview.h"
#include <map>
#include "..\..\General\gen_ml/itemlist.h"
#include "..\..\General\gen_ml/childwnd.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/wa_dlg.h"
#include "config.h"

class ListField {
public:
	int field;
	int width;
	int pos;
	wchar_t * name;
	bool hidden, hiddenDefault;
	ListField() : field(0), width(0), pos(0), name(0), hidden(0), hiddenDefault(0) {};
	ListField(int field, int defwidth, wchar_t * name, C_Config * config, bool hidden=false);
	~ListField() { free(name); }
	void ResetPos();
};

class ListContents {
public:
	C_ItemList fields, hiddenfields;
	C_Config * config;
	int cloud, cloudcol;
	typedef std::map<size_t, int> CloudCache;
	CloudCache cloud_cache;
	Device *dev;
	ListContents() : config(NULL), cloud(0), cloudcol(-1), dev(0) {};
	virtual ~ListContents();
	virtual int GetNumColumns()=0;
	virtual int GetNumRows()=0;
	virtual wchar_t * GetColumnTitle(int num)=0;
	virtual int GetColumnWidth(int num)=0;
	virtual void GetCellText(int row, int col, wchar_t * buf, int buflen)=0;
	virtual int GetSortColumn();
	virtual int GetSortDirection(); // return true for acending.
	virtual void ColumnClicked(int col); // sort contents and update list
	virtual void ColumnResize(int col, int newWidth); // called once only just before deconstuctor if changed
	virtual void GetInfoString(wchar_t * buf);
	void SortColumns();
	virtual bool CustomizeColumns(HWND parent, BOOL showmenu);
	virtual void ResetColumns();
	virtual pmpart_t GetArt(int row){return NULL;}
	virtual void SetMode(int mode){}
	virtual songid_t GetTrack(int pos)=0;
};

class PrimaryListContents : public ListContents {
public:
  virtual void RemoveTrack(songid_t song){}
};

class SkinnedListView {
protected:
	HWND libraryParent, headerWindow;
	int dlgitem;
	bool enableHeaderMenu;
public:
	int skinlistview_handle;
	ListContents * contents;
	W_ListView listview;
	SkinnedListView(ListContents * lc, int dlgitem, HWND libraryParent, HWND parent, bool enableHeaderMenu=true);
	virtual ~SkinnedListView(){}
	virtual BOOL DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
	static LRESULT pmp_listview(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT pmp_listview_alt(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void UpdateList(bool softUpdate=false);
	virtual int GetFindItemColumn();
	virtual HMENU GetMenu(bool isFilter, int filterNum, C_Config *c, HMENU themenu);
	virtual void ProcessMenuResult(int r, bool isFilter, int filterNum, C_Config *c, HWND parent);
	virtual void InitializeFilterData(int filterNum, C_Config *config);
};

#endif //_SKINNEDLISTVIEW_H_