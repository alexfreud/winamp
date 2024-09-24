#include "main.h"
#include "api__ml_local.h"
#include "../nu/sort.h"
#include "AlbumArtFilter.h"
#include "resource.h"
#include "../nu/AutoUrl.h"
#include <shlwapi.h>
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include "AlbumArtContainer.h"
#include <tataki/canvas/canvas.h>
#include <tataki/bitmap/autobitmap.h>
#include <api/service/waServiceFactory.h>
#include <api/service/svcs/svc_imgload.h>

static size_t m_sort_by, m_sort_dir, m_sort_which;

void emptyQueryListObject(queryListObject *obj);
int reallocQueryListObject(queryListObject *obj);
void freeQueryListObject(queryListObject *obj);
extern C_Config *g_config;
int config_use_alternate_colors=1;

#define WM_EX_GETREALLIST		(WM_USER + 0x01)
#define WM_EX_GETCOUNTPERPAGE	(WM_USER + 0x04)
#define LVN_EX_SIZECHANGED		(LVN_LAST)

AlbumArtFilter::AlbumArtFilter(HWND hwndDlg, int dlgitem, C_Config *c) : hwndDlg(hwndDlg), dlgitem(dlgitem),
		notfound(L"winamp.cover.notfound"), notfound60(L"winamp.cover.notfound.60"), notfound90(L"winamp.cover.notfound.90"),
		hbmpNames(NULL), ratingHotItem((DWORD)-1), bgBrush(NULL)
{
	mode = c->ReadInt(L"albumartviewmode",1);
	icons_only = c->ReadInt(L"albumarticonmode",0);
	ZeroMemory(classicnotfound, sizeof(classicnotfound));
	DialogProc(hwndDlg,WM_INITDIALOG,0,0);
}

AlbumArtFilter::~AlbumArtFilter()
{
	if (hbmpNames) DeleteObject(hbmpNames);
	if (bgBrush) DeleteBrush(bgBrush);
}

static COLORREF GetWAColor(INT index)
{
	static int (*wad_getColor)(int idx) = NULL;
	if (!wad_getColor) *(void **)&wad_getColor=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,1,ML_IPC_SKIN_WADLG_GETFUNC);
	return (wad_getColor) ? wad_getColor(index) : 0xFF00FF;
}

static void getImgSize(int mode, int &w, int &h)
{
	switch (mode)
	{
		case 0: w=h=60; break;
		case 1: w=h=90; break;
		case 2: w=h=120; break;
		case 3: w=h=60; break;
		case 4: w=h=90; break;
		case 5: w=h=120; break;
		case 6: w=h=180; break;
	}
}

static bool isDetailsMode(int mode)
{
	return mode == 0 || mode == 1 || mode == 2;
}

