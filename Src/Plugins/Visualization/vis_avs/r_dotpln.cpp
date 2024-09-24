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
#include <math.h>
#include <commctrl.h>
#include "resource.h"
#include "r_defs.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

#define C_THISCLASS C_DotPlaneClass
#define MOD_NAME "Render / Dot Plane"

#define NUM_WIDTH 64


class C_THISCLASS : public C_RBASE {
	protected:
		float r;
		float atable[NUM_WIDTH*NUM_WIDTH];
		float vtable[NUM_WIDTH*NUM_WIDTH];
		int   ctable[NUM_WIDTH*NUM_WIDTH];
		int color_tab[64];
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int, int);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_RENDER_DOT_PLANE,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

		int rotvel,angle;
		int colors[5];

		void initcolortab();
};


#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))

void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { rotvel=GET_INT(); pos+=4; }
	int x;
	for (x = 0; x < 5; x ++)
	{
		if (len-pos >= 4) { colors[x]=GET_INT(); pos+=4; }
	}
	if (len-pos >= 4) { angle=GET_INT(); pos+=4; }
	if (len-pos >= 4) { int rr=GET_INT(); pos+=4; r=rr/32.0f;}
 
  
	initcolortab();
}

int  C_THISCLASS::save_config(unsigned char *data)
{
  int rr;
	int pos=0;
	int x;
	PUT_INT(rotvel); pos+=4;
	for (x = 0; x < 5; x ++)
	{
		PUT_INT(colors[x]); pos+=4;
	}
  PUT_INT(angle); pos+=4;
  rr=(int)(r*32.0f);
  PUT_INT(rr); pos+=4;
	return pos;
}


void C_THISCLASS::initcolortab()
{
	int x,r,g,b,dr,dg,db,t;
	for (t=0; t < 4; t ++)
	{
		r=(colors[t]&255)<<16;
		g=((colors[t]>>8)&255)<<16;
		b=((colors[t]>>16)&255)<<16;
		dr=(((colors[t+1]&255)-(colors[t]&255))<<16)/16;
		dg=((((colors[t+1]>>8)&255)-((colors[t]>>8)&255))<<16)/16;
		db=((((colors[t+1]>>16)&255)-((colors[t]>>16)&255))<<16)/16;
		for (x = 0; x < 16; x ++)
		{
			color_tab[t*16+x]=(r>>16)|((g>>16)<<8)|((b>>16)<<16);
			r+=dr;g+=dg;b+=db;
		}
	}
}

C_THISCLASS::C_THISCLASS()
{
	colors[0]=RGB(24,107,28); // reverse BGR :)
	colors[1]=RGB(35,10,255);
	colors[2]=RGB(116,29,42);
	colors[3]=RGB(217,54,144);
	colors[4]=RGB(255,136,107);
	initcolortab();

	memset(atable,0,sizeof(atable));
	memset(vtable,0,sizeof(vtable));
	memset(ctable,0,sizeof(ctable));

  angle=-20;
	rotvel=16;
}

C_THISCLASS::~C_THISCLASS()
{
}


