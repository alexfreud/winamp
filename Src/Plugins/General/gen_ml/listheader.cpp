#include "main.h"
#include <windowsx.h>
#include "config.h"
#include "../winamp/wa_dlg.h"

static int m_column_resize=0; //0=normal, 1=selected item only, 2=proportional


static int m_origsizes[32], m_origwidth;
static float m_origperc[32];

static void columnTrackStart(HWND hwnd, int item)
{
  int l=Header_GetItemCount(hwnd);
  m_origwidth=0;
  int i;
  for(i=item+1;i<l;i++) {
	  HD_ITEM hi;
		hi.mask = HDI_WIDTH;
		Header_GetItem(hwnd, i, &hi);
    m_origwidth+=hi.cxy;
  }
  for(i=item;i<l;i++) {
    if(i==-1) continue;
	  HD_ITEM hi;
		hi.mask = HDI_WIDTH;
		Header_GetItem(hwnd, i, &hi);
    m_origsizes[i]=hi.cxy;
    if(m_origwidth==0) 
      m_origperc[i]=0;
    else
      m_origperc[i]=(float)hi.cxy/(float)m_origwidth;
  }
}

static void columnAutoResizeProp(HWND hwnd, int item)
{
  SendMessage(GetParent(hwnd),WM_SETREDRAW,FALSE,0);
  int l=Header_GetItemCount(hwnd);
  int width=0;
  int i;

  HD_ITEM hi;
  hi.mask = HDI_WIDTH;
  Header_GetItem(hwnd, item, &hi);
  width=m_origwidth-(hi.cxy-m_origsizes[item]);
  float rest=0;
  for(i=item+1;i<l;i++) {
	  HD_ITEM hi;
		hi.mask = HDI_WIDTH;
    float l=m_origperc[i]*(float)width;
    l+=rest;
    rest=0;
    int l2=(int)l;
    rest+=l-(float)l2;
    hi.cxy=l2;
		Header_SetItem(hwnd, i, &hi);
  }
  SendMessage(GetParent(hwnd),WM_SETREDRAW,TRUE,0);
}

static void columnAutoResizeItem(HWND hwnd, int item)
{
  HD_ITEM hi;
  hi.mask = HDI_WIDTH;
  Header_GetItem(hwnd, item, &hi);
  int diff=hi.cxy-m_origsizes[item];

  hi.mask = HDI_WIDTH;
  if(Header_GetItem(hwnd, item+1, &hi)) {
    SendMessage(GetParent(hwnd),WM_SETREDRAW,FALSE,0);
    hi.cxy-=diff;
    Header_SetItem(hwnd,item+1,&hi);
    SendMessage(GetParent(hwnd),WM_SETREDRAW,TRUE,0);
  }
  m_origsizes[item]+=diff;
}

static void size_autoResizeStart(HWND h)
{
  RECT r;
  GetClientRect(h,&r);
  char tmp[128];
  wsprintf(tmp,"start:%i\n",r.right-r.left);
  OutputDebugString(tmp);
}

static void size_autoResizeProp(HWND h)
{
  RECT r;
  GetClientRect(h,&r);
  char tmp[128];
  wsprintf(tmp,"prop:%i\n",r.right-r.left);
  OutputDebugString(tmp);
}