void DrawRect(HDC dc, int x, int y, int w, int h)
{
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

static int getStrExtent(HDC dc, const wchar_t * s)
{
	int ret=0;
	while (s && *s)
	{
		int f;
		GetCharWidth32(dc,*s,*s,&f);
		s++;
		ret+=f;
	}
	return int(ret);
}

ARGB32 * loadImg(const void * data, int len, int *w, int *h, bool ldata=false)
{
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = plugin.service->service_getNumServices(imgload);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = plugin.service->service_enumService(imgload,i);
		if (sf)
		{
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if (l)
			{
				if (l->testData(data,len))
				{
					ARGB32* ret;
					if (ldata) ret = l->loadImageData(data,len,w,h);
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
	HGLOBAL resourceHandle = WASABI_API_LOADRESFROMFILEA((LPCSTR)sec, MAKEINTRESOURCEA(id), &size);
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

void AlbumArtFilter::drawArt(AlbumArtContainer *art, DCCanvas *pCanvas, RECT *prcDst, int itemid, int imageIndex)
{
	int res = AlbumArtContainer::DRAW_NOART;
	if (art)
	{
		art->updateMsg.hwnd = hwndRealList;
		art->updateMsg.message = LVM_REDRAWITEMS;
		art->updateMsg.lParam = itemid;
		art->updateMsg.wParam = itemid;
		res = art->drawArt(pCanvas, prcDst);
	}

	if (res != AlbumArtContainer::DRAW_SUCCESS)
	{
		SkinBitmap *noart;

		// Martin> This will just work for Modern Skins, but looks definitly better
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
				int w = prcDst->right - prcDst->left;
				int h = prcDst->bottom - prcDst->top;
				DrawRect(pCanvas->getHDC(), prcDst->left, prcDst->top, w, h);
				wchar_t str1[32] = {0}, str2[32] = {0};
				WASABI_API_LNGSTRINGW_BUF(IDS_NO_IMAGE,str1,32);
				WASABI_API_LNGSTRINGW_BUF(IDS_AVAILABLE,str2,32);
				ExtTextOutW(pCanvas->getHDC(), w/2 - 22 + prcDst->left, w/2 - 14 + prcDst->top, 0,NULL,str1,wcslen(str1),0);
				ExtTextOutW(pCanvas->getHDC(), w/2 - 22 + prcDst->left, w/2 + 1 + prcDst->top, 0,NULL,str2,wcslen(str2),0);
			}
		}
		else
			noart->stretchToRectAlpha(pCanvas, prcDst);
	}
}

// icon view
BOOL AlbumArtFilter::DrawItemIcon(NMLVCUSTOMDRAW *plvcd, DCCanvas *pCanvas, UINT itemState, RECT *prcClip, BOOL bWndActive)
{
	HDC hdc = plvcd->nmcd.hdc;
	RECT ri, re, rcText;
	int w=0, h=0, imageIndex=0;
	getImgSize(mode,w,h);

	SetBkColor(hdc, plvcd->clrTextBk);
	SetTextColor(hdc, plvcd->clrText);
	SetRect(&rcText, plvcd->nmcd.rc.left, (plvcd->nmcd.rc.bottom - textHeight),
			plvcd->nmcd.rc.right, plvcd->nmcd.rc.bottom - (icons_only ? textHeight : 4));

	// background
	SetRect(&re, plvcd->nmcd.rc.left, plvcd->nmcd.rc.top, plvcd->nmcd.rc.right, (icons_only ? (plvcd->nmcd.rc.bottom - textHeight - 5) : rcText.top));
	
	if (IntersectRect(&ri, &re, prcClip)) 
		ExtTextOutW(hdc,0, 0, ETO_OPAQUE, &ri, L"", 0, 0);

	if (LVIS_SELECTED & itemState) 
		imageIndex = (bWndActive) ? 1 : 2;

	INT of = ((plvcd->nmcd.rc.right - plvcd->nmcd.rc.left) - w)/2;
	SetRect(&re, plvcd->nmcd.rc.left + of, plvcd->nmcd.rc.top + 2, plvcd->nmcd.rc.left + of + w, plvcd->nmcd.rc.top + 2 + h);
	if (IntersectRect(&ri, &re, prcClip))
	{
		AlbumArtContainer *art = GetArt(plvcd->nmcd.dwItemSpec+1);
		drawArt(art, pCanvas, &re, plvcd->nmcd.dwItemSpec, imageIndex);
	}

	if (IntersectRect(&ri, &rcText, prcClip))
	{
		ExtTextOutW(hdc,0, 0, ETO_OPAQUE, &ri, L"", 0, 0);

		const wchar_t *p = GetText(plvcd->nmcd.dwItemSpec);
		if (p && *p)
		{
			SetRect(&ri, rcText.left + of, rcText.top - (textHeight/2) - 1, rcText.right - of, rcText.top + textHeight);
			DrawTextW(hdc, p, -1, &ri, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
		}
	}

	if ((LVIS_FOCUSED & itemState) && bWndActive)
	{
		if (hwndRealList && 0 == (0x01/*UISF_HIDEFOCUS*/ & SendMessageW(hwndRealList, 0x0129/*WM_QUERYUISTATE*/, 0, 0L)))
		{
			/*SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor(hdc, GetSysColor(COLOR_WINDOW));*/
			SetBkColor(hdc, GetWAColor(WADLG_ITEMBG));

			if (icons_only)
			{
				InflateRect(&re, 2, 2);
			}
			else
				plvcd->nmcd.rc.bottom -= 4;

			DrawFocusRect(hdc, (icons_only ? &re : &plvcd->nmcd.rc));
		}
	}

	return TRUE;
}

//detail view
BOOL AlbumArtFilter::PrepareDetails(HDC hdc)
{
	INT width(0), height, len;
	HFONT hFont,hFontBold, hOldFont;
	HDC hdcTmp;
	HBITMAP hbmpOld;
	LOGFONT l={0};
	RECT ri;
	wchar_t ratingstr[100] = {0}, buf[100] = {0};

	hdcTmp = CreateCompatibleDC(hdc);
	if (!hdcTmp) return FALSE;

	hFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
	GetObject(hFont, sizeof(LOGFONT), &l);
	l.lfWeight = FW_BOLD;
	hFontBold = CreateFontIndirect(&l);

	hOldFont = (HFONT)SelectObject(hdcTmp, hFontBold);

	int bypassColumnCount = 0;

	for ( ListField *l_showcolumn : showncolumns )
	{
		if ( l_showcolumn->field == ALBUMFILTER_COLUMN_LASTUPD) // let's hide last updated from details view
		{
			bypassColumnCount++;
			continue;
		}
		int of = getStrExtent(hdcTmp, l_showcolumn->name);
		if (of > width) width = of;
	}
	if (width) width += 20;

	height = (showncolumns.size()-bypassColumnCount) * textHeight;
	hbmpNames = CreateCompatibleBitmap(hdc, width * 4, height); 
	hbmpOld = (HBITMAP)SelectObject(hdcTmp, hbmpNames);
	
	SetRect(&ri, 0, 0, width, height);

	WASABI_API_LNGSTRINGW_BUF(IDS_RATING,ratingstr,100);
	INT clrText[4]  = { WADLG_ITEMFG, WADLG_SELBAR_FGCOLOR, WADLG_INACT_SELBAR_FGCOLOR, WADLG_ITEMFG2, };
	INT clrBk[4]  = { WADLG_ITEMBG, WADLG_SELBAR_BGCOLOR, WADLG_INACT_SELBAR_BGCOLOR, WADLG_ITEMBG2, };

	for (int j = 0; j < 4; j++)
	{
		SetTextColor(hdcTmp, GetWAColor(clrText[j]));
		SetBkColor(hdcTmp, GetWAColor(clrBk[j]));
		
		ExtTextOutW(hdcTmp,0, 0, ETO_OPAQUE, &ri, L"", 0, 0);
		
		for (size_t i=0, top = 0; i < showncolumns.size(); i++, top += textHeight)
		{
			if (showncolumns[i]->field == ALBUMFILTER_COLUMN_LASTUPD) // let's hide last updated from details view
			{
				top -= textHeight;
				continue;
			}
			if (-1 == ratingrow && 0 == lstrcmpW(showncolumns[i]->name, ratingstr)) ratingrow = i;
			StringCchCopyW(buf, 100, (showncolumns[i]->name) ? showncolumns[i]->name : L"");
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

BOOL AlbumArtFilter::DrawItemDetail(NMLVCUSTOMDRAW *plvcd, DCCanvas *pCanvas, UINT itemState, RECT *prcClip, BOOL bWndActive, HDC hdcNames, INT namesWidth)
{
	RECT ri, re;
	INT imageIndex = 0;
	HDC hdc = plvcd->nmcd.hdc;

	SetTextColor(hdc, plvcd->clrText);
	SetBkColor(hdc, plvcd->clrTextBk);

	// background
	SetRect(&re, plvcd->nmcd.rc.left + 4, plvcd->nmcd.rc.top, plvcd->nmcd.rc.right, plvcd->nmcd.rc.bottom - 5);
	if (IntersectRect(&ri, &re, prcClip)) ExtTextOutW(hdc,0, 0, ETO_OPAQUE, &ri, L"", 0, 0);
	if (LVIS_SELECTED & itemState) { imageIndex = (bWndActive) ? 1 : 2; }

	int w, h;
	getImgSize(mode, w, h);	
	SetRect(&re, 6+plvcd->nmcd.rc.left, 3+plvcd->nmcd.rc.top, 6+ plvcd->nmcd.rc.left + w, 3+ plvcd->nmcd.rc.top + h);
	if (IntersectRect(&ri, &re, prcClip))
	{
		AlbumArtContainer *art = GetArt(plvcd->nmcd.dwItemSpec + 1);
		drawArt(art, pCanvas, &re, plvcd->nmcd.dwItemSpec, imageIndex);
	}

	// text
	int limCY, limCX;

	//select 4th bitmap for alternate coloring
	if (g_config->ReadInt(L"alternate_items", config_use_alternate_colors) && imageIndex==0 && plvcd->nmcd.dwItemSpec%2)
		imageIndex=3;

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
		for (size_t i=0; i < showncolumns.size() && ri.top < limCY; i++, ri.top += textHeight)
		{
			if (showncolumns[i]->field == ALBUMFILTER_COLUMN_LASTUPD) // let's hide last updated from details view
			{
				ri.top -= textHeight;
				continue;
			}

			wchar_t buf[100] = {0};
			const wchar_t *p = CopyText2(plvcd->nmcd.dwItemSpec,i,buf,100);
			ri.bottom += textHeight;
			if (ri.bottom > limCY) ri.bottom = limCY;

			if ((INT)i == ratingrow)  // this is the ratings column, so draw graphical stars
			{
				int rating = wcslen(buf);
				RATINGDRAWPARAMS p = {sizeof(RATINGDRAWPARAMS),hdc,
										{ri.left, ri.top + ratingTop, ri.right,ri.bottom},
										rating,5,0,RDS_SHOWEMPTY,NULL,0};
				if(ratingHotItem == plvcd->nmcd.dwItemSpec) { p.fStyle |= RDS_HOT; p.trackingValue = ratingHotValue; }
				MLRating_Draw(plugin.hwndLibraryParent,&p);
			}
			else {
				//ri.left = re.left;
				ri.right = plvcd->nmcd.rc.right;
				DrawTextW(hdc, p, wcslen(p), &ri, DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOCLIP | DT_NOPREFIX);
			}
		}
	}

	// bottom line
	MoveToEx(hdc,plvcd->nmcd.rc.left + 4,plvcd->nmcd.rc.bottom - 3,NULL);
	LineTo(hdc,plvcd->nmcd.rc.right, plvcd->nmcd.rc.bottom - 3);

	// focus rect
	SetRect(&ri, plvcd->nmcd.rc.left + 4, plvcd->nmcd.rc.top, plvcd->nmcd.rc.right, plvcd->nmcd.rc.bottom - 5);
	if ((LVIS_FOCUSED & itemState) && bWndActive)
	{
		if (hwndRealList &&	0 == (0x01/*UISF_HIDEFOCUS*/ & SendMessageW(hwndRealList, 0x0129/*WM_QUERYUISTATE*/, 0, 0L)))
		{
			/*SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor(hdc, GetSysColor(COLOR_WINDOW));*/
			SetBkColor(hdc, GetWAColor(WADLG_ITEMBG));
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

BOOL AlbumArtFilter::OnKeyDown(NMLVKEYDOWN *plvkd)
{
	switch (plvkd->wVKey)
	{
		case 'A':
			if (GetAsyncKeyState(VK_CONTROL))
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

BOOL AlbumArtFilter::OnCustomDrawIcon(NMLVCUSTOMDRAW *plvcd, LRESULT *pResult)
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
			bActive = (GetFocus() == hwndRealList);
			activeCanvas.cloneDC(plvcd->nmcd.hdc, NULL);
			*pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
			return TRUE;

		case CDDS_ITEMPREPAINT:
			{
				UINT itemState = SendMessageW(plvcd->nmcd.hdr.hwndFrom, LVM_GETITEMSTATE, plvcd->nmcd.dwItemSpec,
											  LVIS_FOCUSED | LVIS_SELECTED | LVIS_DROPHILITED | LVIS_CUT);

				plvcd->nmcd.rc.left = LVIR_BOUNDS;
				SendMessageW(plvcd->nmcd.hdr.hwndFrom, LVM_GETITEMRECT, plvcd->nmcd.dwItemSpec, (LPARAM)&plvcd->nmcd.rc);

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

BOOL AlbumArtFilter::OnCustomDrawDetails(NMLVCUSTOMDRAW *plvcd, LRESULT *pResult)
{
	static RECT rcClip;
	static BOOL bActive;
	static HDC hdcNames;
	static HBITMAP hbmpOld;
	static HPEN penOld;
	static DCCanvas activeCanvas;

	*pResult = CDRF_DODEFAULT;
	switch(plvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
		{
			if (0 == plvcd->nmcd.rc.bottom && 0 == plvcd->nmcd.rc.right)
			{
				*pResult = CDRF_SKIPDEFAULT;
				return TRUE;
			}
			CopyRect(&rcClip, &plvcd->nmcd.rc);
			FillRect(plvcd->nmcd.hdc,&plvcd->nmcd.rc,bgBrush);

			if (!hbmpNames) PrepareDetails(plvcd->nmcd.hdc);
			if (hbmpNames)
			{
				BITMAP bi;
				GetObject(hbmpNames, sizeof(BITMAP), &bi);
				namesWidth = bi.bmWidth/4;
				hdcNames = CreateCompatibleDC(plvcd->nmcd.hdc);
				hbmpOld = (hdcNames) ? (HBITMAP)SelectObject(hdcNames, hbmpNames) : NULL; 
			}
			else
			{
				hdcNames = NULL;
				namesWidth = 0;
			}
			bActive = (GetFocus() == hwndRealList);
			
			penOld = (HPEN)SelectObject(plvcd->nmcd.hdc, CreatePen(PS_SOLID,1, GetWAColor(WADLG_HILITE)));
			activeCanvas.cloneDC(plvcd->nmcd.hdc, NULL);
			*pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
			return TRUE;
		}

		case CDDS_ITEMPREPAINT:
			{
				UINT itemState = SendMessageW(plvcd->nmcd.hdr.hwndFrom, LVM_GETITEMSTATE, plvcd->nmcd.dwItemSpec,
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
				HPEN pen = (HPEN)SelectObject(plvcd->nmcd.hdc, penOld);
				if (pen) DeleteObject(pen);
				penOld = NULL;
			}
			return TRUE;
	}
	return FALSE;
}

BOOL AlbumArtFilter::CalcuateItemHeight(void)
{
	HDC hdc;
	int w, h;
	HWND hwndList;
	TEXTMETRIC tm = {0};

	getImgSize(mode, w, h);
	
	hwndList = GetDlgItem(hwndDlg, dlgitem);

	textHeight = 0;

	hdc = GetDC(hwndDlg);
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

		int bypassColumnCount = 0;
		for ( ListField *l_showcolumn : showncolumns )
		{
			if ( l_showcolumn->field == ALBUMFILTER_COLUMN_LASTUPD) // let's hide last updated from details view
			{
				bypassColumnCount++;
				continue;
			}
		}

		int newHeight = max(h, textHeight * ((INT)showncolumns.size() - bypassColumnCount)) + 12;
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
		if (!hIL || !ImageList_GetIconSize(hIL, &w, &h)) { h += 4; w+= 4; }
		SendMessageW(hwndList, LVM_SETICONSPACING, 0, MAKELPARAM(w, h + (icons_only ? 0 : textHeight)));
		if (!tm.tmAveCharWidth) tm.tmAveCharWidth = 2;
		itemHeight = w / tm.tmAveCharWidth;
	}
	return TRUE;
}

INT_PTR AlbumArtFilter::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			HWND hwnd = GetDlgItem(hwndDlg,dlgitem);
			RECT r;
			if (hwnd)
			{
				GetWindowRect(hwnd,&r);
				MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT*)&r, 2);
				DestroyWindow(hwnd);
			}
			else SetRect(&r, 0, 0, 1, 1);

			if (isDetailsMode(mode))
			{			
				hwnd = CreateSmoothScrollList(hwndDlg, r.left, r.top, r.right - r.left, r.bottom - r.top, dlgitem);
			}
			else
			{
				hwnd = CreateHeaderIconList(hwndDlg, r.left, r.top, r.right - r.left, r.bottom - r.top, dlgitem);
				int w=0,h=0;
				getImgSize(mode,w,h);
				HIMAGELIST il = ImageList_Create(w + 4,h + 4,ILC_COLOR24,0,1); // add borders
				ListView_SetImageList(hwnd,il,LVSIL_NORMAL);
			}

			hwndRealList = (HWND)SendMessageW(hwnd, WM_EX_GETREALLIST, 0, 0L);

			MLSKINWINDOW skin = {0};
			skin.hwndToSkin = hwndDlg;
			skin.skinType   = SKINNEDWND_TYPE_AUTO;
			skin.style      = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER;
			MLSkinWindow(plugin.hwndLibraryParent, &skin);

			if (isDetailsMode(mode) && NULL != hwndRealList)
				MLSkinnedWnd_SetStyle(hwndRealList, MLSkinnedWnd_GetStyle(hwndRealList) | SWLVS_ALTERNATEITEMS);
		}

		case WM_DISPLAYCHANGE:
		{
			if (hbmpNames)
				DeleteObject(hbmpNames);

			hbmpNames = NULL;
			ratingrow = -1;

			if(bgBrush)
				DeleteBrush(bgBrush);

			bgBrush = CreateSolidBrush(GetWAColor(WADLG_ITEMBG));
			CalcuateItemHeight();

			//TODO
			//config_use_alternate_colors = g_config->ReadInt("alternate_items", 1);
			
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
					else
						classicnotfound[i] = bmp;

					adjustbmp(classicnotfound[i],rw*rh, GetWAColor(color[i]));
				}
			}

			if(WM_DISPLAYCHANGE == uMsg)
				PostMessageW(GetDlgItem(hwndDlg,dlgitem),uMsg,wParam,lParam);
			else
				ShowWindow(GetDlgItem(hwndDlg,dlgitem), SW_SHOWNORMAL);
		}
			break;

		case WM_DESTROY:
			for (int i = sizeof(classicnotfound)/sizeof(classicnotfound[0]) -1; i > -1; i--)
			{
				if(classicnotfound[i])
					WASABI_API_MEMMGR->sysFree(classicnotfound[i]);

				classicnotfound[i] = NULL;
			}
			break;

		case WM_MEASUREITEM:
			if (wParam == (WPARAM)dlgitem)
			{
				((MEASUREITEMSTRUCT*)lParam)->itemHeight = itemHeight;
				SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, TRUE);
				return TRUE;
			}
			break;

		case WM_USER+600:
			AppendMenuW((HMENU)wParam,MF_SEPARATOR,lParam++,0);
			// disabled 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
			//AppendMenuW((HMENU)wParam,MF_STRING,lParam++,WASABI_API_LNGSTRINGW(IDS_GET_ALBUM_ART));
			AppendMenuW((HMENU)wParam,MF_STRING,lParam++,WASABI_API_LNGSTRINGW(IDS_REFRESH_ALBUM_ART));
			AppendMenuW((HMENU)wParam,MF_STRING,lParam++,WASABI_API_LNGSTRINGW(IDS_OPEN_FOLDER));
			AppendMenuW((HMENU)wParam,MF_STRING,lParam++,WASABI_API_LNGSTRINGW(IDS_MORE_ARTIST_INFO)); 
			break;

		case WM_USER+601:
			RemoveMenu((HMENU)wParam,lParam++,MF_BYCOMMAND);
			// disabled 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
			//RemoveMenu((HMENU)wParam,lParam++,MF_BYCOMMAND);
			RemoveMenu((HMENU)wParam,lParam++,MF_BYCOMMAND);
			RemoveMenu((HMENU)wParam,lParam++,MF_BYCOMMAND);
			RemoveMenu((HMENU)wParam,lParam++,MF_BYCOMMAND);
			break;

		case WM_USER+602:
			// disabled 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
			#if 0
			if (lParam == 1)
			{
				int c = list->GetCount();
				for (int x = 0; x < c; x ++)
				{
					if(list->GetSelected(x) && albumList.Items[x+1].art)
					{
						artFetchData d = {sizeof(d),hwndDlg,albumList.Items[x+1].artist,albumList.Items[x+1].name,0};
						d.gracenoteFileId = albumList.Items[x+1].gracenoteFileId;
						d.showCancelAll = 1;
						int r = (int)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(LPARAM)&d,IPC_FETCH_ALBUMART);
						if(r == -2) break; // cancel all was pressed
						if(r == 0 && d.imgData && d.imgDataLen) // success, save art in correct location
						{
							AGAVE_API_ALBUMART->SetAlbumArt(albumList.Items[x+1].art->filename,L"cover",0,0,d.imgData,d.imgDataLen,d.type);
							WASABI_API_MEMMGR->sysFree(d.imgData);
							// clear the cache...
							ClearCache(albumList.Items[x+1].art->filename);
							albumList.Items[x+1].art->updateMsg.hwnd=0;
							albumList.Items[x+1].art->Reset();
							SendMessage(GetDlgItem(hwndDlg,dlgitem),LVM_REDRAWITEMS,x,x);
						}
					}
				}		
			}
			else
			#endif
			if(lParam == 1)
			{
				int c = list->GetCount();
				for (int x = 0; x < c; x ++)
				{
					if(list->GetSelected(x) && albumList.Items[x+1].art)
					{
						ClearCache(albumList.Items[x+1].art->filename);
						albumList.Items[x+1].art->updateMsg.hwnd=0;
						albumList.Items[x+1].art->Reset();
						SendMessage(GetDlgItem(hwndDlg,dlgitem),LVM_REDRAWITEMS,x,x);
					}
				}
			}
			else if(lParam == 2)
			{
				int opened=0;
				int c = list->GetCount();
				for (int x = 0; x < c; x ++)
				{
					if((ListView_GetItemState(list->getwnd(), x, LVIS_FOCUSED)&LVIS_FOCUSED) && albumList.Items[x+1].art)
					{
						// TODO change to use the explorer api
						if(opened++ >= 10) break; // that's enough! Opening 400 exploerer windows may seem like fun, but windows _hates_ it.
						wchar_t fn[MAX_PATH] = {0};
						lstrcpynW(fn,albumList.Items[x+1].art->filename,MAX_PATH);
						PathRemoveFileSpecW(fn);
						ShellExecuteW(NULL,L"open",fn,NULL,NULL,SW_SHOW);
					}
				}
			}
			else if (lParam == 3)
			{
				int sel = list->GetNextSelected();
				if (sel != LB_ERR)
				{
					char url[1024] = {0};
					StringCchPrintfA(url, 1024, "http://client.winamp.com/nowplaying/artist?artistName=%s&icid=localmediagetartistinfo", AutoUrl(albumList.Items[sel+1].artist));
					SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(LPARAM)url,IPC_OPEN_URL);
				}
			}
			break;

		case WM_USER+700: // hit-info
			{
				DWORD hotold = ratingHotItem;
				int hotoldval = ratingHotValue;
				ratingHotItem = (DWORD)-1;

				typedef struct {
					int x,y,item;
					HWND hwnd;
					UINT msg;
				} hitinfo;

				hitinfo * info = (hitinfo *)wParam;
				if(mode && info->hwnd == hwndRealList && info->item >= 0 && info->item < albumList.Size)
				{
					int w = 0, h = 0;
					getImgSize(mode,w,h);
					RATINGHITTESTPARAMS ratingHitTest = {sizeof(ratingHitTest),{info->x,info->y},{16+w+namesWidth,3+textHeight*ratingrow,0,0},5,RDS_NORMAL,NULL,-1,(UINT)-1};
					ratingHitTest.rc.bottom = ratingHitTest.rc.top + textHeight;
					ratingHitTest.rc.right = ratingHitTest.rc.left + 200;
					
					LONG hitVal = MLRating_HitTest(plugin.hwndLibraryParent,&ratingHitTest);
					
					ratingHitTest.rc.left -= 10; //this 10px is the area in which you can click to set a zero rating (see the PtInRect call below)
					
					if(!hitVal && !PtInRect(&ratingHitTest.rc, ratingHitTest.pt))
					{
						if(hotold >=0) { ReleaseCapture(); ListView_RedrawItems(hwndRealList,hotold,hotold); }
						break;
					}

					if(info->msg == WM_MOUSEMOVE)
					{
						ratingHotValue = hitVal;
						ratingHotItem = info->item;
						if(hotold != ratingHotItem || hotoldval != ratingHotValue)
						{
							ListView_RedrawItems(hwndRealList,info->item,info->item);
							if(hotold >=0 && hotold != ratingHotItem)
								ListView_RedrawItems(hwndRealList,hotold,hotold);

							SetCapture(hwndRealList);
						}
					}
					else if(info->msg == WM_LBUTTONDOWN)
					{
						/*
						This is a slight race condition. We are part of the WM_MOUSEMOVE message in the listview. Selection state hasn't changed yet.
						By doing a PostMessage, we're betting that by the time that the message gets processed, selections state HAS changed.
						Haven't managed to make it misbehave yet though.
						(Passing the this pointer is not dangerous, because we check it on the other side :)
						*/
						PostMessage(hwndDlg,WM_USER+710,hitVal,(LPARAM)this);
						ReleaseCapture();
					}
					else if(hotold >=0) { ReleaseCapture(); ListView_RedrawItems(hwndRealList,hotold,hotold); }
				}
				else if(hotold >=0) { ReleaseCapture(); ListView_RedrawItems(hwndRealList,hotold,hotold); }
			}
			break;

		case WM_NOTIFY:
			if (wParam == (WPARAM)dlgitem)
			{
				BOOL bProcessed(FALSE);
				LRESULT result(0);
				switch (((NMHDR*)lParam)->code)
				{
					case LVN_KEYDOWN:	bProcessed = OnKeyDown((NMLVKEYDOWN*)lParam); break;
					case NM_CUSTOMDRAW:	
						bProcessed = (isDetailsMode(mode)) ? OnCustomDrawDetails((NMLVCUSTOMDRAW*)lParam, &result) : OnCustomDrawIcon((NMLVCUSTOMDRAW*)lParam, &result);
						break;
					case LVN_GETDISPINFOW: 
						return TRUE;
					case LVN_GETINFOTIPW:
					{
						const wchar_t *p = GetText(((NMLVGETINFOTIPW*)lParam)->iItem);
						if (p && *p && lstrlenW(p) > itemHeight)  // we use itemHeight to write number of average characters that fits label in icon mode
						{
							StringCchCopyW(((NMLVGETINFOTIPW*)lParam)->pszText, ((NMLVGETINFOTIPW*)lParam)->cchTextMax, p);
						}
						return TRUE;
					}
					case LVN_EX_SIZECHANGED:
					{
						INT hint = (INT)SendMessageW(((NMHDR*)lParam)->hwndFrom, WM_EX_GETCOUNTPERPAGE, 0, 0L);
						if (hint > 0) HintCacheSize(hint);
					}
						return TRUE;
				}

				if (bProcessed) SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)result);
				return bProcessed;
			}
			break;
	}
	return 0;
}

