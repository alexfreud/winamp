/*
  LICENSE
  -------
Copyright 2005 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

  * Neither the name of Nullsoft nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
// alphachannel safe 11/21/99
#include <windows.h>
#include <commctrl.h>
#include "r_defs.h"

#include <math.h>
#include "resource.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

#define C_THISCLASS C_DotGridClass
#define MOD_NAME "Render / Dot Grid"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_RENDER_DOT_GRID,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

    int num_colors;
		int colors[16];

    int color_pos;

    int xp;
    int yp;
    int x_move;
    int y_move;
    int spacing;
    int blend;
};


#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))

void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
  int x=0;
	if (len-pos >= 4) { num_colors=GET_INT(); pos+=4; }
	if (num_colors <= 16) while (len-pos >= 4 && x < num_colors) { colors[x++]=GET_INT(); pos+=4; }
  else num_colors=0;
  if (len-pos >= 4) { spacing=GET_INT(); pos+=4; }
  if (len-pos >= 4) { x_move=GET_INT(); pos+=4; }
  if (len-pos >= 4) { y_move=GET_INT(); pos+=4; }
  if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
}

int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0,x=0;
	PUT_INT(num_colors); pos+=4;
  while (x < num_colors) { PUT_INT(colors[x]); x++;  pos+=4; }
  PUT_INT(spacing); pos+=4;
  PUT_INT(x_move); pos+=4;
  PUT_INT(y_move); pos+=4;
  PUT_INT(blend); pos+=4;
	return pos;
}



C_THISCLASS::C_THISCLASS()
{
  num_colors=1;
  memset(colors,0,sizeof(colors));
  colors[0]=RGB(255,255,255);
  color_pos=0;
  xp=0;
  yp=0;
  x_move=128;
  y_move=128;
  spacing=8;
  blend=3;
}

C_THISCLASS::~C_THISCLASS()
{
}
	
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
	int x,y;
  int current_color;

  if (isBeat&0x80000000) return 0;
  if (!num_colors) return 0;
  color_pos++;
  if (color_pos >= num_colors * 64) color_pos=0;

  {
    int p=color_pos/64;
    int r=color_pos&63;
    int c1,c2;
    int r1,r2,r3;
    c1=colors[p];
    if (p+1 < num_colors)
      c2=colors[p+1];
    else c2=colors[0];

    r1=(((c1&255)*(63-r))+((c2&255)*r))/64;
    r2=((((c1>>8)&255)*(63-r))+(((c2>>8)&255)*r))/64;
    r3=((((c1>>16)&255)*(63-r))+(((c2>>16)&255)*r))/64;
    
    current_color=r1|(r2<<8)|(r3<<16);
  }
  if (spacing<2)spacing=2;
  while (yp < 0) yp+=spacing*256;
  while (xp < 0) xp+=spacing*256;

  int sy=(yp>>8)%spacing;
  int sx=(xp>>8)%spacing;
  framebuffer += sy*w;
  for (y = sy; y < h; y += spacing)
  {
    if (blend==1)
      for (x = sx; x < w; x += spacing)
        framebuffer[x]=BLEND(framebuffer[x],current_color);
    else if (blend == 2)
      for (x = sx; x < w; x += spacing)
        framebuffer[x]=BLEND_AVG(framebuffer[x],current_color);
    else if (blend == 3)
      for (x = sx; x < w; x += spacing)
        BLEND_LINE(framebuffer+x,current_color);
    else
      for (x = sx; x < w; x += spacing)
        framebuffer[x]=current_color;
    framebuffer += w*spacing;
  }
  xp+=x_move;
  yp+=y_move;

  return 0;
}

C_RBASE *R_DotGrid(char *desc)
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}


static C_THISCLASS *g_this;

static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	int *a=NULL;
	switch (uMsg)
	{
		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT *)lParam;
			if (di->CtlID == IDC_DEFCOL && g_this->num_colors>0)
			{
      	int x;
        int w=di->rcItem.right-di->rcItem.left;
        int l=0,nl;
        for (x = 0; x < g_this->num_colors; x ++)
        {
          int color=g_this->colors[x];
          nl = (w*(x+1))/g_this->num_colors;
          color = ((color>>16)&0xff)|(color&0xff00)|((color<<16)&0xff0000);

	        HPEN hPen,hOldPen;
	        HBRUSH hBrush,hOldBrush;
	        LOGBRUSH lb={ (COLORREF)BS_SOLID,(COLORREF)color,(COLORREF)0};
	        hPen = (HPEN)CreatePen(PS_SOLID,0,color);
	        hBrush = CreateBrushIndirect(&lb);
	        hOldPen=(HPEN)SelectObject(di->hDC,hPen);
	        hOldBrush=(HBRUSH)SelectObject(di->hDC,hBrush);
	        Rectangle(di->hDC,di->rcItem.left+l,di->rcItem.top,di->rcItem.left+nl,di->rcItem.bottom);
	        SelectObject(di->hDC,hOldPen);
	        SelectObject(di->hDC,hOldBrush);
	        DeleteObject(hBrush);
	        DeleteObject(hPen);
          l=nl;
			  }
      }
	  }
		return 0;
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,33);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,g_this->x_move/32+16);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETRANGEMAX,0,33);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETPOS,1,g_this->y_move/32+16);

      SetDlgItemInt(hwndDlg,IDC_NUMCOL,g_this->num_colors,FALSE);
      SetDlgItemInt(hwndDlg,IDC_EDIT1,g_this->spacing,FALSE);
      if (g_this->blend==1)
        CheckDlgButton(hwndDlg,IDC_RADIO2,BST_CHECKED);
      else if (g_this->blend==2)
        CheckDlgButton(hwndDlg,IDC_RADIO3,BST_CHECKED);
      else if (g_this->blend==3)
        CheckDlgButton(hwndDlg,IDC_RADIO4,BST_CHECKED);
      else
        CheckDlgButton(hwndDlg,IDC_RADIO1,BST_CHECKED);
    return 1;
		case WM_HSCROLL:
			{
				HWND swnd = (HWND) lParam;
				int t = (int) SendMessage(swnd,TBM_GETPOS,0,0);
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
				{
					g_this->x_move=(t-16)*32;
				}
        if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER2))
        {
					g_this->y_move=(t-16)*32;
        }
			}
    return 0;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
        case IDC_BUTTON1:
          g_this->x_move=0;
  			  SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,16);
      return 0;
        case IDC_BUTTON3:
          g_this->y_move=0;
  			  SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETPOS,1,16);
        return 0;
        case IDC_RADIO1:
        case IDC_RADIO2:
        case IDC_RADIO3:
        case IDC_RADIO4:
          if (IsDlgButtonChecked(hwndDlg,IDC_RADIO1)) g_this->blend=0;
          else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO2)) g_this->blend=1;
          else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO3)) g_this->blend=2;
          else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO4)) g_this->blend=3;
        break;
        case IDC_NUMCOL:
          {
            int p;
            BOOL tr=FALSE;
            p=GetDlgItemInt(hwndDlg,IDC_NUMCOL,&tr,FALSE);
            if (tr)
            {
              if (p > 16) p = 16;
              g_this->num_colors=p;
              InvalidateRect(GetDlgItem(hwndDlg,IDC_DEFCOL),NULL,TRUE);
            }
          }
        break;
        case IDC_EDIT1:
          {
            int p;
            BOOL tr=FALSE;
            p=GetDlgItemInt(hwndDlg,IDC_EDIT1,&tr,FALSE);
            if (tr)
            {
              if (p < 2) p = 2;
              g_this->spacing=p;
            }
          }
        break;
        case IDC_DEFCOL:
          {
            int wc=-1,w,h;
            POINT p;
            RECT r;
            GetCursorPos(&p);
            GetWindowRect(GetDlgItem(hwndDlg,IDC_DEFCOL),&r);
            p.x -= r.left;
            p.y -= r.top;
            w=r.right-r.left;
            h=r.bottom-r.top;
            if (p.x >= 0 && p.x < w  && p.y >= 0 && p.y < h)
            {
              wc = (p.x*g_this->num_colors)/w;
            }
            if (wc>=0) 
            {
              GR_SelectColor(hwndDlg,g_this->colors+wc);
              InvalidateRect(GetDlgItem(hwndDlg,IDC_DEFCOL),NULL,TRUE);            
            }
          }
			}

	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_DOTGRID,hwndParent,g_DlgProc);
}

#else
C_RBASE *R_DotGrid(char *desc) { return NULL; }
#endif
