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
#ifdef LASER
#include <windows.h>
#include <commctrl.h>
#include "../r_defs.h"
#include "../resource.h"

#include <math.h>

#define C_THISCLASS CLASER_MovingConeClass
#define MOD_NAME "Render / Moving Cone"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); // returns 1 if fbout has dest
		virtual char *get_desc() { return MOD_NAME; }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);


		int maxdist[2],size,size2,num_seg;

		int s_pos;
    int beatcnt;

		double c[2];
		double v[2];
		double p[2];

    int mode;

		int colors[16],num_colors;

    int color_pos;
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
	if (len-pos >= 4) { maxdist[0]=maxdist[1]=GET_INT(); pos+=4; }
	if (len-pos >= 4) { size=GET_INT(); pos+=4; }
	if (len-pos >= 4) { size2=GET_INT(); pos+=4; }
	if (len-pos >= 4) { num_seg=GET_INT(); pos+=4; }
	if (len-pos >= 4) { mode=GET_INT(); pos+=4; }
	if (len-pos >= 4) { maxdist[1]=GET_INT(); pos+=4; }
	s_pos=size;
}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0,x=0;
	PUT_INT(num_colors); pos+=4;
  while (x < num_colors) { PUT_INT(colors[x]); x++;  pos+=4; }
	PUT_INT(maxdist[0]); pos+=4;
	PUT_INT(size); pos+=4;
	PUT_INT(size2); pos+=4;
	PUT_INT(num_seg); pos+=4;
	PUT_INT(mode); pos+=4;
	PUT_INT(maxdist[1]); pos+=4;
	return pos;
}


C_THISCLASS::C_THISCLASS()
{
  num_seg=8;
	size=size2=s_pos=8;
	maxdist[0]=maxdist[1]=16;
	c[0]=c[1]=0.0f;
	v[0]=-0.01551;
	v[1]=0.0;

	p[0]=-0.6;
	p[1]=0.3;
  num_colors=1;
  memset(colors,0,sizeof(colors));
  colors[0]=RGB(255,255,255);
  color_pos=0;
  beatcnt=0;
  mode=0;
}

C_THISCLASS::~C_THISCLASS()
{
}




int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h) // returns 1 if fbout has dest
{
  if (isBeat&0x80000000) return 0;
  LineType l;
	int x;
  int current_color;

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

	float xp,yp;

	if (isBeat)
	{
		c[0]=((rand()%33)-16)/48.0f;
		c[1]=((rand()%33)-16)/48.0f;
	}


	{
		if (p[0] >= 0.0000001 || p[0] <= -0.0000001) v[0] -= 0.004*(p[0]-c[0]);
		if (p[1] >= 0.0000001 || p[1] <= -0.0000001) v[1] -= 0.004*(p[1]-c[1]);
	}

	p[0]+=v[0];
	p[1]+=v[1];

	v[0]*=0.991;
	v[1]*=0.991;

	xp=(float) (p[0]*(maxdist[0]/32.0));
	yp=(float) (p[1]*(maxdist[1]/32.0));
	if (isBeat) 
		s_pos=size2;
	int sz=s_pos;
	s_pos=(s_pos+size)/2;
  {
    float dx,dy;
    float lx,ly;
    lx=(float) (cos(0.0)*s_pos/75.0);
    ly=(float) (sin(0.0)*s_pos/75.0);
    for (x = 0; x < num_seg; x ++)
    {
      l.mode=mode;
      l.color=current_color;
      dx=(float) (cos(0.0+(x+1)*3.14159*2.0/num_seg)*s_pos/75.0);
      dy=(float) (sin(0.0+(x+1)*3.14159*2.0/num_seg)*s_pos/75.0);
      if (!mode)
      {
        l.x2=xp+lx;
        l.y2=yp+ly;
      }
      else l.x2=l.y2=0;
      l.x1=xp+dx;
      l.y1=yp+dy;
      lx=dx;
      ly=dy;
      int x=(int) (l.x1*(w/2)+(w/2));
      int y=(int) (l.y1*(h/2)+(h/2));

      g_laser_linelist->AddLine(&l);
    }
  }
  return 0;
}

C_RBASE *RLASER_Cone(char *desc)
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
	        LOGBRUSH lb={BS_SOLID,color,0};
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
      SetDlgItemInt(hwndDlg,IDC_NUMCOL,g_this->num_colors,FALSE);
      SetDlgItemInt(hwndDlg,IDC_EDIT1,g_this->num_seg,FALSE);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,1);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,32);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,g_this->maxdist[0]);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER7,TBM_SETRANGEMIN,0,1);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER7,TBM_SETRANGEMAX,0,32);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER7,TBM_SETPOS,1,g_this->maxdist[1]);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER3,TBM_SETRANGEMIN,0,1);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER3,TBM_SETRANGEMAX,0,128);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER3,TBM_SETPOS,1,g_this->size);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER4,TBM_SETRANGEMIN,0,1);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER4,TBM_SETRANGEMAX,0,128);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER4,TBM_SETPOS,1,g_this->size2);
      if (g_this->mode) CheckDlgButton(hwndDlg,IDC_RADIO1,BST_CHECKED);
      else CheckDlgButton(hwndDlg,IDC_RADIO2,BST_CHECKED);

			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
        case IDC_RADIO1:
        case IDC_RADIO2:
          if (IsDlgButtonChecked(hwndDlg,IDC_RADIO1))
            g_this->mode=1;
          else g_this->mode=0;
        break;
        case IDC_EDIT1:
          {
            int p;
            BOOL tr=FALSE;
            p=GetDlgItemInt(hwndDlg,IDC_EDIT1,&tr,FALSE);
            if (tr)
            {
              g_this->num_seg=p;
            }
          }
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
          }			}
		return 0;
		case WM_HSCROLL:
			{
				HWND swnd = (HWND) lParam;
				int t = (int) SendMessage(swnd,TBM_GETPOS,0,0);
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
				{
					g_this->maxdist[0]=t;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER7))
				{
					g_this->maxdist[1]=t;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER3))
				{
					g_this->s_pos=g_this->size=t;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER4))
				{
					g_this->s_pos=g_this->size2=t;
				}
			}
		return 0;


	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_LASER_CONE,hwndParent,g_DlgProc);
}
#endif