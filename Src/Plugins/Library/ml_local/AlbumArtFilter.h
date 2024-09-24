#ifndef NULLSOFT_ML_LOCAL_ALBUMART_FILTER_H_
#define NULLSOFT_ML_LOCAL_ALBUMART_FILTER_H_

#include "AlbumFilter.h"
#include <tataki/bitmap/autobitmap.h>

class AlbumArtFilter : public AlbumFilter
{
public:
	AlbumArtFilter(HWND hwndDlg, int dlgitem, C_Config *c);
	virtual ~AlbumArtFilter();
	virtual char * GetConfigId(){return "av_art";}
	virtual int Size() { return albumList.Size - 1; }
	virtual const wchar_t *GetText(int row) { return AlbumFilter::GetText(row+1); }
	virtual void CopyText(int row, size_t column, wchar_t *dest, int destCch)  { return AlbumFilter::CopyText(row+1,column,dest,destCch); }
	virtual const wchar_t *CopyText2(int row, size_t column, wchar_t *dest, int destCch) { return AlbumFilter::CopyText2(row+1,column,dest,destCch); }

	virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
	AlbumArtContainer * GetArt(int row) { return albumList.Items[row].art; }

	BOOL DrawItemIcon(NMLVCUSTOMDRAW *plvcd, DCCanvas *pCanvas, UINT itemState, RECT *prcClip, BOOL bWndActive);
	BOOL PrepareDetails(HDC hdc);
	BOOL DrawItemDetail(NMLVCUSTOMDRAW *plvcd, DCCanvas *pCanvas, UINT itemState, RECT *prcClip, BOOL bWndActive, HDC hdcNaes, INT namesWidth);
	void drawArt(AlbumArtContainer *art, DCCanvas *pCanvas, RECT *prcDst, int itemid, int imageIndex);

	void AddColumns2();
	void Fill(itemRecordW *items, int numitems, int *killswitch, int numitems2);
	virtual bool HasTopItem() {return false;}
	static int __fastcall MyBuildSortFunc(const void *elem1, const void *elem2, const void *context);
	virtual bool MakeFilterQuery(int x, GayStringW *query);

	virtual HMENU GetMenu(bool isFilter, int filterNum, C_Config *c, HMENU themenu);
	virtual void ProcessMenuResult(int r, bool isFilter, int filterNum, C_Config *c, HWND parent);

	virtual void MetaUpdate(int r, const wchar_t *metaItem, const wchar_t *value);

protected:
	BOOL OnCustomDrawDetails(NMLVCUSTOMDRAW *plvcd, LRESULT *pResult);
	BOOL OnCustomDrawIcon(NMLVCUSTOMDRAW *plvcd, LRESULT *pResult);
	BOOL OnKeyDown(NMLVKEYDOWN *plvkd);
	BOOL CalcuateItemHeight(void);
protected:
	DWORD ratingHotItem;
	int ratingHotValue;
	int dlgitem;
	HWND hwndDlg;
	HWND hwndRealList;
	int mode;
	int icons_only;
	AutoSkinBitmap notfound;
	AutoSkinBitmap notfound60;
	AutoSkinBitmap notfound90;
	ARGB32 * classicnotfound[3];
	int classicnotfoundW,classicnotfoundH;
	INT ratingrow;
	HBITMAP hbmpNames;
	INT itemHeight;
	INT textHeight;
	INT ratingTop;
	INT namesWidth;
	HBRUSH bgBrush;
};

#endif // NULLSOFT_ML_LOCAL_ALBUMART_FILTER_H_