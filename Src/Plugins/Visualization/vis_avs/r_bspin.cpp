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
// alphachannel safe (sets alpha to 0 on rendered portions) 11/21/99

#include <windows.h>
#include <math.h>
#include "r_defs.h"

#include "resource.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

#define C_THISCLASS C_BSpinClass
#define MOD_NAME "Render / Bass Spin"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_RENDER_BASS_SPIN,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

		void my_triangle(int *fb, int points[6], int width, int height, int color);
		int enabled;
		int colors[2];
		int mode;


		int last_a;
		int lx[2][2],ly[2][2];
		double r_v[2];
		double v[2];
		double dir[2];
};


#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { colors[0]=GET_INT(); pos+=4; }
	if (len-pos >= 4) { colors[1]=GET_INT(); pos+=4; }
	if (len-pos >= 4) { mode=GET_INT(); pos+=4; }
}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
	PUT_INT(enabled); pos+=4;
	PUT_INT(colors[0]); pos+=4;
	PUT_INT(colors[1]); pos+=4;
	PUT_INT(mode); pos+=4;
	return pos;
}



C_THISCLASS::C_THISCLASS()
{
	last_a=0;
	enabled=3;
	colors[0]=RGB(255,255,255);
	colors[1]=RGB(255,255,255);
	memset(lx,0,sizeof(lx));
	memset(ly,0,sizeof(ly));
	memset(v,0,sizeof(v));
	r_v[0]=3.14159;
	r_v[1]=0.0;
	dir[0]=-1.0;
	dir[1]=1.0;
	mode=1;
}

C_THISCLASS::~C_THISCLASS()
{
}
	
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
	int y,x;
  if (isBeat&0x80000000) return 0;
	for (y = 0; y < 2; y ++)
	{
		if (!(enabled&(1<<y))) continue;
		unsigned char *fa_data=(unsigned char *)visdata[0][y];
		int xp,yp;
		int ss=min(h/2,(w*3)/8);
		double s=(double)ss;
		int c_x = (!y?w/2-ss/2:w/2+ss/2);
		int a=0,d=0;
		int nc=1;
		int oc6 = colors[y];
		for (x = 0; x < 44; x ++)
		{
			d+=fa_data[x];
		}
		
		a=(d*512)/(last_a+30*256);
		
		last_a=d;

		if (a > 255) a =255;
		v[y] = 0.7*(max(a-104,12)/96.0) + 0.3*v[y];
		r_v[y] += 3.14159/6.0 * v[y] * dir[y];

		s *= a*1.0/256.0f;
		yp=(int)(sin(r_v[y])*s);
		xp=(int)(cos(r_v[y])*s);
		if (mode==0)
		{
			if (lx[0][y] || ly[0][y]) line(framebuffer,lx[0][y],ly[0][y],xp+c_x,yp+h/2,w,h,oc6,(g_line_blend_mode&0xff0000)>>16);
			lx[0][y]=xp+c_x;
			ly[0][y]=yp+h/2;
			line(framebuffer,c_x,h/2,c_x+xp,h/2+yp,w,h,oc6,(g_line_blend_mode&0xff0000)>>16);
			if (lx[1][y] || ly[1][y]) line(framebuffer,lx[1][y],ly[1][y],c_x-xp,h/2-yp,w,h,oc6,(g_line_blend_mode&0xff0000)>>16);
			lx[1][y]=c_x-xp;
			ly[1][y]=h/2-yp;
			line(framebuffer,c_x,h/2,c_x-xp,h/2-yp,w,h,oc6,(g_line_blend_mode&0xff0000)>>16);
		}	
		else if (mode==1)
		{
			if (lx[0][y] || ly[0][y])
			{
				int points[6] = { c_x,h/2, lx[0][y], ly[0][y], xp+c_x,yp+h/2 };
				my_triangle(framebuffer,points,w,h,oc6);
			}
			lx[0][y]=xp+c_x;
			ly[0][y]=yp+h/2;
			if (lx[1][y] || ly[1][y])
			{
				int points[6] = { c_x,h/2, lx[1][y], ly[1][y], c_x-xp,h/2-yp };
				my_triangle(framebuffer,points,w,h,oc6);
			}
			lx[1][y]=c_x-xp;
			ly[1][y]=h/2-yp;
		}
	}
  return 0;
}

