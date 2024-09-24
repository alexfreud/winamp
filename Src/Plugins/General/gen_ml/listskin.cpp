#include "main.h"
#include "listskin.h"
#include "scrollwnd.h"
#include "../nu/CCVersion.h"

#ifndef LVS_EX_DOUBLEBUFFER  //this will work XP only
#define LVS_EX_DOUBLEBUFFER		0x00010000
#endif

COLORREF Blender(COLORREF fg, COLORREF bg){
	return RGB((GetRValue(fg)+GetRValue(bg))/2,(GetGValue(fg)+GetGValue(bg))/2,(GetBValue(fg)+GetBValue(bg))/2);
}

//listview's wndproc
static INT_PTR CALLBACK wndproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  ListSkin *ls=(ListSkin *)(LONG_PTR)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
  INT_PTR r=handleListViewHeaderMsgs(hwndDlg,
								ls->m_headerwnd,
								ls->m_listwnd,
								uMsg,
								wParam,
								lParam,
								ls->sortShow,
								ls->sortAscending,
								ls->sortIndex);
  if(r) return r;
  if (uMsg == WM_ENABLE)
  {
	  // custom handling of this allows us to handle disabled listview controls correctly
	  // so we don't have the part skinned / part OS colouring which happened before
	  InvalidateRect(hwndDlg,0,1);
	  ls->m_enabled = wParam;
	  return 1;
  }
  if (uMsg == WM_ERASEBKGND) 
  {
    //fg> removes the header's region from the erasebackground's hdc clipping region
    // so that they do not flicker when scrolling the list with the horizontal scrollbar
    // in transparency mode
    HDC dc = (HDC)wParam;
    RECT r;
    RECT hr;
    GetClientRect(hwndDlg, &r);
    GetClientRect(ListView_GetHeader(hwndDlg), &hr);
    RECT lr;
    SubtractRect(&lr, &r, &hr);
    HRGN rgn = CreateRectRgnIndirect(&lr);
    int rt = GetClipRgn(dc, rgn);
    if (rt == 0) {
      SelectClipRgn(dc, rgn);
    } else if (rt > 0) {
      HRGN trg = CreateRectRgnIndirect(&lr);
      HRGN res = CreateRectRgn(0,0,0,0);
      IntersectRgn(res, rgn, trg);
      SelectClipRgn(dc, res);
      DeleteRgn(res);
      DeleteRgn(trg);
    }
    DeleteRgn(rgn);
  }
  if (uMsg == WM_KEYUP && wParam == VK_RETURN)
  {
    NMHDR nmh=
    {
      ls->m_listwnd,
      GetDlgCtrlID(ls->m_listwnd),
      NM_RETURN,
    };
    SendMessage(ls->m_hwnd,WM_NOTIFY,0,(LPARAM)&nmh);
    return 0;
  }
  return CallWindowProc(ls->m_old_wndproc,hwndDlg,uMsg,wParam,lParam);
}

