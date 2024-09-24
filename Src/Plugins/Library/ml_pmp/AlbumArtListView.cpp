#include "AlbumArtListView.h"
#include "api__ml_pmp.h"
#include "resource1.h"
#include "../tataki/export.h"
#include <api/service/waServiceFactory.h>
#include <api/service/svcs/svc_imgload.h>
#include "./local_menu.h"

extern winampMediaLibraryPlugin plugin;

#define WM_EX_GETREALLIST	(WM_USER + 0x01)

#define HORZ_SPACING	4
#define VERT_SPACING	4

AlbumArtListView::AlbumArtListView(ListContents * lc, int dlgitem, HWND libraryParent, HWND parent, bool enableHeaderMenu)
	:	SkinnedListView(lc,dlgitem,libraryParent,parent,enableHeaderMenu), hbmpNames(NULL),
		classicnotfoundW(0), classicnotfoundH(0), ratingrow(-1), itemHeight(0), textHeight(0), ratingTop(0),
		notfound(L"winamp.cover.notfound"), notfound60(L"winamp.cover.notfound.60"), notfound90(L"winamp.cover.notfound.90")
{
	this->hwndDlg = parent;
	this->dlgitem = dlgitem;
	mode = lc->config->ReadInt(L"albumartviewmode",0);
	lc->SetMode(mode);
	
	ZeroMemory(classicnotfound, sizeof(classicnotfound));
}

AlbumArtListView::~AlbumArtListView() {
	if (hbmpNames) DeleteObject(hbmpNames);
}

int AlbumArtListView::GetFindItemColumn() {
	if(mode > 1) return 1;
	return contents->GetSortColumn();
}

static COLORREF GetWAColor(INT index)
{
	static int (*wad_getColor)(int idx) = NULL;
	if (!wad_getColor) *(void **)&wad_getColor=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,1,ML_IPC_SKIN_WADLG_GETFUNC);
	return (wad_getColor) ? wad_getColor(index) : 0xFF00FF;
}

static void getImgSize(int mode, int &w, int &h) {
	switch(mode) {
		case 0: w=h=60; break;
		case 1: w=h=90; break;
		case 2: w=h=120; break;
		case 3: w=h=60; break;
		case 4: w=h=90; break;
		case 5: w=h=120; break;
	}
}

static bool isDetailsMode(int mode) {
	return mode == 0 || mode == 1 || mode == 2;
}

void DrawRect(HDC dc, int x, int y, int w, int h) {
	w-=1;
	h-=1;
	MoveToEx(dc,x,y,NULL);
	LineTo(dc,x,y+h);
	MoveToEx(dc,x,y+h,NULL);
	LineTo(dc,x+w,y+h);
	MoveToEx(dc,x+w,y+h,NULL);
	LineTo(dc,x+w,y);
	MoveToEx(dc,x+w,y,NULL);
	LineTo(dc,x,y);
}

static int getStrExtent(HDC dc, const wchar_t * s) {
	int ret=0;
	while(s && *s) {
		int f=0;
		GetCharWidth32(dc,*s,*s,&f);
		s++;
		ret+=f;
	}
	return int(ret);
}