C_RBASE *R_BSpin(char *desc)
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
				switch (di->CtlID)
				{
					case IDC_LC:
						GR_DrawColoredButton(di,g_this->colors[0]);
					break;
					case IDC_RC:
						GR_DrawColoredButton(di,g_this->colors[1]);
					break;
				}
			}
		return 0;
		case WM_INITDIALOG:
			if (g_this->enabled&1) CheckDlgButton(hwndDlg,IDC_LEFT,BST_CHECKED);
			if (g_this->enabled&2) CheckDlgButton(hwndDlg,IDC_RIGHT,BST_CHECKED);
			if (g_this->mode==0) CheckDlgButton(hwndDlg,IDC_LINES,BST_CHECKED);
			if (g_this->mode==1) CheckDlgButton(hwndDlg,IDC_TRI,BST_CHECKED);

			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_LINES:
					g_this->mode=IsDlgButtonChecked(hwndDlg,IDC_LINES)?0:1;
				return 0;
				case IDC_TRI:
					g_this->mode=IsDlgButtonChecked(hwndDlg,IDC_TRI)?1:0;
				return 0;
				case IDC_LEFT:
					g_this->enabled&=~1;
					g_this->enabled|=IsDlgButtonChecked(hwndDlg,IDC_LEFT)?1:0;
				return 0;
				case IDC_RIGHT:
					g_this->enabled&=~2;
					g_this->enabled|=IsDlgButtonChecked(hwndDlg,IDC_RIGHT)?2:0;
				return 0;
				case IDC_LC:
					if (!a) a=&g_this->colors[0];
				case IDC_RC:
					if (!a) a=&g_this->colors[1];
					GR_SelectColor(hwndDlg,a);
					InvalidateRect(GetDlgItem(hwndDlg,LOWORD(wParam)),NULL,FALSE);
				return 0;


			}

	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_BSPIN,hwndParent,g_DlgProc);
}



#define F16(x) ((x)<<16)

void C_THISCLASS::my_triangle(int *fb, int points[6], int width, int height, int color)
{
	int ymax;
	int p;
	int y;
	int dx1,dx2;
	int x1,x2;
	for (y = 0; y < 2; y ++)
	{
		if (points[1] > points[3])
		{
			p=points[2]; points[2]=points[0]; points[0]=p;
			p=points[3]; points[3]=points[1]; points[1]=p;
		}
		if (points[3] > points[5])
		{
			p=points[4]; points[4]=points[2]; points[2]=p;
			p=points[5]; points[5]=points[3]; points[3]=p;
		}
	}

	x1=x2=F16(points[0]);
	if (points[1] < points[3])
	{
		dx1 = F16(points[2]-points[0])/(points[3]-points[1]);
	} else dx1=0;

	if (points[1] < points[5]) 
		dx2 = F16(points[4]-points[0])/(points[5]-points[1]);
	else dx2=0;

	fb += points[1]*width;
	ymax = min(points[5],height);
	for (y = points[1]; y < ymax; y ++)
	{
		if (y == points[3])
		{
			if (y == points[5]) return;
			x1=F16(points[2]);
			dx1=F16(points[4]-points[2])/(points[5]-points[3]);
		}
		if (y >= 0) {
			int x,xl;
			x=(min(x1,x2)-32768)>>16;
			xl=((max(x1,x2)+32768)>>16)-x;
			if (xl < 0) xl=-xl;
			if (!xl) xl++;
			{
				int *t=fb+x;
				if (x < 0) { t-=x; xl-=x; }
				if (x+xl >= width) xl=width-x;
				if (xl>0) while (xl--) BLEND_LINE(t++,color);
			}
		}
		fb += width;
		x1+=dx1;
		x2+=dx2;
	}

}

#else
C_RBASE *R_BSpin(char *desc) { return NULL; }
#endif