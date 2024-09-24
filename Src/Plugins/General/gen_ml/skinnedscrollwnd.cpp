// some code taken from (freeware) Cool ScrollBar library by J Brown
#include "./skinnedscrollwnd.h"
#include "main.h"
#include <commctrl.h>
#include <windowsx.h>
#include "../winamp/wa_dlg.h"
#include "api__gen_ml.h"
#include "./colors.h"
#include <tataki/bitmap/bitmap.h>
#include <tataki/bitmap/autobitmap.h>
#include <tataki/canvas/canvas.h>
#include <api/wnd/api_window.h>
#include "./stockobjects.h"


/* minimum size of scrollbar before inserted buttons are hidden to make room when the window is sized too small */
#define MIN_COOLSB_SIZE  24
/* min size of scrollbar when resizing a button, before the  resize is stopped because the scrollbar has gotten too small */
#define MINSCROLLSIZE   50
/* a normal scrollbar "snaps" its scroll-thumb back into position if
   you move the mouse too far away from the window, whilst you are
   dragging the thumb, that is. #undeffing this results in the thumb
   never snapping back into position, no matter how far away you move
   the mouse */
#define SNAP_THUMB_BACK
/* distance (in pixels) the mouse must move away from the thumb
   during tracking to cause the thumb bar to snap back to its
   starting place. Has no effect unless SNAP_THUMB_BACK is defined */
#define THUMBTRACK_SNAPDIST 128
// To complement the exisiting SB_HORZ, SB_VERT, SB_BOTH
// scrollbar identifiers
#define COOLSB_NONE (-1)
#define SB_INSBUT	(-2)

//	Arrow size defines
#define SYSTEM_METRIC (-1)


// general scrollbar styles
//
// use the standard ESB_DISABLE_xxx flags to represent the
// enabled / disabled states. (defined in winuser.h)
//
#define CSBS_THUMBALWAYS			0x0004
#define CSBS_VISIBLE			0x0008
#define CSBS_TRACKING			0x0010
#define CSBS_FLATSB				0x0020
#define CSBS_BTNVISBEFORE		0x0040	//if the buttons to the left are visible
#define CSBS_BTNVISAFTER		0x0080	//if the buttons to the right are visible
#define CSBS_HOVERING       0x0100	//if the buttons to the right are visible

//cool scrollbar styles for Flat scrollbars
#define CSBS_NORMAL			0
#define CSBS_FLAT			1
#define CSBS_HOTTRACKED		2


//	Button mask flags for indicating which members of SCROLLBUT
//	to use during a button insertion / modification
#define SBBF_TYPE			0x0001
#define SBBF_ID				0x0002
#define SBBF_PLACEMENT		0x0004
#define SBBF_SIZE			0x0008
#define SBBF_BITMAP			0x0010
#define SBBF_ENHMETAFILE	0x0020
//#define SBBF_OWNERDRAW		0x0040	//unused at present
#define SBBF_CURSOR			0x0080
#define SBBF_BUTMINMAX		0x0100
#define SBBF_STATE			0x0200

//button styles (states)
#define SBBS_NORMAL			0
#define SBBS_PUSHED			1
#define SBBS_CHECKED		SBBS_PUSHED

// scrollbar button types
#define SBBT_PUSHBUTTON		1	//standard push button
#define SBBT_TOGGLEBUTTON	2	//toggle button
#define SBBT_FIXED			3	//fixed button (non-clickable)
#define SBBT_FLAT			4	//blank area (flat, with border)
#define SBBT_BLANK			5	//blank area (flat, no border)
#define SBBT_DARK			6	//dark blank area (flat)
#define SBBT_OWNERDRAW		7	//user draws the button via a WM_NOTIFY

#define SBBT_MASK			0x1f	//mask off low 5 bits

//button type modifiers
#define SBBM_RECESSED		0x0020	//recessed when clicked (like Word 97)
#define SBBM_LEFTARROW		0x0040
#define SBBM_RIGHTARROW		0x0080
#define SBBM_UPARROW			0x0100
#define SBBM_DOWNARROW		0x0200
#define SBBM_RESIZABLE		0x0400
#define SBBM_TYPE2			0x0800
#define SBBM_TYPE3			0x1000
#define SBBM_TOOLTIPS		0x2000	//currently unused (define COOLSB_TOOLTIPS in userdefs.h)

//button placement flags
#define SBBP_LEFT	1
#define SBBP_RIGHT  2
#define SBBP_TOP	1	//3
#define SBBP_BOTTOM 2	//4

#define DFCS_HOVER 0x800
//
//	Button command notification codes
//	for sending with a WM_COMMAND message
//
#define CSBN_BASE	0
#define CSBN_CLICKED (1 + CSBN_BASE)
#define CSBN_HILIGHT (2 + CSBN_BASE)

//	Minimum size in pixels of a scrollbar thumb
#define MINTHUMBSIZE_NT4   9
#define MINTHUMBSIZE_2000  7

//define some more hittest values for our cool-scrollbar
#define HTSCROLL_LEFT		(SB_LINELEFT)
#define HTSCROLL_RIGHT		(SB_LINERIGHT)
#define HTSCROLL_UP			(SB_LINEUP)
#define HTSCROLL_DOWN		(SB_LINEDOWN)
#define HTSCROLL_THUMB		(SB_THUMBTRACK)
#define HTSCROLL_PAGEGUP	(SB_PAGEUP)
#define HTSCROLL_PAGEGDOWN	(SB_PAGEDOWN)
#define HTSCROLL_PAGELEFT	(SB_PAGELEFT)
#define HTSCROLL_PAGERIGHT	(SB_PAGERIGHT)

#define HTSCROLL_NONE		(-1)
#define HTSCROLL_NORMAL		(-1)

#define HTSCROLL_INSERTED	(128)
#define HTSCROLL_PRE		(32 | HTSCROLL_INSERTED)
#define HTSCROLL_POST		(64 | HTSCROLL_INSERTED)

//	SCROLLBAR datatype. There are two of these structures per window
typedef struct _SCROLLBAR
{
	UINT		fScrollFlags;		//flags
	BOOL		fScrollVisible;		//if this scrollbar visible?
	SCROLLINFO	scrollInfo;			//positional data (range, position, page size etc)

	//data for inserted buttons
	int			nButSizeBefore;		//size to the left / above the bar
	int			nButSizeAfter;		//size to the right / below the bar

	int			nMinThumbSize;
	int			nBarType;			//SB_HORZ / SB_VERT

} SCROLLBAR;

static WORD wCheckPat[8] =
{
	0xaaaa, 0x5555, 0xaaaa, 0x5555, 0xaaaa, 0x5555, 0xaaaa, 0x5555
};


// scrollwnd styles
#define SWS_UPDATEFRAME			0x0001
#define SWS_LEFT					0x0002
#define SWS_DISABLENOSCROLL		0x0004
#define SWS_HIDEHSCROLL			0x0008
#define SWS_LISTVIEW				0x0010
#define SWS_TREEVIEW				0x0020
#define SWS_HIDEVSCROLL			0x0040
#define SWS_COMBOLBOX			0x0080
#define SWS_USEFREEFORM			0x0100

//
//	PRIVATE INTERNAL FUNCTIONS
//
#define COOLSB_TIMERID1			65533		//initial timer
#define COOLSB_TIMERID2			65534		//scroll message timer
#define COOLSB_TIMERID3			-14			//mouse hover timer
#define COOLSB_TIMERINTERVAL1	300
#define COOLSB_TIMERINTERVAL2	55
#define COOLSB_TIMERINTERVAL3	20			//mouse hover time

//
//	direction: 0 - same axis as scrollbar (i.e.  width of a horizontal bar)
//             1 - perpendicular dimesion (i.e. height of a horizontal bar)
//
#define SM_CXVERTSB 1
#define SM_CYVERTSB 0
#define SM_CXHORZSB 0
#define SM_CYHORZSB 1
#define SM_SCROLL_WIDTH	1
#define SM_SCROLL_LENGTH 0

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL                   0x020A
#endif


#define INACTIVEBAR_ALPHA			127

//
//	Special thumb-tracking variables
//
//
static UINT uCurrentScrollbar = COOLSB_NONE;	//SB_HORZ / SB_VERT
static UINT uCurrentScrollPortion = HTSCROLL_NONE;
static UINT uCurrentButton = 0;

static RECT rcThumbBounds;		//area that the scroll thumb can travel in
static int  nThumbSize;			//(pixels)
static int  nThumbPos;			//(pixels)
static int  nThumbMouseOffset;	//(pixels)
static int  nLastPos = -1;		//(scrollbar units)
static int  nThumbPos0;			//(pixels) initial thumb position
static int 	trackThumbPos;
//
//	Temporary state used to auto-generate timer messages
//
static UINT uScrollTimerMsg = 0;
static UINT uScrollTimerPortion = HTSCROLL_NONE;
static UINT_PTR uScrollTimerId = 0;
static HWND hwndCurCoolSB = 0;

static INT bUseUpdateRgn = -1;
static BOOL bDoHover=FALSE;
static BOOL ignoreCaptureChange = FALSE;
static BOOL captureSet = FALSE;
static HBRUSH hbrChecked = NULL;

#define GetSBForeColor() WADlg_getColor(WADLG_SCROLLBAR_FGCOLOR)
#define GetSBBackColor() WADlg_getColor(WADLG_SCROLLBAR_BGCOLOR)

//	Send a WM_VSCROLL or WM_HSCROLL message
#define SendScrollMessage(__hwnd, __srcMsg, __srcId, __pos) ::SendMessageW(__hwnd, __srcMsg, (MAKEWPARAM(__srcId, __pos)), 0)

static UINT GetPortion(SCROLLBAR *sb, HWND hwnd, RECT *rect, int x, int y, DWORD scrollFlags);

static void RenderBaseTexture(Canvas *canvas, const RECT *r, HWND hwnd)
{
	// TODO: find the ifc_window * object for the media library container, and call renderBaseTexture on it
	if (WASABI_API_WND)
	{
		HWND checkWnd = GetParent(hwnd);
		while (checkWnd)
		{
			ifc_window *window = WASABI_API_WND->rootWndFromOSHandle(checkWnd);
			if (window && window->getRenderBaseTexture())
			{
				window->renderBaseTexture(canvas, r);
				return;
			}
			checkWnd = GetParent(checkWnd);
		}
	}

	// fallback code
	COLORREF bgcolor =  WADlg_getColor(WADLG_WNDBG/*WADLG_SCROLLBAR_BGCOLOR*/);
	canvas->fillRect(r, bgcolor);
}

//	swap the rectangle's x coords with its y coords
static void __stdcall RotateRect(RECT *rect)
{
	LONG temp;
	temp = rect->left;
	rect->left = rect->top;
	rect->top = temp;

	temp = rect->right;
	rect->right = rect->bottom;
	rect->bottom = temp;
}

//	swap the coords if the scrollbar is a SB_VERT
#define RotateRect0(__psb, __prc) ((__psb && __psb->nBarType == SB_VERT) ? RotateRect(__prc) : 0)

static bool UseFreeformScrollbars()
{
	if (config_use_ff_scrollbars && WASABI_API_SKIN && WASABI_API_SKIN->skin_isLoaded())
	{
		return WASABI_API_SKIN->skin_getVersion() >= 1.3;
	}
	else
	{
		return false;
	}
}

//	Calculate if the SCROLLINFO members produce an enabled or disabled scrollbar
static BOOL IsScrollInfoActive(SCROLLINFO *si)
{
	return (si->nPage <= (UINT)si->nMax && si->nMax > si->nMin && si->nMax != 0);
}

//	Return if the specified scrollbar is enabled or not
static BOOL IsScrollbarActive(SCROLLBAR *sb)
{
	return (((sb->fScrollFlags & ESB_DISABLE_BOTH) == ESB_DISABLE_BOTH) ||
	        !(sb->fScrollFlags & CSBS_THUMBALWAYS) && !IsScrollInfoActive(&sb->scrollInfo)) ? FALSE : TRUE;
}

enum
{
	HORIZ_LEFT,
	HORIZ_LEFT_PRESSED,
	HORIZ_LEFT_HOVER,
	HORIZ_LEFT_INACTIVE,
	HORIZ_RIGHT,
	HORIZ_RIGHT_PRESSED,
	HORIZ_RIGHT_HOVER,
	HORIZ_RIGHT_INACTIVE,
	VERT_UP,
	VERT_UP_PRESSED,
	VERT_UP_HOVER,
	VERT_UP_INACTIVE,
	VERT_DOWN,
	VERT_DOWN_PRESSED,
	VERT_DOWN_HOVER,
	VERT_DOWN_INACTIVE,
};

static int GetBitmapEnum(UINT state, BOOL hover)
{
	int offset=0;
	if (state&DFCS_PUSHED)
		offset=1;
	if (state&DFCS_INACTIVE)
		offset=3;
	else if (hover)
		offset=2;

	switch (state&3)
	{
	case DFCS_SCROLLRIGHT:
		return HORIZ_RIGHT+offset;
	case DFCS_SCROLLLEFT:
		return HORIZ_LEFT+offset;
	case DFCS_SCROLLDOWN:
		return VERT_DOWN+offset;
	default://case DFCS_SCROLLUP:
		return VERT_UP+offset;
	}
}

class ScrollBitmaps
{
public:
	ScrollBitmaps() : v_up(L"wasabi.scrollbar.vertical.background.top"),
					  v_down(L"wasabi.scrollbar.vertical.background.bottom"),
					  v_mid(L"wasabi.scrollbar.vertical.background.middle"),
					  h_left(L"wasabi.scrollbar.horizontal.background.left"),
					  h_mid(L"wasabi.scrollbar.horizontal.background.middle"),
					  h_right(L"wasabi.scrollbar.horizontal.background.right")
	{
	}
	AutoSkinBitmap v_up, v_down, v_mid, h_left, h_mid, h_right;
};

static ScrollBitmaps *scrollBitmaps=0;

void SkinnedScrollWnd_Init()
{
	scrollBitmaps = new ScrollBitmaps();
}

void SkinnedScrollWnd_Quit()
{
	if (scrollBitmaps)
	{
		delete scrollBitmaps;
		scrollBitmaps=0;
	}
}

static HBITMAP hbmpCachedDib = NULL;

//	Paint a checkered rectangle, with each alternate pixel being assigned a different colour