ARGB32 * loadImg(const void * data, int len, int *w, int *h, bool ldata=false) {
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = plugin.service->service_getNumServices(imgload);
	for(int i=0; i<n; i++) {
		waServiceFactory *sf = plugin.service->service_enumService(imgload,i);
		if(sf) {
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if(l) {
				if(l->testData(data,len)) {
					ARGB32* ret;
					if(ldata) ret = l->loadImageData(data,len,w,h);
					else ret = l->loadImage(data,len,w,h);
					sf->releaseInterface(l);
					return ret;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}

ARGB32 * loadRrc(int id, char * sec, int *w, int *h, bool data=false)
{
	DWORD size = 0;
	// as a nice little treat, allow lang packs to contain a custom IDR_IMAGE_NOTFOUND file
	HGLOBAL resourceHandle = WASABI_API_LOADRESFROMFILEA((LPCSTR)sec, (LPCSTR)MAKEINTRESOURCEA(id), &size);
	if(resourceHandle)
	{
		ARGB32* ret = loadImg(resourceHandle,size,w,h,data);
		UnlockResource(resourceHandle);
		return ret;
	}
	return NULL;
}

void adjustbmp(ARGB32 * p, int len, COLORREF fg)
{
	ARGB32 * end = p+len;
	while (p < end)
	{
		int a = (*p>>24)&0xff ;
		int b = a*((*p&0xff) * (fg&0xff)) / (0xff*0xff);
		int g = a*(((*p>>8)&0xff) * ((fg>>8)&0xff)) / (0xff*0xff);
		int r = a*(((*p>>16)&0xff) * ((fg>>16)&0xff)) / (0xff*0xff);
		*p = (a<<24) | (r&0xff) | ((g&0xff)<<8) | ((b&0xff)<<16);
		p++;
	}
}

void AlbumArtListView::drawArt(pmpart_t art, DCCanvas *pCanvas, RECT *prcDst, int itemid, int imageIndex)
{
	int x = prcDst->left;
	int y = prcDst->top;
	int w = prcDst->right - prcDst->left;
	int h = prcDst->bottom - prcDst->top;
	HDC dc = pCanvas->getHDC();
	if(art) contents->dev->setArtNaturalSize(art,w,h);
	// draw image 4,4,w,h
	if(art && contents->dev->drawArt(art,pCanvas->getHDC(),x,y,w,h)) {
		// drawn by plugin!
	} 
	else 
	{
		SkinBitmap *noart;

		int h = prcDst->right - prcDst->left;
		if (h == 60)
			noart = notfound60.getBitmap();
		else if (h == 90)
			noart = notfound90.getBitmap();
		else
			noart = notfound.getBitmap();

		if (!noart || noart->isInvalid())
		{
			if(classicnotfound[imageIndex])
				SkinBitmap(classicnotfound[imageIndex],classicnotfoundW,classicnotfoundH).stretchToRectAlpha(pCanvas, prcDst);
			else
			{
				DrawRect(dc,x,y,w,h);
				wchar_t str1[32] = {0}, str2[32] = {0};
				WASABI_API_LNGSTRINGW_BUF(IDS_NO_IMAGE,str1,32);
				WASABI_API_LNGSTRINGW_BUF(IDS_AVAILABLE,str2,32);
				ExtTextOutW(dc, w/2 - 22 + x, w/2 - 14 + y, 0,NULL,str1,wcslen(str1),0);
				ExtTextOutW(dc, w/2 - 22 + x, w/2 + 1 + y, 0,NULL,str2,wcslen(str2),0);
			}
		}
		else
			noart->stretch(pCanvas,x,y,w,h);
	}
}

// icon view
BOOL AlbumArtListView::DrawItemIcon(NMLVCUSTOMDRAW *plvcd, DCCanvas *pCanvas, UINT itemState, RECT *prcClip, BOOL bWndActive)
{
	HDC hdc = plvcd->nmcd.hdc;
	RECT ri, re, rcText;
	int w=0,h=0, imageIndex;
	getImgSize(mode,w,h);

	SetBkColor(hdc, plvcd->clrTextBk);
	SetTextColor(hdc, plvcd->clrText);
	SetRect(&rcText, plvcd->nmcd.rc.left, plvcd->nmcd.rc.bottom - (textHeight + 2), plvcd->nmcd.rc.right, plvcd->nmcd.rc.bottom);
	imageIndex = 0;
	if ((LVIS_SELECTED | LVIS_FOCUSED) & itemState)
	{
		SetRect(&re, plvcd->nmcd.rc.left, plvcd->nmcd.rc.top, plvcd->nmcd.rc.right, rcText.top - 4);
		if (IntersectRect(&ri, &re, prcClip)) ExtTextOutW(hdc,0, 0, ETO_OPAQUE, &ri, L"", 0, 0);
		if (LVIS_SELECTED & itemState) imageIndex = (bWndActive) ? 1 : 2;
	}

	SetRect(&re, plvcd->nmcd.rc.left + 2, plvcd->nmcd.rc.top + 2, plvcd->nmcd.rc.left + 2 + w, plvcd->nmcd.rc.top + 2 + h);
	if (IntersectRect(&ri, &re, prcClip))
	{
		pmpart_t art = contents->GetArt(plvcd->nmcd.dwItemSpec);
		drawArt(art, pCanvas, &re, plvcd->nmcd.dwItemSpec, imageIndex);
	}

	if (IntersectRect(&ri, &rcText, prcClip))
	{
		if ((LVIS_SELECTED | LVIS_FOCUSED) & itemState) ExtTextOutW(hdc,0, 0, ETO_OPAQUE, &ri, L"", 0, 0);

		wchar_t buf[104] = {0};
		contents->GetCellText(plvcd->nmcd.dwItemSpec,1,buf,100);
		const wchar_t *p = buf;
		if (p && *p)
		{
			SetRect(&ri, rcText.left + 2, rcText.top + 1, rcText.right - 2, rcText.bottom -1);
			DrawTextW(hdc, p, -1, &ri, DT_CENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
		}
	
		if ((LVIS_FOCUSED & itemState) && bWndActive)
		{
			HWND hwndRealList;
			hwndRealList = (HWND)SendMessageW(plvcd->nmcd.hdr.hwndFrom, WM_EX_GETREALLIST, 0, 0L);
			if (hwndRealList &&	0 == (0x01/*UISF_HIDEFOCUS*/ & SendMessageW(hwndRealList, 0x0129/*WM_QUERYUISTATE*/, 0, 0L)))
			{
				SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
				SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
				DrawFocusRect(hdc, &rcText);
			}
		}
	}
	return TRUE;
}

//detail view
BOOL AlbumArtListView::PrepareDetails(HDC hdc)
{
	INT width(0), height, len;
	HFONT hFont,hFontBold, hOldFont;
	HDC hdcTmp;
	HBITMAP hbmpOld;
	LOGFONT l={0};
	RECT ri = {0};
	wchar_t ratingstr[100] = {0}, buf[100] = {0};

	hdcTmp = CreateCompatibleDC(hdc);
	if (!hdcTmp) return FALSE;

	hFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
	GetObject(hFont, sizeof(LOGFONT), &l);
	l.lfWeight = FW_BOLD;
	hFontBold = CreateFontIndirect(&l);

	hOldFont = (HFONT)SelectObject(hdcTmp, hFontBold);

	for (int i=0; i < contents->GetNumColumns(); i++)
	{
		int of = getStrExtent(hdcTmp, contents->GetColumnTitle(i));
		if (of > width) width = of;
	}
	if(width) width += 20;
	height = contents->GetNumColumns() * textHeight;
	hbmpNames = CreateCompatibleBitmap(hdc, width * 3, height); 
	hbmpOld = (HBITMAP)SelectObject(hdcTmp, hbmpNames);

	SetRect(&ri, 0, 0, width, height);

	WASABI_API_LNGSTRINGW_BUF(IDS_RATING,ratingstr,100);
	INT clrText[3]  = { WADLG_ITEMFG, WADLG_SELBAR_FGCOLOR, WADLG_INACT_SELBAR_FGCOLOR, };
	INT clrBk[3]  = { WADLG_ITEMBG, WADLG_SELBAR_BGCOLOR, WADLG_INACT_SELBAR_BGCOLOR, };
	for (int j = 0; j < 3; j++)
	{
		SetTextColor(hdcTmp, GetWAColor(clrText[j]));
		SetBkColor(hdcTmp, GetWAColor(clrBk[j]));

		ExtTextOutW(hdcTmp,0, 0, ETO_OPAQUE, &ri, L"", 0, 0);

		for (int i=0, top = 0; i < contents->GetNumColumns(); i++, top += textHeight)
		{
			if (-1 == ratingrow && 0 == lstrcmpW(contents->GetColumnTitle(i), ratingstr)) ratingrow = i;
			StringCchCopyW(buf, 100, contents->GetColumnTitle(i));
			len = wcslen(buf);
			if (len > 0 && len < 99) { buf[len] = L':'; len ++; }
			ExtTextOutW(hdcTmp, ri.left + 1, top, ETO_CLIPPED, &ri, buf, len, NULL);
		}
		OffsetRect(&ri, width, 0);
	}

	SelectObject(hdcTmp, hbmpOld);
	SelectObject(hdcTmp, hOldFont);
	DeleteObject(hFontBold);
	DeleteDC(hdcTmp);
	return TRUE;
}

BOOL AlbumArtListView::DrawItemDetail(NMLVCUSTOMDRAW *plvcd, DCCanvas *pCanvas, UINT itemState, RECT *prcClip, BOOL bWndActive, HDC hdcNames, INT namesWidth)
{
	RECT ri, re;
	HDC hdc;

	INT imageIndex;

	hdc = plvcd->nmcd.hdc;

	SetTextColor(hdc, plvcd->clrText);
	SetBkColor(hdc, plvcd->clrTextBk);

	if (LVIS_SELECTED & itemState)
	{
		// background
		SetRect(&re, plvcd->nmcd.rc.left + 4, plvcd->nmcd.rc.top, plvcd->nmcd.rc.right, plvcd->nmcd.rc.bottom - 5);
		if (IntersectRect(&ri, &re, prcClip)) ExtTextOutW(hdc,0, 0, ETO_OPAQUE, &ri, L"", 0, 0);
		imageIndex = (bWndActive) ? 1 : 2;
	}
	else imageIndex = 0;

	int w, h;
	getImgSize(mode, w, h);	
	SetRect(&re, 6+plvcd->nmcd.rc.left, 3+plvcd->nmcd.rc.top, 6+ plvcd->nmcd.rc.left + w, 3+ plvcd->nmcd.rc.top + h);
	if (IntersectRect(&ri, &re, prcClip))
	{
		pmpart_t art = contents->GetArt(plvcd->nmcd.dwItemSpec);
		drawArt(art, pCanvas, &re, plvcd->nmcd.dwItemSpec, imageIndex);
	}

	// text
	int limCY, limCX;
	wchar_t buf[100] = {0};

	limCY = plvcd->nmcd.rc.bottom;
	if (prcClip->bottom < plvcd->nmcd.rc.bottom) limCY = prcClip->bottom;
	limCX = plvcd->nmcd.rc.right -1;
	if (prcClip->right < plvcd->nmcd.rc.right) limCX = prcClip->right;

	SetRect(&ri, w+16+plvcd->nmcd.rc.left, 3+plvcd->nmcd.rc.top, limCX, limCY);

	if (hdcNames && ri.left < ri.right)
	{
		BitBlt(hdc, ri.left, ri.top, min(ri.right - ri.left, namesWidth), ri.bottom - ri.top, hdcNames, namesWidth*imageIndex, 0, SRCCOPY);
		ri.left += namesWidth;
	}

	ri.bottom = ri.top;

	if (ri.left < ri.right)
	{
		for (int i=0; i < contents->GetNumColumns() && ri.top < limCY; i++, ri.top += textHeight)
		{
			contents->GetCellText(plvcd->nmcd.dwItemSpec,i,buf,100);
			const wchar_t *p = buf;

			ri.bottom += textHeight;
			if (ri.bottom > limCY) ri.bottom = limCY;

			if ((INT)i == ratingrow)  // this is the ratings column, so draw graphical stars
			{
				int rating = wcslen(buf);
				RATINGDRAWPARAMS p = {sizeof(RATINGDRAWPARAMS),hdc,
										{ri.left, ri.top + ratingTop, ri.right,ri.bottom},
										rating,5,0,RDS_SHOWEMPTY,NULL,0};
				MLRating_Draw(plugin.hwndLibraryParent,&p);
			}
			else ExtTextOutW(hdc, ri.left, ri.top, ETO_CLIPPED ,&ri,p,wcslen(p),NULL);
		}
	}

	// bottom line
	MoveToEx(hdc,plvcd->nmcd.rc.left + 4,plvcd->nmcd.rc.bottom - 3,NULL);
	LineTo(hdc,plvcd->nmcd.rc.right, plvcd->nmcd.rc.bottom - 3);

	// focus rect
	SetRect(&ri, plvcd->nmcd.rc.left + 4, plvcd->nmcd.rc.top, plvcd->nmcd.rc.right, plvcd->nmcd.rc.bottom - 5);
	if ((LVIS_FOCUSED & itemState) && bWndActive)
	{
		HWND hwndRealList;
		hwndRealList = (HWND)SendMessageW(plvcd->nmcd.hdr.hwndFrom, WM_EX_GETREALLIST, 0, 0L);
		if (hwndRealList &&	0 == (0x01/*UISF_HIDEFOCUS*/ & SendMessageW(hwndRealList, 0x0129/*WM_QUERYUISTATE*/, 0, 0L)))
		{
			SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
			DrawFocusRect(hdc, &ri);
		}
	}

	return TRUE;
}

static HWND CreateSmoothScrollList(HWND parent, int x, int y, int cx, int cy, int dlgid) {
	DWORD flags = WS_CHILD | WS_VSCROLL | DS_CONTROL | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	HWND h = CreateWindowExW(WS_EX_CONTROLPARENT, L"SmoothScrollList", L"",flags, x, y, cx, cy, parent,(HMENU)dlgid, NULL, (LPVOID)0);
	SendMessage(h,WM_INITDIALOG,0,0);
	return h;
}

static HWND CreateHeaderIconList(HWND parent, int x, int y, int cx, int cy, int dlgid) {
	DWORD flags = WS_CHILD | WS_VSCROLL | DS_CONTROL | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	HWND h = CreateWindowExW(WS_EX_CONTROLPARENT, L"HeaderIconList", L"",flags, x, y, cx, cy, parent,(HMENU)dlgid, NULL, (LPVOID)0);
	SendMessage(h,WM_INITDIALOG,0,0);
	return h;
}

BOOL AlbumArtListView::OnKeyDown(NMLVKEYDOWN *plvkd)
{
	switch (plvkd->wVKey)
	{
		case 'A':
			if (GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT))
			{
				LVITEM item;
				item.state = LVIS_SELECTED;
				item.stateMask = LVIS_SELECTED;
				SendMessageW(plvkd->hdr.hwndFrom, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&item);
				return TRUE;
			}
		break;
	}
	return FALSE;
}

BOOL AlbumArtListView::OnCustomDrawIcon(NMLVCUSTOMDRAW *plvcd, LRESULT *pResult)
{
	static RECT rcClip;
	static BOOL bActive;
	static DCCanvas activeCanvas;

	*pResult = CDRF_DODEFAULT;
	switch(plvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			if (0 == plvcd->nmcd.rc.bottom && 0 == plvcd->nmcd.rc.right)
			{
				*pResult = CDRF_SKIPDEFAULT;
				return TRUE;
			}
			CopyRect(&rcClip, &plvcd->nmcd.rc);
			bActive = (GetFocus() == (HWND)SendMessageW(plvcd->nmcd.hdr.hwndFrom, WM_EX_GETREALLIST, 0, 0L));
			activeCanvas.cloneDC(plvcd->nmcd.hdc, NULL);
			*pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
		return TRUE;

		case CDDS_ITEMPREPAINT:
		{
			UINT itemState;
			itemState = SendMessageW(plvcd->nmcd.hdr.hwndFrom, LVM_GETITEMSTATE, plvcd->nmcd.dwItemSpec, 
										LVIS_FOCUSED | LVIS_SELECTED | LVIS_DROPHILITED | LVIS_CUT);

			plvcd->nmcd.rc.left = LVIR_BOUNDS;
			SendMessageW(plvcd->nmcd.hdr.hwndFrom, LVM_GETITEMRECT, plvcd->nmcd.dwItemSpec, (LPARAM)&plvcd->nmcd.rc);
			plvcd->nmcd.rc.right -= HORZ_SPACING;
			if (rcClip.left < plvcd->nmcd.rc.right) 
			{
				DrawItemIcon(plvcd, &activeCanvas, itemState, &rcClip, bActive);
				*pResult = CDRF_SKIPDEFAULT;
			}
		}
		return TRUE;

		case CDDS_POSTPAINT:
		return TRUE;
	}
	return FALSE;
}

BOOL AlbumArtListView::OnCustomDrawDetails(NMLVCUSTOMDRAW *plvcd, LRESULT *pResult)
{
	static RECT rcClip;
	static BOOL bActive;
	static HDC hdcNames;
	static INT namesWidth;
	static HBITMAP hbmpOld;
	static HPEN penOld;
	static DCCanvas activeCanvas;

	*pResult = CDRF_DODEFAULT;
	switch(plvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			if (0 == plvcd->nmcd.rc.bottom && 0 == plvcd->nmcd.rc.right)
			{
				*pResult = CDRF_SKIPDEFAULT;
				return TRUE;
			}
			CopyRect(&rcClip, &plvcd->nmcd.rc);
			if (!hbmpNames) PrepareDetails(plvcd->nmcd.hdc);
			if (hbmpNames)
			{
				BITMAP bi;
				GetObject(hbmpNames, sizeof(BITMAP), &bi);
				namesWidth = bi.bmWidth/3;
				hdcNames = CreateCompatibleDC(plvcd->nmcd.hdc);
				hbmpOld = (hdcNames) ? (HBITMAP)SelectObject(hdcNames, hbmpNames) : NULL; 
			}
			else
			{
				hdcNames = NULL;
				namesWidth = 0;
			}
			bActive = (GetFocus() == (HWND)SendMessageW(plvcd->nmcd.hdr.hwndFrom, WM_EX_GETREALLIST, 0, 0L));

			penOld = (HPEN)SelectObject(plvcd->nmcd.hdc, CreatePen(PS_SOLID,1, GetWAColor(WADLG_HILITE)));
			activeCanvas.cloneDC(plvcd->nmcd.hdc, NULL);
			*pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
		return TRUE;

		case CDDS_ITEMPREPAINT:
		{
			UINT itemState;
			itemState = SendMessageW(plvcd->nmcd.hdr.hwndFrom, LVM_GETITEMSTATE, plvcd->nmcd.dwItemSpec, 
										LVIS_FOCUSED | LVIS_SELECTED | LVIS_DROPHILITED | LVIS_CUT);

			plvcd->nmcd.rc.left = LVIR_BOUNDS;
			SendMessageW(plvcd->nmcd.hdr.hwndFrom, LVM_GETITEMRECT, plvcd->nmcd.dwItemSpec, (LPARAM)&plvcd->nmcd.rc);
			
			if (rcClip.left < plvcd->nmcd.rc.right) 
			{
				DrawItemDetail(plvcd, &activeCanvas, itemState, &rcClip, bActive, hdcNames, namesWidth);
				*pResult = CDRF_SKIPDEFAULT;
			}
		}
		return TRUE;

		case CDDS_POSTPAINT:
			if (hdcNames)
			{
				SelectObject(hdcNames, hbmpOld);
				DeleteDC(hdcNames);
			}
			if (penOld)
			{
				HPEN pen;
				pen = (HPEN)SelectObject(plvcd->nmcd.hdc, penOld);
				if (pen) DeleteObject(pen);
				penOld = NULL;
			}
		return TRUE;
	}
	return FALSE;
}

BOOL AlbumArtListView::CalcuateItemHeight(void)
{
	int w, h;
	HWND hwndList = GetDlgItem(hwndDlg, dlgitem);
	TEXTMETRIC tm = {0};

	getImgSize(mode, w, h);

	textHeight = 0;

	HDC hdc = GetDC(hwndDlg);
	if (hdc)
	{
		HFONT hFont = (HFONT)SendMessageW(hwndList, WM_GETFONT, 0, 0L);
		if (!hFont) hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		HFONT hFontOld = (HFONT)SelectObject(hdc, hFont);

		GetTextMetrics(hdc, &tm);
		textHeight = tm.tmHeight;
		SelectObject(hdc, hFontOld);
		ReleaseDC(hwndDlg, hdc);
	}

	if (isDetailsMode(mode))
	{
		if (textHeight < 14) textHeight = 14;
		RECT r;
		MLRating_CalcRect(plugin.hwndLibraryParent, NULL, 5, &r);
		r.bottom -= r.top;

		if ( r.bottom >= textHeight ) ratingTop = 0;
		else 
		{
			if (tm.tmAscent > (r.bottom + (r.bottom/2))) ratingTop = tm.tmAscent - r.bottom;
			else ratingTop = (textHeight - r.bottom)/2 + 1;
		}

		int newHeight = max(h, textHeight * (INT)contents->GetNumColumns()) + 12;
		if (newHeight != itemHeight)
		{
			itemHeight = newHeight;
			RECT rw;
			GetWindowRect(hwndList, &rw);
			SetWindowPos(hwndList, NULL, 0, 0, rw.right - rw.left - 1, rw.bottom - rw.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW); 
			SetWindowPos(hwndList, NULL, 0, 0, rw.right - rw.left, rw.bottom - rw.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER); 
		}
	}
	else
	{
		HIMAGELIST  hIL = (HIMAGELIST)SendMessageW(hwndList, LVM_GETIMAGELIST, 0, 0L);
		if (!hIL || !ImageList_GetIconSize(hIL, &w, &h)) 
		{ h += 4; w+= 4; }
 		SendMessageW(hwndList, LVM_SETICONSPACING, 0, MAKELPARAM(w + HORZ_SPACING, h  + textHeight + 6 + VERT_SPACING));
		if (!tm.tmAveCharWidth) tm.tmAveCharWidth = 2;
		itemHeight = w / tm.tmAveCharWidth;
	}
	return TRUE;
}

BOOL AlbumArtListView::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch(uMsg) {
		case WM_INITDIALOG:
		{
			HWND hwnd = GetDlgItem(hwndDlg,dlgitem);

			RECT r;
			if (hwnd)
			{
				GetWindowRect(hwnd,&r);
				MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT*)&r, 2);
			}
			else SetRect(&r, 0, 0, 1, 1);

			if (isDetailsMode(mode))
			{
				if (hwnd) DestroyWindow(hwnd);
				hwnd = CreateSmoothScrollList(hwndDlg, r.left, r.top, r.right - r.left, r.bottom - r.top, dlgitem);
				SendMessage(hwnd,WM_USER+6,0,0);
				ShowWindow(hwnd, SW_SHOWNORMAL);
			}
			else
			{
				if (hwnd) DestroyWindow(hwnd);
				hwnd = CreateHeaderIconList(hwndDlg, r.left, r.top, r.right - r.left, r.bottom - r.top, dlgitem);
				ShowWindow(hwnd,SW_SHOWNORMAL);
				int w=0,h=0;
				getImgSize(mode,w,h);
				HIMAGELIST il = ImageList_Create(w + 4,h + 4,ILC_COLOR24,0,1); // add borders
				ListView_SetImageList(hwnd,il,LVSIL_NORMAL);
			}
		}
		break;

		case WM_DISPLAYCHANGE:
		{
			if (hbmpNames) DeleteObject(hbmpNames);
			hbmpNames = NULL;
			ratingrow = -1;
			CalcuateItemHeight();

			int rw,rh;
			ARGB32 * bmp = loadRrc(IDR_IMAGE_NOTFOUND,"PNG",&rw,&rh,true);
			classicnotfoundW = rw;
			classicnotfoundH = rh;
			INT color[] = { WADLG_ITEMFG, WADLG_SELBAR_FGCOLOR, WADLG_INACT_SELBAR_FGCOLOR, };

			for (int i = sizeof(classicnotfound)/sizeof(classicnotfound[0]) -1; i > -1; i--)
			{
				if(classicnotfound[i])	WASABI_API_MEMMGR->sysFree(classicnotfound[i]);
				classicnotfound[i] = NULL;

				if(bmp) 
				{
					if (0 != i) 
					{
						classicnotfound[i] = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(sizeof(ARGB32)*rw*rh);
						CopyMemory(classicnotfound[i], bmp, sizeof(ARGB32)*rw*rh);
					}
					else classicnotfound[i] = bmp;
					adjustbmp(classicnotfound[i],rw*rh, GetWAColor(color[i]));
				}
			}

			if(uMsg == WM_DISPLAYCHANGE) PostMessageW(GetDlgItem(hwndDlg,dlgitem),uMsg,wParam,lParam);
		}
		break;

		case WM_DESTROY:
			for (int i = sizeof(classicnotfound)/sizeof(classicnotfound[0]) -1; i > -1; i--)
			{
				if(classicnotfound[i])	WASABI_API_MEMMGR->sysFree(classicnotfound[i]);
				classicnotfound[i] = NULL;
			}
		break;

		case WM_MEASUREITEM:
			if (wParam == (WPARAM)dlgitem)
			{
				((MEASUREITEMSTRUCT*)lParam)->itemHeight = itemHeight;
				return TRUE;
			}
			break;
		case WM_NOTIFY:
			if (wParam == (WPARAM)dlgitem)
			{
				BOOL bProcessed(FALSE);
				LRESULT result(0);
				switch (((NMHDR*)lParam)->code)
				{
					case LVN_KEYDOWN: bProcessed = OnKeyDown((NMLVKEYDOWN*)lParam); break;
					case NM_CUSTOMDRAW:	
						bProcessed = (isDetailsMode(mode)) ? OnCustomDrawDetails((NMLVCUSTOMDRAW*)lParam, &result) : OnCustomDrawIcon((NMLVCUSTOMDRAW*)lParam, &result);
						break;
					case LVN_GETDISPINFOW:
						return TRUE;
					case LVN_GETINFOTIPW:
					{
						wchar_t buf[256]=L"";
						contents->GetCellText(((NMLVGETINFOTIPW*)lParam)->iItem,1,buf,256);
						const wchar_t *p = buf;
						if (p && *p && lstrlenW(p) > itemHeight)  // we use itemHeight to write  number of average characters that fits label in icon mode
						{
							StringCchCopyW(((NMLVGETINFOTIPW*)lParam)->pszText, ((NMLVGETINFOTIPW*)lParam)->cchTextMax, p);
						}
					}
					return TRUE;
				}

				if (bProcessed) SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)result);
				return bProcessed;
			}
			{
				LPNMHDR l=(LPNMHDR)lParam;
				if(l->code == NM_RCLICK && l->hwndFrom == ListView_GetHeader(listview.getwnd())) {
					extern HMENU m_context_menus;
					HMENU menu = GetSubMenu(m_context_menus, 10);
					POINT p;
					GetCursorPos(&p);
					int checked=0;
					switch(mode) {
						case 0: checked=ID_ARTHEADERMENU_SMALLDETAILS; break;
						case 1: checked=ID_ARTHEADERMENU_MEDIUMDETAILS; break;
						case 2: checked=ID_ARTHEADERMENU_LARGEDETAILS; break;
						case 3: checked=ID_ARTHEADERMENU_SMALLICON; break;
						case 4: checked=ID_ARTHEADERMENU_MEDIUMICON; break;
						case 5: checked=ID_ARTHEADERMENU_LARGEICON; break;
					}
					CheckMenuItem(menu,checked,MF_CHECKED | MF_BYCOMMAND);
					int r = Menu_TrackSkinnedPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, p.x, p.y, l->hwndFrom, NULL);
					CheckMenuItem(menu,checked,MF_UNCHECKED | MF_BYCOMMAND);
					if(!r) return TRUE;
					ProcessMenuResult(r,false,0,NULL,hwndDlg);
					return TRUE;
				} else if(l->code == LVN_GETINFOTIP && l->idFrom == dlgitem) {
					NMLVGETINFOTIP *n = (NMLVGETINFOTIP*)l;
					wchar_t artist[100] = {0}, album[100] = {0}, tracks[10] = {0}, year[10] = {0};
					contents->GetCellText(n->iItem,0,artist,100);
					contents->GetCellText(n->iItem,1,album,100);
					contents->GetCellText(n->iItem,2,tracks,10);
					contents->GetCellText(n->iItem,3,year,10);
					wchar_t trackstr[100] = {0};
					WASABI_API_LNGSTRINGW_BUF(IDS_TRACKS,trackstr,100);
					CharLower(trackstr);
					if(year[0]) StringCchPrintf(n->pszText,n->cchTextMax,L"%s - %s (%s): %s %s",artist,album,year,tracks,trackstr);
					else StringCchPrintf(n->pszText,n->cchTextMax,L"%s - %s: %s %s",artist,album,tracks,trackstr);
				}
			}
		break;

		case LVM_REDRAWITEMS:
		{
			RECT ri;
			HWND hwndList = GetDlgItem(hwndDlg, dlgitem);
			if (hwndList)
			{
				HWND hwndRealList = (HWND)SendMessageW(hwndList, WM_EX_GETREALLIST, 0, 0L);
				if (hwndRealList)
				{
					int w, h;
					getImgSize(mode, w, h);	
					if (isDetailsMode(mode))
					{
						for(int i = (INT)wParam; i <= (INT)lParam; i++)
						{
							ri.left = LVIR_BOUNDS;
							if (SendMessageW(hwndRealList, LVM_GETITEMRECT, i, (LPARAM)&ri))
							{
								ri.left += 6;
								ri.top += 3;
								ri.right  = ri.left + w;
								ri.bottom = ri.top + h;
								InvalidateRect(hwndRealList, &ri, FALSE);
							}
						}
					}
					else
					{
						for(int i = (INT)wParam; i <= (INT)lParam; i++)
						{
							ri.left = LVIR_ICON;
							if (SendMessageW(hwndRealList, LVM_GETITEMRECT, i, (LPARAM)&ri))
							{
								ri.left += 2;
								ri.top += 2;
								ri.right  = ri.left + w;
								ri.bottom = ri.top + h;
								InvalidateRect(hwndRealList, &ri, FALSE);
							}
						}
					}
				}
				else SendMessageW(hwndList, LVM_REDRAWITEMS, wParam, lParam);
			}
		}
		return TRUE;
	}
	return SkinnedListView::DialogProc(hwndDlg,uMsg,wParam,lParam);
}

