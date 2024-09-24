#include "main.h"
#include "api__gen_ml.h"
#include <windowsx.h>
#include <time.h>
#include <rpc.h>
#include "../winamp/gen.h"
#include "resource.h"
#include "../nu/ns_wc.h"
#include "config.h"
#include "../winamp/ipc_pe.h"
#include "../winamp/wa_dlg.h"

#include "ml.h"
#include "ml_ipc.h"
#include "sendto.h"
#include "../gen_hotkeys/wa_hotkeys.h"
#include "MediaLibraryCOM.h"
#include "../nu/CCVersion.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"

#include "./navigation.h"

#include "./ml_imagelist.h"
#include "./ml_imagefilter.h"
#include "./imagefilters.h" // default filters 

#include <shlwapi.h>
#include "./ml_ipc_0313.h"
#include "./skinning.h"
#include "./ml_ratingcolumn.h"
#include "./ml_cloudcolumn.h"
#include "./colors.h"
#include "./stockobjects.h"
#include "./service.h"
#include "./skinnedbutton.h"
#include "../Winamp/wasabicfg.h"
#ifdef _DEBUG
#define BETA
#endif

// messages
#define WMML_FIRST				(WM_APP + 0)
#define WMML_UPDATEVIEW			(WMML_FIRST + 1)
#define WMML_SHOWCONTAINER		(WMML_FIRST + 10)

#define BLOCK_ACCELTOGGLE_PROP TEXT("BLOCK_ACCELTOGGLE")

// timers
#define TIMER_UPDATEVIEW_ID			1970
#define TIMER_UPDATEVIEW_DELAY		7
#define TIMER_NAVAUTOSCROLL_ID		1971
#define TIMER_NAVAUTOSCROLL_DELAY	80
#define TIMER_NAVAUTOEXPAND_ID		1972
#define TIMER_NAVAUTOEXPAND_DELAY	500
#define TIMER_UNBLOCKACCELTOGGLE_ID	1973
#define TIMER_UNBLOCKACCELTOGGLE_DELAY	200

#define IDC_CURRENTVIEW			0x1001
#define IDC_NAVIGATION			0x03FD // (this is the same that old versions used)


#define MEDIALIBRARY_HELP_URL		L"https://help.winamp.com/hc/articles/8105304048660-The-Winamp-Media-Library"

SendToMenu *main_sendtomenu;
HMENU main_sendto_hmenu;
int main_sendto_mode;

extern "C" HWND g_ownerwnd;

static int ldiv_clickoffs, ldiv_paintleftborder;
static WNDPROC ldiv_oldWndProc;
static WNDPROC add_oldWndProc;

HNAVITEM g_treedrag_lastSel;
static HNAVITEM m_query_moving_dragplace;
static HNAVITEM m_query_moving_item, m_query_moving_lastdest;
static int m_query_moving_dragplaceisbelow;
static int m_query_moving;
static int m_query_moving_type;

HWND m_curview_hwnd = NULL;

HNAVCTRL hNavigation = NULL;			// navigation control
HMLIMGLST hmlilRating = NULL;			// default rating images
HMLIMGLST hmlilCloud = NULL;			// default cloud images
UINT ratingGlobalStyle = RATING_DEFAULT_STYLE;

static HMLIMGLST hmlilNavigation = NULL;	// default navigation image list
HMLIMGFLTRMNGR hmlifMngr = NULL;			/// default image filters

static HRGN g_rgnUpdate = NULL;
static INT	divider_pos;

static BOOL firstShow = TRUE;
static BOOL m_nav_autoscroll = FALSE;
static HNAVITEM m_nav_autoexpand_item = NULL;

static HIMAGELIST m_nav_dragitem_imagelist = NULL;

void OpenMediaLibraryPreferences();

typedef struct _LAYOUT
{
	INT		id;
	HWND	hwnd;
	INT		x;
	INT		y;
	INT		cx;
	INT		cy;
	DWORD	flags;
	HRGN	rgn;
}LAYOUT, PLAYOUT;

#define SETLAYOUTPOS(_layout, _x, _y, _cx, _cy) { _layout->x=_x; _layout->y=_y;_layout->cx=_cx;_layout->cy=_cy;_layout->rgn=NULL; }

static void LayoutWindows( HWND hwnd, INT divX, BOOL fRedraw )
{
	static BOOL useDefer = ( GetVersion() < 0x80000000 );
	static INT controls[] = { IDC_BTN_LIB, IDC_NAVIGATION, IDC_VDELIM, IDC_CURRENTVIEW, IDC_NO_VIEW };
	RECT rc, ri, roTree;
	LAYOUT layout[ sizeof( controls ) / sizeof( controls[ 0 ] ) ], *pl;

	INT	divCX = 0, btnY;

	GetClientRect( hwnd, &rc );
	if ( rc.bottom == rc.top || rc.left == rc.right )
		return;

	if ( rc.bottom > WASABI_API_APP->getScaleY( 2 ) )
		rc.bottom -= WASABI_API_APP->getScaleY( 2 );

	HRGN rgn = CreateRectRgn( 0, 0, 0, 0 );

	if ( divX > rc.right - WASABI_API_APP->getScaleY( 41 ) )
		divX = rc.right - WASABI_API_APP->getScaleY( 9 );

	if ( divX < WASABI_API_APP->getScaleY( 32 ) )
		divX = 0;
	else if ( divX > ( rc.right - rc.left ) )
		divX = ( rc.right - rc.left );

	pl = layout;
	btnY = rc.bottom; // in case button is broken

	InvalidateRgn( hwnd, NULL, TRUE );

	for ( int i = 0; i < sizeof( controls ) / sizeof( controls[ 0 ] ); i++ )
	{
		pl->id = controls[ i ];
		switch ( pl->id )
		{
			case IDC_NAVIGATION:	pl->hwnd = NavCtrlI_GetHWND( hNavigation ); break;
			case IDC_CURRENTVIEW:	pl->hwnd = m_curview_hwnd; break;
			default:				pl->hwnd = GetDlgItem( hwnd, pl->id ); break;
		}

		if ( !pl->hwnd )
			continue;

		pl->rgn = NULL;
		GetWindowRect( pl->hwnd, &ri );
		MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&ri, 2 );

		pl->flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS;

		switch ( pl->id )
		{
			case IDC_BTN_LIB:
			{
				wchar_t buffer[ 128 ] = { 0 };
				GetWindowTextW( pl->hwnd, buffer, ARRAYSIZE( buffer ) );
				LRESULT idealSize = MLSkinnedButton_GetIdealSize( pl->hwnd, buffer );
				btnY = WASABI_API_APP->getScaleY( HIWORD( idealSize ) );
				SETLAYOUTPOS( pl, rc.left, rc.bottom - btnY, rc.left + divX, btnY );
				break;
			}
			case IDC_NAVIGATION:
				GetClientRect( pl->hwnd, &roTree );
				NavCtrlI_MapPointsTo( hNavigation, hwnd, (LPPOINT)&roTree, 2 );
				SETLAYOUTPOS( pl, rc.left, rc.top, rc.left + divX, rc.bottom - btnY - WASABI_API_APP->getScaleY( 3 ) );
				ldiv_paintleftborder = ( pl->cx > 0 );
				break;
			case IDC_VDELIM:
				divCX = ( ri.right - ri.left );
				SETLAYOUTPOS( pl, divX + WASABI_API_APP->getScaleX( 1 ), rc.top, divCX, max( 0, rc.bottom - rc.top ) );
				break;
			case IDC_CURRENTVIEW: // current view;
				SETLAYOUTPOS( pl, divX + divCX, rc.top, max( 0, rc.right - pl->x - WASABI_API_APP->getScaleX( 2 ) ), max( 0, rc.bottom - rc.top ) );
				if ( !SendMessage( pl->hwnd, WM_USER + 0x200, 0, 0 ) ) pl->flags &= ~( SWP_NOREDRAW );
				break;
			case IDC_NO_VIEW:
				SETLAYOUTPOS( pl, divX + divCX + 20, rc.top, max( 0, rc.right - pl->x - WASABI_API_APP->getScaleX( 2 ) - 20 ), max( 0, rc.bottom - rc.top ) );
				if ( !SendMessage( pl->hwnd, WM_USER + 0x200, 0, 0 ) ) pl->flags &= ~( SWP_NOREDRAW );
				break;
		}

		if ( pl->x == ri.left && pl->y == ri.top )
			pl->flags |= SWP_NOMOVE;

		if ( pl->cx == ( ri.right - ri.left ) && pl->cy == ( ri.bottom - ri.top ) )
			pl->flags |= SWP_NOSIZE;

		if ( ( SWP_NOMOVE | SWP_NOSIZE ) != ( ( SWP_NOMOVE | SWP_NOSIZE ) & pl->flags ) )
			pl++;
		else if ( IsWindowVisible( pl->hwnd ) )
		{
			ValidateRect( hwnd, &ri );
			if ( GetUpdateRect( pl->hwnd, NULL, FALSE ) )
			{
				GetUpdateRgn( pl->hwnd, rgn, FALSE );
				OffsetRgn( rgn, pl->x, pl->y );
				InvalidateRgn( hwnd, rgn, FALSE );
			}
		}
	}

	if ( pl != layout )
	{
		LAYOUT *pc;
		HDWP hdwp = ( useDefer ) ? BeginDeferWindowPos( (INT)( pl - layout ) ) : NULL;
		for ( pc = layout; pc < pl && ( !useDefer || hdwp ); pc++ )
		{
			if ( IDC_CURRENTVIEW == pc->id && ( SWP_NOREDRAW & pc->flags ) && IsWindowVisible( pc->hwnd ) )
			{
				if ( !pc->rgn )
					pc->rgn = CreateRectRgn( 0, 0, pc->cx, pc->cy );

				GetWindowRect( pc->hwnd, &ri );
				NavCtrlI_MapPointsFrom( hNavigation, HWND_DESKTOP, (LPPOINT)&ri, 1 );
				SendMessage( pc->hwnd, WM_USER + 0x201, MAKEWPARAM( pc->x - ri.left, pc->y - ri.top ), (LPARAM)pc->rgn );
			}
			if ( useDefer )
				hdwp = DeferWindowPos( hdwp, pc->hwnd, NULL, pc->x, pc->y, pc->cx, pc->cy, pc->flags );
			else
				SetWindowPos( pc->hwnd, NULL, pc->x, pc->y, pc->cx, pc->cy, pc->flags );
		}

		if ( hdwp )
			EndDeferWindowPos( hdwp );

		for ( pc = layout; pc < pl; pc++ )
		{
			if ( IDC_NAVIGATION == pc->id )
			{
				GetWindowRect( pc->hwnd, &ri );
				OffsetRect( &ri, -ri.left, -ri.top );
				pc->rgn = CreateRectRgnIndirect( &ri );

				GetClientRect( pc->hwnd, &ri );

				NavCtrlI_MapPointsTo( hNavigation, hwnd, (LPPOINT)&ri, 1 );
				IntersectRect( &ri, &roTree, &ri );
				SetRectRgn( rgn, ri.left, ri.top, ri.right, ri.bottom );
				CombineRgn( pc->rgn, pc->rgn, rgn, RGN_DIFF );
				if ( GetUpdateRect( pc->hwnd, NULL, FALSE ) )
				{
					GetUpdateRgn( pc->hwnd, rgn, FALSE );
					CombineRgn( pc->rgn, pc->rgn, rgn, RGN_OR );
				}
			}
			else if ( IDC_CURRENTVIEW == pc->id && pc->rgn )
				SendMessage( pc->hwnd, WM_USER + 0x201, 0, 0L );

			if ( IsWindowVisible( pc->hwnd ) )
			{
				GetWindowRect( pc->hwnd, &ri );
				MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&ri, 2 );
				ValidateRect( hwnd, &ri );
			}
		}

		if ( fRedraw )
		{
			HRGN rgnTmp = CreateRectRgn( 0, 0, 0, 0 );
			GetUpdateRgn( hwnd, rgn, FALSE );
			for ( pc = layout; pc < pl; pc++ )
			{
				if ( SWP_NOREDRAW & pc->flags )
				{
					if ( pc->rgn )
						OffsetRgn( pc->rgn, pc->x, pc->y );
					else
						SetRectRgn( rgnTmp, pc->x, pc->y, pc->x + pc->cx, pc->y + pc->cy );

					CombineRgn( rgn, rgn, ( pc->rgn ) ? pc->rgn : rgnTmp, RGN_OR );
				}
			}

			DeleteObject( rgnTmp );
			RedrawWindow( hwnd, NULL, rgn, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN );
		}
		else if ( g_rgnUpdate )
		{
			GetUpdateRgn( hwnd, g_rgnUpdate, FALSE );
			ValidateRect( hwnd, NULL );
			for ( pc = layout; pc < pl; pc++ )
			{
				if ( SWP_NOREDRAW & pc->flags )
				{
					if ( pc->rgn )
						OffsetRgn( pc->rgn, pc->x, pc->y );
					else
						SetRectRgn( rgn, pc->x, pc->y, pc->x + pc->cx, pc->y + pc->cy );

					CombineRgn( g_rgnUpdate, g_rgnUpdate, ( pc->rgn ) ? pc->rgn : rgn, RGN_OR );
				}
			}
		}

		for ( pc = layout; pc < pl; pc++ )
			if ( pc->rgn )
				DeleteObject( pc->rgn );
	}

	if ( rgn )
		DeleteObject( rgn );
}