//dialog's wndproc
static INT_PTR CALLBACK mainwndproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  ListSkin *ls=(ListSkin *)(LONG_PTR)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);

  if(uMsg==WM_USER+0x411) return 0;//ls->m_changing_item_sel;

  if(uMsg==WM_NOTIFY)
  {
    LPNMCUSTOMDRAW lpnmcd = (NMCUSTOMDRAW *)lParam;
    //HWND listwnd=ls->m_listwnd;
    HWND listwnd=lpnmcd->hdr.hwndFrom;
    if(lpnmcd->hdr.code == NM_CUSTOMDRAW && lpnmcd->hdr.hwndFrom == listwnd) 
    {
      LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)lParam;
      NMCUSTOMDRAW   &nmcd = lplvcd->nmcd;
      //static bool bHighlighted = false;
      
      switch(lplvcd->nmcd.dwDrawStage)
      {
      case CDDS_PREPAINT :
        return CDRF_NOTIFYITEMDRAW;
        
        // Modify item text and or background
      case CDDS_ITEMPREPAINT:
        {
          int iRow = (int)nmcd.dwItemSpec;
          
          bool bHighlighted = (ListView_GetItemState(listwnd, iRow, LVIS_SELECTED) != 0);
          
          if (bHighlighted)
          {
            if(GetFocus()==listwnd && ls->m_enabled)
            {
              lplvcd->clrText   = WADlg_getColor(WADLG_SELBAR_FGCOLOR);
              lplvcd->clrTextBk = WADlg_getColor(WADLG_SELBAR_BGCOLOR);
            } else {
			  lplvcd->clrText   = (!ls->m_enabled?Blender(WADlg_getColor(WADLG_INACT_SELBAR_FGCOLOR),WADlg_getColor(WADLG_WNDBG)):WADlg_getColor(WADLG_INACT_SELBAR_FGCOLOR));
			  if(ls->m_enabled)
				lplvcd->clrTextBk = WADlg_getColor(WADLG_INACT_SELBAR_BGCOLOR);
			  else
				lplvcd->clrTextBk = WADlg_getColor(WADLG_ITEMBG);
            }
		  }
		  else
		  {
		    if(!ls->m_enabled)
			{
			  lplvcd->clrText = Blender(WADlg_getColor(WADLG_INACT_SELBAR_FGCOLOR),WADlg_getColor(WADLG_WNDBG));
			  lplvcd->clrTextBk = WADlg_getColor(WADLG_ITEMBG);
			}
		  }

            // Turn off listview highlight otherwise it uses the system colors!
            //ls->m_changing_item_sel=1;
            //ListView_SetItemState(listwnd, iRow, 0, LVIS_SELECTED);
            lplvcd->nmcd.uItemState &= ~CDIS_SELECTED;

          return CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
        }
        
        // Modify sub item text and/or background
      case CDDS_ITEMPOSTPAINT:
        {
          //if(bHighlighted)
          //{
   //         int  iRow = (int)nmcd.dwItemSpec;
            // Turn listview control's highlighting back on now that we have
            // drawn the row in the colors we want.
            //LockWindowUpdate(listwnd);
 //           lplvcd->nmcd.uItemState |= CDIS_SELECTED;
 //             ListView_SetItemState(listwnd, iRow, 0xff, LVIS_SELECTED);
            //ls->m_changing_item_sel=0;
            //LockWindowUpdate(NULL);
            //CT> now validate the window so it doesn't flicker
            //RECT r;
            //ListView_GetItemRect(listwnd,iRow,&r,LVIR_BOUNDS);
            //ValidateRect(listwnd,&r);
          //}
        }
      default:
        return CDRF_DODEFAULT;
      }
    }
  }

  return CallWindowProc(ls->m_old_mainwndproc,hwndDlg,uMsg,wParam,lParam);
}

//list header's wndproc
static INT_PTR CALLBACK header_wndproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  ListSkin *ls=(ListSkin *)(LONG_PTR)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
  INT_PTR a=handleListViewHeaderPaintMsgs(ls->m_headerwnd,uMsg,wParam,lParam);
  if(a) return a;
  return CallWindowProc(ls->m_old_header_wndproc,hwndDlg,uMsg,wParam,lParam);
}

ListSkin::ListSkin(HWND hwnd)
{
  m_hwnd=GetParent(hwnd);
  m_listwnd=hwnd;

  sortShow		= FALSE;
  sortAscending	= TRUE;
  sortIndex		= 0;

	if (comctlVersion >= PACKVERSION(6,0))
				ListView_SetExtendedListViewStyleEx(m_listwnd, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER);
  m_headerwnd=ListView_GetHeader(hwnd);
  SetWindowLongPtr(hwnd,GWLP_USERDATA, (LONGX86)(LONG_PTR)this);
  m_old_wndproc=(WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd,GWLP_WNDPROC, (LONGX86)(LONG_PTR)wndproc);
  SetWindowLongPtr(m_headerwnd,GWLP_USERDATA, (LONGX86)(LONG_PTR)this);
  m_old_header_wndproc=(WNDPROC)(LONG_PTR)SetWindowLongPtr(m_headerwnd,GWLP_WNDPROC, (LONGX86)(LONG_PTR)header_wndproc);
  m_scrollwnd=new ScrollWnd(hwnd, SCROLLBAR_LISTVIEW);
  m_old_mainwndproc=NULL;
  m_enabled = IsWindowEnabled(hwnd);
  //m_changing_item_sel=0;
  if(!GetWindowLongPtr(m_hwnd,GWLP_USERDATA))
  {
    SetWindowLongPtr(m_hwnd,GWLP_USERDATA, (LONGX86)(LONG_PTR)this);
    m_old_mainwndproc=(WNDPROC)(LONG_PTR)SetWindowLongPtr(m_hwnd,GWLP_WNDPROC, (LONGX86)(LONG_PTR)mainwndproc);
  }
	
}

ListSkin::~ListSkin()
{
  delete m_scrollwnd;
	m_scrollwnd=0;
  SetWindowLongPtr(m_listwnd,GWLP_WNDPROC, (LONGX86)(LONG_PTR)m_old_wndproc);
  SetWindowLongPtr(m_headerwnd,GWLP_WNDPROC, (LONGX86)(LONG_PTR)m_old_header_wndproc);
  if(m_old_mainwndproc) SetWindowLongPtr(m_hwnd,GWLP_WNDPROC, (LONGX86)(LONG_PTR)m_old_mainwndproc);
}

void ListSkin::updateScrollWnd()
{
  m_scrollwnd->update();
}

void ListSkin::disableHorzScroll()
{
  m_scrollwnd->disableHorzScroll();
}