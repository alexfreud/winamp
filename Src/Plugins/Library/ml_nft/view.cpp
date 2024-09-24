#include <strsafe.h>

#include "main.h"

#include "../nu/DialogSkinner.h"
#include "../nu/ListView.h"

#include "../../General/gen_ml/ml_ipc.h"
#include "../../General/gen_ml/menu.h"

#include "../WAT/WAT.h"

INT_PTR CALLBACK view_NFTDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static W_ListView m_nft_list;
static HWND m_headerhwnd, m_hwnd;
extern C_Config *g_config;
extern char *g_ext_list;
static int customAllowed;
static viewButtons view;

C_Config *g_wa_config = 0;
char     *g_ext_list  = 0;

// used for the send-to menu bits
static INT_PTR IPC_LIBRARY_SENDTOMENU;
static librarySendToMenuStruct s;
BOOL myMenu = FALSE;

extern HMENU g_context_menus, g_context_menus2;
extern HCURSOR hDragNDropCursor;


static int NFT_contextMenu( INT_PTR param1, HWND hHost, POINTS pts )
{
	return TRUE;
}

static int pluginHandleIpcMessage(int msg, int param) 
{
	return (int)SendMessage( plugin.hwndLibraryParent, WM_ML_IPC, param, msg );
}

INT_PTR nft_pluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	if (message_type == ML_MSG_NO_CONFIG)
	{
		return TRUE;
	}
	else if (message_type == ML_MSG_TREE_ONCREATEVIEW && param1 == nft_treeItem )
	{
		return (INT_PTR)WASABI_API_CREATEDIALOGW(IDD_VIEW_NFT, (HWND)param2, view_NFTDialogProc);
	}
	else if (message_type == ML_MSG_NAVIGATION_CONTEXTMENU)
	{
		return NFT_contextMenu(param1, (HWND)param2, MAKEPOINTS(param3));
	}
	else if (message_type == ML_MSG_ONSENDTOSELECT || message_type == ML_MSG_TREE_ONDROPTARGET)
	{
		// set with droptarget defaults =)
		UINT_PTR type = 0,data = 0;

		if (message_type == ML_MSG_ONSENDTOSELECT)
		{
			if (param3 != (INT_PTR)nft_pluginMessageProc) return 0;

			type=(int)param1;
			data = (int)param2;
		}
		else
		{
			if (param1 != nft_treeItem ) return 0;

			type=(int)param2;
			data=(int)param3;

			if (!data)
			{
				return (type == ML_TYPE_ITEMRECORDLISTW || type == ML_TYPE_ITEMRECORDLIST ||
						type == ML_TYPE_FILENAMES || type == ML_TYPE_STREAMNAMES ||
						type == ML_TYPE_FILENAMESW || type == ML_TYPE_STREAMNAMESW ||
						type == ML_TYPE_CDTRACKS ||
						type == ML_TYPE_PLAYLIST || type == ML_TYPE_PLAYLISTS) ? 1 : -1;
			}
		}
	}


	return 0;
}


static HRGN g_rgnUpdate = NULL;
static int offsetX = 0, offsetY = 0;

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
}
LAYOUT, PLAYOUT;

#define SETLAYOUTPOS(_layout, _x, _y, _cx, _cy) { _layout->x=_x; _layout->y=_y;_layout->cx=_cx;_layout->cy=_cy;_layout->rgn=NULL; }
#define SETLAYOUTFLAGS(_layout, _r)																						\
	{																													\
		BOOL fVis;																										\
		fVis = (WS_VISIBLE & (LONG)GetWindowLongPtr(_layout->hwnd, GWL_STYLE));											\
		if (_layout->x == _r.left && _layout->y == _r.top) _layout->flags |= SWP_NOMOVE;									\
		if (_layout->cx == (_r.right - _r.left) && _layout->cy == (_r.bottom - _r.top)) _layout->flags |= SWP_NOSIZE;	\
		if ((SWP_HIDEWINDOW & _layout->flags) && !fVis) _layout->flags &= ~SWP_HIDEWINDOW;								\
		if ((SWP_SHOWWINDOW & _layout->flags) && fVis) _layout->flags &= ~SWP_SHOWWINDOW;									\
	}

#define LAYOUTNEEEDUPDATE(_layout) ((SWP_NOMOVE | SWP_NOSIZE) != ((SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW | SWP_SHOWWINDOW) & _layout->flags))

#define GROUP_MIN			0x1
#define GROUP_MAX			0x2
#define GROUP_STATUSBAR		0x1
#define GROUP_MAIN			0x2