// a default right-pane view to show if a plugin failed to return us an HWND
static INT_PTR CALLBACK view_Error( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_INITDIALOG )
	{
		ShowWindow( GetDlgItem( g_hwnd, IDC_NO_VIEW ), SW_SHOW );
	}

	return WADlg_handleDialogMsgs( hwndDlg, uMsg, wParam, lParam );
}

static void CreateCurrentView( HWND hwndDlg )
{
	HNAVITEM hItem;
	DLGPROC proc;
	proc = view_Error;

	hItem = NavCtrlI_GetSelection( hNavigation );
	if ( hItem )
	{
		INT itemId = NavItemI_GetId( hItem );

#if 0
#ifdef BETA
		wchar_t pszText[ 32 ] = { 0 };
		if ( NavItemI_GetInvariantText( hItem, pszText, 32 ) && !lstrcmpW( pszText, L"winamp_labs" ) )
		{
			OmService *om_service;
			OmService::CreateInstance( SERVICE_LABS, L"Winamp Labs", &om_service );

			if ( AGAVE_OBJ_BROWSER )
			{
				HWND hView = 0;
				HRESULT hr = AGAVE_OBJ_BROWSER->CreateView( om_service, (HWND)hwndDlg, 0, 0, &hView );
				om_service->Release();
				if ( SUCCEEDED( hr ) )
				{
					m_curview_hwnd = hView;
				}
			}
		}
		else
#endif
#endif
			m_curview_hwnd = (HWND)plugin_SendMessage( ML_MSG_TREE_ONCREATEVIEW, (INT_PTR)itemId, (INT_PTR)hwndDlg, 0 );
	}

	if ( !IsWindow( m_curview_hwnd ) )
		m_curview_hwnd = WASABI_API_CREATEDIALOGPARAMW( IDD_VIEW_EMPTY, hwndDlg, proc, 0 );
	else
		ShowWindow( GetDlgItem( g_hwnd, IDC_NO_VIEW ), SW_HIDE );

	if ( IsWindow( m_curview_hwnd ) )
	{
		SetWindowPos( m_curview_hwnd, NavCtrlI_GetHWND( hNavigation ), 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );
		LayoutWindows( hwndDlg, divider_pos, FALSE );
		// moved 08/12/09 from start of block to here along with SkinnedWnd::OnSkinChanged(..) change to hopefully resolve bold font issues
		MLSkinnedWnd_SkinChanged( m_curview_hwnd, TRUE, FALSE );
		ShowWindow( m_curview_hwnd, SW_SHOWNORMAL );
		RedrawWindow( m_curview_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN );
		PostMessage( m_curview_hwnd, WMML_UPDATEVIEW, 0, 0 ); //refresh view
	}
}

static void DisplayItemDragImage( HNAVITEM hItem, LPCWSTR pszTip, HWND hwndOwner, POINT *ppt ) // ppt in hwndOwner coordinates
{
	if ( !hwndOwner )
		return;

	if ( m_nav_dragitem_imagelist )
	{
		ImageList_DragLeave( hwndOwner );
		ImageList_EndDrag();
		ImageList_Destroy( m_nav_dragitem_imagelist );
	}

	m_nav_dragitem_imagelist = NavItemI_CreateDragImage( hItem, pszTip );

	if ( m_nav_dragitem_imagelist )
	{
		INT cx, cy;
		ImageList_GetIconSize( m_nav_dragitem_imagelist, &cx, &cy );
		if ( ImageList_BeginDrag( m_nav_dragitem_imagelist, 0, cx / 2, cy / 2 ) )
		{
			NavCtrlI_Update( hNavigation );
			ImageList_DragEnter( hwndOwner, ppt->x, ppt->y );
		}
	}
}

VOID CALLBACK CreateViewTimerProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
	if ( idEvent == TIMER_UPDATEVIEW_ID )
	{
		KillTimer( hwnd, TIMER_UPDATEVIEW_ID );
		CreateCurrentView( hwnd );
	}
}


static void CALLBACK OnNavCtrl_Selected( HNAVCTRL hMngr, HNAVITEM hItemOld, HNAVITEM hItemNew )
{
	KillTimer( g_hwnd, TIMER_UPDATEVIEW_ID );
	if ( IsWindow( m_curview_hwnd ) )
	{
		RedrawWindow( g_hwnd, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN );
		SendMessageW( g_hwnd, WM_SETREDRAW, FALSE, 0L );  // freeze window
		ShowWindow( m_curview_hwnd, SW_HIDE );
		SendMessageW( g_hwnd, WM_SETREDRAW, TRUE, 0L );
		DestroyWindow( m_curview_hwnd );

		m_curview_hwnd = NULL;
	}
	// 07/05/2010 DRO - changed to use a callback proc as ml_disc was managing to trigger
	// the same timer message multiple times on (at least) older XP machines which was
	// causing multiple ml view dialogs to be created but hidden behind the currently
	// opened view - intermittent issue i've been seeing for a year on my XP machine.
	SetTimer( g_hwnd, TIMER_UPDATEVIEW_ID, TIMER_UPDATEVIEW_DELAY, CreateViewTimerProc );
}

