#ifndef _FILTERS_H
#define _FILTERS_H

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <shlobj.h>
#include <time.h>
#include "..\..\General\gen_ml/ml.h"
#include "../nu/listview.h"
#include "..\..\General\gen_ml/itemlist.h"
#include "..\..\General\gen_ml/childwnd.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/wa_dlg.h"
#include "pmp.h"
#include "SkinnedListView.h" 
#include "config.h"

class FilterItem {
public:
	FilterItem() : numTracks(0), independentTracks(0), nextFilter(0), independentNextFilter(0),
				   numNextFilter(0), size(0), length(0), cloudState(0), independentCloudState(0),
				   independentSize(0), independentLength(0) {}
	virtual ~FilterItem();
	int numTracks, cloudState;
	__int64 size, length;
	int independentTracks, independentCloudState;
	__int64 independentSize, independentLength;
	C_ItemList * independentNextFilter;
	C_ItemList * nextFilter;
	int numNextFilter;
	virtual int compareTo(FilterItem *that)=0;
	virtual int compareTo2(FilterItem *that, int use_by, int use_dir)=0;
	virtual void GetCellText(int col, wchar_t * buf, int buflen)=0;
	void GetDefaultColumnsCellText(int col, wchar_t*buf, int buflen);
	int DefaultSortAction(FilterItem *that, int use_by, int use_dir);
	virtual bool isWithoutGroup()=0;
	virtual pmpart_t GetArt(){return NULL;}
};

class Filter {
public:
	Filter() : nextFilter(0), namePlural(0), name(0) {}
	virtual ~Filter() { free(name); free(namePlural); }
	virtual void AddColumns(Device * dev,C_ItemList * fields,C_Config * config, bool cloud = false)=0;
	void AddDefaultColumns(Device * dev,C_ItemList * fields,C_Config * config, int columnStart, bool cloud = false);
	virtual int sortFunc(Device * dev, songid_t a, songid_t b)=0;
	virtual bool isInGroup(Device *dev, songid_t song, FilterItem * group)=0;
	virtual void addToGroup(Device *dev, songid_t song, FilterItem * group);
	virtual FilterItem * newGroup(Device *dev, songid_t song)=0;
	virtual bool HaveTopItem(){return true;}
	virtual void SetMode(int mode){}
	Filter * nextFilter;
	wchar_t * namePlural;
	wchar_t * name;
};

#endif //_FILTERS_H