static int getWidth(int def, int col)
{
	wchar_t buf[100] = {0};
	char * name = AlbumFilter::getColConfig(col);
	StringCchPrintfW(buf, ARRAYSIZE(buf), L"av_art_col_%hs", name);
	return g_view_metaconf->ReadInt(buf, def);
}

void AlbumArtFilter::AddColumns2()
{
	showncolumns.push_back(new ListField(7,getWidth(90,7),IDS_ARTIST,g_view_metaconf,"",false,false,false,0));
	showncolumns.push_back(new ListField(0,getWidth(90,0),IDS_ALBUM,g_view_metaconf,"",false,false,false,1));
	showncolumns.push_back(new ListField(3,getWidth(50,3),IDS_TRACKS_MENU,g_view_metaconf,"",false,false,false,2));
	showncolumns.push_back(new ListField(1,getWidth(50,1),IDS_YEAR,g_view_metaconf,"",false,false,false,3));
	if (mode != 0 && mode != 3)
	{
		showncolumns.push_back(new ListField(8,getWidth(50,8),IDS_GENRE,g_view_metaconf,"",false,false,false,4));
		showncolumns.push_back(new ListField(9,getWidth(50,9),IDS_RATING,g_view_metaconf,"",false,false,false,5));
		if (mode != 1 && mode != 4)
		{
			showncolumns.push_back(new ListField(6,getWidth(50,6),IDS_LENGTH,g_view_metaconf,"",false,false,false,6));
			showncolumns.push_back(new ListField(5,getWidth(50,5),IDS_SIZE,g_view_metaconf,"",false,false,false,7));
		}
	}
	showncolumns.push_back(new ListField(10,getWidth(90,10),IDS_LAST_UPDATED,g_view_metaconf,"",false,false,false,8));
}