static BOOL CALLBACK OnNavCtrl_Click( HNAVCTRL hMngr, HNAVITEM hItem, INT actionId )
{
	if ( hItem )
	{
		INT mlAction = -1;

		switch ( actionId )
		{
			case ACTION_CLICKL_I:		mlAction = ML_ACTION_LCLICK; break;
			case ACTION_CLICKR_I:		mlAction = ML_ACTION_RCLICK; break;
			case ACTION_ENTER_I:		mlAction = ML_ACTION_ENTER; break;
			case ACTION_DBLCLICKL_I:	mlAction = ML_ACTION_DBLCLICK; break;
			case ACTION_DBLCLICKR_I:	break;
		}

		if ( -1 != mlAction )
		{
			BOOL ret = (BOOL)plugin_SendMessage( ML_MSG_TREE_ONCLICK,
				NavItemI_GetId( hItem ),
				mlAction,
				(INT_PTR)GetParent( NavCtrlI_GetHWND( hMngr ) ) );

			if ( mlAction != ML_ACTION_LCLICK )
				return ret;
		}
	}
	return FALSE;
}

static BOOL CALLBACK OnNavCtrl_KeyDown( HNAVCTRL hMngr, HNAVITEM hItem, NMTVKEYDOWN *ptvkd )
{
	return ( hItem ) ? (BOOL)plugin_SendMessage( ML_MSG_TREE_ONKEYDOWN,
		NavItemI_GetId( hItem ),
		(INT_PTR)ptvkd,
		(INT_PTR)NavCtrlI_GetHWND( hMngr ) ) : FALSE;
}

static BOOL CALLBACK OnNavCtrl_BeginTitleEdit( HNAVCTRL hMngr, HNAVITEM hItem )
{
	return !(BOOL)plugin_SendMessage( ML_MSG_NAVIGATION_ONBEGINTITLEEDIT, (INT_PTR)hItem, 0, 0 );
}

static BOOL CALLBACK OnNavCtrl_EndTitleEdit( HNAVCTRL hMngr, HNAVITEM hItem, LPCWSTR pszNewTitle )
{
	return (BOOL)plugin_SendMessage( ML_MSG_NAVIGATION_ONENDTITLEEDIT, (INT_PTR)hItem, (INT_PTR)pszNewTitle, 0 );
}

static void CALLBACK OnNavItem_Delete( HNAVCTRL hMngr, HNAVITEM hItem )
{
	plugin_SendMessage( ML_MSG_NAVIGATION_ONDELETE, (INT_PTR)hItem, 0, 0 );
}

static void CALLBACK OnNavCtrl_Destroy( HNAVCTRL hMngr )
{
	plugin_SendMessage( ML_MSG_NAVIGATION_ONDESTROY, 0, 0, 0 );
}

static INT CALLBACK OnNavItem_CustomDraw( HNAVCTRL hMngr, HNAVITEM hItem, NAVITEMDRAW_I *pnicd, LPARAM lParam )
{
	INT result = (INT)plugin_SendMessage( ML_MSG_NAVIGATION_ONCUSTOMDRAW, (INT_PTR)hItem, (INT_PTR)pnicd, (INT_PTR)lParam );

	if ( NICDRF_NOTMINE == result )
		result = NICDRF_DODEFAULT_I;

	return result;
}

static INT CALLBACK OnNavItem_SetCursor( HNAVCTRL hMngr, HNAVITEM hItem, LPARAM lParam )
{
	INT result = (INT)plugin_SendMessage( ML_MSG_NAVIGATION_ONSETCURSOR, (INT_PTR)hItem, (INT_PTR)0, (INT_PTR)lParam );

	return ( result > 0 );
}

static void CALLBACK OnNavItem_HitTest( HNAVCTRL hMngr, POINT pt, UINT *pHitFlags, HNAVITEM *phItem, LPARAM lParam )
{
	NAVHITTEST ht;

	ht.flags = *pHitFlags;
	ht.pt = pt;
	ht.hItem = *phItem;

	if ( 0 != plugin_SendMessage( ML_MSG_NAVIGATION_ONHITTEST, (INT_PTR)*phItem, (INT_PTR)&ht, (INT_PTR)lParam ) )
	{
		*pHitFlags = ht.flags;
		*phItem = ht.hItem;
	}
}

static void CALLBACK OnNavCtrl_BeginDrag( HNAVCTRL hMngr, HNAVITEM hItem, POINT pt )
{
#ifdef BETA
	wchar_t pszText[ 32 ] = { 0 };
	if ( ( !( sneak & 4 ) ) && NavItemI_GetInvariantText( hItem, pszText, 32 ) && !lstrcmpW( pszText, L"winamp_labs" ) )
	{
		return;
	}

#endif
	if ( plugin_SendMessage( ML_MSG_TREE_ONDRAG, NavItemI_GetId( hItem ), (INT_PTR)&pt, (INT_PTR)&m_query_moving_type ) < 1 )
	{
		HNAVITEM hParent;
		hParent = NavItemI_GetParent( hItem );
		m_query_moving_type = ( NULL == hParent || NIS_ALLOWCHILDMOVE_I == NavItemI_GetStyle( hParent, NIS_ALLOWCHILDMOVE_I ) ) ?
			ML_TYPE_TREEITEM : ML_TYPE_UNKNOWN;
	}

	m_query_moving = 1;
	m_query_moving_item = hItem;
	m_query_moving_dragplace = NULL;
	m_query_moving_lastdest = NULL;
	m_query_moving_dragplaceisbelow = FALSE;

	HWND hwndDlg = GetParent( NavCtrlI_GetHWND( hMngr ) );
	NavCtrlI_MapPointsTo( hMngr, hwndDlg, &pt, 1 );
	DisplayItemDragImage( hItem, NULL, hwndDlg, &pt );
	SetCapture( g_hwnd );
}

static INT CALLBACK OnNavCtrl_GetImageIndex( HNAVCTRL hMngr, HNAVITEM hItem, INT imageType )
{
	if ( NavItemI_HasChildren( hItem ) )
	{
		INT_PTR tag;
		tag = ( NavItemI_HasChildrenReal( hItem ) ) ?
			( ( NavItemI_IsExpanded( hItem ) ) ? MLTREEIMAGE_BRANCH_EXPANDED : MLTREEIMAGE_BRANCH_COLLAPSED ) :
			MLTREEIMAGE_BRANCH_NOCHILD;
		INT	mlilIndex = MLImageListI_GetIndexFromTag( NavCtrlI_GetImageList( hMngr ), tag );

		return ( -1 != mlilIndex ) ? mlilIndex : 0;
	}

	return 0;
}

static void OnWindowPosChanged( HWND hwnd, WINDOWPOS *pwp )
{
	if ( SWP_NOSIZE != ( ( SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW ) & pwp->flags ) )
	{
		LayoutWindows( hwnd, divider_pos, ( 0 == ( SWP_NOREDRAW & pwp->flags ) ) );
	}
}

HWND wa3wnd_fixwnd( HWND h )
{
	if ( IsChild( h, g_hwnd ) )
		return h; // if the library window is a child of this window, don't touch it

	if ( IsChild( h, g_PEWindow ) )
		return g_PEWindow; // if the playlist window is a child of this window, treat it as the playlist window

	HWND hParent = (HWND)SENDWAIPC( plugin.hwndParent, IPC_GETDIALOGBOXPARENT, 0 );
	if ( plugin.hwndParent != hParent && h == hParent )
		h = plugin.hwndParent;

	//DWORD pid;
	//DWORD threadID = GetWindowThreadProcessId(h, &pid);
	//if (pid != GetCurrentProcessId() || threadID != GetCurrentThreadId()) return h;	// if other process, dont mangle
	//char buf[64] = {0};
	//GetClassName(h, buf, sizeof(buf) - 1);
	//if (!strcmp(buf, "BaseWindow_RootWnd")) return plugin.hwndParent; // if a wa3 window, treat as main window
	return h;
}


static void CALLBACK OnDividerMoved( HWND hdiv, INT nPos, LPARAM param )
{
	HWND hwndParent;
	divider_pos = nPos;
	hwndParent = GetParent( hdiv );
	LayoutWindows( hwndParent, nPos, TRUE );
}

static void Navigation_DropHelper( HWND hwndDlg, HWND hwndNav, HNAVITEM hItemHit, UINT hitFlags, POINT pt )
{
	if ( hItemHit && ( ( NAVHT_ONITEMINDENT_I | NAVHT_ONITEMRIGHT_I | NAVHT_ONITEM_I | NAVHT_ONITEMBUTTON_I ) & hitFlags ) )
	{
		RECT rc;
		GetClientRect( hwndNav, &rc );
		if ( !m_nav_autoscroll && ( rc.left <= pt.x && rc.right >= pt.x ) )
		{
			INT sbCommand;
			if ( rc.top <= pt.y && pt.y < ( rc.top + 24 ) )
				sbCommand = 1;
			else if ( ( rc.bottom - 24 ) < pt.y && pt.y <= rc.bottom )
				sbCommand = 2;
			else
				sbCommand = 0;

			if ( sbCommand )
			{
				SCROLLINFO si = { 0 };
				si.cbSize = sizeof( SCROLLINFO );
				si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
				if ( !GetScrollInfo( hwndNav, SB_VERT, &si ) )
					ZeroMemory( &si, sizeof( SCROLLINFO ) );

				if ( ( 1 == sbCommand && si.nMin < si.nPos ) || ( 2 == sbCommand && si.nMax >= (INT)( si.nPos + si.nPage ) ) )
				{
					if ( SetTimer( hwndDlg, TIMER_NAVAUTOSCROLL_ID, TIMER_NAVAUTOSCROLL_DELAY, NULL ) )
						m_nav_autoscroll = TRUE;
				}
			}
		}

		if ( ( NAVHT_ONITEMBUTTON_I & hitFlags ) && m_nav_autoexpand_item != hItemHit && !NavItemI_IsExpanded( hItemHit ) )
		{
			KillTimer( hwndDlg, TIMER_NAVAUTOEXPAND_ID );
			if ( SetTimer( hwndDlg, TIMER_NAVAUTOEXPAND_ID, TIMER_NAVAUTOEXPAND_DELAY, NULL ) )
				m_nav_autoexpand_item = hItemHit;
		}
	}
}