INT_PTR handleListViewHeaderMsgs(HWND hwndDlg,
							 HWND headerWnd,
							 HWND listWnd,
							 UINT uMsg,
							 WPARAM wParam,
							 LPARAM lParam,
							 BOOL sortShow,
							 BOOL sortAscending,
							 int sortIndex) 
{
  if (uMsg == WM_NOTIFY) {
    LPNMCUSTOMDRAW lpnmcd = (NMCUSTOMDRAW *)lParam;
    if(lpnmcd->hdr.code == NM_CUSTOMDRAW && lpnmcd->hdr.hwndFrom == headerWnd) {
      switch (lpnmcd->dwDrawStage) { 
        // prior to painting 
        case CDDS_PREPAINT: 
          return CDRF_NOTIFYITEMDRAW; // tell windows we want individual notification of each item being drawn 
        // notification of each item being drawn 
        case CDDS_ITEMPREPAINT: 
          {
            LOGBRUSH lb={BS_SOLID,WADlg_getColor(WADLG_LISTHEADER_BGCOLOR)};
            HBRUSH brush;
            brush=CreateBrushIndirect(&lb);
            HDC hdc=lpnmcd->hdc;
            RECT *rc=&lpnmcd->rc;
            FillRect(hdc,rc,brush);
            DeleteObject(brush);

            int selected=(lpnmcd->uItemState&CDIS_SELECTED)?1:0;
            
            HPEN pen;

            if (!selected) pen=CreatePen(PS_SOLID,1,WADlg_getColor(WADLG_LISTHEADER_FRAME_TOPCOLOR));
            else pen=CreatePen(PS_SOLID,1,WADlg_getColor(WADLG_LISTHEADER_FRAME_BOTTOMCOLOR));

            HGDIOBJ oldobj=SelectObject(hdc,pen);
            //SelectPen(hdc,pen);
            MoveToEx(hdc,rc->left,rc->top,NULL);
            LineTo(hdc,rc->right,rc->top);
            MoveToEx(hdc,rc->left,rc->top,NULL);
            LineTo(hdc,rc->left,rc->bottom);

            if (!selected)
            {
              SelectObject(hdc,oldobj);
              DeleteObject(pen);
              pen=CreatePen(PS_SOLID,1,WADlg_getColor(WADLG_LISTHEADER_FRAME_BOTTOMCOLOR));
              oldobj=SelectObject(hdc,pen);
            }

            MoveToEx(hdc,rc->right-1,rc->top,NULL);
            LineTo(hdc,rc->right-1,rc->bottom);
            MoveToEx(hdc,rc->right-1,rc->bottom-1,NULL);
            LineTo(hdc,rc->left-1,rc->bottom-1);

            SelectObject(hdc,oldobj);
            DeleteObject(pen);

            if(!selected) {
              pen=CreatePen(PS_SOLID,1,WADlg_getColor(WADLG_LISTHEADER_FRAME_MIDDLECOLOR));
              oldobj=SelectObject(hdc,pen);
              MoveToEx(hdc,rc->right-2,rc->top+1,NULL);
              LineTo(hdc,rc->right-2,rc->bottom-2);
              MoveToEx(hdc,rc->right-2,rc->bottom-2,NULL);
              LineTo(hdc,rc->left,rc->bottom-2);
              SelectObject(hdc,oldobj);
              DeleteObject(pen);
            }
            
            DWORD_PTR i=lpnmcd->dwItemSpec;
            char txt[128];
            LVCOLUMN lv={LVCF_TEXT,0,0,txt,sizeof(txt)-1,};
            ListView_GetColumn(listWnd,i,&lv);
            SetBkMode(hdc,TRANSPARENT);
			      SetTextColor(hdc,WADlg_getColor(WADLG_LISTHEADER_FONTCOLOR));
            RECT rc2=*rc;
            rc2.left+=5; rc2.right-=3;
            rc2.top+=2; rc2.bottom-=2;
            if(selected) {
              rc2.left++; 
              rc2.top++;
            }
			
            if (sortShow && (i == sortIndex) && ((rc->right - rc->left) > 40) )
			{
				rc2.right -= 14;
				HPEN penDark;
				pen=CreatePen(PS_SOLID,1,WADlg_getColor(WADLG_LISTHEADER_FRAME_TOPCOLOR));
				penDark=CreatePen(PS_SOLID,1,WADlg_getColor(WADLG_LISTHEADER_EMPTY_BGCOLOR));
				oldobj=SelectObject(hdc,pen);
				if (sortAscending)
				{
					//strcpy(txt, " /\\ ");
					// Draw triangle pointing upwards
					MoveToEx(hdc, rc->right - 10, rc->top + 5, NULL);
					LineTo(hdc, rc->right - 6, rc->bottom - 6);
					LineTo(hdc, rc->right - 15, rc->bottom - 6 );
					MoveToEx(hdc, rc->right - 14, rc->bottom - 7, NULL );

					SelectObject(hdc, penDark);
					LineTo(hdc,  rc->right - 10, rc->top + 5);
				}
				else
				{
					// Draw triangle pointing downwords
					MoveToEx(hdc, rc->right - 7, rc->top + 7, NULL);
					LineTo(hdc, rc->right - 11, rc->bottom - 6 );
					MoveToEx(hdc, rc->right - 11, rc->bottom - 6, NULL);

					SelectObject(hdc, penDark);
					LineTo(hdc, rc->right - 15, rc->top + 6 );
					LineTo(hdc, rc->right - 6, rc->top + 6);
				}
				SelectObject(hdc,oldobj);
				DeleteObject(pen);
				DeleteObject(penDark);
			}
			DrawText(hdc,txt,-1,&rc2,DT_VCENTER|DT_SINGLELINE|DT_LEFT);

          }
          return CDRF_SKIPDEFAULT; 
      }
    }
    HD_NOTIFY   *pHDN = (HD_NOTIFY*)lParam;
    if(pHDN->hdr.code == HDN_BEGINTRACKW || pHDN->hdr.code == HDN_BEGINTRACKA)
    {
      m_column_resize=g_config->ReadInt("column_resize_mode",m_column_resize);
      if(m_column_resize) columnTrackStart(pHDN->hdr.hwndFrom, pHDN->iItem);
    }

    if(pHDN->hdr.code==HDN_ENDTRACKW || pHDN->hdr.code==HDN_ENDTRACKA || pHDN->hdr.code==HDN_ITEMCHANGINGW || pHDN->hdr.code==HDN_ITEMCHANGINGA) {
      static int disable=0;
      if(disable) return FALSE;
      disable=1;
      if(m_column_resize==1) columnAutoResizeItem(pHDN->hdr.hwndFrom, pHDN->iItem);
      if(m_column_resize==2) columnAutoResizeProp(pHDN->hdr.hwndFrom, pHDN->iItem);
      disable=0;
      SendMessage(hwndDlg,WM_USER+0x3443,0,0); //update ScrollWnd
    }
  }
  /*if(uMsg==WM_WINDOWPOSCHANGING) {
    HWND h=ListView_GetHeader(hwndDlg);
    if(h) size_autoResizeStart(h);
  }
  if(uMsg==WM_SIZE) {
    HWND h=ListView_GetHeader(hwndDlg);
    if(h) size_autoResizeProp(h);
  }*/
  return 0;
}