static BOOL DrawFrameCtrl(HDC hdc, LPRECT lprc, UINT uType, UINT state, BOOL hover, BOOL freeform)
{
	int startx, starty, alpha = 255;

	const wchar_t *bitmapid=0;
	const wchar_t *backgroundid=0;
	SkinBitmap *bg=0;
	switch (GetBitmapEnum(state, hover))
	{
	case HORIZ_LEFT:
		bitmapid = L"wasabi.scrollbar.horizontal.left";
		if (scrollBitmaps)
		bg=scrollBitmaps->h_left.getBitmap();
		startx = 0; starty = 45; break;
	case HORIZ_LEFT_PRESSED:
		bitmapid = L"wasabi.scrollbar.horizontal.left.pressed";
		if (scrollBitmaps)
		bg=scrollBitmaps->h_left.getBitmap();
		startx = 28; starty = 45; break;
	case HORIZ_LEFT_HOVER:
		bitmapid = L"wasabi.scrollbar.horizontal.left.hover";
		if (scrollBitmaps)
		bg=scrollBitmaps->h_left.getBitmap();
		startx = 0; starty = 45; break;
	case HORIZ_LEFT_INACTIVE:
		alpha = INACTIVEBAR_ALPHA;
		bitmapid = L"wasabi.scrollbar.horizontal.left";
		if (scrollBitmaps)
		bg=scrollBitmaps->h_left.getBitmap();
		startx = 0; starty = 45; break;
	case HORIZ_RIGHT:
		bitmapid = L"wasabi.scrollbar.horizontal.right";
		if (scrollBitmaps)
		bg=scrollBitmaps->h_right.getBitmap();
		startx = 14; starty = 45; break;
	case HORIZ_RIGHT_PRESSED:
		bitmapid = L"wasabi.scrollbar.horizontal.right.pressed";
		if (scrollBitmaps)
		bg=scrollBitmaps->h_right.getBitmap();
		startx = 42; starty = 45; break;
	case HORIZ_RIGHT_HOVER:
		bitmapid = L"wasabi.scrollbar.horizontal.right.hover";
		if (scrollBitmaps)
		bg=scrollBitmaps->h_right.getBitmap();
		startx = 14; starty = 45; break;
	case HORIZ_RIGHT_INACTIVE:
		alpha = INACTIVEBAR_ALPHA;
		bitmapid = L"wasabi.scrollbar.horizontal.right";
		if (scrollBitmaps)
		bg=scrollBitmaps->h_right.getBitmap();
		startx = 14; starty = 45; break;
	case VERT_UP:
		bitmapid = L"wasabi.scrollbar.vertical.left";
		if (scrollBitmaps)
		bg=scrollBitmaps->v_up.getBitmap();
		startx = 0;	starty = 31;	break;
	case VERT_UP_PRESSED:
		bitmapid = L"wasabi.scrollbar.vertical.left.pressed";
		if (scrollBitmaps)
		bg=scrollBitmaps->v_up.getBitmap();
		startx = 28;	starty = 31;	break;
	case VERT_UP_HOVER:
		bitmapid = L"wasabi.scrollbar.vertical.left.hover";
		if (scrollBitmaps)
		bg=scrollBitmaps->v_up.getBitmap();
		startx = 0;	starty = 31;	break;
	case VERT_UP_INACTIVE:
		alpha = INACTIVEBAR_ALPHA;
		bitmapid = L"wasabi.scrollbar.vertical.left";
		if (scrollBitmaps)
		bg=scrollBitmaps->v_up.getBitmap();
		startx = 0;	starty = 31;	break;

	case VERT_DOWN:
		bitmapid = L"wasabi.scrollbar.vertical.right";
		if (scrollBitmaps)
		bg=scrollBitmaps->v_down.getBitmap();
		startx = 14; starty = 31; break;
	case VERT_DOWN_PRESSED:
		bitmapid = L"wasabi.scrollbar.vertical.right.pressed";
		if (scrollBitmaps)
		bg=scrollBitmaps->v_down.getBitmap();
		startx = 42; starty = 31; break;
	case VERT_DOWN_HOVER:
		bitmapid = L"wasabi.scrollbar.vertical.right.hover";
		if (scrollBitmaps)
		bg=scrollBitmaps->v_down.getBitmap();
		startx = 14; starty = 31; break;
	case VERT_DOWN_INACTIVE:
		alpha = INACTIVEBAR_ALPHA;
		bitmapid = L"wasabi.scrollbar.vertical.right";
		if (scrollBitmaps)
		bg=scrollBitmaps->v_down.getBitmap();
		startx = 14; starty = 31; break;
	}

	if (freeform)
	{
		SkinBitmap bmp(bitmapid);
		if (!bmp.isInvalid() && bg && !bg->isInvalid())
		{
			DCCanvas canvas(hdc);
			bg->stretchToRectAlpha(&canvas, lprc, alpha);
			bmp.stretchToRectAlpha(&canvas, lprc, alpha);
			return 1;
		}
	}

	// fallback code
	HDC hdcbmp;
	HBITMAP hbmpOld, hbmp;

	hbmp = WADlg_getBitmap();
	if (!hbmp) return FALSE;

	hdcbmp = (HDC)MlStockObjects_Get(CACHED_DC);
	if (!hdcbmp) return FALSE;

	hbmpOld = (HBITMAP)SelectObject(hdcbmp, hbmp);


	
	if (255 == alpha) 
		StretchBlt(hdc, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top, hdcbmp, startx, starty, 14, 14, SRCCOPY);
	else
	{	
		HDC hdcTmp = CreateCompatibleDC(hdc);

		DIBSECTION dibSection;
		if (NULL == hbmpCachedDib || 
			sizeof(DIBSECTION) != GetObjectW(hbmpCachedDib, sizeof(DIBSECTION), &dibSection)
			|| dibSection.dsBm.bmWidth < (lprc->right - lprc->left) || ABS(dibSection.dsBm.bmHeight) < (lprc->bottom - lprc->top))
		{
			if (hbmpCachedDib) DeleteObject(hbmpCachedDib);

			BITMAPINFOHEADER bi;
			ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
			bi.biSize = sizeof(BITMAPINFOHEADER);
			bi.biWidth = lprc->right - lprc->left;
			bi.biHeight = -(lprc->bottom - lprc->top);
			bi.biPlanes = 1;
			bi.biBitCount = 32;
			bi.biCompression = BI_RGB;
			hbmpCachedDib = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (VOID**)&dibSection.dsBm.bmBits, NULL, 0);
			dibSection.dsBm.bmHeight = bi.biHeight;
			dibSection.dsBm.bmWidth = bi.biWidth;
		}		
		
		ASSERT(hbmpCachedDib != 0);

		HBITMAP hbmpTmp = (HBITMAP)SelectObject(hdcTmp, hbmpCachedDib);
	
		StretchBlt(hdcTmp, 0, 0, lprc->right - lprc->left, lprc->bottom - lprc->top, hdcbmp, startx, starty, 14, 14, SRCCOPY);
			
		LONG pitch = dibSection.dsBm.bmWidth*4, cy = lprc->bottom - lprc->top, x;
		LPBYTE cursor, line;

		COLORREF rgbBk = WADlg_getColor(WADLG_WNDBG);// BlendColors(GetSBBackColor(), WADlg_getColor(WADLG_ITEMBG), ((float)INACTIVEBAR_ALPHA)/255.0f);

		BYTE k = (((255 - alpha)*255 + 127)/255);
		BYTE r = (GetRValue(rgbBk)*k + 127)/255, g = (GetGValue(rgbBk)*k + 127)>>8, b = (GetBValue(rgbBk)*k + 127)/255;
			
		for (line = (BYTE*)dibSection.dsBm.bmBits; cy-- != 0; line += pitch )
		{	
			for (x = (lprc->right - lprc->left), cursor = line; x-- != 0; cursor += 4) 
			{
				cursor[0] = (cursor[0]*alpha)/255 + b;
				cursor[1] = (cursor[1]*alpha)/255 + g;
				cursor[2] = (cursor[2]*alpha)/255 + r;
				cursor[3] = 0xFF;
			}
		}
		
		BitBlt(hdc, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top, hdcTmp, 0, 0, SRCCOPY);
		SelectObject(hdcTmp, hbmpTmp);
		DeleteDC(hdcTmp);
	}

	SelectObject(hdcbmp, hbmpOld);

	return 1;
}

//	Draw a standard scrollbar arrow
static int DrawScrollArrow(SCROLLBAR *sbar, HDC hdc, RECT *rect, UINT arrow, BOOL fMouseDown, BOOL fMouseOver, BOOL freeform)
{
	UINT ret;
	UINT flags = arrow;

	//HACKY bit so this routine can be called by vertical and horizontal code
	if (sbar->nBarType == SB_VERT)
	{
		if (flags & DFCS_SCROLLLEFT)	flags = flags & ~DFCS_SCROLLLEFT  | DFCS_SCROLLUP;
		if (flags & DFCS_SCROLLRIGHT)	flags = flags & ~DFCS_SCROLLRIGHT | DFCS_SCROLLDOWN;
	}

	if (fMouseDown) flags |= (DFCS_FLAT | DFCS_PUSHED);

	ret = DrawFrameCtrl(hdc, rect, DFC_SCROLL, flags, fMouseOver, freeform);

	return ret;
}

//	Return the size in pixels for the specified scrollbar metric, for the specified scrollbar
static int GetScrollMetric(SCROLLBAR *psb, int metric, DWORD scrollFlags)
{
	int type (psb ? psb->nBarType : SB_VERT);
	switch (type)
	{
		case SB_HORZ:
		{
			switch (metric)
			{
				case SM_CXHORZSB:
					if (SWS_USEFREEFORM & scrollFlags)
					{
						SkinBitmap button(L"wasabi.scrollbar.horizontal.left"); // we assume symmetry which isn't necessary safe
						if (!button.isInvalid())
							return WASABI_API_APP->getScaleX(button.getWidth());
					}
					return WASABI_API_APP->getScaleX(14); // classic skin fixes this at 14
				default:
					if (SWS_USEFREEFORM & scrollFlags)
					{
						SkinBitmap button(L"wasabi.scrollbar.horizontal.left"); // we assume symmetry which isn't necessary safe
						if (!button.isInvalid())
							return WASABI_API_APP->getScaleY(button.getHeight());
					}
					return WASABI_API_APP->getScaleY(14); // classic skin fixes this at 14
					break;
			}
		}
		break;

		default: // case SB_VERT:
		{
			switch (metric)
			{
				case SM_CYVERTSB:
					if (SWS_USEFREEFORM & scrollFlags)
					{
						SkinBitmap button(L"wasabi.scrollbar.vertical.left"); // we assume symmetry which isn't necessary safe
						if (!button.isInvalid())
							return WASABI_API_APP->getScaleY(button.getHeight());
					}
					return WASABI_API_APP->getScaleY(14); // classic skin fixes this at 14
				default:
					if (SWS_USEFREEFORM & scrollFlags)
					{
						SkinBitmap button(L"wasabi.scrollbar.vertical.left"); // we assume symmetry which isn't necessary safe
						if (!button.isInvalid())
							return WASABI_API_APP->getScaleX(button.getWidth());
					}
					return WASABI_API_APP->getScaleX(14); // classic skin fixes this at 14
					break;
			}
		}
		break;
	}

	/*
		if ((SB_HORZ == psb->nBarType && metric == SM_CXHORZSB) || (SB_VERT == psb->nBarType && metric == SM_CYVERTSB))
			return (psb->nArrowLength == SYSTEM_METRIC) * ((psb->nArrowLength < 0) ? -14 : 1);
		else return psb->nArrowWidth * ((psb->nArrowWidth < 0) ? -14 : 1);
		*/
}

//	Fill the specifed rectangle using a solid colour
static void PaintRect(HDC hdc, RECT *rect, COLORREF color)
{
	COLORREF oldcol = SetBkColor(hdc, color);
	ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, rect, L"", 0, 0);
	SetBkColor(hdc, oldcol);
}

//
//	Set the minimum size, in pixels, that the thumb box will shrink to.
//

//	Draw a simple blank scrollbar push-button. Can be used
//	to draw a push button, or the scrollbar thumb
//	drawflag - could set to BF_FLAT to make flat scrollbars
static void DrawBlankButton(HDC hdc, const RECT *rect, UINT drawflag, int pushed, int vertical)
{
	HBITMAP hbmp, hbmpOld;
	hbmp = WADlg_getBitmap();
	if (!hbmp) return;

	HDC hdcbmp = (HDC)MlStockObjects_Get(CACHED_DC);
	if (!hdcbmp) return;

	hbmpOld = (HBITMAP)SelectObject(hdcbmp, hbmp);

#define PART1SIZE 4  //copied top
#define PART2SIZE 5  //stretched top
#define PART3SIZE 10  //copied middle
#define PART4SIZE 5  //stretched bottom
#define PART5SIZE 4  //copied bottom

	if (vertical)
	{
		int middle = (rect->bottom - rect->top) / 2;
		int startx = pushed ? 70 : 56;
		//top
		StretchBlt(hdc, rect->left, rect->top, rect->right - rect->left, PART1SIZE, hdcbmp, startx, 31, 14, PART1SIZE, SRCCOPY);
		int p = PART1SIZE;
		//stretched top
		int l = middle - PART1SIZE - (PART3SIZE / 2);
		if (l > 0)
		{
			StretchBlt(hdc, rect->left, rect->top + p, rect->right - rect->left, l, hdcbmp, startx, 31 + PART1SIZE, 14, PART2SIZE, SRCCOPY);
			p += middle - PART1SIZE - (PART3SIZE / 2);
		}
		//copied middle
		int m = (rect->bottom - rect->top) - PART1SIZE - PART5SIZE; //space that's available for middle
		m = min(m, PART3SIZE);
		if (m > 0)
		{
			StretchBlt(hdc, rect->left, rect->top + p, rect->right - rect->left, m, hdcbmp, startx, 31 + PART1SIZE + PART2SIZE, 14, m, SRCCOPY);
			p += m;
		}
		//stretched bottom
		l = rect->bottom - rect->top - p - PART5SIZE;
		if (l > 0) StretchBlt(hdc, rect->left, rect->top + p, rect->right - rect->left, l, hdcbmp, startx, 31 + PART1SIZE + PART2SIZE + PART3SIZE, 14, PART4SIZE, SRCCOPY);
		//bottom
		StretchBlt(hdc, rect->left, rect->bottom - PART5SIZE, rect->right - rect->left, PART5SIZE, hdcbmp, startx, 31 + PART1SIZE + PART2SIZE + PART3SIZE + PART4SIZE, 14, PART5SIZE, SRCCOPY);
	}
	else
	{
		int middle = (rect->right - rect->left) / 2;
		int starty = pushed ? 45 : 31;
		//top
		StretchBlt(hdc, rect->left, rect->top, PART1SIZE, rect->bottom - rect->top, hdcbmp, 84, starty, PART1SIZE, 14, SRCCOPY);
		int p = PART1SIZE;
		//stretched top
		int l = middle - PART1SIZE - (PART3SIZE / 2);
		if (l > 0)
		{
			StretchBlt(hdc, rect->left + p, rect->top, l, rect->bottom - rect->top, hdcbmp, 84 + PART1SIZE, starty, PART2SIZE, 14, SRCCOPY);
			p += middle - PART1SIZE - (PART3SIZE / 2);
		}
		//copied middle
		int m = (rect->right - rect->left) - PART1SIZE - PART5SIZE; //space that's available for middle
		m = min(m, PART3SIZE);
		if (m > 0)
		{
			StretchBlt(hdc, rect->left + p, rect->top, m, rect->bottom - rect->top, hdcbmp, 84 + PART1SIZE + PART2SIZE, starty, m, 14, SRCCOPY);
			p += m;
		}
		//stretched bottom
		l = rect->right - rect->left - p - PART5SIZE;
		if (l > 0) StretchBlt(hdc, rect->left + p, rect->top, l, rect->bottom - rect->top, hdcbmp, 84 + PART1SIZE + PART2SIZE + PART3SIZE, starty, PART4SIZE, 14, SRCCOPY);
		//bottom
		StretchBlt(hdc, rect->right - PART5SIZE, rect->top, PART5SIZE, rect->bottom - rect->top, hdcbmp, 84 + PART1SIZE + PART2SIZE + PART3SIZE + PART4SIZE, starty, PART5SIZE, 14, SRCCOPY);
	}

	SelectObject(hdcbmp, hbmpOld);
}


//	Calculate the screen coordinates of the area taken by
//  the horizontal scrollbar. Take into account the size
//  of the window borders
static BOOL GetHScrollRect(SkinnedScrollWnd *pWnd, RECT *prc)
{	
	if (pWnd->psbHorz->fScrollVisible)
	{
		GetClientRect(pWnd->hwnd, prc);
		MapWindowPoints(pWnd->hwnd, HWND_DESKTOP, (POINT*)prc, 2);
		prc->top = prc->bottom;
		prc->bottom += GetScrollMetric(pWnd->psbHorz, SM_CYHORZSB, pWnd->scrollFlags);
	}
	else SetRect(prc, 0, 0, 0, 0);
	return TRUE;
}