int handleDragDropMove( HWND hwndDlg, int type, POINT p, int do_cursors )
{
	BOOL valid = FALSE;
	HWND h = WindowFromPoint( p );
	HNAVITEM hItemPrevHilited = g_treedrag_lastSel;
	g_treedrag_lastSel = NULL;

	if ( h )
	{
		h = wa3wnd_fixwnd( h );

		if ( IsChild( plugin.hwndParent, h ) )
			h = plugin.hwndParent;
		else if ( g_PEWindow && IsChild( g_PEWindow, h ) )
			h = g_PEWindow;
		else
		{
			HWND vid = (HWND)SendMessage( plugin.hwndParent, WM_WA_IPC, IPC_GETWND_VIDEO, IPC_GETWND );
			if ( vid )
			{
				if ( h == vid || IsChild( vid, h ) )
					h = plugin.hwndParent;
			}
		}

		if ( h && ( h == plugin.hwndParent || h == g_PEWindow ) )
		{
			valid = ( type == ML_TYPE_ITEMRECORDLISTW || type == ML_TYPE_ITEMRECORDLIST ||
				type == ML_TYPE_FILENAMES || type == ML_TYPE_STREAMNAMES ||
				type == ML_TYPE_CDTRACKS ||
				type == ML_TYPE_FILENAMESW || type == ML_TYPE_STREAMNAMESW );
		}
		else if ( h == NavCtrlI_GetHWND( hNavigation ) )
		{
			HNAVITEM hItemHit, hItemSel;
			UINT hitFlags;
			POINT pt = p;
			MapWindowPoints( HWND_DESKTOP, h, &pt, 1 );

			hItemHit = NavCtrlI_HitTest( hNavigation, &pt, &hitFlags );
			hItemSel = NavCtrlI_GetSelection( hNavigation );

			Navigation_DropHelper( hwndDlg, h, hItemHit, hitFlags, pt );
			if ( hItemHit && hItemHit != hItemSel && ( !m_query_moving_item || NavItemI_GetParent( m_query_moving_item ) != hItemHit ) )
			{
				valid = ( plugin_SendMessage( ML_MSG_TREE_ONDROPTARGET, NavItemI_GetId( hItemHit ), type, NULL ) > 0 );
				if ( valid )
					g_treedrag_lastSel = hItemHit;
			}
		}
		else if ( IsWindow( m_curview_hwnd ) && IsWindow( h ) && ( h == m_curview_hwnd || IsChild( m_curview_hwnd, h ) ) )
		{
			mlDropItemStruct dis = { 0, };
			dis.type = type;
			dis.p = p;
			while ( 1 )
			{
				SendMessage( h, WM_ML_CHILDIPC, (WPARAM)&dis, ML_CHILDIPC_DROPITEM );

				if ( dis.result || h == m_curview_hwnd )
					break;

				h = GetParent( h );   // traverse up the tree
			}

			valid = dis.result > 0;
		}
	}

	if ( g_treedrag_lastSel != hItemPrevHilited )
	{
		ImageList_DragShowNolock( FALSE );
		if ( hItemPrevHilited )
			NavItemI_SetState( hItemPrevHilited, 0, NIS_DROPHILITED_I );

		if ( g_treedrag_lastSel )
			NavItemI_SetState( g_treedrag_lastSel, NIS_DROPHILITED_I, NIS_DROPHILITED_I );

		NavCtrlI_Update( hNavigation );
		ImageList_DragShowNolock( TRUE );
	}

	if ( do_cursors )
		SetCursor( ( valid ) ? hDragNDropCursor : LoadCursor( NULL, IDC_NO ) );

	return valid;
}

static void CancelNavigationMove( void )
{
	m_query_moving_dragplace = NULL;
	ImageList_DragShowNolock( FALSE );
	NavCtrlI_SetInsertMark( hNavigation, NULL, FALSE );
	NavCtrlI_Update( hNavigation );
	ImageList_DragShowNolock( TRUE );
}

static void OnInitMenuPopUp( HMENU menu );
static void OnDisplayChange( HWND hwndDlg, UINT imgDepth, UINT hRes, UINT vRes );
static int OnInitDialog( HWND hwndDlg );
static void OnDestroy( HWND hwndDlg );
static void OnClose( HWND hwndDlg );
static void OnSize( HWND hwndDlg, UINT type, int cx, int cy );
static void OnShowWindow( HWND hwndDlg, BOOL fShow );
LRESULT OnMediaLibraryIPC( HWND hwndDlg, WPARAM wParam, LPARAM lParam );
static void OnGetMaxMinInfo( MINMAXINFO *info );
static void OnBtnLibraryClick( HWND hwndBtn );

static HWND hwndTweak = NULL;
static BOOL CALLBACK RatingTweak_OnApplyChanges( UINT fStyle, BOOL bClosing )
{
	ratingGlobalStyle = fStyle;
	g_config->WriteInt( L"rating_style", ratingGlobalStyle );

	MLRatingColumnI_Update();

	if ( IsWindow( m_curview_hwnd ) )
		PostMessage( m_curview_hwnd, WM_DISPLAYCHANGE, 0, MAKELPARAM( 0, 0 ) );

	if ( bClosing )
		hwndTweak = NULL;

	return TRUE;
}

static void OnTimer_NavAutoScroll( HWND hwndDlg )
{
	RECT rc;
	POINT pt;
	HWND hwndNav = NavCtrlI_GetHWND( hNavigation );
	INT command = 0;

	KillTimer( hwndDlg, TIMER_NAVAUTOSCROLL_ID );

	if ( !hwndNav || !IsWindowVisible( hwndNav ) )
	{
		m_nav_autoscroll = FALSE;
		return;
	}

	GetCursorPos( &pt );
	NavCtrlI_MapPointsFrom( hNavigation, HWND_DESKTOP, &pt, 1 );

	GetClientRect( hwndNav, &rc );

	if ( rc.left <= pt.x && rc.right >= pt.x )
	{
		if ( rc.top <= pt.y && pt.y < ( rc.top + 24 ) )
			command = 1;
		else if ( ( rc.bottom - 24 ) < pt.y && pt.y <= rc.bottom )
			command = 2;
	}

	if ( command )
	{
		SCROLLINFO si = { 0 };
		si.cbSize = sizeof( SCROLLINFO );
		si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		if ( !GetScrollInfo( hwndNav, SB_VERT, &si ) )
			ZeroMemory( &si, sizeof( SCROLLINFO ) );

		if ( ( 1 == command && si.nMin == si.nPos ) || ( 2 == command && si.nMax < (INT)( si.nPos + si.nPage ) ) )
			command = 0;
		else
		{
			ImageList_DragShowNolock( FALSE );
			SendMessageW( hwndNav, WM_VSCROLL, MAKEWPARAM( ( 1 == command ) ? SB_LINEUP : SB_LINEDOWN, 0 ), NULL );
			NavCtrlI_Update( hNavigation );
			ImageList_DragShowNolock( TRUE );
			SetTimer( hwndDlg, TIMER_NAVAUTOSCROLL_ID, TIMER_NAVAUTOSCROLL_DELAY, NULL );
		}
	}

	if ( !command )
		m_nav_autoscroll = FALSE;
}

static void OnTimer_NavAutoExpand( HWND hwndDlg )
{
	POINT pt;
	HWND hwndNav = NavCtrlI_GetHWND( hNavigation );
	HNAVITEM hItem;
	UINT hitFlags;

	KillTimer( hwndDlg, TIMER_NAVAUTOEXPAND_ID );

	if ( !hwndNav || !IsWindowVisible( hwndNav ) )
		return;

	GetCursorPos( &pt );
	NavCtrlI_MapPointsFrom( hNavigation, HWND_DESKTOP, &pt, 1 );

	hItem = NavCtrlI_HitTest( hNavigation, &pt, &hitFlags );

	if ( NAVHT_ONITEMBUTTON_I & hitFlags && hItem == m_nav_autoexpand_item )
	{
		ImageList_DragShowNolock( FALSE );
		NavCtrlI_SetInsertMark( hNavigation, NULL, FALSE );
		NavItemI_Expand( hItem, NAVITEM_EXPAND_I );
		NavCtrlI_Update( hNavigation );
		ImageList_DragShowNolock( TRUE );
	}
	m_nav_autoexpand_item = NULL;
}