static int RectInRect(RECT *rect1, RECT *rect2)
{ 
  // this has a bias towards true

  // this could probably be optimized a lot
  return ((rect1->top >= rect2->top && rect1->top <= rect2->bottom) ||
      (rect1->bottom >= rect2->top && rect1->bottom <= rect2->bottom) ||
      (rect2->top >= rect1->top && rect2->top <= rect1->bottom) ||
      (rect2->bottom >= rect1->top && rect2->bottom <= rect1->bottom)) // vertical intersect
      &&
      ((rect1->left >= rect2->left && rect1->left <= rect2->right) ||
      (rect1->right >= rect2->left && rect1->right <= rect2->right) ||
      (rect2->left >= rect1->left && rect2->left <= rect1->right) ||
      (rect2->right >= rect1->left && rect2->right <= rect1->right)) // horiz intersect
      ;
}

INT_PTR handleListViewHeaderPaintMsgs(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if(uMsg==WM_ERASEBKGND) 
  {
    return 1; //we erase the background below
  }
  if(uMsg==WM_PAINT)
  {
    //process the grey area with our color
    RECT r;
    GetClientRect(hwndDlg,&r);
    int n=Header_GetItemCount(hwndDlg);
    if(n)
    {
      RECT r2;
      Header_GetItemRect(hwndDlg,n-1,&r2);
      r.left=r2.right;
    }
    RECT ur;
    GetUpdateRect(hwndDlg,&ur,FALSE);
    if(RectInRect(&r,&ur))
    {
      HDC hdc=GetDC(hwndDlg);
      HBRUSH b=CreateSolidBrush(WADlg_getColor(WADLG_LISTHEADER_EMPTY_BGCOLOR));
      FillRect(hdc,&r,b);
      DeleteObject(b);
      ReleaseDC(hwndDlg,hdc);
      ValidateRect(hwndDlg,&r);
    }
  }
  return 0;
}
