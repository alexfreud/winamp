/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include <windowsx.h>

#include "main.h"
#include "api.h"

void draw_paint_emb( HWND, int, int, int );
void draw_embed_tbar( HWND, int, int );
void draw_embed_tbutton( HWND, int, int );
void draw_embed( HDC, int, int );

// all of this stuff is barely working. I will be fixing it soon soon.

/// embed ui shit
#define inreg(x,y,x2,y2) \
        ((mouse_x <= ( x2 ) && mouse_x >= ( x ) &&  \
        mouse_y <= ( y2 ) && mouse_y >= ( y )))

enum
{
	NO_CAP, TITLE_CAP, TB_CAP, SZ_CAP
};

static void do_titlebar( HWND hwnd, embedWindowState *state );
static void do_titlebuttons( HWND hwnd, embedWindowState *state );
static void do_size( HWND hwnd, embedWindowState *state );

static int mouse_x, mouse_y, mouse_type, mouse_stats;
static int which_cap = 0;
static HWND capwnd;

void embedui_handlemouseevent( HWND hwnd, int x, int y, int type, int stats, embedWindowState *state )
{
	if ( which_cap != NO_CAP && hwnd != capwnd ) return;

	mouse_x = x;
	mouse_y = y;
	mouse_type = type;
	mouse_stats = stats;
	switch ( which_cap )
	{
		case TITLE_CAP:	do_titlebar( hwnd, state ); return;
		case TB_CAP:	do_titlebuttons( hwnd, state ); return;
		case SZ_CAP:	do_size( hwnd, state );	return;
		default: break;
	}
	do_titlebuttons( hwnd, state );
	do_size( hwnd, state );
	do_titlebar( hwnd, state );

	if ( which_cap != NO_CAP ) capwnd = hwnd; // not sure if this is gonna work
}