//	Calculate the screen coordinates of the area taken by the
//  vertical scrollbar
static BOOL GetVScrollRect(SkinnedScrollWnd *pWnd, RECT *prc)
{
	if (pWnd->psbVert->fScrollVisible)
	{
		GetClientRect(pWnd->hwnd, prc);
		MapWindowPoints(pWnd->hwnd, HWND_DESKTOP, (POINT*)prc, 2);
		if (SWS_LEFT & pWnd->scrollFlags)
		{
			prc->right = prc->left;
			prc->left -= GetScrollMetric(pWnd->psbVert, SM_CXVERTSB, pWnd->scrollFlags);
		}
		else 
		{
			prc->left = prc->right;
			prc->right += GetScrollMetric(pWnd->psbVert, SM_CXVERTSB, pWnd->scrollFlags);
		}
	}
	else SetRect(prc, 0, 0, 0, 0);

	
	return TRUE;
}

//	Depending on what type of scrollbar nBar refers to, call the
//  appropriate Get?ScrollRect function
//
static BOOL GetScrollRect(SkinnedScrollWnd *pWnd, UINT nBar, RECT *prc)
{
	if (nBar == SB_HORZ) return GetHScrollRect(pWnd, prc);
	else if (nBar == SB_VERT) return GetVScrollRect(pWnd, prc);
	else return FALSE;
}

//
//	Work out the scrollbar width/height for either type of scrollbar (SB_HORZ/SB_VERT)
//	rect - coords of the scrollbar.
//	store results into *thumbsize and *thumbpos
//
static int CalcThumbSize(SCROLLBAR *sbar, const RECT *rect, int *pthumbsize, int *pthumbpos, DWORD scrollFlags)
{
	SCROLLINFO *si;
	int scrollsize;			//total size of the scrollbar including arrow buttons
	int workingsize;		//working area (where the thumb can slide)
	int siMaxMin;
	int butsize;
	int startcoord;
	int thumbpos = 0, thumbsize = 0;

	//work out the width (for a horizontal) or the height (for a vertical)
	//of a standard scrollbar button
	butsize = GetScrollMetric(sbar, SM_SCROLL_LENGTH, scrollFlags);

	if (1) //sbar->nBarType == SB_HORZ)
	{
		scrollsize = rect->right - rect->left;
		startcoord = rect->left;
	}
	/*else if(sbar->nBarType == SB_VERT)
	{
		scrollsize = rect->bottom - rect->top;
		startcoord = rect->top;
	}
	else
	{
		return 0;
	}*/

	si = &sbar->scrollInfo;
	siMaxMin = si->nMax - si->nMin + 1;

	workingsize = scrollsize - butsize * 2;

	//
	// Work out the scrollbar thumb SIZE
	//
	if (si->nPage == 0)
	{
		thumbsize = butsize;
	}
	else if (siMaxMin > 0)
	{
		/*if (SWS_HIDEHSCROLL & scrollFlags)
			thumbsize = MulDiv(si->nPage, workingsize, si->nMax);
		else*/
			thumbsize = MulDiv(si->nPage, workingsize, siMaxMin);

		if (SWS_USEFREEFORM & scrollFlags)
		{
			SkinBitmap thumb((sbar->nBarType == SB_VERT)?L"wasabi.scrollbar.vertical.button":L"wasabi.scrollbar.horizontal.button");
			if (!thumb.isInvalid())
				thumbsize = (sbar->nBarType == SB_VERT)?thumb.getHeight():thumb.getWidth();
		}

		if (thumbsize < sbar->nMinThumbSize)
			thumbsize = sbar->nMinThumbSize;
	}

	//
	// Work out the scrollbar thumb position
	//
	if (siMaxMin > 0)
	{
		int pagesize = max(1, si->nPage);
		thumbpos = MulDiv(si->nPos - si->nMin, workingsize - thumbsize, siMaxMin - pagesize);
		/*if (SWS_HIDEHSCROLL & scrollFlags)
		{
			thumbpos = (si->nPos == (si->nMax - si->nPage)) ? 
					(workingsize - thumbsize) : MulDiv(si->nPos, workingsize, si->nMax);
		}*/

		if (thumbpos < 0)
			thumbpos = 0;

		if (thumbpos >= workingsize - thumbsize)
			thumbpos = workingsize - thumbsize;
	}

	thumbpos += startcoord + butsize;

	*pthumbpos  = thumbpos;
	*pthumbsize = thumbsize;

	return 1;
}

//
//	return a hit-test value for whatever part of the scrollbar x,y is located in
//	rect, x, y: SCREEN coordinates
//	the rectangle must not include space for any inserted buttons
//	(i.e, JUST the scrollbar area)
//
static UINT GetHorzScrollPortion(SCROLLBAR *sbar, HWND hwnd, const RECT *rect, int x, int y, DWORD scrollFlags)
{
	int thumbwidth, thumbpos;
	int butwidth = GetScrollMetric(sbar, SM_SCROLL_LENGTH, scrollFlags);
	int scrollwidth  = rect->right - rect->left;
	int workingwidth = scrollwidth - butwidth * 2;

	if (y < rect->top || y >= rect->bottom)
		return HTSCROLL_NONE;

	CalcThumbSize(sbar, rect, &thumbwidth, &thumbpos, scrollFlags);

	//if we have had to scale the buttons to fit in the rect,
	//then adjust the button width accordingly
	if (scrollwidth <= butwidth * 2)
	{
		butwidth = scrollwidth / 2;
	}

	//check for left button click
	if (x >= rect->left && x < rect->left + butwidth)
	{
		return (ESB_DISABLE_LEFT & sbar->fScrollFlags) ? HTSCROLL_NONE : HTSCROLL_LEFT;
	}
	//check for right button click
	else if (x >= rect->right - butwidth && x < rect->right)
	{
		return (ESB_DISABLE_RIGHT & sbar->fScrollFlags) ? HTSCROLL_NONE : HTSCROLL_RIGHT;
	}

	//if the thumb is too big to fit (i.e. it isn't visible)
	//then return a NULL scrollbar area
	if (thumbwidth >= workingwidth)
		return HTSCROLL_NONE;

	//check for point in the thumbbar
	if (x >= thumbpos && x < thumbpos + thumbwidth)
	{
		return HTSCROLL_THUMB;
	}
	//check for left margin
	else if (x >= rect->left + butwidth && x < thumbpos)
	{
		return HTSCROLL_PAGELEFT;
	}
	else if (x >= thumbpos + thumbwidth && x < rect->right - butwidth)
	{
		return HTSCROLL_PAGERIGHT;
	}

	return HTSCROLL_NONE;
}

//
//	For vertical scrollbars, rotate all coordinates by -90 degrees
//	so that we can use the horizontal version of this function
//
static UINT GetVertScrollPortion(SCROLLBAR *sb, HWND hwnd, RECT *rect, int x, int y, DWORD scrollFlags)
{
	UINT r;

	RotateRect(rect);
	r = GetHorzScrollPortion(sb, hwnd, rect, y, x, scrollFlags);
	RotateRect(rect);
	return r;
}

static const wchar_t *GetThumbID(UINT barType, UINT scrollFlags, BOOL hover)
{
	if (barType == SB_VERT)
	{
		if (scrollFlags & CSBS_TRACKING)
			return L"wasabi.scrollbar.vertical.button.pressed";
		else if (hover)
			return L"wasabi.scrollbar.vertical.button.hover";
		else
			return L"wasabi.scrollbar.vertical.button";
	}
	else
	{
		if (scrollFlags & CSBS_TRACKING)
			return L"wasabi.scrollbar.horizontal.button.pressed";
		else if (hover)
			return L"wasabi.scrollbar.horizontal.button.hover";
		else
			return L"wasabi.scrollbar.horizontal.button";
	}
}

static HBRUSH SetWindowPatternBrush(HWND hwnd, HDC hdc, UINT nBarType)
{
	RECT rw;
	GetWindowRect(hwnd, &rw);
	HWND hwndAncestor = GetAncestor(hwnd, GA_ROOT);
	if (hwndAncestor) MapWindowPoints(HWND_DESKTOP, hwndAncestor, (POINT*)&rw, 2);
	POINT ptOrg;
	if (GetViewportOrgEx(hdc, &ptOrg)) OffsetRect(&rw, ptOrg.x, ptOrg.y);
	if (nBarType == SB_VERT)
	{
		if (0 == (GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_LEFTSCROLLBAR)) rw.left = rw.right;
	}
	else rw.top = rw.bottom;

	SetBrushOrgEx(hdc, rw.left, rw.top, (POINT*)&rw);
	return (HBRUSH)SelectObject(hdc, hbrChecked);
}

//
//	Draw a complete HORIZONTAL scrollbar in the given rectangle
//	Don't draw any inserted buttons in this procedure
//
//	uDrawFlags - hittest code, to say if to draw the
//  specified portion in an active state or not.
//
//
static LRESULT NCDrawHScrollbar(SCROLLBAR *sb, HWND hwnd, HDC hdc, const RECT *rect, UINT uDrawFlags, UINT hoverFlags, DWORD scrollFlags)
{
	SCROLLINFO *si;
	RECT ctrl, thumb;
	RECT sbm;
	int butwidth	 = GetScrollMetric(sb, SM_SCROLL_LENGTH, scrollFlags);
	int scrollwidth  = rect->right - rect->left;
	int workingwidth = scrollwidth - butwidth * 2;
	int thumbwidth   = 0, thumbpos = 0;
	int siMaxMin;

	BOOL fMouseDownL = 0, fMouseOverL = (hoverFlags == HTSCROLL_LEFT);
	BOOL fMouseDownR = 0, fMouseOverR = (hoverFlags == HTSCROLL_RIGHT);
	BOOL fMouseOverThumb = (hoverFlags == HTSCROLL_THUMB);

	COLORREF crCheck1   = GetSBForeColor();
	COLORREF crCheck2   = GetSBBackColor();
	COLORREF crInverse1 = WADlg_getColor(WADLG_SCROLLBAR_INV_FGCOLOR);
	COLORREF crInverse2 = WADlg_getColor(WADLG_SCROLLBAR_INV_BGCOLOR);

	UINT uDFCFlat = (CSBS_FLATSB & sb->fScrollFlags) ? DFCS_FLAT : 0;
	UINT uDEFlat  = (CSBS_FLATSB & sb->fScrollFlags) ? BF_FLAT   : 0;

	//drawing flags to modify the appearance of the scrollbar buttons
	UINT uLeftButFlags  = DFCS_SCROLLLEFT;
	UINT uRightButFlags = DFCS_SCROLLRIGHT;

	if (scrollwidth <= 0)
		return 0;

	si = &sb->scrollInfo;
	siMaxMin = si->nMax - si->nMin;

	if (hwnd != hwndCurCoolSB)
		uDrawFlags = HTSCROLL_NONE;
	//
	// work out the thumb size and position
	//
	CalcThumbSize(sb, rect, &thumbwidth, &thumbpos, scrollFlags);
	if ((CSBS_TRACKING & sb->fScrollFlags) && trackThumbPos != -1)
	{
		thumbpos=trackThumbPos;
	}

	if (sb->fScrollFlags & ESB_DISABLE_LEFT)		uLeftButFlags  |= DFCS_INACTIVE;
	if (sb->fScrollFlags & ESB_DISABLE_RIGHT)	uRightButFlags |= DFCS_INACTIVE;

	//if we need to grey the arrows because there is no data to scroll
	if ((0 == (DFCS_INACTIVE & uLeftButFlags) || 0 == (DFCS_INACTIVE & uRightButFlags)) &&
		!(sb->fScrollFlags & CSBS_THUMBALWAYS) && !IsScrollInfoActive(si))
	{
		uLeftButFlags  |= DFCS_INACTIVE;
		uRightButFlags |= DFCS_INACTIVE;
	}

	if ((DFCS_INACTIVE & uLeftButFlags) &&  (DFCS_INACTIVE & uRightButFlags))
	{
		COLORREF rgbBk = WADlg_getColor(WADLG_WNDBG);
		crCheck1 = BlendColors(crCheck1, rgbBk, INACTIVEBAR_ALPHA);
		crCheck2 = BlendColors(crCheck2, rgbBk, INACTIVEBAR_ALPHA);
	}

	if (hwnd == hwndCurCoolSB)
	{
		fMouseDownL = (uDrawFlags == HTSCROLL_LEFT);
		fMouseDownR = (uDrawFlags == HTSCROLL_RIGHT);
	}

	if (NULL == hbrChecked) //recreate pattern brush if needed
	{
		HBITMAP hbmp = CreateBitmap(8, 8, 1, 1, wCheckPat);
		hbrChecked = CreatePatternBrush(hbmp);
		DeleteObject(hbmp);
		if (NULL == hbrChecked) return 0;
	}

	

	HBRUSH  hbrOld = NULL;

	COLORREF rgbFgOld, rgbBkOld;
	rgbFgOld = SetTextColor(hdc, crCheck1);
	rgbBkOld = SetBkColor(hdc, crCheck2);

	//
	// Draw the scrollbar now
	//
	if (scrollwidth > butwidth*2)
	{
		DCCanvas canvas(hdc);
		if (SWS_USEFREEFORM & scrollFlags)
		{
			CopyRect(&ctrl, rect);
			RotateRect0(sb, &ctrl);
			RenderBaseTexture(&canvas, &ctrl, hwnd);
		}
			
		//LEFT ARROW
		SetRect(&ctrl, rect->left, rect->top, rect->left + butwidth, rect->bottom);

		RotateRect0(sb, &ctrl);

		DrawScrollArrow(sb, hdc, &ctrl, uLeftButFlags, fMouseDownL, fMouseOverL, (SWS_USEFREEFORM & scrollFlags));

		RotateRect0(sb, &ctrl);

		//MIDDLE PORTION
		//if we can fit the thumbbar in, then draw it
		if (thumbwidth > 0 && thumbwidth <= workingwidth
		    && IsScrollInfoActive(si) && ((sb->fScrollFlags & ESB_DISABLE_BOTH) != ESB_DISABLE_BOTH))
		{
			SkinBitmap *bg = (scrollBitmaps && (SWS_USEFREEFORM & scrollFlags)) ? 
				((sb->nBarType== SB_VERT) ? scrollBitmaps->v_mid.getBitmap(): scrollBitmaps->h_mid.getBitmap())
				: 0;

			if ((SWS_USEFREEFORM & scrollFlags) && bg && !bg->isInvalid())
			{
				SetRect(&sbm, rect->left + butwidth, rect->top, rect->right-butwidth, rect->bottom);
				RotateRect0(sb, &sbm);

				bg->stretchToRectAlpha(&canvas, &sbm);

				SkinBitmap thumbBitmap(GetThumbID(sb->nBarType, sb->fScrollFlags, fMouseOverThumb));
				SetRect(&sbm, thumbpos, rect->top, thumbpos+thumbwidth, rect->bottom);
				RotateRect0(sb, &sbm);
				if (!thumbBitmap.isInvalid())
					thumbBitmap.stretchToRectAlpha(&canvas, &sbm);
				else
					DrawBlankButton(hdc, &sbm, uDEFlat, (CSBS_TRACKING & sb->fScrollFlags), sb->nBarType == SB_VERT);
			}
			else
			{
				//Draw the scrollbar margin above the thumb
				SetRect(&sbm, rect->left + butwidth, rect->top, thumbpos, rect->bottom);

				RotateRect0(sb, &sbm);

				if (HTSCROLL_PAGELEFT == uDrawFlags)
				{
					SetTextColor(hdc, crInverse1);
					SetBkColor(hdc, crInverse2);
				}
				
				if (GetTextColor(hdc) == GetBkColor(hdc)) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &sbm, NULL, 0, NULL); 
				else 
				{
					if (NULL == hbrOld) hbrOld = SetWindowPatternBrush(hwnd, hdc, sb->nBarType);
					PatBlt(hdc, sbm.left, sbm.top, sbm.right - sbm.left, sbm.bottom - sbm.top, PATCOPY);
				}
				
				if (HTSCROLL_PAGELEFT == uDrawFlags)
				{
					SetTextColor(hdc, crCheck1);
					SetBkColor(hdc, crCheck2);
				}

				RotateRect0(sb, &sbm);

				//Draw the margin below the thumb
				sbm.left = thumbpos + thumbwidth;
				sbm.right = rect->right - butwidth;

				RotateRect0(sb, &sbm);
				
				if (HTSCROLL_PAGERIGHT == uDrawFlags)
				{
					SetTextColor(hdc, crInverse1);
					SetBkColor(hdc, crInverse2);
				}
				
				if (GetTextColor(hdc) == GetBkColor(hdc)) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &sbm, NULL, 0, NULL); 
				else 
				{
					if (NULL == hbrOld) hbrOld = SetWindowPatternBrush(hwnd, hdc, sb->nBarType);
					PatBlt(hdc, sbm.left, sbm.top, sbm.right - sbm.left, sbm.bottom - sbm.top, PATCOPY);
				}
				
				if (HTSCROLL_PAGERIGHT == uDrawFlags)
				{
					SetTextColor(hdc, crCheck1);
					SetBkColor(hdc, crCheck2);
				}

				RotateRect0(sb, &sbm);

				//Draw the THUMB finally
				SetRect(&thumb, thumbpos, rect->top, thumbpos + thumbwidth, rect->bottom);

				RotateRect0(sb, &thumb);

				DrawBlankButton(hdc, &thumb, uDEFlat, (CSBS_TRACKING & sb->fScrollFlags), sb->nBarType == SB_VERT);
				RotateRect0(sb, &thumb);
			}

		}
		//otherwise, just leave that whole area blank
		else
		{
			OffsetRect(&ctrl, butwidth, 0);
			ctrl.right = rect->right - butwidth;

			//if we always show the thumb covering the whole scrollbar,
			//then draw it that way
			if (!IsScrollInfoActive(si)	&& (sb->fScrollFlags & CSBS_THUMBALWAYS)
			    && ctrl.right - ctrl.left > sb->nMinThumbSize)
			{
				//leave a 1-pixel gap between the thumb + right button
				ctrl.right --;
				RotateRect0(sb, &ctrl);

				DrawBlankButton(hdc, &ctrl, uDEFlat, 0, sb->nBarType == SB_VERT);
				RotateRect0(sb, &ctrl);

				//draw the single-line gap
				ctrl.left = ctrl.right;
				ctrl.right += 1;

				RotateRect0(sb, &ctrl);

				PaintRect(hdc, &ctrl, GetSysColor(COLOR_SCROLLBAR));

				RotateRect0(sb, &ctrl);
			}
			//otherwise, paint a blank if the thumb doesn't fit in
			else
			{
				RotateRect0(sb, &ctrl);
				BOOL classic(TRUE);
				if (SWS_USEFREEFORM & scrollFlags)
				{
					SkinBitmap background(sb->nBarType== SB_VERT?L"wasabi.scrollbar.vertical.background.middle":L"wasabi.scrollbar.horizontal.background.middle");
					if (!background.isInvalid()) 
					{	
						background.stretchToRectAlpha(&canvas, &ctrl,  
									((DFCS_INACTIVE & uLeftButFlags) && (DFCS_INACTIVE & uRightButFlags)) ? INACTIVEBAR_ALPHA : 255);
						classic = FALSE;
					}
				}

				if (classic)
				{
					if (crCheck1 == crCheck2) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &ctrl, NULL, 0, NULL); 
					else 
					{
						if (NULL == hbrOld) hbrOld = SetWindowPatternBrush(hwnd, hdc, sb->nBarType);
						PatBlt(hdc, ctrl.left, ctrl.top, ctrl.right - ctrl.left, ctrl.bottom - ctrl.top, PATCOPY);
					}
				}
				

				RotateRect0(sb, &ctrl);
			}
		}

		//RIGHT ARROW
		SetRect(&ctrl, rect->right - butwidth, rect->top, rect->right, rect->bottom);

		RotateRect0(sb, &ctrl);

		DrawScrollArrow(sb, hdc, &ctrl, uRightButFlags, fMouseDownR, fMouseOverR, (SWS_USEFREEFORM & scrollFlags));

		RotateRect0(sb, &ctrl);
	}
	//not enough room for the scrollbar, so just draw the buttons (scaled in size to fit)
	else
	{
		butwidth = scrollwidth / 2;

		//LEFT ARROW
		SetRect(&ctrl, rect->left, rect->top, rect->left + butwidth, rect->bottom);

		RotateRect0(sb, &ctrl);
		DrawScrollArrow(sb, hdc, &ctrl, uLeftButFlags, fMouseDownL, fMouseOverL, (SWS_USEFREEFORM & scrollFlags));
		RotateRect0(sb, &ctrl);

		//RIGHT ARROW
		OffsetRect(&ctrl, scrollwidth - butwidth, 0);

		RotateRect0(sb, &ctrl);
		DrawScrollArrow(sb, hdc, &ctrl, uRightButFlags, fMouseDownR, fMouseOverR, (SWS_USEFREEFORM & scrollFlags));
		RotateRect0(sb, &ctrl);

		//if there is a gap between the buttons, fill it with a solid color
		//if(butwidth & 0x0001)
		if (ctrl.left != rect->left + butwidth)
		{
			ctrl.left --;
			ctrl.right -= butwidth;
			RotateRect0(sb, &ctrl);
			BOOL classic(TRUE);

			if (SWS_USEFREEFORM & scrollFlags)
			{
				SkinBitmap background(sb->nBarType== SB_VERT?L"wasabi.scrollbar.vertical.background.middle":L"wasabi.scrollbar.horizontal.background.middle");
				if (!background.isInvalid())
				{
					DCCanvas canvas(hdc);
					RenderBaseTexture(&canvas, &ctrl, hwnd);
					background.stretchToRectAlpha(&canvas, &ctrl, 
							((DFCS_INACTIVE & uLeftButFlags) && (DFCS_INACTIVE & uRightButFlags)) ? INACTIVEBAR_ALPHA : 255);
					classic = FALSE;
				}
			}

			if (classic)
			{
				if (crCheck1 == crCheck2) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &ctrl, NULL, 0, NULL); 
				else 
				{
					if (NULL == hbrOld) hbrOld = SetWindowPatternBrush(hwnd, hdc, sb->nBarType);
					PatBlt(hdc, ctrl.left, ctrl.top, ctrl.right - ctrl.left, ctrl.bottom - ctrl.top, PATCOPY);
				}
			}

			RotateRect0(sb, &ctrl);
		}

	}

	SetBkColor(hdc, rgbBkOld);
	SetTextColor(hdc, rgbFgOld);
	if (hbrOld)
	{
		SelectObject(hdc, hbrOld);
		SetBrushOrgEx(hdc, 0, 0, NULL);
	}
	return 0;
}