static void LayoutWindows(HWND hwnd, BOOL fRedraw, BOOL fUpdateAll = FALSE)
{
	static INT controls[] =
	{
		GROUP_STATUSBAR, GROUP_MAIN, IDC_LIST_NFT
	};

	INT index;
	RECT rc, rg, ri;
	LAYOUT layout[sizeof(controls)/sizeof(controls[0])], *pl;
	BOOL skipgroup;
	HRGN rgn = NULL;

	GetClientRect(hwnd, &rc);
	if ( rc.right == rc.left || rc.bottom == rc.top )
		return;

	if ( rc.right > WASABI_API_APP->getScaleX( 4 ) )
		rc.right -= WASABI_API_APP->getScaleX( 4 );

	SetRect(&rg, rc.left, rc.top, rc.right, rc.top);

	pl = layout;
	skipgroup = FALSE;

	InvalidateRect(hwnd, NULL, TRUE);

	for (index = 0; index < sizeof(controls) / sizeof(*controls); index++)
	{
		if (controls[index] >= GROUP_MIN && controls[index] <= GROUP_MAX) // group id
		{
			skipgroup = FALSE;
			switch (controls[index])
			{
				case GROUP_MAIN:
					SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1), rc.top, rc.right, rc.bottom);
					break;
			}
			continue;
		}
		if (skipgroup) continue;

		pl->id = controls[index];
		pl->hwnd = GetDlgItem(hwnd, pl->id);
		if (!pl->hwnd) continue;

		GetWindowRect(pl->hwnd, &ri);
		MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&ri, 2);
		pl->flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS;

		switch (pl->id)
		{
			case IDC_LIST_NFT:
				pl->flags |= (rg.top < rg.bottom) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS(pl, rg.left, rg.top + WASABI_API_APP->getScaleY(1),
							 rg.right - rg.left + WASABI_API_APP->getScaleY(1),
							 (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(2));
				break;
		}

		SETLAYOUTFLAGS(pl, ri);
		if (LAYOUTNEEEDUPDATE(pl))
		{
			if (SWP_NOSIZE == ((SWP_HIDEWINDOW | SWP_SHOWWINDOW | SWP_NOSIZE) & pl->flags) && 
				ri.left == (pl->x + offsetX) && ri.top == (pl->y + offsetY) && IsWindowVisible(pl->hwnd))
			{
				SetRect(&ri, pl->x, pl->y, pl->cx + pl->x, pl->y + pl->cy);
				ValidateRect(hwnd, &ri);
			}
			pl++;
		}
		else if ((fRedraw || (!offsetX && !offsetY)) && IsWindowVisible(pl->hwnd)) 
		{
			ValidateRect(hwnd, &ri);
			if (GetUpdateRect(pl->hwnd, NULL, FALSE)) 
			{
				if (!rgn) rgn = CreateRectRgn(0, 0, 0, 0);
				GetUpdateRgn(pl->hwnd, rgn, FALSE);
				OffsetRgn(rgn, pl->x, pl->y);
				InvalidateRgn(hwnd, rgn, FALSE);
			}
		}
	}

	if (pl != layout)
	{
		LAYOUT *pc;
		HDWP hdwp = BeginDeferWindowPos((INT)(pl - layout));
		for(pc = layout; pc < pl && hdwp; pc++)
		{
			hdwp = DeferWindowPos(hdwp, pc->hwnd, NULL, pc->x, pc->y, pc->cx, pc->cy, pc->flags);
		}
		if (hdwp) EndDeferWindowPos(hdwp);

		if (!rgn) rgn = CreateRectRgn(0, 0, 0, 0);

		if (fRedraw) 
		{
			GetUpdateRgn(hwnd, rgn, FALSE);
			for(pc = layout; pc < pl && hdwp; pc++)
			{
				if (pc->rgn) 
				{
					OffsetRgn(pc->rgn, pc->x, pc->y);
					CombineRgn(rgn, rgn, pc->rgn, RGN_OR);
				}
			}

			RedrawWindow(hwnd, NULL, rgn, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
		}
		if (g_rgnUpdate)
		{
			GetUpdateRgn(hwnd, g_rgnUpdate, FALSE);
			for(pc = layout; pc < pl && hdwp; pc++)
			{
				if (pc->rgn) 
				{
					OffsetRgn(pc->rgn, pc->x, pc->y);
					CombineRgn(g_rgnUpdate, g_rgnUpdate, pc->rgn, RGN_OR);
				}
			}
		}

		for(pc = layout; pc < pl && hdwp; pc++) if (pc->rgn) DeleteObject(pc->rgn);
	}

	if (rgn) DeleteObject(rgn);
	ValidateRgn(hwnd, NULL);
}

