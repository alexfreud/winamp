#include "main.h"

#include "comboskin.h"

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

static INT_PTR CALLBACK wndproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  ComboSkin *cs=(ComboSkin *)(LONG_PTR)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
  if (uMsg == WM_PAINT) 
  {
    RECT r;
    GetClientRect(hwndDlg,&r);
    RECT ur;
    GetUpdateRect(hwndDlg,&ur,FALSE);
    if(RectInRect(&r,&ur)) //hmmm not sure about testing this, should probably use a backbuffer or something
    {
      //Fill with bg color
      HDC hdc=GetDC(hwndDlg);
      HBRUSH b=CreateSolidBrush(WADlg_getColor(WADLG_WNDBG));
      HBRUSH b2=CreateSolidBrush(WADlg_getColor(WADLG_HILITE)); //sunken

      //top line
      {
        RECT a={0,0,r.right-r.left,3};
        FillRect(hdc,&a,b);
        ValidateRect(hwndDlg,&a);
      }
      //bottom lines
      {
        RECT a={0,r.bottom-2,r.right-r.left,r.bottom};
        FillRect(hdc,&a,b);
        ValidateRect(hwndDlg,&a);
      }
      {
        //sunken part
        RECT a={0,r.bottom-3,r.right-r.left,r.bottom-2};
        FillRect(hdc,&a,b2);
        ValidateRect(hwndDlg,&a);
      }
      //left
      {
        RECT a={0,0,2,r.bottom-r.top};
        FillRect(hdc,&a,b);
        ValidateRect(hwndDlg,&a);
      }
      //right
      {
        RECT a={r.right-2,0,r.right,r.bottom-r.top};
        FillRect(hdc,&a,b);
        ValidateRect(hwndDlg,&a);
      }

      //paint the arrow
      HBITMAP bmp=WADlg_getBitmap();
      HDC hdcbmp = CreateCompatibleDC(hdc);
      SelectObject(hdcbmp,bmp);
      int pushed=0;
      if(GetAsyncKeyState(MK_LBUTTON) & 0x8000)
      {
        //check if in arrow down area
        POINT cursor;
        GetCursorPos(&cursor);
        ScreenToClient(hwndDlg,&cursor);
        if(cursor.x >= r.right-20 && cursor.x <= r.right) 
		{
			pushed=1;
		}
      }
      int startx=14;
      int starty=31;
      if(pushed) startx+=28;
      int left=r.right-18;
      int top=r.top+4;
      StretchBlt(hdc,left,top,14,14,hdcbmp,startx,starty,14,14,SRCCOPY);
      DeleteDC(hdcbmp);
      RECT a={left,top,left+14,top+14};
      ValidateRect(hwndDlg,&a);
      //paint arrow borders
      {
        HBRUSH b=CreateSolidBrush(WADlg_getColor(WADLG_ITEMBG));
        RECT a={left,3,left+14,4};
        FillRect(hdc,&a,b);
        ValidateRect(hwndDlg,&a);

        RECT c={left,17,left+14,18};
        FillRect(hdc,&c,b);
        ValidateRect(hwndDlg,&c);

        RECT d={left+14,3,left+15,18};
        FillRect(hdc,&d,b);
        ValidateRect(hwndDlg,&d);

        RECT e={left+15,3,left+16,19};
        FillRect(hdc,&e,b2);
        ValidateRect(hwndDlg,&e);

        DeleteObject(b);
      }
      
      DeleteObject(b);
      DeleteObject(b2);
      ReleaseDC(hwndDlg,hdc);
    }
  }

  if(uMsg == WM_WINDOWPOSCHANGING)
  {
    //move it up 1 pixel so it's correctly centered
    WINDOWPOS *wp=(WINDOWPOS *)lParam;
    wp->y--;
  }

  if(uMsg == WM_KILLFOCUS || uMsg == WM_LBUTTONUP)
  {
    InvalidateRect(hwndDlg,NULL,TRUE);
  }
  
  return CallWindowProc(cs->m_old_wndproc,hwndDlg,uMsg,wParam,lParam);
}

ComboSkin::ComboSkin(HWND hwnd)
{
  m_hwnd=hwnd;

  SetWindowLongPtr(m_hwnd,GWLP_USERDATA, (LONGX86)(LONG_PTR)this);
  m_old_wndproc=(WNDPROC)(LONG_PTR)SetWindowLongPtr(m_hwnd,GWLP_WNDPROC, (LONGX86)(LONG_PTR)wndproc);
}

ComboSkin::~ComboSkin()
{
  SetWindowLongPtr(m_hwnd,GWLP_WNDPROC, (LONGX86)(LONG_PTR)m_old_wndproc);
}