static const wchar_t* getArtist(const itemRecordW *p)
{
	if (p->albumartist) return p->albumartist;
	else if (p->artist) return p->artist;
	return L"";
}

static wchar_t* retainArtist(itemRecordW *p)
{
	if (p->albumartist) 
	{
		ndestring_retain(p->albumartist);
		return p->albumartist;
	}
	else if (p->artist) 
	{
		ndestring_retain(p->artist);
		return p->artist;
	}
	return emptyQueryListString;
}

int AlbumArtFilter::MyBuildSortFunc(const void *elem1, const void *elem2, const void *context)
{
	AlbumArtFilter *sortFilter = (AlbumArtFilter *)context;
	itemRecordW *a = (itemRecordW *)elem1;
	itemRecordW *b = (itemRecordW *)elem2;
	int v=WCSCMP_NULLOK(getArtist(a),getArtist(b));
	if (v) 
		return v;
	v=WCSCMP_NULLOK(a->album,b->album);
	if (v) 
		return v;
	if (sortFilter->nextFilter)
	{
		wchar_t ab[100] = {0}, bb[100] = {0};
		v = WCSCMP_NULLOK(sortFilter->nextFilter->GroupText(a,ab,100),sortFilter->nextFilter->GroupText(b,bb,100));
		if (v) 
			return v;
	}
	v = a->track - b->track;
	if (v)
		return v;
	return WCSCMP_NULLOK(a->filename,b->filename);
}