static BOOL NFT_OnDisplayChange()
{
	ListView_SetTextColor(m_nft_list.getwnd(),dialogSkinner.Color(WADLG_ITEMFG));
	ListView_SetBkColor(m_nft_list.getwnd(),dialogSkinner.Color(WADLG_ITEMBG));
	ListView_SetTextBkColor(m_nft_list.getwnd(),dialogSkinner.Color(WADLG_ITEMBG));
	m_nft_list.SetFont(dialogSkinner.GetFont());
	LayoutWindows(m_hwnd, TRUE);
	return 0;
}


enum
{
	BPM_ECHO_WM_COMMAND = 0x1, // send WM_COMMAND and return value
	BPM_WM_COMMAND      = 0x2, // just send WM_COMMAND
};

static BOOL NFT_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	return FALSE;
}

static BOOL NFT_OnDestroy(HWND hwnd)
{
	m_hwnd = 0;

	return FALSE;
}

static BOOL NFT_OnNotify(HWND hwnd, NMHDR *notification)
{
	if (notification->idFrom== IDC_LIST_NFT )
	{
		if (notification->code == LVN_BEGINDRAG)
		{
			SetCapture(hwnd);
		}
	}
	return FALSE;
}

static BOOL NFT_OnInitDialog(HWND hwndDlg, HWND hwndFocus, LPARAM lParam)
{
	m_hwnd = hwndDlg;
	m_nft_list.setwnd(GetDlgItem(hwndDlg, IDC_LIST_NFT ));

	if (!view.play)
	{
		SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_GET_VIEW_BUTTON_TEXT, (WPARAM)&view);
	}


	NFT_OnDisplayChange();

	m_headerhwnd = ListView_GetHeader( m_nft_list.getwnd() );

	// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
	//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
	customAllowed = FALSE;

	MLSKINWINDOW m = {0};
	m.skinType     = SKINNEDWND_TYPE_DIALOG;
	m.style        = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	m.hwndToSkin   = hwndDlg;

	MLSkinWindow(plugin.hwndLibraryParent, &m);


	ShellExecuteW( NULL, L"open", NFT_BASE_URL, NULL, NULL, SW_SHOWNORMAL );

	delete g_wa_config;

	SetTimer( hwndDlg, 100, 15, NULL );
	

	return TRUE;
}

INT_PTR CALLBACK view_NFTDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	INT_PTR a = dialogSkinner.Handle(hwndDlg, uMsg, wParam, lParam);
	if (a) return a;

	switch(uMsg) 
	{
		HANDLE_MSG(hwndDlg, WM_INITDIALOG, NFT_OnInitDialog);
		HANDLE_MSG(hwndDlg, WM_COMMAND,    NFT_OnCommand);
		HANDLE_MSG(hwndDlg, WM_DESTROY,    NFT_OnDestroy);

		case WM_WINDOWPOSCHANGED:
			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & ((WINDOWPOS*)lParam)->flags) || 
				(SWP_FRAMECHANGED & ((WINDOWPOS*)lParam)->flags))
			{
				LayoutWindows(hwndDlg, !(SWP_NOREDRAW & ((WINDOWPOS*)lParam)->flags));
			}
			return 0;

		case WM_USER + 0x200:
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 1); // yes, we support no - redraw resize
			return TRUE;

		case WM_USER + 0x201:
			offsetX = (short)LOWORD(wParam);
			offsetY = (short)HIWORD(wParam);
			g_rgnUpdate = (HRGN)lParam;
			return TRUE;

		case WM_PAINT:
			{
				int tab[] = { IDC_LIST_NFT |DCW_SUNKENBORDER};
				dialogSkinner.Draw(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
			}
			return 0;

		case WM_INITMENUPOPUP:
			if (wParam && (HMENU)wParam == s.build_hMenu && s.mode==1)
			{
				myMenu = TRUE;
				if(SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU)==0xffffffff)
					s.mode=2;
				myMenu = FALSE;
			}
			return 0;

		case WM_DISPLAYCHANGE:
			return NFT_OnDisplayChange();

		case WM_NOTIFY:
			return NFT_OnNotify(hwndDlg, (LPNMHDR)lParam);
	}

	return FALSE;
}