//
//	Draw a vertical scrollbar using the horizontal draw routine, but
//	with the coordinates adjusted accordingly
//
static LRESULT NCDrawVScrollbar(SCROLLBAR *sb, HWND hwnd, HDC hdc, const RECT *rect, UINT uDrawFlags, UINT hoverFlags, DWORD scrollFlags)
{
	LRESULT ret;
	RECT rc;

	rc = *rect;
	RotateRect(&rc);
	ret = NCDrawHScrollbar(sb, hwnd, hdc, &rc, uDrawFlags, hoverFlags, scrollFlags);
	RotateRect(&rc);

	return ret;
}

//
//	Generic wrapper function for the scrollbar drawing
//
static LRESULT NCDrawScrollbar(SCROLLBAR *sb, HWND hwnd, HDC hdc, const RECT *rect, UINT uDrawFlags, UINT hoverFlags, DWORD scrollFlags)
{
	if (sb->nBarType == SB_HORZ)
		return NCDrawHScrollbar(sb, hwnd, hdc, rect, uDrawFlags, hoverFlags, scrollFlags);
	else
		return NCDrawVScrollbar(sb, hwnd, hdc, rect, uDrawFlags, hoverFlags, scrollFlags);
}



void SkinnedScrollWnd::PaintNonClient(HDC hdc)
{
	RECT winrect, rcH, rcV;
	BOOL drawH = FALSE, drawV = FALSE;

	if (!psbHorz->fScrollVisible && !psbVert->fScrollVisible)
	{
		DrawBorder(hdc);
		return;
	}
	
	if (0 == (SWS_UPDATEFRAME & scrollFlags))
	{
		HWND hwndActive;
		hwndActive = GetActiveWindow();
		if (hwndActive != hwnd && !IsChild(hwndActive, hwnd)) scrollFlags |= SWS_UPDATEFRAME;
	}

	GetWindowRect(hwnd, &winrect);


	if (psbHorz->fScrollVisible)
	{
		GetHScrollRect(this, &rcH);
		OffsetRect(&rcH, -winrect.left, -winrect.top);
		if (rcH.right > rcH.left && rcH.bottom > rcH.top && RectVisible(hdc, &rcH)) drawH = TRUE;
	}

	if (psbVert->fScrollVisible)
	{
		GetVScrollRect(this, &rcV);
		OffsetRect(&rcV, -winrect.left, -winrect.top);
		
		if (rcV.right > rcV.left && rcV.bottom > rcV.top && RectVisible(hdc, &rcV)) drawV = TRUE;
	}
	DrawBorder(hdc);
	
	POINT ptOrg;
	GetViewportOrgEx(hdc, &ptOrg);

	if (drawH)
	{		
        UINT fDraw, fHover;
		if (uCurrentScrollbar == SB_HORZ) { fDraw = uScrollTimerPortion; fHover = HTSCROLL_NONE; }
		else 
		{ 
			fDraw = HTSCROLL_NONE; 
			fHover = (NULL != psbHorz && 0 != (CSBS_HOVERING & psbHorz->fScrollFlags)) ? scrollPortionHover : HTSCROLL_NONE;
		}

		if (SWS_USEFREEFORM & scrollFlags)
		{
			DCBltCanvas buffer;
			buffer.cloneDC(hdc, &rcH);
			NCDrawHScrollbar(psbHorz, hwnd, buffer.getHDC(), &rcH, fDraw, fHover, scrollFlags);
		}
		else
		{				
			SetViewportOrgEx(hdc, ptOrg.x + rcH.left, ptOrg.y + rcH.top, NULL);	
			OffsetRect(&rcH, -rcH.left, -rcH.top);
			NCDrawHScrollbar(psbHorz, hwnd, hdc, &rcH, fDraw, fHover, scrollFlags);
			SetViewportOrgEx(hdc, ptOrg.x, ptOrg.y, NULL);
		}
	}

	if (drawV)
	{					
		UINT fDraw, fHover;
		if (uCurrentScrollbar == SB_VERT) { fDraw = uScrollTimerPortion; fHover = HTSCROLL_NONE; }
		else 
		{ 
			fDraw = HTSCROLL_NONE; 
			fHover = (NULL != psbVert && 0 != (CSBS_HOVERING & psbVert->fScrollFlags)) ? scrollPortionHover : HTSCROLL_NONE; 
		}

		if (SWS_USEFREEFORM & scrollFlags)
		{
			DCBltCanvas buffer;
			buffer.cloneDC(hdc, &rcV);
			NCDrawVScrollbar(psbVert, hwnd, buffer.getHDC(), &rcV, fDraw, fHover, scrollFlags);
			
		}
		else
		{				
			SetViewportOrgEx(hdc, ptOrg.x + rcV.left, ptOrg.y + rcV.top, NULL);	
			OffsetRect(&rcV, -rcV.left, -rcV.top);
			NCDrawVScrollbar(psbVert, hwnd, hdc, &rcV, fDraw, fHover, scrollFlags);
			SetViewportOrgEx(hdc, ptOrg.x, ptOrg.y, NULL);
		}
		
	}

	SetViewportOrgEx(hdc, ptOrg.x, ptOrg.y, NULL);

	// DRAW THE DEAD AREA
	// only do this if the horizontal and vertical bars are visible
	if (psbHorz->fScrollVisible && psbVert->fScrollVisible)
	{
		GetClientRect(hwnd, &rcH);
		MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&rcH, 2);
		OffsetRect(&rcH, -winrect.left, -winrect.top);

		rcH.top  = rcH.bottom;
		rcH.bottom += GetScrollMetric(psbHorz, SM_CYHORZSB, scrollFlags);

		if (SWS_LEFT & scrollFlags)
		{
			rcH.right = rcH.left;
			rcH.left -= GetScrollMetric(psbVert, SM_CXVERTSB, scrollFlags);
		}
		else
		{
			rcH.left = rcH.right;
			rcH.right += GetScrollMetric(psbVert, SM_CXVERTSB, scrollFlags);
		}

		if (RectVisible(hdc, &rcH))
		{
			
			PaintRect(hdc, &rcH, WADlg_getColor(WADLG_SCROLLBAR_DEADAREA_COLOR));
		}
	}

	
}

void SkinnedScrollWnd::OnNcPaint(HRGN rgnUpdate)
{		
	UINT flags = DCX_PARENTCLIP | DCX_CACHE | DCX_WINDOW | DCX_CLIPSIBLINGS |
				DCX_INTERSECTUPDATE | DCX_VALIDATE;
	
	HDC hdc = GetDCEx(hwnd, ((HRGN)NULLREGION != rgnUpdate) ? rgnUpdate : NULL, flags);
		

	if (NULL == hdc) 
	{
		return;
	}

		
	PaintNonClient(hdc);
	
	
	ReleaseDC(hwnd, hdc);
}

//
// Need to detect if we have clicked in the scrollbar region or not
//
INT SkinnedScrollWnd::OnNcHitTest(POINTS pts)
{
	RECT rc;
	
	INT r = __super::OnNcHitTest(pts);
	if (r == HTTRANSPARENT) 
	{		
		
		
		return r;
	}

	if (psbHorz->fScrollVisible && GetHScrollRect(this, &rc) &&
	    pts.x >= rc.left && pts.x <= rc.right && pts.y >= rc.top && pts.y <= rc.bottom) return HTHSCROLL;
	if (psbVert->fScrollVisible && GetVScrollRect(this, &rc) &&
	    pts.x >= rc.left && pts.x <= rc.right && pts.y >= rc.top && pts.y <= rc.bottom) return HTVSCROLL;

	return r;
}

//
//	Return a HT* value indicating what part of the scrollbar was clicked
//	Rectangle is not adjusted
//
static UINT GetHorzPortion(SCROLLBAR *sb, HWND hwnd, RECT *rect, int x, int y, DWORD scrollFlags)
{
	RECT rc = *rect;

	if (y < rc.top || y >= rc.bottom) return HTSCROLL_NONE;

	//Now we have the rectangle for the scrollbar itself, so work out
	//what part we clicked on.
	return GetHorzScrollPortion(sb, hwnd, &rc, x, y, scrollFlags);
}

//
//	Just call the horizontal version, with adjusted coordinates
//
static UINT GetVertPortion(SCROLLBAR *sb, HWND hwnd, RECT *rect, int x, int y, DWORD scrollFlags)
{
	UINT ret;
	RotateRect(rect);
	ret = GetHorzPortion(sb, hwnd, rect, y, x, scrollFlags);
	RotateRect(rect);
	return ret;
}

//
//	Wrapper function for GetHorzPortion and GetVertPortion
//
static UINT GetPortion(SCROLLBAR *sb, HWND hwnd, RECT *rect, int x, int y, DWORD scrollFlags)
{
	if (sb->nBarType == SB_HORZ) return GetHorzPortion(sb, hwnd, rect, x, y, scrollFlags);
	else if (sb->nBarType == SB_VERT) return GetVertPortion(sb, hwnd, rect, x, y, scrollFlags);
	return HTSCROLL_NONE;
}