void AlbumArtFilter::Fill(itemRecordW *items, int numitems, int *killswitch, int numitems2)
{
	if (numitems > 1)
		qsort_itemRecord(items,numitems,this, MyBuildSortFunc);

	if (killswitch && *killswitch) return;

	emptyQueryListObject(&albumList);
	reallocQueryListObject(&albumList);

	itemRecordW *p = items;
	int n = numitems;
	int numalbumstotal = 0;
	wchar_t *lastartist = 0;
	wchar_t *lastalbum = 0;
	const wchar_t *lastalb=0;
	wchar_t albbuf[100] = {0}, albbuf2[100] = {0};
	ZeroMemory(&albumList.Items[0],sizeof(queryListItem));
	int isbl = 0;
	while (n--)
	{
		if (killswitch && *killswitch) return;
		if ((!lastartist || WCSCMP_NULLOK(lastartist, getArtist(p))) || (!lastalbum || WCSCMP_NULLOK(lastalbum, p->album)))
		{
			albumList.Size++;
			if (reallocQueryListObject(&albumList)) break;
			wchar_t *albumGain = p->replaygain_album_gain;
			ndestring_retain(albumGain);
			wchar_t *gracenoteFileId = getRecordExtendedItem_fast(p, extended_fields.GracenoteFileID);
			ZeroMemory(&albumList.Items[albumList.Size],sizeof(queryListItem));
			albumList.Items[albumList.Size].albumGain = albumGain;
			albumList.Items[albumList.Size].gracenoteFileId = gracenoteFileId;
			ndestring_retain(gracenoteFileId);

			lastartist = albumList.Items[albumList.Size].artist = retainArtist(p);
			if (p->album)
			{
				ndestring_retain(p->album);
				lastalbum = albumList.Items[albumList.Size].name = p->album;
			}
			else
				lastalbum = albumList.Items[albumList.Size].name = emptyQueryListString;

			lastalb=0;
			SKIP_THE_AND_WHITESPACEW(lastalbum) // optimization technique
			SKIP_THE_AND_WHITESPACEW(lastartist) // optimization technique

			albumList.Items[albumList.Size].art = new AlbumArtContainer();
			ndestring_retain(p->filename);
			albumList.Items[albumList.Size].art->filename = p->filename;

			if (*lastalbum) numalbumstotal++;
		}
		if (p->year>0)
		{
			int y = albumList.Items[albumList.Size].ifields[2];
			if (y == 0) y = MAKELONG((short)p->year,(short)p->year);
			else if (p->year > (short)LOWORD(y)) y = MAKELONG((short)p->year,(short)HIWORD(y));
			else if (p->year < (short)HIWORD(y)) y = MAKELONG((short)LOWORD(y),(short)p->year);
			albumList.Items[albumList.Size].ifields[2] = y;
		}

		if (!albumList.Items[albumList.Size].genre && p->genre) 
		{
			wchar_t *genre = p->genre;
			ndestring_retain(genre);
			albumList.Items[albumList.Size].genre = genre;
		}

		if (p->rating > 0) albumList.Items[albumList.Size].rating += p->rating;

		if (p->lastupd > albumList.Items[albumList.Size].lastupd) albumList.Items[albumList.Size].lastupd = p->lastupd;

		if (!p->album || !*p->album) isbl++;
		if (albumList.Size)
		{
			albumList.Items[albumList.Size].ifields[1]++;
			if (p->length>0) albumList.Items[albumList.Size].length += p->length;
			if (p->filesize>0) albumList.Items[albumList.Size].size += p->filesize;
		}
		if (nextFilter && (!lastalb ||  WCSCMP_NULLOK(lastalb,nextFilter->GroupText(p,albbuf2,100))))
		{
			lastalb = nextFilter->GroupText(p,albbuf,100);
			if (lastalb && *lastalb) albumList.Items[albumList.Size].ifields[0]++;
			if (lastalb) SKIP_THE_AND_WHITESPACEW(lastalb) // optimization technique
			}
		p++;
	}
	wchar_t buf[64] = {0}, sStr[16] = {0}, langBuf[64] = {0};
	if (isbl)
	{
		wsprintfW(buf, WASABI_API_LNGSTRINGW_BUF(IDS_ALL_X_ALBUMS_X_WITHOUT_ALBUM, langBuf, 64), albumList.Size - 1, WASABI_API_LNGSTRINGW_BUF(albumList.Size == 2 ? IDS_ALBUM : IDS_ALBUMS,sStr,16), isbl);
	}
	else
	{
		wsprintfW(buf, WASABI_API_LNGSTRINGW_BUF(IDS_ALL_X_ALBUMS, langBuf, 64), albumList.Size, WASABI_API_LNGSTRINGW_BUF(albumList.Size == 1 ? IDS_ALBUM : IDS_ALBUMS,sStr,16));
	}
	albumList.Items[0].name = ndestring_wcsdup(buf);
	albumList.Items[0].ifields[1] = numitems;
	albumList.Items[0].ifields[0] = numitems2;
	albumList.Size++;
	numGroups = numalbumstotal;
}

