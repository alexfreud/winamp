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
#include <math.h>
#include "r_defs.h"

#include "resource.h"
#include "../Agave/Language/api_language.h"


#define C_THISCLASS C_OscStarClass
#define MOD_NAME "Render / Oscilliscope Star"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_RENDER_OSCILLOSCOPE_STAR,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

		int effect;
    int num_colors;
		int colors[16];
    int size, rot;

    int color_pos;
    double m_r;
};


#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))

void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
  int x=0;
	if (len-pos >= 4) { effect=GET_INT(); pos+=4; }
	if (len-pos >= 4) { num_colors=GET_INT(); pos+=4; }
	if (num_colors <= 16) while (len-pos >= 4 && x < num_colors) { colors[x++]=GET_INT(); pos+=4; }
  else num_colors=1;
  if (len-pos >= 4) { size=GET_INT(); pos+=4; }
  if (len-pos >= 4) { rot=GET_INT(); pos+=4; }
}

int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0,x=0;
	PUT_INT(effect); pos+=4;
	PUT_INT(num_colors); pos+=4;
  while (x < num_colors) { PUT_INT(colors[x]); x++;  pos+=4; }
	PUT_INT(size); pos+=4;
	PUT_INT(rot); pos+=4;
	return pos;
}



C_THISCLASS::C_THISCLASS()
{
	effect=0|(2<<2)|(2<<4);
  num_colors=1;
  memset(colors,0,sizeof(colors));
  colors[0]=RGB(255,255,255);
  color_pos=0;
  m_r=0.0;
  size=8; // of 16
  rot=3; // of 16
}

C_THISCLASS::~C_THISCLASS()
{
}
	
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
	int x;
  int current_color;
  unsigned char *fa_data;
  char center_channel[576];
  int which_ch=(effect>>2)&3;
  int y_pos=(effect>>4);

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

  if (which_ch>=2)
  {
    for (x = 0; x < 576; x ++) center_channel[x]=visdata[1][0][x]/2+visdata[1][1][x]/2;
  }
  if (which_ch < 2) fa_data=(unsigned char *)&visdata[1][which_ch][0];
  else fa_data=(unsigned char *)center_channel;

  {
    double s=size/32.0;
#ifdef LASER
	  double c_x;
	  double is=s;
    if (y_pos == 2) c_x = 0;
    else if (y_pos == 0) c_x=-0.5;
    else c_x=0.5;
#else
	  int c_x;
	  int is=min((int)(h*s),(int)(w*s));
    if (y_pos == 2) c_x = w/2;
    else if (y_pos == 0) c_x=(w/4);
    else c_x=w/2+w/4;
#endif
	  {
		  int q,ii=0;
		  for (q = 0; q < 5; q ++)
		  {
			  double s,c;
			  s=sin(m_r+q*(3.14159*2.0/5.0));
			  c=cos(m_r+q*(3.14159*2.0/5.0));
			  double p=0.0;
#ifdef LASER
			  double lx=c_x;
			  double ly=0.0;
			  int t=6;
#else
        if (y_pos == 2) c_x = w/2;
        else if (y_pos == 0) c_x=(w/4);
			  int lx=c_x;
			  int ly=h/2;
			  int t=64;
#endif
			  double dp=is/(double)t;
			  double dfactor=1.0/1024.0f;
			  double hw=is;
			  while (t--)
			  {
				  double ale=(((fa_data[ii]^128)-128)*dfactor*hw);
#ifdef LASER
				  double x,y;
          ii+=8;
				  x=c_x+(c*p)-(s*ale);
				  y=(s*p)+(c*ale);
				  if ((x > -1.0 && x < 1.0 && y > -1.0 && y < 1.0) ||
              (lx > -1.0 && lx < 1.0 && ly > -.10 && ly < 1.0))
          {
              LineType l;
              l.color=current_color;
              l.mode=0;
              l.x1=(float)x;
              l.x2=(float)lx;
              l.y1=(float)y;
              l.y2=(float)ly;
              g_laser_linelist->AddLine(&l);
          }
#else
          int x,y;
          ii++;
				  x=c_x+(int)(c*p)-(int)(s*ale);
				  y=h/2+(int)(s*p)+(int)(c*ale);
				  if ((x >= 0 && x < w && y >= 0 && y < h) ||
              (lx >= 0 && lx < w && ly >= 0 && ly < h))
          {
              line(framebuffer,x,y,lx,ly,w,h,current_color,(g_line_blend_mode&0xff0000)>>16);
          }
#endif
				  lx=x;
				  ly=y;
				  p+=dp;
				  dfactor -= ((1.0/1024.0f)-(1.0/128.0f))/64.0f;
			  }
		  }

		  m_r+=0.01 * (double)rot;
		  if (m_r >= 3.14159*2)
			  m_r -= 3.14159*2;
	  }
  }
  return 0;
}

C_RBASE *R_OscStars(char *desc)
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
	        LOGBRUSH lb={(COLORREF)BS_SOLID,(COLORREF)color,(COLORREF)0};
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
		case WM_HSCROLL:
			{
				HWND swnd = (HWND) lParam;
				int t = (int) SendMessage(swnd,TBM_GETPOS,0,0);
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
				{
					g_this->size=t;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER2))
				{
					g_this->rot=t-16;
				}
			}
    return 0;
		case WM_INITDIALOG:
      switch ((g_this->effect>>2)&3)
      {
        case 0: CheckDlgButton(hwndDlg,IDC_LEFTCH,BST_CHECKED); break;
        case 1: CheckDlgButton(hwndDlg,IDC_RIGHTCH,BST_CHECKED); break;
        case 2: CheckDlgButton(hwndDlg,IDC_MIDCH,BST_CHECKED); break;
      }
      switch ((g_this->effect>>4)&3)
      {
        case 0: CheckDlgButton(hwndDlg,IDC_TOP,BST_CHECKED); break;
        case 1: CheckDlgButton(hwndDlg,IDC_BOTTOM,BST_CHECKED); break;
        case 2: CheckDlgButton(hwndDlg,IDC_CENTER,BST_CHECKED); break;
      }
      SetDlgItemInt(hwndDlg,IDC_NUMCOL,g_this->num_colors,FALSE);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,32);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,g_this->size);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETRANGEMAX,0,32);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETPOS,1,g_this->rot+16);
    return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_LEFTCH: g_this->effect&=~12; break;
				case IDC_RIGHTCH: g_this->effect&=~12; g_this->effect|=4; break;
				case IDC_MIDCH: g_this->effect&=~12; g_this->effect|=8; break;
				case IDC_TOP: g_this->effect&=~48; break;
				case IDC_BOTTOM: g_this->effect&=~48; g_this->effect|=16; break;
				case IDC_CENTER: g_this->effect&=~48; g_this->effect|=32; break;
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
	return WASABI_API_CREATEDIALOG(IDD_CFG_OSCSTAR,hwndParent,g_DlgProc);
}