//
//	Input: rectangle of the total scrollbar area
//	Output: adjusted to take the inserted buttons into account
//
static void GetRealHorzScrollRect(SCROLLBAR *sb, RECT *rect)
{
	if (CSBS_BTNVISBEFORE & sb->fScrollFlags) rect->left += sb->nButSizeBefore;
	if (CSBS_BTNVISAFTER & sb->fScrollFlags)  rect->right -= sb->nButSizeAfter;
}

//
//	Input: rectangle of the total scrollbar area
//	Output: adjusted to take the inserted buttons into account
//
static void GetRealVertScrollRect(SCROLLBAR *sb, RECT *rect)
{
	if (CSBS_BTNVISBEFORE & sb->fScrollFlags) rect->top += sb->nButSizeBefore;
	if (CSBS_BTNVISAFTER & sb->fScrollFlags)  rect->bottom -= sb->nButSizeAfter;
}

//
//	Decide which type of scrollbar we have before calling
//  the real function to do the job
//
static void GetRealScrollRect(SCROLLBAR *sb, RECT *rect, DWORD scrollFlags)
{
	if (sb->nBarType == SB_HORZ)
	{
		GetRealHorzScrollRect(sb, rect);
	}
	else if (sb->nBarType == SB_VERT)
	{
		GetRealVertScrollRect(sb, rect);
	}
}

//
//	Left button click in the non-client area
//
void SkinnedScrollWnd::OnNcLButtonDown(UINT nHitTest, POINTS pts)
{
	RECT rect, winrect;
	SCROLLBAR *psb;

	hwndCurCoolSB = hwnd;

	//
	//	HORIZONTAL SCROLLBAR PROCESSING
	//
	if (HTHSCROLL == nHitTest)
	{
		psb = psbHorz;
		uScrollTimerMsg = WM_HSCROLL;
		uCurrentScrollbar = SB_HORZ;
		//get the total area of the normal Horz scrollbar area
		GetHScrollRect(this, &rect);
		uCurrentScrollPortion = GetHorzPortion(psbHorz, hwnd, &rect, pts.x, pts.y, scrollFlags);
	}
	//
	//	VERTICAL SCROLLBAR PROCESSING
	//
	else if (HTVSCROLL== nHitTest)
	{
		psb = psbVert;
		uScrollTimerMsg = WM_VSCROLL;
		uCurrentScrollbar = SB_VERT;
		//get the total area of the normal Horz scrollbar area
		GetVScrollRect(this, &rect);
		uCurrentScrollPortion = GetVertPortion(psbVert, hwnd, &rect, pts.x, pts.y, scrollFlags);
	}
	//
	//	NORMAL PROCESSING
	//
	else
	{
		uCurrentScrollPortion = HTSCROLL_NONE;
		__super::WindowProc(WM_NCLBUTTONDOWN, (WPARAM)nHitTest, *(LPARAM*)&pts);
		return;
	}

	//
	// we can now share the same code for vertical
	// and horizontal scrollbars
	//
	switch (uCurrentScrollPortion)
	{
		//inserted buttons to the left/right
	case HTSCROLL_THUMB:

		//if the scrollbar is disabled, then do no further processing
		if (!IsScrollbarActive(psb)) return;

		GetRealScrollRect(psb, &rect, scrollFlags);
		RotateRect0(psb, &rect);
		CalcThumbSize(psb, &rect, &nThumbSize, &nThumbPos, scrollFlags);
		RotateRect0(psb, &rect);

		//remember the bounding rectangle of the scrollbar work area
		rcThumbBounds = rect;

		trackThumbPos=-1;
		psb->fScrollFlags |= CSBS_TRACKING;
		psb->scrollInfo.nTrackPos = psb->scrollInfo.nPos;

		if (nHitTest == HTVSCROLL) nThumbMouseOffset = pts.y - nThumbPos;
		else nThumbMouseOffset = pts.x - nThumbPos;

		nLastPos = psb->scrollInfo.nPos;
		nThumbPos0 = nThumbPos;

		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_POS;
		info.nPos = nLastPos;

		SetScrollInfo(hwnd, psb->nBarType, &info, FALSE);

		SendScrollMessage(hwnd, uScrollTimerMsg, SB_THUMBTRACK, nLastPos);
		//if(sb->fFlatScrollbar)
		//{
		GetWindowRect(hwnd, &winrect);
		OffsetRect(&rect, -winrect.left, -winrect.top);
		InvalidateNC(InvalidateFlag_Normal, uCurrentScrollbar);
		//}

		break;

		//Any part of the scrollbar
	case HTSCROLL_LEFT:
		if (psb->fScrollFlags & ESB_DISABLE_LEFT) return;
		goto target1;

	case HTSCROLL_RIGHT:
		if (psb->fScrollFlags & ESB_DISABLE_RIGHT) return;
		goto target1;


	case HTSCROLL_PAGELEFT:
	case HTSCROLL_PAGERIGHT:

target1:

		//if the scrollbar is disabled, then do no further processing
		if (!IsScrollbarActive(psb))
			break;

		//ajust the horizontal rectangle to NOT include
		//any inserted buttons
		GetRealScrollRect(psb, &rect, scrollFlags);

		SendScrollMessage(hwnd, uScrollTimerMsg, uCurrentScrollPortion, 0);

		// Check what area the mouse is now over :
		// If the scroll thumb has moved under the mouse in response to
		// a call to SetScrollPos etc, then we don't hilight the scrollbar margin
		if (uCurrentScrollbar == SB_HORZ)
			uScrollTimerPortion = GetHorzScrollPortion(psb, hwnd, &rect, pts.x, pts.y, scrollFlags);
		else
			uScrollTimerPortion = GetVertScrollPortion(psb, hwnd, &rect, pts.x, pts.y, scrollFlags);

		GetWindowRect(hwnd, &winrect);
		OffsetRect(&rect, -winrect.left, -winrect.top);


		//if we aren't hot-tracking, then don't highlight
		//the scrollbar thumb unless we click on it
		if (uScrollTimerPortion == HTSCROLL_THUMB) uScrollTimerPortion = HTSCROLL_NONE;

		InvalidateNC(InvalidateFlag_Normal, uCurrentScrollbar);

		//Post the scroll message!!!!
		uScrollTimerPortion = uCurrentScrollPortion;

		//set a timer going on the first click.
		//if this one expires, then we can start off a more regular timer
		//to generate the auto-scroll behaviour
		uScrollTimerId = SetTimer(hwnd, COOLSB_TIMERID1, COOLSB_TIMERINTERVAL1, 0);
		UpdateScrollBars(FALSE);
		break;
	default:
		__super::WindowProc(WM_NCLBUTTONDOWN, (WPARAM)nHitTest, *(LPARAM*)&pts);
		return;

	}

	if ((0 == (SWS_COMBOLBOX & scrollFlags)) && hwnd != GetCapture()) 
	{		
		ignoreCaptureChange = TRUE;
		SetCapture(hwnd);
		ignoreCaptureChange = FALSE;
		captureSet = TRUE;
		
	}
}

//
//	Left button released
//
void SkinnedScrollWnd::Emulate_LeftButtonUp(UINT nFlags, POINTS pts, BOOL forwardMessage)
{
	//current scrollportion is the button that we clicked down on
	if (uCurrentScrollPortion != HTSCROLL_NONE)
	{
		RECT rect;
		//UINT thisportion;
		POINT pt;
		RECT winrect;

		SCROLLBAR *psb;

		if (captureSet && (0 == (SWS_COMBOLBOX & scrollFlags)) && hwnd == GetCapture()) 
		{
			ignoreCaptureChange = TRUE;
			ReleaseCapture();
			ignoreCaptureChange = FALSE;
		}
		captureSet = FALSE;

		GetWindowRect(hwnd, &winrect);
		POINTSTOPOINT(pt, pts);

		//emulate the mouse input on a scrollbar here...
		if (SB_VERT == uCurrentScrollbar)
		{
			//get the total area of the normal Horz scrollbar area
			psb = psbVert;
			GetVScrollRect(this, &rect);
		}
		else
		{
			//get the total area of the normal Horz scrollbar area
			psb = psbHorz;
			GetHScrollRect(this, &rect);
		}

		//we need to do different things depending on if the
		//user is activating the scrollbar itself, or one of
		//the inserted buttons
		switch (uCurrentScrollPortion)
		{
			//The scrollbar is active
		case HTSCROLL_LEFT:
		case HTSCROLL_RIGHT:
		case HTSCROLL_PAGELEFT:
		case HTSCROLL_PAGERIGHT:
		case HTSCROLL_NONE:
			KillTimer(hwnd, uScrollTimerId);
		case HTSCROLL_THUMB:
			UpdateScrollBars(FALSE);

			//In case we were thumb tracking, make sure we stop NOW
			if (CSBS_TRACKING & psb->fScrollFlags)
			{
				SCROLLINFO info;
				info.cbSize = sizeof(SCROLLINFO);
				info.fMask = SIF_POS;
				info.nPos = nLastPos;

				SetScrollInfo(hwnd, psb->nBarType, &info, FALSE);
				SendScrollMessage(hwnd, uScrollTimerMsg, SB_THUMBPOSITION, nLastPos);
				psb->fScrollFlags &= ~CSBS_TRACKING;
			}

			//send the SB_ENDSCROLL message now that scrolling has finished
			SendScrollMessage(hwnd, uScrollTimerMsg, SB_ENDSCROLL, 0);

			//adjust the total scroll area to become where the scrollbar
			//really is (take into account the inserted buttons)
			GetRealScrollRect(psb, &rect, scrollFlags);
			OffsetRect(&rect, -winrect.left, -winrect.top);
			InvalidateNC(InvalidateFlag_Normal, uCurrentScrollbar);
			break;
		}

		//reset our state to default
		uCurrentScrollPortion = HTSCROLL_NONE;
		uScrollTimerPortion	  = HTSCROLL_NONE;
		uScrollTimerId		  = 0;

		uScrollTimerMsg       = 0;
		uCurrentScrollbar     = COOLSB_NONE;

		return;
	}
	else
	{
	
		/*
		// Can't remember why I did this!
		if(GetCapture() == hwnd)
		{
			ReleaseCapture();
		}*/
	}

	//sw->update();

	if (FALSE != forwardMessage)
	{
		__super::WindowProc(WM_LBUTTONUP, (WPARAM)nFlags, *(LPARAM*)&pts);
	}
}
void SkinnedScrollWnd::OnLButtonUp(UINT nFlags, POINTS pts)
{
	Emulate_LeftButtonUp(nFlags, pts, TRUE);
}

static int
ListView_ScrollWindow(HWND hwnd, int dy)
{
	RECT rect;

	if (0 == dy)
		return NULLREGION;
	
	if (FALSE == GetClientRect(hwnd, &rect))
		return ERROR;
	
	if (0 == (LVS_NOCOLUMNHEADER & GetWindowLongPtrW(hwnd, GWL_STYLE)))
	{
		HWND headerWindow;
		headerWindow = (HWND)SendMessageW(hwnd, LVM_GETHEADER, 0, 0L);
		if (NULL != headerWindow && 
			0 != (WS_VISIBLE & GetWindowLongPtrW(headerWindow, GWL_STYLE)))
		{
			HDLAYOUT headerLayout;
			WINDOWPOS headerPos;

			headerLayout.prc = &rect;
			headerLayout.pwpos = &headerPos;
			SendMessageW(headerWindow, HDM_LAYOUT, 0, (LPARAM)&headerLayout);
		}
	}

	return ScrollWindowEx(hwnd, 0, dy, &rect, &rect, NULL, NULL, SW_INVALIDATE);
}

static BOOL ListView_ScrollReportModeVert(HWND hwnd, INT linesVert, BOOL horzBarHidden)
{
	int max, pos, page;
	int itemHeight, prevPos, dy;
	RECT rect;
	unsigned long windowStyle;
	
	if (0 == linesVert)
		return TRUE;
	
	windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);

	pos = (int)SendMessageW(hwnd, LVM_GETTOPINDEX, 0, 0L);
	max = (int)SendMessageW(hwnd, LVM_GETITEMCOUNT, 0, 0L);
	page = (int)SendMessageW(hwnd, LVM_GETCOUNTPERPAGE, 0, 0L);

	if (FALSE == horzBarHidden)
		max++;
	
	if ((linesVert < 0  && pos <= 0) ||
	   (linesVert > 0 && (pos + page) >= max))
	{
		return TRUE;
	}

	if (linesVert < 0 && (pos + linesVert) < 0)
		linesVert = -pos;
	else if (linesVert > 0 && (pos + page + linesVert) > max)
		linesVert = max - (page + pos);
	
	rect.left = LVIR_BOUNDS;
	if (!SendMessageW(hwnd, LVM_GETITEMRECT, 0, (LPARAM)&rect))
		return FALSE;

	if (rect.top < 0) 
		OffsetRect(&rect, 0, -rect.top);

    itemHeight = rect.bottom - rect.top;

	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);
				
	dy = linesVert * itemHeight;
	SendMessageW(hwnd, LVM_SCROLL, 0, dy);
				
	if (0 == (WS_VISIBLE & windowStyle))
		return TRUE;

	SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle);

	prevPos = pos;
	pos = (int)SendMessageW(hwnd, LVM_GETTOPINDEX, 0, 0L);
	linesVert = pos - prevPos;
	
	dy = linesVert * itemHeight;

	if (ERROR == ListView_ScrollWindow(hwnd, -dy))
		InvalidateRect(hwnd, NULL, FALSE);
	
	return TRUE;
}

static BOOL
ListView_ScrollReportModeVertPx(HWND hwnd, int dy, BOOL horzBarHidden)
{
	int itemHeight, lines;
	RECT rect;
	unsigned long windowStyle;

	if (0 == dy)
		return TRUE;

	windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);

	rect.left = LVIR_BOUNDS;
	if (!SendMessageW(hwnd, LVM_GETITEMRECT, 0, (LPARAM)&rect))
		return FALSE;

	if (rect.top < 0) 
		OffsetRect(&rect, 0, -rect.top);

    itemHeight = rect.bottom - rect.top;


	lines = dy / itemHeight;
	if (0 != lines)
	{
		if (0 != (WS_VISIBLE & windowStyle))
			SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

		SendMessageW(hwnd, LVM_SCROLL, 0, lines);
				
		if (0 != (WS_VISIBLE & windowStyle))
		{
			windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
			windowStyle |= WS_VISIBLE;
			SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle);
		}
	}

	if (0 != (WS_VISIBLE & windowStyle))
	{		
		if (ERROR == ListView_ScrollWindow(hwnd, -dy))
			InvalidateRect(hwnd, NULL, FALSE);
	}
	
	return TRUE;
}