bool AlbumArtFilter::MakeFilterQuery(int x, GayStringW *query)
{
	queryListItem * l = &albumList.Items[x+1];
	// this mess appends this query:
	// (album="l->album" && ((albumartist isempty && artist="l->artist") || (albumartist isnotempty && albumartist="l->artist")))
	query->Append(L"(album=\"");
	GayStringW escaped;
	queryStrEscape(l->name, escaped);
	query->Append(escaped.Get());
	escaped.Set(L"");
	query->Append(L"\" && ((albumartist isempty && artist LIKE \"");
	queryStrEscape(l->artist, escaped);
	query->Append(escaped.Get());
	query->Append(L"\") || (albumartist isnotempty && albumartist LIKE \"");
	query->Append(escaped.Get());
	query->Append(L"\")))");
	return true;
}

HMENU AlbumArtFilter::GetMenu(bool isFilter, int filterNum, C_Config *c, HMENU themenu)
{
	HMENU menu = GetSubMenu(themenu, 6);

	int checked=0;
	switch (mode)
	{
		case 0: checked=ID_ARTHEADERWND_SMALLDETAILS; break;
		case 1: checked=ID_ARTHEADERWND_MEDIUMDETAILS; break;
		case 2: checked=ID_ARTHEADERWND_LARGEDETAILS; break;
		case 3: checked=ID_ARTHEADERWND_SMALLICON; break;
		case 4: checked=ID_ARTHEADERWND_MEDIUMICON; break;
		case 5: checked=ID_ARTHEADERWND_LARGEICON; break;
		case 6: checked=ID_ARTHEADERWND_EXTRALARGEICON; break;
	}
	CheckMenuItem(menu,checked,MF_CHECKED | MF_BYCOMMAND);
	CheckMenuItem(menu,ID_ARTHEADERWND_SHOWTEXT,(icons_only ? MF_UNCHECKED : MF_CHECKED) | MF_BYCOMMAND);

	if (isFilter)
	{
		MENUITEMINFO m={sizeof(m),MIIM_ID,0};
		int i=0;
		while (GetMenuItemInfo(menu,i,TRUE,&m))
		{
			m.wID |= (1+filterNum) << 16;
			SetMenuItemInfo(menu,i,TRUE,&m);
			i++;
		}
	}
	return menu;
}

