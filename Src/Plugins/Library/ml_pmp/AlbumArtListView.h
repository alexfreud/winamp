#ifndef _ALBUMARTLISTVIEW_H_
#define _ALBUMARTLISTVIEW_H_

#include "SkinnedListView.h"
#include <tataki/bitmap/autobitmap.h>
#include <tataki/canvas/bltcanvas.h>

class AlbumArtListView : public SkinnedListView {
public:
	AlbumArtListView(ListContents * lc, int dlgitem, HWND libraryParent, HWND parent, bool enableHeaderMenu=true);

	virtual ~AlbumArtListView();
	virtual int GetFindItemColumn();
	virtual BOOL DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

	virtual HMENU GetMenu(bool isFilter, int filterNum, C_Config *c, HMENU themenu);
	virtual void ProcessMenuResult(int r, bool isFilter, int filterNum, C_Config *c, HWND parent);

	BOOL DrawItemIcon(NMLVCUSTOMDRAW *plvcd, DCCanvas *pCanvas, UINT itemState, RECT *prcClip, BOOL bWndActive);
	BOOL PrepareDetails(HDC hdc);
	BOOL DrawItemDetail(NMLVCUSTOMDRAW *plvcd, DCCanvas *pCanvas, UINT itemState, RECT *prcClip, BOOL bWndActive, HDC hdcNaes, INT namesWidth);
	void drawArt(pmpart_t art, DCCanvas *pCanvas, RECT *prcDst, int itemid, int imageIndex);

protected:
	BOOL OnCustomDrawDetails(NMLVCUSTOMDRAW *plvcd, LRESULT *pResult);
	BOOL OnCustomDrawIcon(NMLVCUSTOMDRAW *plvcd, LRESULT *pResult);
	BOOL OnKeyDown(NMLVKEYDOWN *plvkd);
	BOOL CalcuateItemHeight(void);

	int dlgitem;
	HWND hwndDlg;
	int mode;
	WNDPROC oldproc;
	AutoSkinBitmap notfound, notfound60, notfound90;
	ARGB32 * classicnotfound[3];
	int classicnotfoundW,classicnotfoundH;
	INT ratingrow;
	HBITMAP hbmpNames;
	INT itemHeight;
	INT textHeight;
	INT ratingTop;
};

#endif // _ALBUMARTLISTVIEW_H_