//
//	This function is called whenever the mouse is moved and
//  we are dragging the scrollbar thumb about.
//
static void ThumbTrack(SCROLLBAR *sbar, HWND hwnd, POINTS pts, UINT scrollFlags)
{
	POINT pt;
	RECT rc, winrect, rc2;
	int thumbpos = nThumbPos;
	//int thumbDelta;
	int pos;
	int siMaxMin = 0;
	//UINT flatflag = (CSBS_FLATSB & sbar->fScrollFlags) ? BF_FLAT : 0;

	SCROLLINFO *si;
	si = &sbar->scrollInfo;

	POINTSTOPOINT(pt, pts);
	MapWindowPoints(hwnd, HWND_DESKTOP, &pt, 1);

	if (SB_VERT == sbar->nBarType)
	{
		LONG t;
		t= pt.x; pt.x = pt.y; pt.y = t;
		RotateRect(&rcThumbBounds);
	}

	//draw the thumb at whatever position
	rc = rcThumbBounds;

	SetRect(&rc2, rc.left - THUMBTRACK_SNAPDIST*2, rc.top - THUMBTRACK_SNAPDIST,
	        rc.right + THUMBTRACK_SNAPDIST*2, rc.bottom + THUMBTRACK_SNAPDIST);

	int cxH =  GetScrollMetric(sbar, SM_CXHORZSB, scrollFlags);

	rc.left += cxH;
	rc.right -= cxH;

	//if the mouse is not in a suitable distance of the scrollbar,
	//then "snap" the thumb back to its initial position
#ifdef SNAP_THUMB_BACK
	if (!PtInRect(&rc2, pt))
	{
		thumbpos = nThumbPos0;
	}
	//otherwise, move the thumb to where the mouse is
	else
#endif //SNAP_THUMB_BACK
	{
		//keep the thumb within the scrollbar limits
		thumbpos = pt.x - nThumbMouseOffset;
		if (thumbpos < rc.left) thumbpos = rc.left;
		if (thumbpos > rc.right - nThumbSize) thumbpos = rc.right - nThumbSize;
	}

	GetClientRect(hwnd, &winrect);

	MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&winrect, 2);

	RotateRect0(sbar, &winrect);

	OffsetRect(&rc, -winrect.left, -winrect.top);
	thumbpos -= winrect.left;

	/*if (-1 == trackThumbPos)
		thumbDelta = thumbpos - rc.left;
	else
		thumbDelta = thumbpos - trackThumbPos;*/

	trackThumbPos = thumbpos;

	//post a SB_TRACKPOS message!!!
	siMaxMin = si->nMax - si->nMin;
	pos = (siMaxMin > 0) ? MulDiv(thumbpos - rc.left, siMaxMin - si->nPage + 1, rc.right - rc.left - nThumbSize) : (thumbpos - rc.left);

	if (si->nPage == 0)
		pos = 0; // this supposed to protect from moving on empty scrollbar

	if (pos != nLastPos)
	{
		if (SWS_LISTVIEW & scrollFlags) // list view specific
		{
			// only for listviews
			if (sbar->nBarType == SB_HORZ)
			{
				SCROLLINFO info;
				info.cbSize = sizeof(SCROLLINFO);
				info.fMask = SIF_TRACKPOS;
				if (GetScrollInfo(hwnd, SB_HORZ, &info))
				{
					INT dx = pos - info.nTrackPos;
					if (LVS_LIST == (LVS_TYPEMASK & GetWindowLongPtrW(hwnd, GWL_STYLE)))
					{
						INT cw = (INT)(INT_PTR)SendMessageW(hwnd, LVM_GETCOLUMNWIDTH, 0, 0L);
						dx = dx * cw;
					}
					SendMessageW(hwnd, LVM_SCROLL, dx, 0);
				}
			}
			else if (sbar->nBarType == SB_VERT)
			{
				SCROLLINFO info;
				info.cbSize = sizeof(SCROLLINFO);
				info.fMask = SIF_TRACKPOS;
				if (GetScrollInfo(hwnd, SB_VERT, &info) && pos != info.nTrackPos)
				{
					INT dy = pos - info.nTrackPos;
					if (LVS_REPORT == (LVS_TYPEMASK & GetWindowLongPtrW(hwnd, GWL_STYLE)))
					{
						ListView_ScrollReportModeVert(hwnd, dy, (0 != (SWS_HIDEHSCROLL & scrollFlags)));
					}
					else
					{
						SendMessageW(hwnd, LVM_SCROLL, 0, dy);
					}
				}
			}
		}
		else if ((SWS_TREEVIEW & scrollFlags) && 
				  SB_VERT == sbar->nBarType &&
				  ABS(nLastPos - pos) < 2)
		{
			INT i, cmd;
			i = nLastPos - pos;
			cmd = (i < 0) ? SB_LINEDOWN : SB_LINEUP;
			if (i < 0) i = -i;
			while (i--) 
			{
				SendMessageW(hwnd, WM_VSCROLL, cmd, 0L);
			}
		}
		else
		{
			si->nTrackPos = pos;
			SCROLLINFO info;
			info.cbSize = sizeof(SCROLLINFO);
			info.fMask = SIF_TRACKPOS|SIF_POS;
			info.nTrackPos = pos;
			info.nPos = pos;

			SetScrollInfo(hwnd, sbar->nBarType, &info, FALSE);
			SendScrollMessage(hwnd, uScrollTimerMsg, SB_THUMBTRACK, pos);
		}
	}

	nLastPos = pos;

	if (SB_VERT == sbar->nBarType) RotateRect(&rcThumbBounds);
}

//
//	remember to rotate the thumb bounds rectangle!!
//

//
//	Called when we have set the capture from the NCLButtonDown(...)
//
void SkinnedScrollWnd::OnMouseMove(UINT nFlags, POINTS pts)
{
	RECT rect;
	//static UINT lastbutton = 0;
	RECT winrect;
	//UINT buttonIdx = 0;
	SCROLLBAR *psb;

	if (nFlags)
	{
		if (MK_LBUTTON & nFlags)
		{
			UpdateScrollBars(TRUE);
		}
	}
	psb = (uCurrentScrollbar == SB_VERT) ? psbVert : psbHorz;
	if (CSBS_TRACKING & psb->fScrollFlags)
	{
		ThumbTrack(psb, hwnd, pts, scrollFlags);
		InvalidateNC(InvalidateFlag_Normal, uCurrentScrollbar);
		return;
	}
	if (uCurrentScrollPortion == HTSCROLL_NONE)
	{
		__super::WindowProc(WM_MOUSEMOVE, (WPARAM)nFlags, *(LPARAM*)&pts);
		return;
	}
	else
	{
		static UINT lastportion = 0;
		POINT pt;

		POINTSTOPOINT(pt, pts);

		MapWindowPoints(hwnd, HWND_DESKTOP, &pt, 1);

		GetWindowRect(hwnd, &winrect);

		//get the total area of the normal scrollbar area
		GetScrollRect(this, psb->nBarType, &rect);

		//see if we clicked in the inserted buttons / normal scrollbar
		//thisportion = GetPortion(sb, hwnd, &rect, LOWORD(lParam), HIWORD(lParam));
		UINT thisportion = GetPortion(psb, hwnd, &rect, pt.x, pt.y, scrollFlags);

		//we need to do different things depending on if the
		//user is activating the scrollbar itself, or one of
		//the inserted buttons
		switch (uCurrentScrollPortion)
		{
			//The scrollbar is active
		case HTSCROLL_LEFT:
		case HTSCROLL_RIGHT:
		case HTSCROLL_THUMB:
		case HTSCROLL_PAGELEFT:
		case HTSCROLL_PAGERIGHT:
		case HTSCROLL_NONE:

			//adjust the total scroll area to become where the scrollbar
			//really is (take into account the inserted buttons)
			GetRealScrollRect(psb, &rect, scrollFlags);

			OffsetRect(&rect, -winrect.left, -winrect.top);

			if (thisportion != uCurrentScrollPortion)
			{
				uScrollTimerPortion = HTSCROLL_NONE;
				if (lastportion != thisportion)
				{
					InvalidateNC(InvalidateFlag_Normal, uCurrentScrollbar);
				}
			}
			//otherwise, draw the button in its depressed / clicked state
			else
			{
				uScrollTimerPortion = uCurrentScrollPortion;
				if (lastportion != thisportion)
				{
					InvalidateNC(InvalidateFlag_Normal, uCurrentScrollbar);
				}
			}
			break;
		}

		lastportion = thisportion;
		//lastbutton  = buttonIdx;

		//must return zero here, because we might get cursor anomilies
		//CallWindowProc(sw->oldproc, hwnd, WM_MOUSEMOVE, wParam, lParam);

		return;
	}
}

//
//	We must allocate from in the non-client area for our scrollbars
//	Call the default window procedure first, to get the borders (if any)
//	allocated some space, then allocate the space for the scrollbars
//	if they fit
//
INT SkinnedScrollWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *pncsp)
{
	RECT *prc;
	INT hcy, vcx, result;
	BOOL bSizingDown;
	prc = &pncsp->rgrc[0];
	UINT updateBars = -1;
	
	hcy = GetScrollMetric(psbHorz, SM_CYHORZSB, scrollFlags);
	vcx = GetScrollMetric(psbVert, SM_CXVERTSB, scrollFlags);


	if (SWS_UPDATEFRAME & scrollFlags)
	{
		// need to reset style
		DWORD style;

		scrollFlags &= ~SWS_UPDATEFRAME;
		style = (DWORD)GetWindowLongPtrW(hwnd, GWL_STYLE);
		if ((WS_HSCROLL | WS_VSCROLL) & style) SetWindowLongPtrW(hwnd, GWL_STYLE, style & ~(WS_HSCROLL | WS_VSCROLL));
		CallDefWndProc(WM_NCCALCSIZE, (WPARAM)bCalcValidRects, (LPARAM)pncsp);
		if ((WS_HSCROLL | WS_VSCROLL) & style) SetWindowLongPtrW(hwnd, GWL_STYLE, style);
	}

	result = __super::OnNcCalcSize(bCalcValidRects, pncsp);
	bSizingDown = (bCalcValidRects && 
	               ((pncsp->rgrc[0].right - pncsp->rgrc[0].left) < (pncsp->rgrc[1].right - pncsp->rgrc[1].left) ||
	                (pncsp->rgrc[0].bottom - pncsp->rgrc[0].top) < (pncsp->rgrc[1].bottom - pncsp->rgrc[1].top)));


	//if there is room, allocate some space for the horizontal scrollbar
	//NOTE: Change the ">" to a ">=" to make the horz bar totally fill the
	//window before disappearing
	if ((psbHorz->fScrollFlags & CSBS_VISIBLE) && (prc->bottom - prc->top)
#ifdef COOLSB_FILLWINDOW
	    >=
#else
	    >
#endif
	    hcy)
	{
		prc->bottom -= hcy;
		if (TRUE != psbHorz->fScrollVisible)
		{
			psbHorz->fScrollVisible = TRUE;
			updateBars = SB_HORZ;
		}
	}
	else
	{
		if (FALSE != psbHorz->fScrollVisible)
		{
			psbHorz->fScrollVisible = FALSE; 
			updateBars = SB_HORZ;
		}
	}

	//if there is room, allocate some space for the vertical scrollbar
	if ((psbVert->fScrollFlags & CSBS_VISIBLE) && (prc->right - prc->left) >= vcx)
	{
		if (SWS_LEFT & scrollFlags) 	prc->left += vcx;
		else prc->right -= vcx;
		if (TRUE != psbVert->fScrollVisible)
		{
			psbVert->fScrollVisible = TRUE;
			updateBars = (SB_HORZ == updateBars) ? SB_BOTH : SB_VERT;
		}

	}
	else
	{
		if (FALSE != psbVert->fScrollVisible)
		{
			psbVert->fScrollVisible = FALSE; 
			updateBars = (SB_HORZ == updateBars) ? SB_BOTH : SB_VERT;
		}
	}

	if (-1 != updateBars)
	{		
		if (SWS_COMBOLBOX & scrollFlags)
		{
			InvalidateNC(InvalidateFlag_RedrawNow, updateBars);
		}
		else if (bSizingDown) 
		{
			PostMessageW(hwnd, WM_ML_IPC, TRUE, IPC_ML_SKINNEDSCROLLWND_UPDATEBARS);
		}
			
	}

	return result;
}

void SkinnedScrollWnd::OnNcMouseLeave()
{
	if (HTSCROLL_NONE != scrollPortionHover)
	{
		scrollPortionHover=HTSCROLL_NONE;
		InvalidateNC(InvalidateFlag_Normal, SB_BOTH);
	}
}
//
//	used for hot-tracking over the scroll buttons
//
void SkinnedScrollWnd::OnNcMouseMove(UINT nHitTest, POINTS pts)
{
	if (!bDoHover)
	{
		__super::WindowProc(WM_NCMOUSEMOVE, nHitTest, *(LPARAM*)&pts);
		return;
	}
	SCROLLBAR *psb=0;
	UINT scrollbar=0;
	RECT rect;	
	
	if (psbHorz) psbHorz->fScrollFlags &= ~CSBS_HOVERING;
	if (psbVert) psbVert->fScrollFlags &= ~CSBS_HOVERING;
	if (HTHSCROLL == nHitTest)
	{
		psb = psbHorz;
		scrollbar = SB_HORZ;
		//get the total area of the normal Horz scrollbar area
		GetHScrollRect(this, &rect);
	}
	//
	//	VERTICAL SCROLLBAR PROCESSING
	//
	else if (HTVSCROLL== nHitTest)
	{
		psb = psbVert;
		scrollbar = SB_VERT;
		//get the total area of the normal Horz scrollbar area
		GetVScrollRect(this, &rect);
	}
	//
	//	NORMAL PROCESSING
	//
	else
	{

		scrollPortionHover=HTSCROLL_NONE;
		__super::WindowProc(WM_NCMOUSEMOVE, nHitTest, *(LPARAM*)&pts);

		return;
	}

	if (NULL != psb)
	{
		psb->fScrollFlags |= CSBS_HOVERING;
		UINT thisportion = GetPortion(psb, hwnd, &rect, pts.x, pts.y, scrollFlags);
		if (thisportion != scrollPortionHover)
		{
			TRACKMOUSEEVENT tracker;
			tracker.cbSize = sizeof(tracker);
			tracker.hwndTrack = hwnd;
			tracker.dwHoverTime=0;
			tracker.dwFlags = TME_LEAVE |TME_NONCLIENT; // benski> TME_NONCLIENT doesn't work on NT4.0, and we can work around it anyway
			if (TrackMouseEvent(&tracker))
			{
				scrollPortionHover=thisportion;
				InvalidateNC(InvalidateFlag_Normal, scrollbar);
			}
		}
	}

	__super::WindowProc(WM_NCMOUSEMOVE, nHitTest, *(LPARAM*)&pts);
}

//
//	Timer routine to generate scrollbar messages
//
void SkinnedScrollWnd::OnTimer(UINT_PTR idEvent, TIMERPROC fnTimer)
{
	//let all timer messages go past if we don't have a timer installed ourselves
	if (uScrollTimerId != 0)
	{
		//if the first timer goes off, then we can start a more
		//regular timer interval to auto-generate scroll messages
		//this gives a slight pause between first pressing the scroll arrow, and the
		//actual scroll starting
		if (idEvent == COOLSB_TIMERID1)
		{
			KillTimer(hwnd, uScrollTimerId);
			uScrollTimerId = SetTimer(hwnd, COOLSB_TIMERID2, COOLSB_TIMERINTERVAL2, 0);
			return;
		}
		//send the scrollbar message repeatedly
		else if (idEvent == COOLSB_TIMERID2)
		{
			//need to process a spoof WM_MOUSEMOVE, so that
			//we know where the mouse is each time the scroll timer goes off.
			//This is so we can stop sending scroll messages if the thumb moves
			//under the mouse.
			POINT pt;
			GetCursorPos(&pt);
			MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
			pt.x = POINTTOPOINTS(pt);
			OnMouseMove(MK_LBUTTON, MAKEPOINTS(pt.x));
			if (uScrollTimerPortion != HTSCROLL_NONE) SendScrollMessage(hwnd, uScrollTimerMsg, uScrollTimerPortion, 0);
			UpdateScrollBars(TRUE);
			return;
		}
	}
	__super::WindowProc(WM_TIMER, (WPARAM)idEvent, (LPARAM)fnTimer);
}

//
//	We must intercept any calls to SetWindowLong, to check if
//  left-scrollbars are taking effect or not
//

static UINT curTool = -1;