HMENU AlbumArtListView::GetMenu(bool isFilter, int filterNum, C_Config *c, HMENU themenu) {
	HMENU menu = GetSubMenu(themenu, 10);
	
	int checked=0;
	switch(mode) {
		case 0: checked=ID_ARTHEADERMENU_SMALLDETAILS; break;
		case 1: checked=ID_ARTHEADERMENU_MEDIUMDETAILS; break;
		case 2: checked=ID_ARTHEADERMENU_LARGEDETAILS; break;
		case 3: checked=ID_ARTHEADERMENU_SMALLICON; break;
		case 4: checked=ID_ARTHEADERMENU_MEDIUMICON; break;
		case 5: checked=ID_ARTHEADERMENU_LARGEICON; break;
	}
	CheckMenuItem(menu,checked,MF_CHECKED | MF_BYCOMMAND);

	if(isFilter) {
		MENUITEMINFO m={sizeof(m),MIIM_ID,0};
		int i=0;
		while(GetMenuItemInfo(menu,i,TRUE,&m)) {
			m.wID |= (1+filterNum) << 16;
			SetMenuItemInfo(menu,i,TRUE,&m);
			i++;
		}
	}
	return menu;
}

void AlbumArtListView::ProcessMenuResult(int r, bool isFilter, int filterNum, C_Config *c, HWND parent) {
	int mid = (r >> 16);
	if(!isFilter && mid) return;
	if(isFilter && mid-1 != filterNum) return;
	r &= 0xFFFF;

	switch(r) {
		case ID_ARTHEADERMENU_SMALLDETAILS:  mode=0; break;
		case ID_ARTHEADERMENU_MEDIUMDETAILS: mode=1; break;
		case ID_ARTHEADERMENU_LARGEDETAILS:  mode=2; break;
		case ID_ARTHEADERMENU_SMALLICON:     mode=3; break;
		case ID_ARTHEADERMENU_MEDIUMICON:    mode=4; break;
		case ID_ARTHEADERMENU_LARGEICON:     mode=5; break;
		default: return;
	}

	contents->config->WriteInt(L"albumartviewmode",mode);
	contents->SetMode(mode);
	while (ListView_DeleteColumn(listview.getwnd(), 0));
	DialogProc(parent,WM_INITDIALOG,0,0);
	HWND hwndList = GetDlgItem(parent,dlgitem);
	if (hwndList)
	{
		MLSkinnedWnd_SkinChanged(hwndList, TRUE, TRUE);
		ListView_SetItemCount(hwndList, contents->GetNumRows());
		CalcuateItemHeight();
	}
}