static void OnTimer( HWND hwndDlg, UINT_PTR idTimer )
{
	switch ( idTimer )
	{
		case TIMER_NAVAUTOSCROLL_ID:	OnTimer_NavAutoScroll( hwndDlg );	break;
		case TIMER_NAVAUTOEXPAND_ID:	OnTimer_NavAutoExpand( hwndDlg );	break;
		case TIMER_UNBLOCKACCELTOGGLE_ID:
			KillTimer( hwndDlg, idTimer );
			RemoveProp( hwndDlg, BLOCK_ACCELTOGGLE_PROP );
			break;
	}
}

static HMLIMGLST CreateNavigationImages( HMLIMGFLTRMNGR filterManager )
{
	MLIMAGESOURCE_I is = { 0 };
	static INT resourceId[] = { IDB_TREEITEM_DEFAULT, IDB_TREEITEM_COLLAPSED, IDB_TREEITEM_EXPANDED, IDB_TREEITEM_NOCHILD };
	static INT_PTR imageTag[] = { MLTREEIMAGE_DEFAULT, MLTREEIMAGE_BRANCH_COLLAPSED, MLTREEIMAGE_BRANCH_EXPANDED, MLTREEIMAGE_BRANCH_NOCHILD };
	HMLIMGLST hmlil = MLImageListI_Create( 16, 16, 24, 30, 2, 3, filterManager );

	if ( !hmlil )
		return NULL;

	is.bpp = 24;
	is.hInst = plugin.hDllInstance;
	is.type = SRC_TYPE_BMP_I;
	is.flags = ISF_FORCE_BPP_I;
	for ( INT index = 0; index < sizeof( resourceId ) / sizeof( resourceId[ 0 ] ); index++ )
	{
		is.lpszName = MAKEINTRESOURCEW( resourceId[ index ] );
		MLImageListI_Add( hmlil, &is, MLIF_FILTER1_UID, imageTag[ index ] );
	}
	return hmlil;
}

static void OnMouseMove( HWND hwndDlg, POINT pt, UINT flags )
{
	RECT rc;
	if ( !m_query_moving ) return;

	MapWindowPoints( hwndDlg, HWND_DESKTOP, &pt, 1 );

	GetWindowRect( hwndDlg, &rc );
	ImageList_DragMove( pt.x - rc.left, pt.y - rc.top );

	HWND hWndHit = WindowFromPoint( pt );

	if ( hWndHit == NavCtrlI_GetHWND( hNavigation ) )
	{
		POINT ptMy = pt;
		UINT hitFlags;
		HNAVITEM hItem;

		MapWindowPoints( HWND_DESKTOP, hWndHit, &ptMy, 1 );

		hItem = NavCtrlI_HitTest( hNavigation, &ptMy, &hitFlags );

		if ( !hItem )
		{
			HNAVITEM hItemLast;
			hItemLast = NavCtrlI_GetLastVisible( hNavigation );
			if ( hItemLast )
			{
				RECT ri;
				if ( NavItemI_GetRect( hItemLast, &ri, FALSE ) && ri.bottom < ptMy.y )
				{
					HNAVITEM hMovingParent, hLastParent;
					hMovingParent = NavItemI_GetParent( m_query_moving_item );
					while ( NULL != ( hLastParent = NavItemI_GetParent( hItemLast ) ) && hLastParent != hMovingParent ) hItemLast = hLastParent;
					if ( hMovingParent == hLastParent )
					{
						hItem = hItemLast;
						hitFlags = NAVHT_ONITEM_I;
					}
				}
			}
		}

		if ( ( NAVHT_ONITEMINDENT_I | NAVHT_ONITEMRIGHT_I | NAVHT_ONITEM_I | NAVHT_ONITEMBUTTON_I ) & hitFlags )
		{
			HNAVITEM tempItem;

			Navigation_DropHelper( hwndDlg, hWndHit, hItem, hitFlags, ptMy );

			if ( ML_TYPE_UNKNOWN == m_query_moving_type )
			{
				SetCursor( LoadCursor( NULL, IDC_NO ) );
				return;
			}
			tempItem = hItem;
			while ( tempItem && tempItem != m_query_moving_item ) tempItem = NavItemI_GetParent( tempItem );
			if ( tempItem == m_query_moving_item )
				hItem = m_query_moving_item;

			if ( hItem )
			{
				BOOL hitBelow;

				if ( NavItemI_GetParent( hItem ) != NavItemI_GetParent( m_query_moving_item ) )
				{
					// TODO: I think regular drag-n-drop goes here?
					//SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_NO)));
					CancelNavigationMove();
					m_query_moving_lastdest = NULL;
					mlDropItemStruct dropItem;
					ZeroMemory( &dropItem, sizeof( mlDropItemStruct ) );
					dropItem.p = pt;
					dropItem.type = m_query_moving_type;
					OnMediaLibraryIPC( hwndDlg, (WPARAM)&dropItem, ML_IPC_HANDLEDRAG );
					//handleDragDropMove(hwndDlg, m_query_moving_type, pt, 1);
					//goto outside_window;
					return;
				}

				if ( g_treedrag_lastSel )
				{
					NavItemI_SetState( g_treedrag_lastSel, 0, NIS_DROPHILITED_I );
					g_treedrag_lastSel = NULL;
				}

				NavItemI_GetRect( hItem, &rc, FALSE );
				hitBelow = ( ptMy.y > ( rc.bottom - ( rc.bottom - rc.top ) / 2 ) );

				SetCursor( LoadCursor( NULL, MAKEINTRESOURCE( IDC_ARROW ) ) );

				m_query_moving_lastdest = hItem;

				if ( m_query_moving_dragplace != hItem || m_query_moving_dragplaceisbelow != hitBelow )
				{
					m_query_moving_dragplace = hItem;
					m_query_moving_dragplaceisbelow = hitBelow;
					ImageList_DragShowNolock( FALSE );
					NavCtrlI_SetInsertMark( hNavigation, hItem, hitBelow );
					NavCtrlI_Update( hNavigation );
					ImageList_DragShowNolock( TRUE );
				}
			}
		}
	}
	else
	{
		int type = ML_TYPE_TREEITEM;
		bool canDrag;

		CancelNavigationMove();

		canDrag = ( m_query_moving_item &&
			( plugin_SendMessage( ML_MSG_TREE_ONDRAG, NavItemI_GetId( m_query_moving_item ), (INT_PTR)&pt, (INT_PTR)&type ) < 0 ) );

		if ( !canDrag )
		{
			mlDropItemStruct dropItem;
			ZeroMemory( &dropItem, sizeof( mlDropItemStruct ) );
			dropItem.p = pt;
			dropItem.type = type;
			OnMediaLibraryIPC( hwndDlg, (WPARAM)&dropItem, ML_IPC_HANDLEDRAG );

			//pluginHandleIpcMessage(hwndDlg, ML_IPC_HANDLEDRAG, (WPARAM)&m);
			//SetCursor(dropItem.result > 0 ? hDragNDropCursor : LoadCursor(NULL, IDC_NO));
			//SetCursor(LoadCursor(NULL, IDC_NO));
		}

		m_query_moving_dragplace = NULL;
		m_query_moving_lastdest = NULL;
	}
}

static void OnLButtonUp( HWND hwndDlg, POINT pt, UINT flags )
{
	HWND hwndHit;

	if ( !m_query_moving ) return;

	SetCursor( LoadCursor( NULL, MAKEINTRESOURCE( IDC_ARROW ) ) );

	ImageList_DragLeave( hwndDlg );
	ImageList_EndDrag();

	if ( m_nav_dragitem_imagelist )
	{
		ImageList_Destroy( m_nav_dragitem_imagelist );
		m_nav_dragitem_imagelist = NULL;
	}

	MapWindowPoints( hwndDlg, HWND_DESKTOP, &pt, 1 );

	hwndHit = WindowFromPoint( pt );

	if ( ML_TYPE_UNKNOWN != m_query_moving_type && hwndHit && m_query_moving_item )
	{
		if ( m_query_moving_dragplace )
		{
			//move item in the tree

			if ( ML_TYPE_TREEITEM == m_query_moving_type && m_query_moving_item != m_query_moving_lastdest )
			{
				WORD orderOld;
				orderOld = NavItemI_GetOrder( m_query_moving_item );
				if ( NavItemI_Move( m_query_moving_item, m_query_moving_lastdest, m_query_moving_dragplaceisbelow ) )
				{
					plugin_SendMessage( ML_MSG_NAVIGATION_ONMOVE, (INT_PTR)m_query_moving_item, orderOld, NavItemI_GetOrder( m_query_moving_item ) );
				}
			}
			else
			{
				HNAVITEM whereinsert;
				whereinsert = m_query_moving_lastdest;

				if ( !m_query_moving_dragplaceisbelow )
				{
					BOOL fTest;
					fTest = ( whereinsert == m_query_moving_item );
					whereinsert = NavItemI_GetPrevious( whereinsert );

					if ( !whereinsert && !fTest )
						whereinsert = NavItemI_GetParent( m_query_moving_lastdest );
				}

				if ( whereinsert && m_query_moving_item != whereinsert )
					plugin_SendMessage( ML_MSG_TREE_ONDROP, NavItemI_GetId( m_query_moving_item ), (INT_PTR)&pt, NavItemI_GetId( whereinsert ) );
			}
		}
		else
			plugin_SendMessage( ML_MSG_TREE_ONDROP, NavItemI_GetId( m_query_moving_item ), (INT_PTR)&pt, 0 );
	}
	m_query_moving = 0;

	if ( g_treedrag_lastSel )
	{
		NavItemI_SetState( g_treedrag_lastSel, 0, NIS_DROPHILITED_I );
		g_treedrag_lastSel = NULL;
	}

	if ( m_query_moving_dragplace )
		CancelNavigationMove();

	ReleaseCapture();
}