static LRESULT SendToolTipMessage0(HWND hwndTT, UINT message, WPARAM wParam, LPARAM lParam)
{
	return SendMessageW(hwndTT, message, wParam, lParam);
}

#ifdef COOLSB_TOOLTIPS
#define SendToolTipMessage		SendToolTipMessage0
#else
#define SendToolTipMessage		1 ? (void)0 : SendToolTipMessage0
#endif

void SkinnedScrollWnd::OnStyleChanged(INT styleType, STYLESTRUCT *pss)
{
	if (styleType == GWL_EXSTYLE) scrollFlags = (scrollFlags & ~SWS_LEFT) | ((WS_EX_LEFTSCROLLBAR & pss->styleNew) ? SWS_LEFT : 0);
	__super::OnStyleChanged(styleType, pss);
}

LRESULT SkinnedScrollWnd::OnEraseBackground(HDC hdc)
{
	if (0 == (CSBS_TRACKING & psbVert->fScrollFlags) && 0 == (CSBS_TRACKING & psbHorz->fScrollFlags) && uCurrentScrollPortion == HTSCROLL_NONE)
	{
		LRESULT result;
		result =  __super::WindowProc(WM_ERASEBKGND, (WPARAM)hdc, 0L);
		UpdateScrollBars(TRUE);
		return result;
	}
	return __super::WindowProc(WM_ERASEBKGND, (WPARAM)hdc, 0L);
}


void SkinnedScrollWnd::OnPrint(HDC hdc, UINT options)
{
	if ((PRF_NONCLIENT & options) &&
			(0 == (PRF_CHECKVISIBLE & options) || IsWindowVisible(hwnd))) 
	{
		PaintNonClient(hdc);
		if (PRF_CLIENT & options) 
			CallPrevWndProc(WM_PRINT, (WPARAM)hdc, (LPARAM)(~(PRF_NONCLIENT | PRF_CHECKVISIBLE) & options));
	}
	else __super::OnPrint(hdc, options);
}

LRESULT SkinnedScrollWnd::OnListViewScroll(INT dx, INT dy)
{
	if (0 != dy && 
		0 != (SWS_LISTVIEW & scrollFlags) &&
		0 != (SWS_HIDEHSCROLL & scrollFlags) &&
		(psbHorz->scrollInfo.nPage <= (UINT)psbHorz->scrollInfo.nMax))
	{
		DWORD windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (LVS_REPORT == (LVS_TYPEMASK & windowStyle))
		{
			SCROLLINFO scrollInfo;
			scrollInfo.cbSize = sizeof(SCROLLINFO);
			scrollInfo.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
			if (GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
			{			
				if (0 == dy ||
					(scrollInfo.nPos == scrollInfo.nMin && dy < 0) || 
					(scrollInfo.nPos >= (scrollInfo.nMax - (INT)scrollInfo.nPage) && dy > 0))
					return TRUE;
				
				RECT rc;
				rc.left = LVIR_BOUNDS;
				if (SendMessageW(hwnd, LVM_GETITEMRECT, 0, (LPARAM)&rc))
				{
					dy = dy / (rc.bottom - rc.top);
					if (dy < 0)
					{
						if ((scrollInfo.nPos - dy) < scrollInfo.nMin)
							dy = scrollInfo.nMin - scrollInfo.nPos;
					}
					else if (dy > 0)
					{
						if ((scrollInfo.nPos + dy) >= (scrollInfo.nMax - (INT)scrollInfo.nPage))
							dy = scrollInfo.nMax - (INT)scrollInfo.nPage - scrollInfo.nPos;
					}
					
					dy = dy * (rc.bottom - rc.top);

					if (0 == dy)
						return TRUE;
				}
			}
		}
	}

	LRESULT result = __super::WindowProc(LVM_SCROLL, (WPARAM)dx, (LPARAM)dy);
	if (result) UpdateScrollBars(TRUE);
	return result;
}

void SkinnedScrollWnd::OnVertScroll(UINT code, UINT pos, HWND hwndSB)
{
	if (0 != (SWS_LISTVIEW & scrollFlags) &&
		0 != (SWS_HIDEHSCROLL & scrollFlags) &&
		(psbHorz->scrollInfo.nPage <= (UINT)psbHorz->scrollInfo.nMax))
	{
		DWORD windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (LVS_REPORT == (LVS_TYPEMASK & windowStyle))
		{
			SCROLLINFO scrollInfo;
			scrollInfo.cbSize = sizeof(SCROLLINFO);

			switch(code)
			{
				case SB_LINEDOWN:
					scrollInfo.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
					if (GetScrollInfo(hwnd, SB_VERT, &scrollInfo) 
						&& scrollInfo.nPos >= (scrollInfo.nMax - (INT)scrollInfo.nPage))
						return;
					break;
			}
		}
	}
	__super::WindowProc(WM_VSCROLL, MAKEWPARAM(code, pos), (LPARAM)hwndSB);
}


void SkinnedScrollWnd::OnMouseWheel(INT delta, UINT vtKey, POINTS pts)
{
	if (0 == delta) 
		return;

	if (0 != (SWS_LISTVIEW & scrollFlags))
	{
		DWORD windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (LVS_REPORT == (LVS_TYPEMASK & windowStyle))
		{
			if (0 != (WS_VSCROLL & windowStyle))
			{	
				unsigned int wheelScroll;
				int scrollLines, distance;

				if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &wheelScroll, 0)) 
					wheelScroll = 3;

				if (0 == wheelScroll)
					return;

				if (WHEEL_PAGESCROLL == wheelScroll)
				{				
					SendMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(((delta > 0) ? SB_PAGEUP : SB_PAGEDOWN), 0), 0L);
					SendMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), 0L);
					return;
				}

				distance = delta + wheelCarryover; 
				scrollLines = distance * (INT)wheelScroll / WHEEL_DELTA;

				wheelCarryover = distance - scrollLines * WHEEL_DELTA / (INT)wheelScroll;

				if (ListView_ScrollReportModeVert(hwnd, -scrollLines, (0 != (SWS_HIDEHSCROLL & scrollFlags))))
				{
					InvalidateNC(InvalidateFlag_RedrawNow, SB_VERT);
					return;
				}
			}
			else if (0 != (SWS_HIDEHSCROLL & scrollFlags))
			{
				return;
			}
		}

	}
	__super::WindowProc(WM_MOUSEWHEEL, MAKEWPARAM(vtKey, delta), *(LPARAM*)&pts);
}

void SkinnedScrollWnd::UpdateFrame()
{
	if (SWS_UPDATEFRAME & scrollFlags)
	{
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER  | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOSENDCHANGING | SWP_NOREDRAW);
	}
}

void SkinnedScrollWnd::OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw)
{
	UINT newFlag = scrollFlags & ~SWS_USEFREEFORM;
	if (UseFreeformScrollbars()) 
		newFlag |= SWS_USEFREEFORM;
	
	if (newFlag != scrollFlags)
	{
		RECT rcOld;
		GetClientRect(hwnd, &rcOld);

		scrollFlags = newFlag;
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOREDRAW);
	
	}
	__super::OnSkinChanged(bNotifyChildren, bRedraw);

	if (FALSE != bRedraw)
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE |RDW_ERASE | RDW_FRAME);
}

LRESULT SkinnedScrollWnd::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		/*case SBM_GETSCROLLINFO:
		{
			SCROLLINFO *scrollInfo = (SCROLLINFO *)lParam;
			int x=0;
			x=0;
		}
		break;*/

		case WM_NCMOUSEMOVE:		OnNcMouseMove((UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONUP:
		case WM_NCMBUTTONDOWN:
		case WM_NCMBUTTONUP:		if (wParam == HTHSCROLL || wParam == HTVSCROLL) return 0;
			break;
		case WM_NCLBUTTONDBLCLK:	if (wParam != HTHSCROLL && wParam != HTVSCROLL)	break; // else fall to the nclbuttondown
		case WM_NCLBUTTONDOWN:	OnNcLButtonDown((UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_NCMOUSELEAVE:
		case WM_MOUSELEAVE:		OnNcMouseLeave(); break;
		case WM_LBUTTONUP:		OnLButtonUp((UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_MOUSEMOVE:		OnMouseMove((UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_TIMER:			OnTimer((UINT_PTR)wParam, (TIMERPROC)lParam); return 0;
		case WM_ERASEBKGND:		return 	OnEraseBackground((HDC)wParam);
		case WM_DISPLAYCHANGE:
			SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER  | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOSENDCHANGING);
			break;
		case WM_CAPTURECHANGED:
			if (!ignoreCaptureChange) 
			{
				LONG pts = GetMessagePos();
				POINT pt;
				POINTSTOPOINT(pt, pts);
				MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
				pts = POINTTOPOINTS(pt);
				Emulate_LeftButtonUp((UINT)wParam, MAKEPOINTS(pts), FALSE);
			}
			break;
			// sometimes update frame required when this messges arrive
		case WM_ACTIVATE:
		case WM_SETFOCUS:
		case WM_HSCROLL:
			{
				LRESULT result = __super::WindowProc(uMsg, wParam, lParam);
				UpdateFrame();
				return result;
			}
			break;
		case WM_MOUSEWHEEL:
			OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam), GET_KEYSTATE_WPARAM(wParam), MAKEPOINTS(lParam));
			UpdateFrame();
			return 0;
		case WM_VSCROLL:
			OnVertScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); 
			UpdateFrame();
			return 0;
		case LVM_SCROLL:
			if (SWS_LISTVIEW & scrollFlags)
				return OnListViewScroll((INT)wParam, (INT)lParam); 
			break;
		case WM_KEYDOWN:
			if (0 != ((SWS_LISTVIEW | SWS_TREEVIEW ) & scrollFlags))
			{
				LRESULT result = __super::WindowProc(uMsg, wParam, lParam);	
				switch(wParam)
				{
					case VK_PRIOR:
					case VK_NEXT:
					case VK_UP:
					case VK_DOWN:
					case VK_HOME:
					case VK_END:
						UpdateScrollBars(TRUE);
						break;
				}
				return result;
			}
			break;
		case WM_USER + 0x3443:
			UpdateScrollBars(TRUE);
			break;
	}
	return __super::WindowProc(uMsg, wParam, lParam);
}

//
//	return the default minimum size of a scrollbar thumb
//
static int WINAPI CoolSB_GetDefaultMinThumbSize(void)
{
	DWORD dwVersion = GetVersion();
	// set the minimum thumb size for a scrollbar. This
	// differs between NT4 and 2000, so need to check to see
	// which platform we are running under
	return (dwVersion >= 0x80000000 && LOBYTE(LOWORD(dwVersion)) >= 5) ? MINTHUMBSIZE_2000 : MINTHUMBSIZE_NT4;
}

BOOL SkinnedScrollWnd::ShowScrollBar(int wBar, BOOL fShow)
{
	DWORD styleOld, styleNew;

	styleOld = GetWindowLongPtrW(hwnd, GWL_STYLE);
	styleNew = styleOld;

	if (wBar == SB_HORZ || wBar == SB_BOTH)
	{
		psbHorz->fScrollFlags = (psbHorz->fScrollFlags & ~CSBS_VISIBLE) | ((fShow) ? CSBS_VISIBLE : 0);
		styleNew = (styleNew & ~WS_HSCROLL) | ((fShow) ? WS_HSCROLL :0);
	}

	if (wBar == SB_VERT || wBar == SB_BOTH)
	{
		psbVert->fScrollFlags = (psbVert->fScrollFlags & ~CSBS_VISIBLE) | ((fShow) ? CSBS_VISIBLE : 0);
		styleNew = (styleNew & ~WS_VSCROLL) | ((fShow) ? WS_VSCROLL :0);
	}

	if (styleNew != styleOld)
	{
		SetWindowLongPtrW(hwnd, GWL_STYLE, styleNew);
		SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOREDRAW);
	}

	return TRUE;
}

SkinnedScrollWnd::SkinnedScrollWnd(BOOL bIsDialog) 
	: SkinnedWnd(bIsDialog), psbHorz(0), psbVert(0),
	  scrollFlags(0), scrollPortionHover(0), wheelCarryover(0)
{
	if (-1 == bUseUpdateRgn)
	{
		OSVERSIONINFO osver = {0};
		osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (::GetVersionEx(&osver))
		{
			bUseUpdateRgn =
			  ((VER_PLATFORM_WIN32_NT != osver.dwPlatformId ||
			    (osver.dwMajorVersion < 6 && osver.dwMinorVersion != 2)));
			bDoHover = !(VER_PLATFORM_WIN32_NT == osver.dwPlatformId && osver.dwMajorVersion == 4); // can't use TrackMouseEvent with non-client areas on Windows NT 4.0
		}
	}
}

BOOL SkinnedScrollWnd::Attach(HWND hwndToSkin)
{
	DWORD style;

	if (!SkinnedWnd::Attach(hwndToSkin)) 
		return FALSE;

	SetType(SKINNEDWND_TYPE_SCROLLWND);

	psbHorz = (SCROLLBAR*)calloc(1, sizeof(SCROLLBAR));
	psbVert = (SCROLLBAR*)calloc(1, sizeof(SCROLLBAR));
	if (!psbHorz || !psbVert) return FALSE;

	scrollFlags = 0;
	wheelCarryover = 0;

	psbHorz->scrollInfo.cbSize = sizeof(SCROLLINFO);
	psbHorz->scrollInfo.fMask  = SIF_ALL;
	if (!GetScrollInfo(hwnd, SB_HORZ, &psbHorz->scrollInfo))
		ZeroMemory(&psbHorz->scrollInfo, sizeof(SCROLLINFO));

	psbVert->scrollInfo.cbSize = sizeof(SCROLLINFO);
	psbVert->scrollInfo.fMask  = SIF_ALL;
	if (!GetScrollInfo(hwnd, SB_VERT, &psbVert->scrollInfo))
		ZeroMemory(&psbVert->scrollInfo, sizeof(SCROLLINFO));

	scrollPortionHover = HTSCROLL_NONE;

	//check to see if the window has left-aligned scrollbars
	if (GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_LEFTSCROLLBAR) scrollFlags |= SWS_LEFT;

	style = GetWindowLongPtrW(hwnd, GWL_STYLE);

	if (WS_HSCROLL & style) psbHorz->fScrollFlags = CSBS_VISIBLE;
	if (WS_VSCROLL & style) psbVert->fScrollFlags = CSBS_VISIBLE;

	psbHorz->nBarType	= SB_HORZ;
	psbVert->nBarType	= SB_VERT;

	//set the default arrow sizes for the scrollbars
	psbHorz->nMinThumbSize = CoolSB_GetDefaultMinThumbSize();
	psbVert->nMinThumbSize = psbHorz->nMinThumbSize;

	scrollFlags |= SWS_UPDATEFRAME;
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER  | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOSENDCHANGING | SWP_NOREDRAW);
	InvalidateNC(InvalidateFlag_Frame | InvalidateFlag_RedrawNow, SB_BOTH);

	return TRUE;
}

SkinnedScrollWnd::~SkinnedScrollWnd(void)
{
	InvalidateNC(InvalidateFlag_Frame | InvalidateFlag_RedrawNow, SB_BOTH);
	if (psbHorz) free(psbHorz);
	if (psbVert) free(psbVert);
	if (hbrChecked) 
	{
		DeleteObject(hbrChecked);
		hbrChecked = NULL; 
	}
}

void SkinnedScrollWnd::DisableNoScroll(BOOL bDisable)
{
	if (bDisable) scrollFlags |= SWS_DISABLENOSCROLL; 
	else scrollFlags &= ~SWS_DISABLENOSCROLL; 
}
BOOL SkinnedScrollWnd::IsNoScrollDisabled()
{
	return (0 != (SWS_DISABLENOSCROLL & scrollFlags));
}