void AlbumArtFilter::ProcessMenuResult(int r, bool isFilter, int filterNum, C_Config *c, HWND parent)
{
	int mid = (r >> 16), newMode;
	BOOL updateChache = FALSE;
	BOOL iconsOnly = FALSE;
	if (!isFilter && mid) return;
	if (isFilter && mid-1 != filterNum) return;

	switch (LOWORD(r))
	{
		case ID_ARTHEADERWND_SMALLDETAILS:  newMode=0; break;
		case ID_ARTHEADERWND_MEDIUMDETAILS: newMode=1; break;
		case ID_ARTHEADERWND_LARGEDETAILS:  newMode=2; break;
		case ID_ARTHEADERWND_SMALLICON:     newMode=3; break;
		case ID_ARTHEADERWND_MEDIUMICON:    newMode=4; break;
		case ID_ARTHEADERWND_LARGEICON:     newMode=5; break;
		case ID_ARTHEADERWND_EXTRALARGEICON:newMode=6; break;
		case ID_ARTHEADERWND_SHOWTEXT:
		{
			newMode = mode;
			icons_only = !icons_only;
			// only update view if in icon view
			if (!isDetailsMode(mode)) {
				iconsOnly = TRUE;
			}
		}
		break;
		default: return;
	}

	if (mode == newMode && iconsOnly == FALSE) return;

	if (!iconsOnly) {
		int w1, w2, h1, h2;
		getImgSize(mode, w1, h1);
		getImgSize(newMode, w2, h2);
		updateChache = (w1 != w2 || h1 != h2);
		mode = newMode;
	}

	c->WriteInt(L"albumartviewmode",mode);
	c->WriteInt(L"albumarticonmode",icons_only);
	SaveColumnWidths();
	while (ListView_DeleteColumn(list->getwnd(), 0));
	if (updateChache) FlushCache();

	for (int i=0;i!=albumList.Size;i++)
	{
		if (albumList.Items[i].art)
		{
			albumList.Items[i].art->Reset();
			albumList.Items[i].art->updateMsg.hwnd=0;
		}
	}

	DialogProc(parent,WM_INITDIALOG,0,0);
	if (updateChache) ResumeCache();
	HWND hwndList = GetDlgItem(parent,dlgitem);
	if (hwndList)
	{
		MLSkinnedWnd_SkinChanged(hwndList, TRUE, TRUE);
		list->setwnd(hwndList);
		AddColumns();
		CalcuateItemHeight();
		ListView_SetItemCount(hwndList, Size());
	}
}

void AlbumArtFilter::MetaUpdate(int r, const wchar_t *metaItem, const wchar_t *value)
{
	if(_wcsicmp(metaItem, DB_FIELDNAME_rating )==0)
		albumList.Items[r+1].rating = _wtoi(value) * albumList.Items[r+1].ifields[1];

	SendMessage(GetDlgItem(hwndDlg,dlgitem),LVM_REDRAWITEMS,r,r);
	UpdateWindow(GetDlgItem(hwndDlg,dlgitem));
}