void listbuild( wchar_t **buf, int &buf_size, int &buf_pos, const wchar_t *tbuf )
{
	if ( !*buf )
	{
		*buf = (wchar_t *)calloc( 4096, sizeof( wchar_t ) );
		if ( *buf )
		{
			buf_size = 4096;
			buf_pos = 0;
		}
		else
		{
			buf_size = buf_pos = 0;
		}
	}

	int newsize = buf_pos + lstrlenW( tbuf ) + 1;
	if ( newsize < buf_size )
	{
		size_t old_buf_size = buf_size;
		buf_size = newsize + 4096;
		wchar_t *data = (wchar_t *)realloc( *buf, ( buf_size + 1 ) * sizeof( wchar_t ) );
		if ( data )
		{
			*buf = data;
		}
		else
		{
			data = (wchar_t *)malloc( ( buf_size + 1 ) * sizeof( wchar_t ) );
			if ( data )
			{
				memcpy( data, *buf, sizeof( wchar_t ) * old_buf_size );
				free( *buf );
				*buf = data;
			}
			else
				buf_size = old_buf_size;
		}
	}
	StringCchCopyW( *buf + buf_pos, buf_size, tbuf );
	buf_pos = newsize;
}

/*wchar_t * getSelectedList()
{
	wchar_t* path=NULL;
	int buf_pos=0, buf_size=0;
	int download=-1;
	while (GetDownload(download))
	{
		if(listContents[download]->f)
			listbuild(&path,buf_size,buf_pos,listContents[download]->f->path);
	}
	if(path) path[buf_pos] = 0;
	return path;
}*/

static void OnDragDrop( HWND hwndDlg, HDROP hdrop )
{
	UINT hitFlags = NAVHT_NOWHERE_I;
	POINT pt = { 0 };
	DragQueryPoint( hdrop, &pt );

	HNAVITEM hItemHit = NavCtrlI_HitTest( hNavigation, &pt, &hitFlags );
	if ( hItemHit )
	{
		if ( plugin_SendMessage( ML_MSG_TREE_ONDROPTARGET, NavItemI_GetId( hItemHit ), ML_TYPE_FILENAMESW, NULL ) > 0 )
		{
			wchar_t temp[ MAX_PATH ] = { 0 };
			int y = DragQueryFileW( hdrop, 0xffffffff, temp, 1024 );
			if ( y > 0 )
			{
				wchar_t *paths = NULL;
				int buf_pos = 0, buf_size = 0;
				for ( int x = 0; x < y; x++ )
				{
					if ( DragQueryFileW( hdrop, x, temp, MAX_PATH ) )
					{
						listbuild( &paths, buf_size, buf_pos, temp );
					}
				}

				if ( paths )
				{
					paths[ buf_pos ] = 0;
					plugin_SendMessage( ML_MSG_TREE_ONDROPTARGET, NavItemI_GetId( hItemHit ), ML_TYPE_FILENAMESW, (INT_PTR)paths );

					if ( IsWindow( m_curview_hwnd ) )
						SendMessage( m_curview_hwnd, WM_APP + 1, 0, 0 ); //update current view

					free( paths );
				}
			}

			DragFinish( hdrop );
		}
	}
}

static void MlView_ShowWindow( HWND hwnd, BOOL fShow, UINT nStatus )
{
	if ( SW_PARENTOPENING == nStatus )
	{
		BOOL fStored = ( 0 != g_config->ReadInt( L"visible", 1 ) );
		if ( fShow != fStored )
		{
			PostMessageW( hwnd, WMML_SHOWCONTAINER, ( 0 == fStored ) ? SW_HIDE : SW_SHOWNA, 0 );
			return;
		}
	}

	ShowWindow( hwnd, ( FALSE != fShow ) ? SW_SHOWNA : SW_HIDE );

	if ( 0 == nStatus )
	{
		if ( NULL != g_config )
			g_config->WriteInt( L"visible", ( FALSE != fShow ) );

		UINT menuFlags = ( FALSE != fShow ) ? MF_CHECKED : MF_UNCHECKED;
		menuFlags |= MF_BYCOMMAND;

		INT szMenu[] = { 0, 4, };
		for ( INT i = 0; i < ARRAYSIZE( szMenu ); i++ )
		{
			HMENU hMenu = (HMENU)SendMessage( plugin.hwndParent, WM_WA_IPC, szMenu[ i ], IPC_GET_HMENU );
			if ( NULL != hMenu )
				CheckMenuItem( hMenu, WA_MENUITEM_ID, menuFlags );
		}

		MLVisibleChanged( FALSE != fShow );

		if ( FALSE != fShow )
		{
			SendMessageW( g_ownerwnd, 0x0127/*WM_CHANGEUISTATE*/, MAKEWPARAM( 1/*UIS_SET*/, 3/*(UISF_HIDEACCEL | UISF_HIDEFOCUS)*/ ), 0L );
			HWND hRoot = GetAncestor( g_ownerwnd, GA_ROOT );
			if ( NULL != hRoot )
				SetForegroundWindow( hRoot );
		}
	}
}

static LRESULT MlView_OnContainerNotify( HWND hwnd, NMHDR *pnmh )
{
	switch ( pnmh->code )
	{
		case EWN_SHOWWINDOW:
			MlView_ShowWindow( hwnd, ( (EMBEDSHOW *)pnmh )->fShow, ( (EMBEDSHOW *)pnmh )->nStatus );
			break;
	}

	return 0;
}

static LRESULT MlView_OnNotify( HWND hwnd, INT controlId, NMHDR *pnmh )
{
	if ( pnmh->hwndFrom == g_ownerwnd )
	{
		return MlView_OnContainerNotify( hwnd, pnmh );
	}

	return 0;
}

static LRESULT MlView_OnContextMenu( HWND hwnd, HWND hOwner, POINTS pts )
{
	HWND hNav = NavCtrlI_GetHWND( hNavigation );

	if ( NULL != hNav && hOwner == hNav )
	{
		HNAVITEM hItem;
		POINT pt;
		POINTSTOPOINT( pt, pts );

		if ( -1 == pt.x || -1 == pt.y )
		{
			hItem = NavCtrlI_GetSelection( hNavigation );
			if ( NULL != hItem )
			{
				RECT itemRect;
				if ( NavItemI_GetRect( hItem, &itemRect, FALSE ) )
				{
					pt.x = itemRect.left + 1;
					pt.y = itemRect.top + 1;
					MapWindowPoints( hNav, HWND_DESKTOP, &pt, 1 );
				}
			}
		}
		else
		{
			UINT hitFlags;
			MapWindowPoints( HWND_DESKTOP, hNav, &pt, 1 );
			hItem = NavCtrlI_HitTest( hNavigation, &pt, &hitFlags );
			pt.x = pts.x;
			pt.y = pts.y;
		}

		if ( NULL == hItem ) return 0;

		UINT itemState = NavItemI_GetState( hItem, NIS_DROPHILITED_I | NIS_SELECTED_I );

		if ( 0 == ( ( NIS_DROPHILITED_I | NIS_SELECTED_I ) & itemState ) )
			NavItemI_SetState( hItem, NIS_DROPHILITED_I, NIS_DROPHILITED_I );

		INT_PTR result = plugin_SendMessage( ML_MSG_NAVIGATION_CONTEXTMENU,
			(INT_PTR)hItem,
			(INT_PTR)hNav,
			MAKELONG( pt.x, pt.y ) );

		if ( 0 == ( ( NIS_DROPHILITED_I | NIS_SELECTED_I ) & itemState ) )
			NavItemI_SetState( hItem, 0, NIS_DROPHILITED_I );

		return ( 0 != result );
	}

	return 0;
}

static BOOL
MlView_OnHelp( HWND hwnd, HELPINFO *helpInfo )
{
	// TODO review this for handling Shift+F1 as view specific help?
	// make sure we're only doing it for the expect help actions
	// i.e. not Ctrl+F1 which opens the global about dialog
	if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) )
		return FALSE;

	// for view specific handling, we use Shift+F1
	if ( NULL != helpInfo && HELPINFO_WINDOW == helpInfo->iContextType && ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) )
	{
		HWND navigationWindow;
		navigationWindow = NavCtrlI_GetHWND( hNavigation );

		if ( navigationWindow == helpInfo->hItemHandle )
		{
			HNAVITEM selectedItem;
			selectedItem = NavCtrlI_GetSelection( hNavigation );
			if ( NULL != selectedItem && 0 != plugin_SendMessage( ML_MSG_NAVIGATION_HELP, (INT_PTR)selectedItem, (INT_PTR)navigationWindow, MAKELONG( helpInfo->MousePos.x, helpInfo->MousePos.y ) ) )
			{
				return TRUE;
			}
			return TRUE;
		}
	}
	// otherwise we treat it as a generic help request
	return MediaLibrary_OpenHelpUrl( MEDIALIBRARY_HELP_URL );
}

