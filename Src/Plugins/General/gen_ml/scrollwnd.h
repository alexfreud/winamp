#ifndef _SCROLLWND_H
#define _SCROLLWND_H

/* minimum size of scrollbar before inserted buttons are 
   hidden to make room when the window is sized too small */
#define MIN_COOLSB_SIZE  24

/* min size of scrollbar when resizing a button, before the 
   resize is stopped because the scrollbar has gotten too small */
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


#include <windows.h>

// To complement the exisiting SB_HORZ, SB_VERT, SB_BOTH
// scrollbar identifiers
#define COOLSB_NONE (-1)
#define SB_INSBUT	(-2)

//
//	Arrow size defines
//
#define SYSTEM_METRIC (-1)

//
// general scrollbar styles
//
// use the standard ESB_DISABLE_xxx flags to represent the
// enabled / disabled states. (defined in winuser.h)
//
#define CSBS_THUMBALWAYS		4
#define CSBS_VISIBLE			8

//cool scrollbar styles for Flat scrollbars
#define CSBS_NORMAL			0
#define CSBS_FLAT			1
#define CSBS_HOTTRACKED		2

//
//	Button mask flags for indicating which members of SCROLLBUT
//	to use during a button insertion / modification
//	
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

//
// scrollbar button types
//
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
#define SBBM_UPARROW		0x0100
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


//
//	Button command notification codes
//	for sending with a WM_COMMAND message
//
#define CSBN_BASE	0
#define CSBN_CLICKED (1 + CSBN_BASE)
#define CSBN_HILIGHT (2 + CSBN_BASE)

//
//	Minimum size in pixels of a scrollbar thumb
//
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

//
//	SCROLLBAR datatype. There are two of these structures per window
//
#define SCROLLBAR_LISTVIEW 1 // scrollbar is for a listview
typedef struct 
{
	UINT		fScrollFlags;		//flags
	BOOL		fScrollVisible;		//if this scrollbar visible?
	SCROLLINFO	scrollInfo;			//positional data (range, position, page size etc)
	
	int			nArrowLength;		//perpendicular size (height of a horizontal, width of a vertical)
	int			nArrowWidth;		//parallel size (width of horz, height of vert)

	//data for inserted buttons
	int			nButSizeBefore;		//size to the left / above the bar
	int			nButSizeAfter;		//size to the right / below the bar

	BOOL		fButVisibleBefore;	//if the buttons to the left are visible
	BOOL		fButVisibleAfter;	//if the buttons to the right are visible

	int			nBarType;			//SB_HORZ / SB_VERT

	UINT		fFlatScrollbar;		//do we display flat scrollbars?
	int			nMinThumbSize;

	int flags;

} SCROLLBAR;

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

class ScrollWnd {
public:
  ScrollWnd(HWND hwnd, int flags=0);
  ~ScrollWnd();

  void update();
  
  HWND m_hwnd;

	UINT bars;				//which of the scrollbars do we handle? SB_VERT / SB_HORZ / SB_BOTH
	WNDPROC oldproc;			//old window procedure to call for every message
	BOOL	fWndUnicode;		

	SCROLLBAR sbarHorz;		//one scrollbar structure each for 
	SCROLLBAR sbarVert;		//the horizontal and vertical scrollbars

	BOOL fThumbTracking;	// are we currently thumb-tracking??
	BOOL fLeftScrollbar;	// support the WS_EX_LEFTSCROLLBAR style

	//size of the window borders
	int cxLeftEdge, cxRightEdge;
	int cyTopEdge,  cyBottomEdge;

	// To prevent calling original WindowProc in response
	// to our own temporary style change (fixes TreeView problem)
	BOOL bPreventStyleChange;

  void updatesb(int fnBar, BOOL *fRecalcFrame);
  void disableHorzScroll();

  //int setScrollInfo(int fnBar, LPSCROLLINFO lpsi, BOOL fRedraw);

  int m_disable_hscroll;
  int m_xp_theme_disabled;
};

#endif
