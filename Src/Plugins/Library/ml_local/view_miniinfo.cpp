#include "main.h"
#include "api__ml_local.h"
#include "..\..\General\gen_ml/config.h"
#include "resource.h"

#include "..\..\General\gen_ml/ml_ipc_0313.h"

static int g_displayinfo = TRUE, g_displayinfochanged=FALSE;

static HWND m_media_hwnd = NULL;
static HWND m_info_hwnd = NULL;

static HRGN g_rgnUpdate = NULL;
static int offsetX = 0, offsetY = 0;

#define IDC_WEBINFO		0x1000

static void LayoutWindows(HWND hwnd, BOOL fShowInfo, BOOL fRedraw)
{
	RECT rc, ri, rg;
	HRGN rgn = NULL;

	GetClientRect(hwnd, &rc);
	SetRect(&rg, 0, 0, 0, 0);

	if (rc.bottom - rc.top < WASABI_API_APP->getScaleY(180)) fShowInfo = FALSE;
	if (rc.bottom <= rc.top || rc.right <= rc.left) return;

	if (fShowInfo) 
	{
		if (!IsWindow(m_info_hwnd)) // create browser
		{
			WEBINFOCREATE wic = {0};
			wic.hwndParent = hwnd;
			wic.uMsgQuery = WM_QUERYFILEINFO;
			wic.x = rc.left;
			wic.y = rc.bottom - WASABI_API_APP->getScaleY(101);
			wic.cx = (rc.right - rc.left) - WASABI_API_APP->getScaleX(3);
			wic.cy = WASABI_API_APP->getScaleY(100);
			wic.ctrlId = IDC_WEBINFO;
					
			m_info_hwnd = (HWND)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_CREATEWEBINFO, (WPARAM)&wic);
			if (IsWindow(m_info_hwnd))
			{
				SetWindowPos(m_info_hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
							 SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOCOPYBITS);
				ShowWindow(m_info_hwnd, SW_SHOWNORMAL);
			}
		}
		else 
		{
			SetWindowPos(m_info_hwnd, NULL, rc.left, rc.bottom - WASABI_API_APP->getScaleY(101),
						 rc.right - WASABI_API_APP->getScaleX(3), WASABI_API_APP->getScaleY(100),
						 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);
		}

		if (IsWindow(m_info_hwnd))
		{
			rc.bottom -= WASABI_API_APP->getScaleY(104);
			SetRect(&rg, rc.left, rc.bottom - WASABI_API_APP->getScaleY(101),
					rc.right - WASABI_API_APP->getScaleX(10),
					rc.bottom - WASABI_API_APP->getScaleY(1));
		}
	}

	InvalidateRect(hwnd, NULL, TRUE);

	if (IsWindow(m_media_hwnd))
	{
		SetRect(&ri, rc.left, rc.top, rc.right, rc.bottom);
		rgn = CreateRectRgn(0, 0, ri.right - ri.left, ri.bottom - ri.top);	
		SendMessage(m_media_hwnd, WM_USER + 0x201, MAKEWPARAM(offsetX, offsetY), (LPARAM)rgn);
		SetWindowPos(m_media_hwnd, NULL, ri.left, ri.top, ri.right - ri.left, ri.bottom - ri.top,  
					 SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOACTIVATE | ((!fRedraw) ? SWP_NOREDRAW : 0));
		SendMessage(m_media_hwnd, WM_USER + 0x201, 0, 0L);
		if (IsWindowVisible(m_media_hwnd))
		{
			ValidateRect(hwnd, &ri);
			if (GetUpdateRect(m_media_hwnd, NULL, FALSE)) 
			{
				GetUpdateRgn(m_media_hwnd, rgn, FALSE);
				OffsetRgn(rgn, ri.left, ri.top);
				InvalidateRgn(hwnd, rgn, FALSE);
			}
		}
	}

	if (fRedraw) 
	{
		UpdateWindow(hwnd);
		if (IsWindow(m_media_hwnd)) UpdateWindow(m_media_hwnd);
	}
	if (g_rgnUpdate)
	{
		GetUpdateRgn(hwnd, g_rgnUpdate, FALSE);
		if (rgn)
		{
			OffsetRgn(rgn, rc.left, rc.top);
			CombineRgn(g_rgnUpdate, g_rgnUpdate, rgn, RGN_OR);
		}
	}
	ValidateRgn(hwnd, NULL);
	if (rgn) DeleteObject(rgn);
}