void SkinnedScrollWnd::InvalidateNC(InvalidateFlags invalidate, UINT bars)
{
	HRGN rgnH = NULL, rgnV = NULL;
	RECT rc;
	int scrollLength;
	unsigned int flags;
	long frameEdge, clientEdge;

	if (0 != (InvalidateFlag_Frame & invalidate))
	{
		flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | 
				SWP_FRAMECHANGED | SWP_NOSENDCHANGING | SWP_NOREDRAW;
	
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0, flags);
	}

	if (FALSE == GetClientRect(hwnd, &rc))
		return;

	if (SB_HORZ == bars || SB_BOTH == bars)
	{
		scrollLength = GetScrollMetric(psbHorz, SM_CYVERTSB, scrollFlags);

		clientEdge = rc.bottom;
		if (0 != (InvalidateFlag_HorzBarRemoved & invalidate))
			clientEdge -= scrollLength;

		frameEdge = clientEdge + scrollLength;

		rgnH = CreateRectRgn(rc.left, clientEdge, rc.right, frameEdge);
	}
	else 
		rgnH = NULL;

	if (SB_VERT == bars || SB_BOTH == bars)
	{
		scrollLength = GetScrollMetric(psbVert, SM_CXVERTSB, scrollFlags);

		if (0 != (SWS_LEFT & scrollFlags))
		{
			clientEdge = rc.left;
			if (0 == (InvalidateFlag_VertBarRemoved & invalidate))
				clientEdge -= scrollLength;
		}
		else
		{
			clientEdge = rc.right;
			if (0 != (InvalidateFlag_VertBarRemoved & invalidate))
				clientEdge -= scrollLength;

			frameEdge = clientEdge + scrollLength;
		}

		rgnV = CreateRectRgn(clientEdge, rc.top, frameEdge, rc.bottom);

		if (NULL != rgnV && SB_BOTH == bars) 
			CombineRgn(rgnH, rgnH, rgnV, RGN_OR);
	}
	else 
		rgnV = NULL;

	flags = RDW_INVALIDATE | /*RDW_INTERNALPAINT  | RDW_ERASE |*/ RDW_FRAME | RDW_NOCHILDREN;
	if (0 != (InvalidateFlag_RedrawNow & invalidate))
		flags |= (RDW_UPDATENOW | RDW_ERASENOW);

	HRGN rgn = ((NULL != rgnH) ? rgnH : rgnV);
	if (rgn)
		RedrawWindow(hwnd, NULL, rgn, flags);

	if (rgnH) 
		DeleteRgn(rgnH);

	if (rgnV) 
		DeleteRgn(rgnV);
}

void SkinnedScrollWnd::UpdateScrollBars(BOOL fInvalidate)
{
	UINT bars;
	InvalidateFlags invalidateFlags;

	SCROLLINFO tsi;
	tsi.cbSize = sizeof(SCROLLINFO);
	tsi.fMask = SIF_ALL;

	bars = -1;
	invalidateFlags = InvalidateFlag_Normal /*| InvalidateFlag_RedrawNow*/;

	if (0 == (SWS_HIDEHSCROLL & scrollFlags))
	{
		if (GetScrollInfo(hwnd, SB_HORZ, &tsi) &&
			memcmp(&tsi, &psbHorz->scrollInfo, sizeof(SCROLLINFO)))
		{
			memcpy(&psbHorz->scrollInfo, &tsi, sizeof(SCROLLINFO));
			UpdateScrollBar(psbHorz, &invalidateFlags);
			
			psbHorz->scrollInfo.cbSize = sizeof(SCROLLINFO);
			psbHorz->scrollInfo.fMask = SIF_ALL;
			if (!GetScrollInfo(hwnd, SB_HORZ, &psbHorz->scrollInfo))
				ZeroMemory(&psbHorz->scrollInfo, sizeof(SCROLLINFO));

			bars = SB_HORZ;
		}
	}
	else
	{
		psbHorz->scrollInfo.cbSize = sizeof(SCROLLINFO);
		psbHorz->scrollInfo.fMask = SIF_ALL;
		if (!GetScrollInfo(hwnd, SB_HORZ, &psbHorz->scrollInfo))
			ZeroMemory(&psbHorz->scrollInfo, sizeof(SCROLLINFO));
	}

	if (0 == (SWS_HIDEVSCROLL & scrollFlags))
	{
		if (GetScrollInfo(hwnd, SB_VERT, &tsi))
		{
			if (0 != (SWS_LISTVIEW & scrollFlags) &&
				LVS_REPORT == (LVS_TYPEMASK & GetWindowLongPtrW(hwnd, GWL_STYLE)))
			{
				if (0 != (SWS_HIDEHSCROLL & scrollFlags))
				{
					tsi.nMax = (int)SendMessageW(hwnd, LVM_GETITEMCOUNT, 0, 0L);
					tsi.nPage = (unsigned int)SendMessageW(hwnd, LVM_GETCOUNTPERPAGE, 0, 0L);
					if(tsi.nMax > 0)
						tsi.nMax--;
											
				//	if (psbHorz->scrollInfo.nPage <= (UINT)psbHorz->scrollInfo.nMax)
				
				}
			}

			if (memcmp(&tsi, &psbVert->scrollInfo, sizeof(SCROLLINFO)))
			{
				memcpy(&psbVert->scrollInfo, &tsi, sizeof(SCROLLINFO));

				UpdateScrollBar(psbVert, &invalidateFlags);
				bars = (SB_HORZ == bars) ? SB_BOTH : SB_VERT;
			}
		}
	}
	else
	{
		psbVert->scrollInfo.cbSize = sizeof(SCROLLINFO);
		psbVert->scrollInfo.fMask = SIF_ALL;
		if (!GetScrollInfo(hwnd, SB_VERT, &psbVert->scrollInfo))
			ZeroMemory(&psbVert->scrollInfo, sizeof(SCROLLINFO));
	}

	if ((fInvalidate || 0 != (InvalidateFlag_Frame & invalidateFlags)) &&
		-1 != bars) 
	{
		InvalidateNC(invalidateFlags, bars);

		if (0 != (InvalidateFlag_Frame & invalidateFlags) && 
			0 != (SWS_LISTVIEW & scrollFlags) &&
			LVS_REPORT == (LVS_TYPEMASK & GetWindowLongPtrW(hwnd, GWL_STYLE)))
		{
			if (0 == (LVS_NOCOLUMNHEADER & GetWindowLongPtrW(hwnd, GWL_STYLE)))
			{
				HWND hHeader = (HWND)SendMessageW(hwnd, LVM_GETHEADER, 0, 0L);
				if (NULL != hHeader)
				{
					RECT clientRect;
					WINDOWPOS wp;

					GetClientRect(hwnd, &clientRect);

					SCROLLINFO si;
					si.cbSize = sizeof(si);
					si.fMask = SIF_POS;
					if (0 != GetScrollInfo(hwnd, SB_HORZ, &si))
						clientRect.left -= si.nPos;	

					HDLAYOUT layout;
					layout.prc = &clientRect;
					layout.pwpos = &wp;
					if (FALSE != SendMessageW(hHeader, HDM_LAYOUT, 0, (LPARAM)&layout))
					{
						if (FALSE == fInvalidate) 
							wp.flags |= SWP_NOREDRAW;
						SetWindowPos(hHeader, wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE);
					}
				}
			}
		}
	}
}

void SkinnedScrollWnd::UpdateScrollBar(SCROLLBAR *psb, InvalidateFlags *invalidateFlags)
{
	SCROLLINFO *psi;
	psi = &psb->scrollInfo;

	if ((psi->nPage > (UINT)psi->nMax || (psi->nPage == (UINT)psi->nMax && psi->nMax == 0) || psi->nMax <= psi->nMin))
	{
		if (psb->fScrollVisible)
		{
			if (SWS_DISABLENOSCROLL & scrollFlags)
			{
				psb->fScrollFlags |= (ESB_DISABLE_LEFT | ESB_DISABLE_RIGHT);
			}
			else
			{
				ShowScrollBar(psb->nBarType, FALSE);
				*invalidateFlags |= InvalidateFlag_Frame;
				if (SB_VERT == psb->nBarType)
					*invalidateFlags |= InvalidateFlag_VertBarRemoved;
				else
					*invalidateFlags |= InvalidateFlag_HorzBarRemoved;
			}
		}
	}
	else 
	{
		if ((!psb->fScrollVisible || ((ESB_DISABLE_LEFT | ESB_DISABLE_RIGHT) & psb->fScrollFlags)) && psi->nPage > 0)
		{
			if ((SWS_DISABLENOSCROLL & scrollFlags) && ((ESB_DISABLE_LEFT | ESB_DISABLE_RIGHT) & psb->fScrollFlags))
			{
				psb->fScrollFlags &= ~(ESB_DISABLE_LEFT | ESB_DISABLE_RIGHT);
			}

			if (!psb->fScrollVisible)
			{
				if (SWS_LISTVIEW & scrollFlags)
				{
					DWORD ws = GetWindowLongPtrW(hwnd, GWL_STYLE);
					if (LVS_ICON == (LVS_TYPEMASK & ws) || LVS_SMALLICON == (LVS_TYPEMASK & ws))
					{
						switch(LVS_ALIGNMASK & ws)
						{
							case LVS_ALIGNLEFT:
								if (SB_HORZ != psb->nBarType) psb->nBarType = ((SB_BOTH == psb->nBarType) ? SB_HORZ : -1);
								break;
							case LVS_ALIGNTOP:
								if (SB_VERT != psb->nBarType) psb->nBarType = ((SB_BOTH == psb->nBarType) ? SB_VERT : -1);
								break;
						}
					}
				}
				if (-1 != psb->nBarType)
				{
					ShowScrollBar(psb->nBarType, TRUE);
					*invalidateFlags |= InvalidateFlag_Frame;
					if (SB_VERT == psb->nBarType)
						*invalidateFlags |= InvalidateFlag_VertBarAppeared;
					else
						*invalidateFlags |= InvalidateFlag_HorzBarAppeared;
				}
			}
		}
		else if (psb->fScrollVisible && 0 == psi->nPage)
		{
			if (SWS_DISABLENOSCROLL & scrollFlags)
			{
				psb->fScrollFlags |= (ESB_DISABLE_LEFT | ESB_DISABLE_RIGHT);
			}
			else
			{
				ShowScrollBar(psb->nBarType, FALSE);
				*invalidateFlags |= InvalidateFlag_Frame;
				if (SB_VERT == psb->nBarType)
					*invalidateFlags |= InvalidateFlag_VertBarRemoved;
				else
					*invalidateFlags |= InvalidateFlag_HorzBarRemoved;
			}
		}
	}
}

void SkinnedScrollWnd::ShowHorzScroll(BOOL fEnable)
{
	scrollFlags = (scrollFlags & ~SWS_HIDEHSCROLL) | ((fEnable) ? 0 : SWS_HIDEHSCROLL);
	psbHorz->fScrollFlags = 0;
	InvalidateNC(InvalidateFlag_Frame | InvalidateFlag_RedrawNow, SB_HORZ);
}

BOOL SkinnedScrollWnd::IsHorzBarHidden()
{
	return (0 != (SWS_HIDEHSCROLL & scrollFlags));
}

BOOL SkinnedScrollWnd::IsVertBarHidden()
{
	return (0 != (SWS_HIDEVSCROLL & scrollFlags));
}

void SkinnedScrollWnd::ShowVertScroll(BOOL fEnable)
{
	scrollFlags = (scrollFlags & ~SWS_HIDEVSCROLL) | ((fEnable) ? 0 : SWS_HIDEVSCROLL);
	psbVert->fScrollFlags = 0;
	InvalidateNC(InvalidateFlag_Frame | InvalidateFlag_RedrawNow, SB_VERT);
}

BOOL SkinnedScrollWnd::SetMode(UINT nMode)
{
	scrollFlags &= ~(SWS_LISTVIEW | SWS_TREEVIEW | SWS_COMBOLBOX);
	switch (0xFF & nMode)
	{
		case SCROLLMODE_STANDARD_I: return TRUE;
		case SCROLLMODE_LISTVIEW_I: scrollFlags |= SWS_LISTVIEW; return TRUE;
		case SCROLLMODE_TREEVIEW_I: scrollFlags |= SWS_TREEVIEW; return TRUE;
		case SCROLLMODE_COMBOLBOX_I: scrollFlags |= SWS_COMBOLBOX; return TRUE;
	}

	return FALSE;
}

UINT SkinnedScrollWnd::GetMode()
{
	if (0 != (SWS_LISTVIEW & scrollFlags))
		return SCROLLMODE_STANDARD_I;

	if (0 != (SWS_TREEVIEW & scrollFlags))
		return SCROLLMODE_TREEVIEW_I;

	if (0 != (SWS_COMBOLBOX & scrollFlags))
		return SCROLLMODE_COMBOLBOX_I;

	return SCROLLMODE_STANDARD_I;
}

INT SkinnedScrollWnd::AdjustHover(UINT nHitTest, POINTS pts)
{
	SCROLLBAR *psb = NULL;
	RECT rect;

	if (psbHorz) psbHorz->fScrollFlags &= ~CSBS_HOVERING;
	if (psbVert) psbVert->fScrollFlags &= ~CSBS_HOVERING;
	scrollPortionHover = HTSCROLL_NONE;

	if (HTHSCROLL == nHitTest)
	{
		psb = psbHorz;
		GetHScrollRect(this, &rect);
	}
	else if (HTVSCROLL== nHitTest)
	{
		psb = psbVert;
		GetVScrollRect(this, &rect);
	}

	if (psb)
	{
		psb->fScrollFlags |= CSBS_HOVERING;
		scrollPortionHover = GetPortion(psb, hwnd, &rect, pts.x, pts.y, scrollFlags);
	}
	return scrollPortionHover;
}

BOOL SkinnedScrollWnd::OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult)
{
	switch (msg)
	{
		case IPC_ML_SKINNEDSCROLLWND_UPDATEBARS:
			UpdateScrollBars((BOOL)param); 
			*pResult = 1; 
			return TRUE;
		case IPC_ML_SKINNEDSCROLLWND_SHOWHORZBAR:
			ShowHorzScroll((BOOL)param); 
			*pResult = 1; 
			return TRUE;
		case IPC_ML_SKINNEDSCROLLWND_SHOWVERTBAR:
			ShowVertScroll((BOOL)param); 
			*pResult = 1; 
			return TRUE;
		case IPC_ML_SKINNEDSCROLLWND_SETMODE:
			*pResult = SetMode((UINT)param); 
			return TRUE;
		case IPC_ML_SKINNEDSCROLLWND_DISABLENOSCROLL: 
			DisableNoScroll(0 != param);
			*pResult = -1; 
			return TRUE;
		case IPC_ML_SKINNEDSCROLLWND_ADJUSTHOVER:
			((SBADJUSTHOVER*)param)->nResult = AdjustHover(((SBADJUSTHOVER*)param)->hitTest, ((SBADJUSTHOVER*)param)->ptMouse); 
			*pResult = TRUE;
			return TRUE;
		case IPC_ML_SKINNEDSCROLLWND_GETHORZBARHIDDEN:
			*pResult = IsHorzBarHidden();
			return TRUE;
		case IPC_ML_SKINNEDSCROLLWND_GETVERTBARHIDDEN:
			*pResult = IsVertBarHidden();
			return TRUE;
		case IPC_ML_SKINNEDSCROLLWND_GETMODE:
			*pResult = GetMode();
			return TRUE;
		case IPC_ML_SKINNEDSCROLLWND_GETNOSCROLLDISABLED:
			*pResult = IsNoScrollDisabled();
			return TRUE;
	}
	return __super::OnMediaLibraryIPC(msg, param, pResult);
}