INT_PTR CALLBACK dialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	switch ( uMsg )
	{
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		{
			POINT pt = { GET_X_LPARAM( lParam ),GET_Y_LPARAM( lParam ) };
			if ( WM_MOUSEMOVE == uMsg )
				OnMouseMove( hwndDlg, pt, (UINT)wParam );
			else if ( WM_LBUTTONUP == uMsg )
				OnLButtonUp( hwndDlg, pt, (UINT)wParam );
		}
		return TRUE;

		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
				case IDC_BTN_LIB: if ( BN_CLICKED == HIWORD( wParam ) ) OnBtnLibraryClick( (HWND)lParam ); break;
				case IDM_LIBRARY_CONFIG:	OpenMediaLibraryPreferences(); 	break;
				case IDM_LIBRARY_HELP:		MediaLibrary_OpenHelpUrl( MEDIALIBRARY_HELP_URL ); break;
				case ID_TOGGLE_LIBRARY:
					if ( 0 == GetProp( hwndDlg, BLOCK_ACCELTOGGLE_PROP ) )
					{
						toggleVisible( 0 );
						SetProp( hwndDlg, BLOCK_ACCELTOGGLE_PROP, (HANDLE)(INT_PTR)1 );
						SetTimer( hwndDlg, TIMER_UNBLOCKACCELTOGGLE_ID, TIMER_UNBLOCKACCELTOGGLE_DELAY, NULL );
					}
					break;
				case ID_WINDOW_CLOSE:
					if ( IsVisible() )
						toggleVisible( 0 );
					break;
				case ID_SHOW_RATINGTWEAK:
					if ( !IsWindow( hwndTweak ) )
						hwndTweak = MLRatingColumnI_TweakDialog( hwndDlg, ratingGlobalStyle, RatingTweak_OnApplyChanges, TRUE );
					else SetWindowPos( hwndTweak, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW );
					break;
				case ID_GO_TO_VIEW_SEARCHBAR:
					SendMessage( m_curview_hwnd, WM_ML_CHILDIPC, 0, ML_CHILDIPC_GO_TO_SEARCHBAR );
					break;
				case ID_REFRESH_SEARCH:
					SendMessage( m_curview_hwnd, WM_ML_CHILDIPC, 0, ML_CHILDIPC_REFRESH_SEARCH );
					break;
				case ID_NEW_PLAYLIST:
				{
					// only process if not in an edit control
					// (as shift+insert is a legacy OS paste shortcut)
					wchar_t szClass[ 32 ] = { 0 };
					HWND hFocus = GetFocus();
					if ( GetClassNameW( hFocus, szClass, sizeof( szClass ) / sizeof( szClass[ 0 ] ) ) &&
						CSTR_EQUAL != CompareStringW( CSTR_INVARIANT, NORM_IGNORECASE, szClass, -1, WC_EDITW, -1 ) )
					{
#define ID_MLFILE_NEWPLAYLIST 40359
						SendMessageW( plugin.hwndParent, WM_COMMAND, MAKEWPARAM( ID_MLFILE_NEWPLAYLIST, 0 ), 0 );
					}
					else SendMessageW( hFocus, WM_PASTE, 0, 0L );
				}
				break;
				case ID_SHOW_HELP:
					// do nothing, just need to still F1 from Winamp
					break;

			}
			break;
		case WM_NOTIFY:
		{
			LRESULT result;
			result = 0;
			if ( !NavCtrlI_ProcessNotifications( hNavigation, (LPNMHDR)lParam, &result ) )
				result = MlView_OnNotify( hwndDlg, (INT)wParam, (NMHDR *)lParam );

			SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, (LONGX86)(LONG_PTR)result );
			return TRUE;
		}

		case WM_CAPTURECHANGED:
		{
			POINT pt = { MAXLONG,MAXLONG };
			OnLButtonUp( hwndDlg, pt, 0 );
			break;
		}

		case WM_DROPFILES:			OnDragDrop( hwndDlg, (HDROP)wParam ); break;
		case WM_INITMENUPOPUP:		OnInitMenuPopUp( (HMENU)wParam ); return TRUE;
		case WM_DISPLAYCHANGE:		OnDisplayChange( hwndDlg, (UINT)wParam, LOWORD( lParam ), HIWORD( lParam ) ); return TRUE;
		case WM_INITDIALOG:			return OnInitDialog( hwndDlg );
		case WM_DESTROY:			OnDestroy( hwndDlg ); break;
		case WM_WINDOWPOSCHANGED:	OnWindowPosChanged( hwndDlg, (WINDOWPOS *)lParam ); return TRUE;
		case WM_CLOSE:				OnClose( hwndDlg ); break;
		case WM_GETMINMAXINFO:		OnGetMaxMinInfo( (LPMINMAXINFO)lParam ); return TRUE;
		case WM_TIMER:				OnTimer( hwndDlg, (UINT_PTR)wParam ); return TRUE;
		case WM_ML_IPC:
			return OnMediaLibraryIPC( hwndDlg, wParam, lParam );
		case WM_USER + 0x200:		SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, TRUE ); return TRUE;
		case WM_USER + 0x201:		g_rgnUpdate = (HRGN)lParam; return TRUE;// parent supports region updates. lParam is a HRGN to set HRGN coordinate mapped to parent client area (this message sent prior to WM_WINDOWPOSCHANGED.
		case WM_SHOWWINDOW:
		{
			if ( wParam ) PostMessage( hwndDlg, WM_USER + 31, wParam, 0 );
			else OnShowWindow( hwndDlg, (BOOL)wParam );
			return TRUE;
		}
		case WM_USER + 30:
		{
			SetWindowRedraw( hwndDlg, FALSE );
			DestroyWindow( m_curview_hwnd );
			SetWindowRedraw( hwndDlg, TRUE );
			CreateCurrentView( hwndDlg );
			break;
		}
		case WM_USER + 31:
			// double-pump this to work around slower plugins loading up
			if ( !lParam )
				PostMessage( hwndDlg, WM_USER + 31, wParam, 1 );
			else
			{
				OnShowWindow( hwndDlg, (BOOL)wParam );
				NavItemI_EnsureVisible( NavCtrlI_GetSelection( hNavigation ) );
			}
			break;
		case WM_CONTEXTMENU:			MSGRESULT( hwndDlg, MlView_OnContextMenu( hwndDlg, (HWND)wParam, MAKEPOINTS( lParam ) ) );
		case WM_HELP:					MSGRESULT( hwndDlg, MlView_OnHelp( hwndDlg, (HELPINFO *)lParam ) );
		case WMML_SHOWCONTAINER:		ShowWindow( g_ownerwnd, (INT)wParam ); return TRUE;

	}
	return FALSE;
}

void OnInitMenuPopUp( HMENU menu )
{
	if ( main_sendtomenu || !main_sendto_hmenu || menu != main_sendto_hmenu )
		return;

	main_sendtomenu = new SendToMenu();
	main_sendtomenu->buildmenu( menu, main_sendto_mode, 0 );
}

void OnDisplayChange( HWND hwndDlg, UINT imgDepth, UINT hRes, UINT vRes )
{
	ResetColors( TRUE ); // must be first
	MlStockObjects_Reset();

	ratingGlobalStyle = g_config->ReadInt( L"rating_style", RATING_DEFAULT_STYLE );
	MLRatingColumnI_Update();

	if ( IsWindow( m_curview_hwnd ) )
	{
		RECT rc;
		GetClientRect( m_curview_hwnd, &rc );
		RedrawWindow( m_curview_hwnd, &rc, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_ERASENOW/* | RDW_UPDATENOW*/ );
		MLSkinnedWnd_SkinChanged( m_curview_hwnd, TRUE, TRUE );
		SendNotifyMessageW( m_curview_hwnd, WM_DISPLAYCHANGE, imgDepth, MAKELPARAM( hRes, vRes ) );
	}

	NavCtrlI_UpdateLook( hNavigation );
	MLSkinnedWnd_SkinChanged( GetDlgItem( hwndDlg, IDC_BTN_LIB ), TRUE, TRUE );
	MLSkinnedWnd_SkinChanged( GetDlgItem( hwndDlg, IDC_VDELIM ), TRUE, TRUE );
	MLSkinnedWnd_SkinChanged( GetDlgItem( hwndDlg, IDC_NO_VIEW ), TRUE, TRUE );
	LayoutWindows( hwndDlg, divider_pos, TRUE );
}

#include "mldwm.h"