INT_PTR CALLBACK view_miniinfoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{ 
	BOOL a=dialogSkinner.Handle(hwndDlg,uMsg,wParam,lParam); if (a) return a;

	switch(uMsg)
	{
		case WM_DISPLAYCHANGE:
			if (IsWindow(m_media_hwnd)) PostMessageW(m_media_hwnd, WM_DISPLAYCHANGE, wParam, lParam);
			if (IsWindow(m_info_hwnd)) PostMessageW(m_info_hwnd, WM_DISPLAYCHANGE, wParam, lParam);
		break;

		case WM_INITDIALOG:
		{
			g_displayinfo = g_view_metaconf->ReadInt(L"midivvis", 1);
			g_displayinfochanged = FALSE;
			m_media_hwnd = (lParam) ? WASABI_API_CREATEDIALOGW(((INT_PTR*)lParam)[1], hwndDlg, (DLGPROC)((INT_PTR*)lParam)[0]) : NULL;
			SetWindowPos(m_media_hwnd, NULL, 0,0,0,0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW | SWP_SHOWWINDOW);

			MLSKINWINDOW m = {0};
			m.skinType = SKINNEDWND_TYPE_DIALOG;
			m.hwndToSkin = hwndDlg;
			m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
			MLSkinWindow(plugin.hwndLibraryParent, &m);
		}
		return TRUE;

		case WM_WINDOWPOSCHANGED:
			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & ((WINDOWPOS*)lParam)->flags) || 
				(SWP_FRAMECHANGED & ((WINDOWPOS*)lParam)->flags))
			{
				LayoutWindows(hwndDlg, g_displayinfo, !(SWP_NOREDRAW & ((WINDOWPOS*)lParam)->flags));
			}
			return 0;

		case WM_APP+1: //sent by parent for resizing window
			if (IsWindow(m_media_hwnd)) return SendMessage(m_media_hwnd,uMsg,wParam,lParam); // forward on
			return 0;

		case WM_APP+2: //sent by media child to get current query
			return SendMessage(GetParent(hwndDlg),uMsg,wParam,lParam); // forward on

		case WM_QUERYFILEINFO:	if(IsWindow(m_media_hwnd)) PostMessageW(m_media_hwnd, uMsg, wParam, lParam);  break;

		case WM_SHOWFILEINFO:				
			if (IsWindow(m_info_hwnd))
			{
				WEBINFOSHOW wis = {0};
				wis.pszFileName = (LPCWSTR)lParam;
				wis.fFlags = (TRUE == wParam) ? WISF_FORCE : WISF_NORMAL;
				SENDMLIPC(m_info_hwnd, ML_IPC_WEBINFO_SHOWINFO, (WPARAM)&wis);
			}
		case WM_USER+66:
			if (wParam == -1) 
			{
				g_displayinfo = !g_displayinfo;
				g_displayinfochanged = !g_displayinfochanged;
				if (IsWindow(m_info_hwnd)) ShowWindow(m_info_hwnd, (g_displayinfo) ? SW_SHOWNA : SW_HIDE);
				LayoutWindows(hwndDlg, g_displayinfo, TRUE);
			}
			SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT, (g_displayinfo) ? 0xff : 0xf0);
		return TRUE;

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
			if (GetDlgItem(hwndDlg, IDC_WEBINFO))
			{
				static int tab[] = { IDC_WEBINFO | DCW_SUNKENBORDER };
				dialogSkinner.Draw(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
			}
			else
				dialogSkinner.Draw(hwndDlg, 0, 0);
		}
		return 0;

		case WM_ERASEBKGND:
		return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT

		case WM_DESTROY:
			if(g_displayinfochanged) g_view_metaconf->WriteInt(L"midivvis", g_displayinfo);
			if (IsWindow(m_info_hwnd)) SENDMLIPC(m_info_hwnd, ML_IPC_WEBINFO_RELEASE, 0);
			m_media_hwnd = NULL; 	 
			m_info_hwnd = NULL;
		break;
	}
	return FALSE;
}