static void do_titlebar( HWND hwnd, embedWindowState *state )
{
	if ( which_cap == TITLE_CAP || ( !which_cap && ( config_easymove || mouse_y < 14 ) ) )
	{
		static int clickx, clicky;
		switch ( mouse_type )
		{
			case 1:
			{
				which_cap = TITLE_CAP;
				clickx = mouse_x;
				clicky = mouse_y;
			}
			break;
			case -1:
				which_cap = 0;
				break;
			case 0:
				if ( which_cap == TITLE_CAP && mouse_stats & MK_LBUTTON )
				{
					// TODO need to convert this into an API method so
					//		we can call it externally e.g. enhancer...
					//		or something like it to allow state->r to
					//		be updated once the move has been finished
					POINT p = { mouse_x, mouse_y };
					ClientToScreen( hwnd, &p );
					int w = state->r.right - state->r.left;
					int h = state->r.bottom - state->r.top;

					state->r.left = p.x - clickx;
					state->r.top = p.y - clicky;
					state->r.right = state->r.left + w;
					state->r.bottom = state->r.top + h;

					if ( !!config_snap + !!( mouse_stats & MK_SHIFT ) == 1 )
					{
						SnapWindowToAllWindows( &state->r, hwnd );
					}
					POINT pt = { state->r.left, state->r.top };
					SendMessageW( hwnd, WM_USER + 0x100, 1, (LPARAM) &pt );
					SetWindowPos( hwnd, 0, state->r.left, state->r.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
				}
				break;
		}
	}
}

static void do_titlebuttons( HWND hwnd, embedWindowState *state )
{
	int w = 0;
	w = inreg( state->r.right - state->r.left - 10, 3, state->r.right - state->r.left - 1, 3 + 9 ) ? 1 : 0;

	if ( w ) // kill button
	{
		if ( mouse_type == -1 && which_cap == TB_CAP )
		{
			which_cap = 0;
			draw_embed_tbutton( hwnd, 0, ( state->r.right - state->r.left ) );
			SendMessageW( hwnd, WM_USER + 101, 0, 0 );
		}
		else if ( mouse_stats & MK_LBUTTON )
		{
			which_cap = TB_CAP;
			draw_embed_tbutton( hwnd, w ? 1 : 0, ( state->r.right - state->r.left ) );
		}
	}
	else if ( which_cap == TB_CAP )
	{
		which_cap = 0;
		draw_embed_tbutton( hwnd, 0, ( state->r.right - state->r.left ) );
	}
}

typedef struct _WNDREPAINT
{
	HWND hwndSender;
	RECT rwR;
	RECT rwB;
} WNDREPAINT;

static BOOL CALLBACK EnumWndRepaintProc( HWND hwnd, LPARAM param )
{
	WNDREPAINT *pwp = (WNDREPAINT *) param;
	if ( hwnd != pwp->hwndSender && IsWindowVisible( hwnd ) )
	{
		RECT rw;
		GetWindowRect( hwnd, &rw );
		if ( ( rw.left < pwp->rwR.right && rw.right > pwp->rwR.left && rw.top < pwp->rwR.bottom && rw.bottom > pwp->rwR.top ) ||
			 ( rw.top < pwp->rwB.bottom && rw.bottom > pwp->rwB.top && rw.left < pwp->rwB.right && rw.right > pwp->rwB.left ) )
		{
			UpdateWindow( hwnd );
		}

	}
	return TRUE;
}
static void do_size( HWND hwnd, embedWindowState *state )
{
	if ( state->flags & EMBED_FLAGS_NORESIZE )
	{
		if ( which_cap == SZ_CAP ) which_cap = 0;
		return;
	}
	if ( which_cap == SZ_CAP || ( !which_cap &&
								  mouse_x > ( state->r.right - state->r.left ) - 20 && mouse_y > ( state->r.bottom - state->r.top ) - 20 &&
								  ( ( ( state->r.right - state->r.left ) - mouse_x + ( state->r.bottom - state->r.top ) - mouse_y ) <= 30 ) ) )
	{
		static int dx, dy;
		if ( !which_cap && mouse_type == 1 )
		{
			dx = ( state->r.right - state->r.left ) - mouse_x;
			dy = ( state->r.bottom - state->r.top ) - mouse_y;
			which_cap = SZ_CAP;
		}
		if ( which_cap == SZ_CAP )
		{
			if ( mouse_type == -1 ) which_cap = 0;

			int x = mouse_x + dx;
			int y = mouse_y + dy;
			//		if (x >= GetSystemMetrics(SM_CXSCREEN)) x = GetSystemMetrics(SM_CXSCREEN)-24;
			//		if (y >= GetSystemMetrics(SM_CYSCREEN)) y = GetSystemMetrics(SM_CYSCREEN)-28;
			if ( !config_embedwnd_freesize )
			{
				x += 24;
				x -= x % 25;
				y += 28;
				y -= y % 29;
			}

			if ( x < 275 ) x = 275;
			if ( y < 20 + 38 + 29 + 29 ) y = 20 + 38 + 29 + 29;

			if ( x != ( state->r.right - state->r.left ) || y != ( state->r.bottom - state->r.top ) )
			{
				// TODO need to ensure this isn't used when freesize is disabled
				//		isn't keeping track of the positions correctly on change
				//		as some windows only part snap e.g. ml won't dock to edges or to bottom of main window
				RECT rw = { 0 }, r = { 0 };
				POINT pt = { x, y };
				SendMessageW( hwnd, WM_USER + 0x101, 1, (LPARAM) &pt );

				GetWindowRect( hwnd, &rw );

				// trying to get classic skins to dock to other windows on resizing
				/*rw.left = state->r.left;
				rw.top = state->r.top;
				rw.right = state->r.left + x;
				rw.bottom = state->r.top + y;
				CopyRect(&r, &rw);
				//if (!!config_snap + !!(mouse_stats & MK_SHIFT) == 1)
				{
					SnapWindowToAllWindows(&rw, hwnd);
				}

				x += (rw.right - r.right);
				y += (rw.bottom - r.bottom);
				SetWindowPos(hwnd, 0, 0, 0, x, y, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);*/

				SetWindowPos( hwnd, 0, 0, 0, x, y, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

				if ( x < ( rw.right - rw.left ) || y < ( rw.bottom - rw.top ) )
				{
					WNDREPAINT wrp;
					wrp.hwndSender = hwnd;
					SetRect( &wrp.rwR, min( rw.left + x, rw.right ), rw.top, rw.right, rw.bottom );
					SetRect( &wrp.rwB, rw.left, min( rw.top + y, rw.bottom ), rw.right, rw.bottom );
					EnumThreadWindows( GetCurrentThreadId(), EnumWndRepaintProc, (LPARAM) &wrp );
				}
			}
		}
	}
}

//// embed window shut

static int emb_OnLButtonUp( HWND hwnd, int x, int y, UINT flags );
static int emb_OnRButtonUp( HWND hwnd, int x, int y, UINT flags );
static int emb_OnLButtonDown( HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags );
static int emb_OnMouseMove( HWND hwnd, int x, int y, UINT keyFlags );
static int emb_OnLButtonDblClk( HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags );
static BOOL emb_OnNCActivate( HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized );

static int emb_OnRButtonUp( HWND hwnd, int x, int y, UINT flags )
{
	//display winamp's main popup menu
	POINT p;
	GetCursorPos( &p );
	int ret = DoTrackPopup( main_menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, hwnd );
	if ( ret ) SendMessageW( hMainWindow, WM_COMMAND, ret, 0 );
	return 1;
}

static int emb_OnLButtonUp( HWND hwnd, int x, int y, UINT flags )
{
	ReleaseCapture();
	embedui_handlemouseevent( hwnd, x, y, -1, flags, (embedWindowState *) GetWindowLongPtrW( hwnd, GWLP_USERDATA ) );
	return 1;
}

static int emb_OnLButtonDown( HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
	SetCapture( hwnd );
	embedui_handlemouseevent( hwnd, x, y, 1, keyFlags, (embedWindowState *) GetWindowLongPtrW( hwnd, GWLP_USERDATA ) );
	return 1;
}

static int emb_OnMouseMove( HWND hwnd, int x, int y, UINT keyFlags )
{
	embedui_handlemouseevent( hwnd, x, y, 0, keyFlags, (embedWindowState *) GetWindowLongPtrW( hwnd, GWLP_USERDATA ) );
	return 1;
}

static BOOL emb_OnNCActivate( HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized )
{
	embedWindowState *state = (embedWindowState *) GetWindowLongPtrW( hwnd, GWLP_USERDATA );
	if ( fActive == FALSE )
	{
		draw_embed_tbar( hwnd, config_hilite ? 0 : 1, ( state->r.right - state->r.left ) );
		which_cap = NO_CAP;
		capwnd = 0;
	}
	else
	{
		draw_embed_tbar( hwnd, 1, ( state->r.right - state->r.left ) );
	}
	return TRUE;
}

static int emb_OnLButtonDblClk( HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
	return 1;
}

CRITICAL_SECTION embedcs;
embedWindowState *embedwndlist; // linked list
int embedwndlist_cnt;

static void EmbedWindow_OnShowWindow( HWND hwnd, BOOL fShow, UINT status )
{
	if ( 0 != status )
	{
		SetPropW( hwnd, L"EmbedWnd_ShowStatus", (HANDLE) status );
		DefWindowProcW( hwnd, WM_SHOWWINDOW, (WPARAM) fShow, (LPARAM) status );
		RemovePropW( hwnd, L"EmbedWnd_ShowStatus" );
		return;
	}

	embedWindowState *state = (embedWindowState *) GetWindowLongPtrW( hwnd, GWLP_USERDATA );

	HWND hChild = FindWindowExW( hwnd, NULL, NULL, NULL );

	INT toggleResult = 1;

	if ( state && FALSE == state->reparenting )
	{
		INT result = Ipc_WindowToggle( (INT_PTR) hwnd, ( FALSE != fShow ) ? 1 : 0 );
		if ( fShow ) toggleResult = result;
	}

	if ( NULL != hChild && 0 != toggleResult )
	{
		if ( FALSE != fShow && NULL != state )
		{
			SetWindowPos( hChild, NULL, 11, 20,
						  ( state->r.right - state->r.left ) - 11 - 8,
						  ( state->r.bottom - state->r.top ) - 20 - 14,
						  SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOOWNERZORDER );
		}

		EMBEDSHOW embedShow;
		embedShow.hdr.code     = EWN_SHOWWINDOW;
		embedShow.hdr.hwndFrom = hwnd;
		embedShow.hdr.idFrom   = GetDlgCtrlID( hwnd );
		embedShow.fShow        = fShow;
		embedShow.nStatus      = (UINT) (UINT_PTR) GetPropW( hwnd, L"EmbedWnd_ShowStatus" );

		SendMessageW( hChild, WM_NOTIFY, (WPARAM) embedShow.hdr.idFrom, (LPARAM) &embedShow );
	}
}

typedef struct __EMBEDWNDPART
{
	INT id;
	RECT rect;
}EMBEDWNDPART;


static INT EmbedWindow_HitTest( HWND hwnd, POINT pt )
{
	DWORD windowStyle = GetWindowLongPtrW( hwnd, GWL_STYLE );
	if ( 0 != ( WS_DISABLED & windowStyle ) )
		return HTERROR;

	MapWindowPoints( HWND_DESKTOP, hwnd, &pt, 1 );

	RECT clientRect;
	if ( !GetClientRect( hwnd, &clientRect ) )
		return HTERROR;

	if ( 0 != ( WS_CHILD & windowStyle ) )
	{
		return ( PtInRect( &clientRect, pt ) ) ? HTCLIENT : HTNOWHERE;
	}

	static EMBEDWNDPART embedWindowParts[] =
	{
		{ HTCLOSE, {-( 275 - 264 ), 3,-( 275 - 272 ), 12}},
		{ HTCAPTION, {0, 0, -1, 13}},
		{ HTBOTTOMRIGHT, {-20,-20,-1,-1}},
	};

	INT hitTest = HTCLIENT;

	RECT part;
	for ( INT i = 0; i < ARRAYSIZE( embedWindowParts ); i++ )
	{
		CopyRect( &part, &embedWindowParts[ i ].rect );
		if ( part.left < 0 ) part.left += clientRect.right;
		if ( part.right < 0 ) part.right += clientRect.right;
		if ( part.top < 0 ) part.top += clientRect.bottom;
		if ( part.bottom < 0 ) part.bottom += clientRect.bottom;

		if ( PtInRect( &part, pt ) )
		{
			hitTest = embedWindowParts[ i ].id;
			break;
		}
	}

	if ( HTBOTTOMRIGHT == hitTest )
	{
		embedWindowState *state = (embedWindowState *) GetWindowLongPtrW( hwnd, GWLP_USERDATA );
		if ( 0 != ( EMBED_FLAGS_NORESIZE & state->flags ) )
			hitTest = HTBORDER;
	}

	return hitTest;
}

static LRESULT EmbedWindow_OnSetCursor( HWND hwnd, HWND hwndCursor, INT hitTest, UINT uMsg )
{
	HCURSOR hCursor = NULL;

	switch ( uMsg )
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			DisabledWindow_OnMouseClick( hwnd );
			break;
	}

	if ( config_usecursors && !disable_skin_cursors )
	{
		int index = 15 + 5; // PNormal.cur
		POINT pt;
		GetCursorPos( &pt );
		hitTest = EmbedWindow_HitTest( hwnd, pt );
		switch ( hitTest )
		{
			case HTCAPTION:
				index = 15 + 2;  // PTBar.cur
				break;
			case HTCLOSE:
				index = 15 + 1; // PClose.cur
				break;
			case HTLEFT:
			case HTRIGHT:
			case HTTOP:
			case HTTOPLEFT:
			case HTTOPRIGHT:
			case HTBOTTOM:
			case HTBOTTOMLEFT:
			case HTBOTTOMRIGHT:
				index = 15 + 4;// PSize.cur
				break;
		}
		
		hCursor = Skin_Cursors[ index ];
	}

	if ( NULL != hCursor )
	{
		SetCursor( hCursor );
		return TRUE;
	}
	
	return DefWindowProcW( hwnd, WM_SETCURSOR, (WPARAM) hwndCursor, MAKELPARAM( hitTest, uMsg ) );
}

extern "C"
{
	LRESULT CALLBACK emb_WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		switch ( uMsg )
		{
			case WM_INITMENUPOPUP:
				return SendMessageW( hMainWindow, uMsg, wParam, lParam ); // for popup menus
			HANDLE_MSG( hwnd, WM_QUERYNEWPALETTE, Main_OnQueryNewPalette );
			HANDLE_MSG( hwnd, WM_PALETTECHANGED, Main_OnPaletteChanged );
			HANDLE_MSG( hwnd, WM_LBUTTONUP, emb_OnLButtonUp );
			HANDLE_MSG( hwnd, WM_RBUTTONUP, emb_OnRButtonUp );
			HANDLE_MSG( hwnd, WM_LBUTTONDOWN, emb_OnLButtonDown );
			HANDLE_MSG( hwnd, WM_MOUSEMOVE, emb_OnMouseMove );
			HANDLE_MSG( hwnd, WM_NCACTIVATE, emb_OnNCActivate );
			HANDLE_MSG( hwnd, WM_LBUTTONDBLCLK, emb_OnLButtonDblClk );
			case WM_SYSCOMMAND:
				if ( ( wParam & 0xfff0 ) == SC_SCREENSAVE || ( wParam & 0xfff0 ) == SC_MONITORPOWER )
					return SendMessageW( hMainWindow, uMsg, wParam, lParam );
			case WM_COMMAND:
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			{
				if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) && wParam == VK_F4 )
				{
					if ( uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN )
						SendMessageW( hwnd, WM_USER + 101, 0, 0 );
				}
				else
				{
					//     HWND hh=FindWindowExW(hwnd,NULL,NULL,NULL);
					//    if (hh) PostMessageW(hh,uMsg,wParam,lParam);
					//		  else PostMessageW(hMainWindow,uMsg,wParam,lParam);
				}
			}
			break;
			case WM_USER + 101:
			{
				HWND hh = FindWindowExW( hwnd, NULL, NULL, NULL );
				if ( hh ) PostMessageW( hh, WM_CLOSE, 0, 0 );
			}
			return 0;
			case WM_USER + 102:
			{
				embedWindowState *state = (embedWindowState *) GetWindowLongPtrW( hwnd, GWLP_USERDATA );
				if ( !state || !state->reparenting )
					ShowWindow( hwnd, SW_SHOWNA );
			}
			break;
			case WM_USER + 103:
				SetFocus( hwnd );
				break;
			case WM_SHOWWINDOW:
				EmbedWindow_OnShowWindow( hwnd, (BOOL) wParam, (UINT) lParam );

				RefreshIconicThumbnail();
				return 0;

			case WM_DISPLAYCHANGE:
			{
				HWND hh = FindWindowExW( hwnd, NULL, NULL, NULL );
				if ( hh )
					SendMessageW( hh, uMsg, wParam, lParam );
			}
			
			InvalidateRect( hwnd, NULL, TRUE );
			return 0;
			case WM_CLOSE:
				SendMessageW( GetParent( hwnd ), WM_CLOSE, 0, 0 );
				return 0;

			case WM_PAINT:
			{
				embedWindowState *state = (embedWindowState *) GetWindowLongPtrW( hwnd, GWLP_USERDATA );
				if ( state ) draw_paint_emb( hwnd, ( state->r.right - state->r.left ), ( state->r.bottom - state->r.top ), state->flags );
			}
			return 0;

			case WM_WINDOWPOSCHANGING:
			{
				/*
				 if extra_data[EMBED_STATE_EXTRA_REPARENTING] is set, we are being reparented by the freeform lib, so we should
				 just ignore this message because our visibility will not change once the freeform
				 takeover/restoration is complete
				*/

				WINDOWPOS *windowPos = (WINDOWPOS *) lParam;
				embedWindowState *state = (embedWindowState *) GetWindowLongPtrW( hwnd, GWLP_USERDATA );
				if ( state && state->reparenting )
				{
					if ( 0 != ( WS_CHILD & GetWindowLongPtrW( windowPos->hwnd, GWL_STYLE ) ) )
						windowPos->flags |= ( SWP_NOREDRAW );
					break;

				}
			}
			break;

			case WM_WINDOWPOSCHANGED:
			{
				embedWindowState *state = (embedWindowState *) GetWindowLongPtrW( hwnd, GWLP_USERDATA );
				if ( state && 0 == ( SWP_NOSIZE & ( (WINDOWPOS *) lParam )->flags ) )
				{

					HWND hh;
					HRGN rgnChild;
					INT cx, cy, ox;//, oy;
					RECT rv;

					ox = state->r.right - state->r.left;
					//oy = state->r.bottom - state->r.top;
					cx = ( (WINDOWPOS *) lParam )->cx;
					cy = ( (WINDOWPOS *) lParam )->cy;

					state->r.right  = state->r.left + cx;
					state->r.bottom = state->r.top + cy;

					hh = FindWindowExW( hwnd, NULL, NULL, NULL );

					if ( 0 == ( SWP_NOREDRAW & ( (WINDOWPOS *) lParam )->flags ) )
						InvalidateRect( hwnd, NULL, FALSE );

					if ( hh )
					{
						INT cx, cy;
						cx = ( state->r.right - state->r.left ) - 11 - 8;
						cy = ( state->r.bottom - state->r.top ) - 20 - 14;

						SetRect( &rv, 11, 20, 11 + cx, 20 + cy );
						ValidateRect( hwnd, &rv );

						rgnChild = CreateRectRgn( 0, 0, cx, cy );

						if ( IsWindowVisible( hh ) )
							SendMessageW( hh, WM_USER + 0x201, MAKEWPARAM( 0, 0 ), (LPARAM) rgnChild );
						
						SetWindowPos( hh, NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS );
						
						if ( IsWindowVisible( hh ) )
							SendMessageW( hh, WM_USER + 0x201, 0, (LPARAM) NULL );

					}
					else rgnChild = NULL;

					if ( ox == cx )
					{
						SetRect( &rv, 0, 0, cx, 14 );
						ValidateRect( hwnd, &rv );
					}

					if ( 0 == ( SWP_NOREDRAW & ( (WINDOWPOS *) lParam )->flags ) )
					{
						HRGN rgnWnd;
						rgnWnd = CreateRectRgn( 0, 0, 0, 0 );

						if ( GetUpdateRect( hwnd, NULL, FALSE ) )
							GetUpdateRgn( hwnd, rgnWnd, FALSE );

						if ( rgnChild )
						{
							OffsetRgn( rgnChild, 11, 20 );
							CombineRgn( rgnWnd, rgnWnd, rgnChild, RGN_OR );
						}

						RedrawWindow( hwnd, NULL, rgnWnd, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN );
						if ( rgnWnd )
							DeleteObject( rgnWnd );
					}

					if ( rgnChild )
						DeleteObject( rgnChild );
				}
			}
			return 0;

			case WM_CREATE:
			{
				EMBEDWND *pew = (EMBEDWND *) calloc( 1, sizeof( EMBEDWND ) );
				SetPropW( hwnd, EMBEDWND_PROPW, pew );

				SetWindowLongPtrW( hwnd, GWLP_USERDATA, (LONG_PTR) ( (LPCREATESTRUCT) lParam )->lpCreateParams );
				embedWindowState *state = (embedWindowState *) ( (LPCREATESTRUCT) lParam )->lpCreateParams;

				state->me = hwnd;

				int w = ( state->r.right - state->r.left );
				int h = ( state->r.bottom - state->r.top );
				if ( !config_embedwnd_freesize )
				{
					w -= w % 25;
					h -= h % 29;
				}
				if ( w < 275 ) w = 275;
				if ( h < 116 ) h = 116;
				state->r.right = state->r.left + w;
				state->r.bottom = state->r.top + h;

				EnterCriticalSection( &embedcs );
				GUID temp = GUID_NULL;
				if ( state->flags & EMBED_FLAGS_GUID )
					temp = state->guid;

				memset( state->extra_data, 0, sizeof( state->extra_data ) );
				if ( state->flags & EMBED_FLAGS_GUID )
					state->guid = temp;

				state->link = embedwndlist;
				embedwndlist = state;
				embedwndlist_cnt++;
				LeaveCriticalSection( &embedcs );

				SetWindowLong( hwnd, GWL_STYLE, GetWindowLongW( hwnd, GWL_STYLE ) & ~( WS_CAPTION ) );
				SetWindowPos( hwnd, 0, state->r.left, state->r.top, state->r.right - state->r.left, state->r.bottom - state->r.top, SWP_NOACTIVATE | SWP_NOZORDER );
			}
			return 0;
			case WM_DESTROY:
			{
				embedWindowState *state = (embedWindowState *) GetWindowLongPtrW( hwnd, GWLP_USERDATA );
				if ( state )
				{
					EnterCriticalSection( &embedcs );
					embedWindowState *p = embedwndlist;
					if ( p == state )
					{
						embedwndlist = state->link;// remove ourselves
						embedwndlist_cnt--;
					}
					else
					{
						while ( p )
						{
							if ( p->link == state )
							{
								p->link = state->link;
								embedwndlist_cnt--;
								break;
							}
							p = p->link;
						}
					}
					LeaveCriticalSection( &embedcs );
				}
				HWND hh = FindWindowExW( hwnd, NULL, NULL, NULL );
				if ( hh ) DestroyWindow( hh );
				EMBEDWND *pew = GetEmbedWnd( hwnd );
				if ( pew )
				{
					RemovePropW( hwnd, EMBEDWND_PROPW );
					free( pew );
				}
			}
			return 0;
			case WM_SETCURSOR:
				return EmbedWindow_OnSetCursor( hwnd, (HWND) wParam, LOWORD( lParam ), HIWORD( lParam ) );

			case WM_GETMINMAXINFO:
			{
				MINMAXINFO *p = (MINMAXINFO *) lParam;
				if ( NULL != p )
				{
					p->ptMaxTrackSize.x = 16384;
					p->ptMaxTrackSize.y = 16384;
				}
			}
			return 0;

			case WM_MOUSEACTIVATE:
				if ( NULL != WASABI_API_APP )
					WASABI_API_APP->ActiveDialog_Register( hwnd );
				break;

			case WM_CHILDACTIVATE:
				if ( NULL != WASABI_API_APP )
					WASABI_API_APP->ActiveDialog_Register( hwnd );
				break;

			case WM_ACTIVATE:
				if ( WA_INACTIVE == LOWORD( wParam ) )
				{
					EMBEDWND *pew = GetEmbedWnd( hwnd );
					if ( pew )
					{
						pew->hLastFocus = GetFocus();
						if ( !IsChild( hwnd, pew->hLastFocus ) )
							pew->hLastFocus = NULL;
					}

					if ( NULL != WASABI_API_APP )
						WASABI_API_APP->ActiveDialog_Unregister( hwnd );
				}
				else
				{
					if ( WA_CLICKACTIVE == LOWORD( wParam ) )
					{
						EMBEDWND *pew = GetEmbedWnd( hwnd );
						if ( pew )
						{
							POINT pt;
							DWORD pts = GetMessagePos();
							POINTSTOPOINT( pt, pts );
							MapWindowPoints( HWND_DESKTOP, hwnd, &pt, 1 );
							
							HWND hTarget = ChildWindowFromPointEx( hwnd, pt, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT );
							if ( hTarget && hTarget != hwnd )
								pew->hLastFocus = hTarget;
						}
					}

					if ( NULL != WASABI_API_APP )
						WASABI_API_APP->ActiveDialog_Register( hwnd );

				}
				break;
			case WM_SETFOCUS:
			{
				HWND hChild, hTab;
				EMBEDWND *pew;

				hChild = FindWindowExW( hwnd, NULL, NULL, NULL );
				pew = GetEmbedWnd( hwnd );
				hTab = NULL;
				if ( pew )
				{
					while ( pew->hLastFocus && IsChild( hwnd, pew->hLastFocus ) )
					{
						if ( IsWindowEnabled( pew->hLastFocus ) && IsWindowVisible( pew->hLastFocus ) && 0 != ( WS_TABSTOP & GetWindowLongPtrW( pew->hLastFocus, GWL_STYLE ) ) )
						{
							hTab = pew->hLastFocus;
							break;
						}
						
						pew->hLastFocus = GetParent( pew->hLastFocus );
					}
				}

				if ( !hTab )
					hTab = ( hChild ) ? GetNextDlgTabItem( hwnd, hChild, FALSE ) : hwnd;
				
				if ( hTab && hTab != hwnd )
				{
					WCHAR szName[ 128 ] = { 0 };
					DWORD lcid = MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), SORT_DEFAULT );
					if ( GetClassNameW( hChild, szName, sizeof( szName ) / sizeof( WCHAR ) ) && CSTR_EQUAL == CompareStringW( lcid, NORM_IGNORECASE, szName, -1, L"#32770", -1 ) )
						SendMessageW( hChild, WM_NEXTDLGCTL, (WPARAM) hTab, TRUE );
					else
						SetFocus( hTab );
					
					//return 0;
				}

			}
			break;
			case WM_KILLFOCUS:
			{
				EMBEDWND *pew = GetEmbedWnd( hwnd );
				if ( pew )
				{
					pew->hLastFocus = GetFocus();
					if ( !IsChild( hwnd, pew->hLastFocus ) )
						pew->hLastFocus = NULL;
				}
			}
			break;
		}

		if ( FALSE != IsDirectMouseWheelMessage( uMsg ) )
		{
			if ( ( WS_CHILD & GetWindowStyle( hwnd ) ) == 0 )
				return TRUE;
			else
			{
				HWND hParent;
				hParent = GetAncestor( hwnd, GA_PARENT );
				if ( hParent != NULL )
					return SendMessageW( hwnd, uMsg, wParam, lParam );

				return FALSE;
			}
		}

		return DefWindowProcW( hwnd, uMsg, wParam, lParam );
	}

	HWND embedWindow( embedWindowState *state )
	{
		HWND hwnd;
		if ( !state ) return NULL;

		hwnd = CreateWindowExW( WS_EX_NOPARENTNOTIFY /*| WS_EX_CONTROLPARENT | WS_EX_TOOLWINDOW*/,
								L"Winamp Gen",
								L"",
								WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
								0, 0, 200, 200,
								hMainWindow,
								NULL,
								hMainInstance,
								state );
		return hwnd;
	}

	BOOL SnapToScreen( RECT *outrc )
	{
		if ( config_keeponscreen & 1 )
		{
			RECT rc;
			int w = outrc->right - outrc->left;
			int h = outrc->bottom - outrc->top;

			getViewport( &rc, NULL, 0, outrc );

			if ( outrc->left < ( rc.left + config_snaplen ) && outrc->left >( rc.left - config_snaplen ) )
			{
				outrc->left  = rc.left;
				outrc->right = rc.left + w;
			}

			if ( outrc->top < ( rc.top + config_snaplen ) && outrc->top >( rc.top - config_snaplen ) )
			{
				outrc->top    = rc.top;
				outrc->bottom = rc.top + h;
			}

			if ( outrc->right > rc.right - config_snaplen && outrc->right < rc.right + config_snaplen )
			{
				outrc->left  = rc.right - w;
				outrc->right = rc.right;
			}
			
			if ( outrc->bottom > rc.bottom - config_snaplen && outrc->bottom < rc.bottom + config_snaplen )
			{
				outrc->top    = rc.bottom - h;
				outrc->bottom = rc.bottom;
				
				return TRUE;
			}
		}
		
		return FALSE;
	}

	void SnapWindowToAllWindows( RECT *outrc, HWND hwndNoSnap )
	{
		RECT rc;
		SnapToScreen( outrc );

		if ( config_pe_open && hwndNoSnap != hPLWindow )
		{
			GetWindowRect( hPLWindow, &rc );
			SnapWindowToWindow( outrc, rc );
		}
		
		if ( config_eq_open && hwndNoSnap != hEQWindow )
		{
			GetWindowRect( hEQWindow, &rc );
			SnapWindowToWindow( outrc, rc );
		}
		
		if ( config_mw_open && hwndNoSnap != hMainWindow )
		{
			GetWindowRect( hMainWindow, &rc );
			FixMainWindowRect( &rc );
			SnapWindowToWindow( outrc, rc );
		}
		
		if ( config_video_open && hwndNoSnap != hVideoWindow )
		{
			GetWindowRect( hVideoWindow, &rc );
			SnapWindowToWindow( outrc, rc );
		}

		EnterCriticalSection( &embedcs );
		embedWindowState *state = embedwndlist;
		while ( state )
		{
			if ( state->me != hwndNoSnap && IsWindowVisible( state->me ) )
				SnapWindowToWindow( outrc, state->r );
			
			state = state->link;
		}

		LeaveCriticalSection( &embedcs );
	}
};