int OnInitDialog( HWND hwndDlg )
{
	firstShow = TRUE;
	MlStockObjects_Init();

	if ( S_OK == MlDwm_LoadLibrary() )
	{
		DWMNCRENDERINGPOLICY ncrp = DWMNCRP_DISABLED;
		BOOL allow = FALSE;
		MlDwm_SetWindowAttribute( GetParent( hwndDlg ), DWMWA_NCRENDERING_POLICY, &ncrp, sizeof( ncrp ) );
		MlDwm_SetWindowAttribute( GetParent( hwndDlg ), DWMWA_ALLOW_NCPAINT, &allow, sizeof( allow ) );
	}

	MLSkinWindow2( hwndDlg, hwndDlg, SKINNEDWND_TYPE_DIALOG, SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS );
	HWND hctrl = GetDlgItem( hwndDlg, IDC_BTN_LIB );
	MLSkinWindow2( hwndDlg, hctrl, SKINNEDWND_TYPE_BUTTON, SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS );

	hctrl = GetDlgItem( hwndDlg, IDC_VDELIM );
	MLSkinWindow2( hwndDlg, hctrl, SKINNEDWND_TYPE_DIVIDER, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWDIV_VERT );
	MLSkinnedDivider_SetCallback( hctrl, OnDividerMoved, NULL );

	hctrl = GetDlgItem( hwndDlg, IDC_NO_VIEW );
	MLSkinWindow2( hwndDlg, hctrl, SKINNEDWND_TYPE_STATIC, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWDIV_VERT );

	if ( !hmlifMngr ) // Initialize Image Filter manager
	{
		hmlifMngr = MLImageFilterI_CreateManager( 8, 4 );
		if ( hmlifMngr )
		{
			RegisterImageFilters( hmlifMngr );
			SkinnedButton::RegisterImageFilter( hmlifMngr );
		}
	}

	if ( !hNavigation )
	{
		hNavigation = NavCtrlI_Create( hwndDlg );
		if ( hNavigation )
		{
			if ( !hmlilNavigation ) hmlilNavigation = CreateNavigationImages( hmlifMngr );
			NavCtrlI_SetConfig( hNavigation, g_config );
			NavCtrlI_SetImageList( hNavigation, hmlilNavigation );
			NavCtrlI_RegisterCallback( hNavigation, OnNavCtrl_Selected, CALLBACK_ONSELECTED_I );
			NavCtrlI_RegisterCallback( hNavigation, OnNavCtrl_Click, CALLBACK_ONCLICK_I );
			NavCtrlI_RegisterCallback( hNavigation, OnNavCtrl_KeyDown, CALLBACK_ONKEYDOWN_I );
			NavCtrlI_RegisterCallback( hNavigation, OnNavCtrl_BeginDrag, CALLBACK_ONBEGINDRAG_I );
			NavCtrlI_RegisterCallback( hNavigation, OnNavCtrl_GetImageIndex, CALLBACK_ONGETIMAGEINDEX_I );
			NavCtrlI_RegisterCallback( hNavigation, OnNavCtrl_BeginTitleEdit, CALLBACK_ONBEGINTITLEEDIT_I );
			NavCtrlI_RegisterCallback( hNavigation, OnNavCtrl_EndTitleEdit, CALLBACK_ONENDTITLEEDIT_I );
			NavCtrlI_RegisterCallback( hNavigation, OnNavItem_Delete, CALLBACK_ONITEMDELETE_I );
			NavCtrlI_RegisterCallback( hNavigation, OnNavItem_CustomDraw, CALLBACK_ONITEMDRAW_I );
			NavCtrlI_RegisterCallback( hNavigation, OnNavItem_SetCursor, CALLBACK_ONSETCURSOR_I );
			NavCtrlI_RegisterCallback( hNavigation, OnNavItem_HitTest, CALLBACK_ONHITTEST_I );
			NavCtrlI_RegisterCallback( hNavigation, OnNavCtrl_Destroy, CALLBACK_ONDESTROY_I );
			hctrl = NavCtrlI_GetHWND( hNavigation );
			SetWindowLongPtr( hctrl, GWLP_ID, IDC_NAVIGATION );
			SetWindowLongPtr( hctrl, GWL_STYLE, GetWindowLongPtr( hctrl, GWL_STYLE ) | WS_GROUP );
		}
	}

	if ( hNavigation )
		SetWindowPos( GetDlgItem( hwndDlg, IDC_BTN_LIB ), NavCtrlI_GetHWND( hNavigation ), 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE );

	divider_pos = g_config->ReadInt( L"ldivpos", 135 );

	if ( !hmlilRating )
	{
		hmlilRating = MLImageListI_Create( 44, 12, MLILC_COLOR24_I, 1, 1, 4, hmlifMngr );
		if ( hmlilRating )
		{
			MLIMAGESOURCE_I is = { 0 };
			is.hInst = plugin.hDllInstance;
			is.lpszName = MAKEINTRESOURCEW( IDB_RATING );
			is.type = SRC_TYPE_PNG_I;
			MLImageListI_Add( hmlilRating, &is, MLIF_FILTER1_UID, 0 );
		}
	}

	ratingGlobalStyle = g_config->ReadInt( L"rating_style", RATING_DEFAULT_STYLE );

	MLRatingColumnI_Initialize();

#ifdef CLOUD
	if ( !hmlilCloud )
	{
		hmlilCloud = MLImageListI_Create( 16, 16, MLILC_COLOR24_I, 1, 1, 4, hmlifMngr );
		if ( hmlilCloud )
		{
			MLIMAGESOURCE_I is = { 0 };
			is.hInst = plugin.hDllInstance;
			is.lpszName = MAKEINTRESOURCEW( IDB_CLOUD_IS_IN );
			is.type = SRC_TYPE_PNG_I;
			MLImageListI_Add( hmlilCloud, &is, MLIF_FILTER1_UID, 0 );

			is.lpszName = MAKEINTRESOURCEW( IDB_CLOUD_PARTIAL );
			MLImageListI_Add( hmlilCloud, &is, MLIF_FILTER1_UID, 0 );

			is.lpszName = MAKEINTRESOURCEW( IDB_CLOUD_UNAVAIL );
			MLImageListI_Add( hmlilCloud, &is, MLIF_FILTER1_UID, 0 );

			is.lpszName = MAKEINTRESOURCEW( IDB_CLOUD_UPLOAD );
			MLImageListI_Add( hmlilCloud, &is, MLIF_FILTER1_UID, 0 );

			is.lpszName = MAKEINTRESOURCEW( IDB_CLOUD_UPLOADING );
			MLImageListI_Add( hmlilCloud, &is, MLIF_FILTER1_UID, 0 );
		}
	}

	MLCloudColumnI_Initialize();
#endif

	OnDisplayChange( hwndDlg, 0, 0, 0 );

	HACCEL hAccel = WASABI_API_LOADACCELERATORSW( IDR_ACCELERATOR_GLOBAL );
	if ( hAccel )
		WASABI_API_APP->app_addAccelerators( hwndDlg, &hAccel, 1, TRANSLATE_MODE_GLOBAL );

	hAccel = WASABI_API_LOADACCELERATORSW( IDR_ACCELERATOR_MAIN );
	if ( hAccel )
		WASABI_API_APP->app_addAccelerators( hwndDlg, &hAccel, 1, TRANSLATE_MODE_CHILD );

	return TRUE;
}

static void OnDestroy( HWND hwndDlg )
{
	HNAVITEM hItem = NavCtrlI_GetSelection( hNavigation );
	if ( hItem )
	{
		wchar_t name[ 1024 ] = { 0 };
		if ( NavItemI_GetFullName( hItem, name, 1024 ) )
			g_config->WriteString( "last_view", AutoChar( name, CP_UTF8 ) );
	}

	g_config->WriteInt( L"ldivpos", divider_pos );

	NavCtrlI_Destroy( hNavigation );
	hNavigation = NULL;
	MLImageListI_Destroy( hmlilNavigation );
	hmlilNavigation = NULL;
	MLImageFilterI_DestroyManager( hmlifMngr );
	hmlifMngr = NULL;
	MLImageListI_Destroy( hmlilRating );
	hmlilRating = NULL;
	MLImageListI_Destroy( hmlilCloud );
	hmlilCloud = NULL;

	RemoveProp( hwndDlg, BLOCK_ACCELTOGGLE_PROP );

	MlStockObjects_Free();
}

static void OnClose( HWND hwndDlg )
{
	toggleVisible( 1 );
}

static void OnShowWindow( HWND hwndDlg, BOOL fShow )
{
	if ( firstShow && fShow )
	{
		firstShow = FALSE;

		MLVisibleChanged( TRUE );
		// setting things to a more user friendly default than just the Local Media view
		char *lastTitleA = g_config->ReadString( "last_view", "Local Media/Audio" );

		HNAVITEM hDefItem = NULL;
		if ( lastTitleA && *lastTitleA )
		{
			wchar_t textW[ 2048 ] = { 0 };
			if ( MultiByteToWideCharSZ( CP_UTF8, 0, lastTitleA, -1, textW, 2048 ) )
				hDefItem = NavCtrlI_FindItemByFullName( hNavigation, LOCALE_INVARIANT, NICF_INVARIANT_I | NICF_DISPLAY_I | NICF_IGNORECASE_I, textW, -1, TRUE );
		}

		if ( !hDefItem )
			hDefItem = NavCtrlI_GetRoot( hNavigation );

		NavItemI_Select( hDefItem );
		NavCtrlI_Show( hNavigation, SW_SHOWNA );
	}
}

LRESULT OnMediaLibraryIPC( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, (LONGX86)(LONG_PTR)pluginHandleIpcMessage( hwndDlg, (INT)lParam, (INT_PTR)wParam ) );

	return TRUE;
}

void OnGetMaxMinInfo( MINMAXINFO *info )
{
	info->ptMinTrackSize.x = 300;
	info->ptMinTrackSize.y = 200;
}

void OnBtnLibraryClick( HWND hwndBtn )
{
	RECT r;
	GetWindowRect( hwndBtn, &r );
	SendMessageW( hwndBtn, BM_SETSTATE, TRUE, 0L );

	MediaLibrary_TrackPopup( GetSubMenu( g_context_menus, 0 ),
		TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_VERNEGANIMATION,
		r.left, r.top,
		GetParent( hwndBtn ) );

	SendMessageW( hwndBtn, BM_SETSTATE, FALSE, 0L );
	UpdateWindow( hwndBtn );
	Sleep( 100 );
	MSG msg;
	while ( PeekMessageW( &msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) ); //eat return
}