int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int width, int height)
{
  if (isBeat&0x80000000) return 0;
	float btable[NUM_WIDTH];
	int fo, p;
	float matrix[16],matrix2[16];
	matrixRotate(matrix,2,r);
	matrixRotate(matrix2,1,(float)angle);
	matrixMultiply(matrix,matrix2);
	matrixTranslate(matrix2,0.0f,-20.0f,400.0f);
	matrixMultiply(matrix,matrix2);

	memcpy(btable,&atable[0],sizeof(float)*NUM_WIDTH);
	for (fo = 0; fo < NUM_WIDTH; fo ++)
	{
		float *i, *o, *v, *ov;
		int *c,*oc;
		int t=(NUM_WIDTH-(fo+2))*NUM_WIDTH;
		i = &atable[t];
		o = &atable[t+NUM_WIDTH];
		v = &vtable[t];
		ov = &vtable[t+NUM_WIDTH];
		c = &ctable[t];
		oc = &ctable[t+NUM_WIDTH];
		if (fo == NUM_WIDTH-1) 
		{
			unsigned char *sd = (unsigned char *)&visdata[0][0][0];
			i = btable;
			for (p = 0; p < NUM_WIDTH; p ++)
			{
				register int t;
				t=max(sd[0],sd[1]);
				t=max(t,sd[2]);
				*o = (float)t;
				t>>=2;
				if (t > 63) t=63;
				*oc++=color_tab[t];

				*ov++ = (*o++ - *i++) / 90.0f;
				sd+=3;
			}
		}
		else for (p = 0; p < NUM_WIDTH; p ++)
		{
			*o = *i++ + *v;
			if (*o < 0.0f) *o=0.0f;
			*ov++ = *v++ - 0.15f*(*o++/255.0f);
			*oc++ = *c++;
		
		}
	}

  float adj=width*440.0f/640.0f;
  float adj2=height*440.0f/480.0f;
  if (adj2 < adj) adj=adj2;
	for (fo = 0; fo < NUM_WIDTH; fo ++)
	{
		int f = (r < 90.0 || r > 270.0) ? NUM_WIDTH-fo-1 : fo;
		float dw=350.0f/(float)NUM_WIDTH, w = -(NUM_WIDTH*0.5f)*dw;
		float q = (f - NUM_WIDTH*0.5f)*dw;
		int *ct = &ctable[f*NUM_WIDTH];
		float *at = &atable[f*NUM_WIDTH];
		int da=1;
		if (r < 180.0)
		{
			da=-1;
			dw=-dw;
			w=-w+dw;
			ct += NUM_WIDTH-1;
			at += NUM_WIDTH-1;
		}
		for (p = 0; p < NUM_WIDTH; p ++)
		{
			float x, y, z;
			matrixApply(matrix,w,64.0f-*at,q,&x,&y,&z);
			z = adj / z;
			register int ix = (int) (x * z) + (width/2);
			register int iy = (int) (y * z) + (height/2);
			if (iy >= 0 && iy < height && ix >= 0 && ix < width) 
			{
        BLEND_LINE(framebuffer + iy*width + ix,*ct);
			}
			w+=dw;
			ct+=da;
			at+=da;
		}
	}
	r += rotvel/5.0f;
	if (r >= 360.0f) r -= 360.0f;
	if (r < 0.0f) r += 360.0f;
  return 0;
}

C_RBASE *R_DotPlane(char *desc)
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
		case WM_COMMAND:
      if (LOWORD(wParam) == IDC_BUTTON1)
      {
        g_this->rotvel=0;
   			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,50);
      }
			if (LOWORD(wParam) >= IDC_C1 && LOWORD(wParam) <= IDC_C5) {
				GR_SelectColor(hwndDlg,&g_this->colors[IDC_C5-LOWORD(wParam)]);
				InvalidateRect(GetDlgItem(hwndDlg,LOWORD(wParam)),NULL,FALSE);
				g_this->initcolortab();
			}
		return 0;
		case WM_DRAWITEM:
			{
				DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT *)lParam;
				if (di->CtlID >= IDC_C1 && di->CtlID <= IDC_C5)
				{
					GR_DrawColoredButton(di,g_this->colors[IDC_C5-di->CtlID]);
				}
			}
		return 0;
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,101);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,g_this->rotvel+50);
			SendDlgItemMessage(hwndDlg,IDC_ANGLE,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_ANGLE,TBM_SETRANGEMAX,0,181);
			SendDlgItemMessage(hwndDlg,IDC_ANGLE,TBM_SETPOS,1,g_this->angle+90);
			
			return 1;

		case WM_HSCROLL:
			{
				HWND swnd = (HWND) lParam;
				int t = (int) SendMessage(swnd,TBM_GETPOS,0,0);
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
				{
					g_this->rotvel=t-50;
				}
        if (swnd == GetDlgItem(hwndDlg,IDC_ANGLE))
        {
          g_this->angle=t-90;
        }
			}
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_DOTPLANE,hwndParent,g_DlgProc);
}

#else
C_RBASE *R_DotPlane(char *desc) { return NULL; }

#endif