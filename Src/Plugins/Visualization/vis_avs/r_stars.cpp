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
#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include <commctrl.h>
#include "resource.h"
#include "r_defs.h"
#include "../Agave/Language/api_language.h"

#define MOD_NAME "Render / Starfield"

#define C_THISCLASS C_StarField

typedef struct
	{
	int X, Y;
	float Z;
	float Speed;
	int OX, OY;
	} StarFormat;

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		float GET_FLOAT(unsigned char *data, int pos);
		void PUT_FLOAT(float f, unsigned char *data, int pos);
		void InitializeStars(void);
		void CreateStar(int A);
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_RENDER_STARFIELD,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
    int enabled;
    int color;
	int MaxStars,MaxStars_set;
	int Xoff; 
	int Yoff;
	int Zoff;
	float WarpSpeed;
	int blend;
	int blendavg;
	StarFormat Stars[4096];
	int Width, Height;
	int onbeat;
	float spdBeat;
	float incBeat;
	int durFrames;
	float CurrentSpeed;
    int nc;
	};


static C_THISCLASS *g_ConfigThis; // global configuration dialog pointer 
static HINSTANCE g_hDllInstance; // global DLL instance pointer (not needed in this example, but could be useful)


C_THISCLASS::~C_THISCLASS() 
{
}

// configuration read/write

C_THISCLASS::C_THISCLASS() // set up default configuration
{
  nc=0;
  color = 0xFFFFFF;
  enabled=1;
  MaxStars = 350;
  MaxStars_set=350;
  Xoff = 0; 
  Yoff = 0;
  Zoff = 255;
  WarpSpeed = 6;
  CurrentSpeed = 6;
  blend = 0;
  blendavg = 0;
  Width = 0;
  Height = 0;
  onbeat = 0;
  spdBeat = 4;
  durFrames = 15;
  incBeat = 0;
}

#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))

float C_THISCLASS::GET_FLOAT(unsigned char *data, int pos)
{
int a = data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24);
float f = *(float *)&a;
return f;
}

void C_THISCLASS::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	int pos=0;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { color=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blendavg=GET_INT(); pos+=4; }
	if (len-pos >= 4) { WarpSpeed=GET_FLOAT(data, pos); pos+=4; }
	CurrentSpeed = WarpSpeed;
	if (len-pos >= 4) { MaxStars_set=GET_INT(); pos+=4; }
	if (len-pos >= 4) { onbeat=GET_INT(); pos+=4; }
	if (len-pos >= 4) { spdBeat=GET_FLOAT(data, pos); pos+=4; }
	if (len-pos >= 4) { durFrames=GET_INT(); pos+=4; }
}

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
void C_THISCLASS::PUT_FLOAT(float f, unsigned char *data, int pos)
{
int y = *(int *)&f;
data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255;
}

int  C_THISCLASS::save_config(unsigned char *data) // write configuration to data, return length. config data should not exceed 64k.
{
  int pos=0;
  PUT_INT(enabled); pos+=4;
  PUT_INT(color); pos+=4;
  PUT_INT(blend); pos+=4;
  PUT_INT(blendavg); pos+=4;
  PUT_FLOAT(WarpSpeed, data, pos); pos+=4;
  PUT_INT(MaxStars_set); pos+=4;
  PUT_INT(onbeat); pos+=4;
  PUT_FLOAT(spdBeat, data, pos); pos+=4;
  PUT_INT(durFrames); pos+=4;
  return pos;
}

void C_THISCLASS::InitializeStars(void)
{
  int i;
#ifdef LASER
  MaxStars = MaxStars_set/12;
  if (MaxStars < 10) MaxStars=10;
#else
  MaxStars = MulDiv(MaxStars_set,Width*Height,512*384);
#endif
  if (MaxStars > 4095) MaxStars=4095;
  for (i=0;i<MaxStars;i++)
	{
    Stars[i].X=(rand()%Width)-Xoff;
    Stars[i].Y=(rand()%Height)-Yoff;
    Stars[i].Z=(float)(rand()%255);
    Stars[i].Speed = (float)(rand()%9+1)/10;
	}
}

void C_THISCLASS::CreateStar(int A)
{
  Stars[A].X = (rand()%Width)-Xoff;
  Stars[A].Y = (rand()%Height)-Yoff;
  Stars[A].Z = (float)Zoff;
}

static unsigned int __inline BLEND_ADAPT(unsigned int a, unsigned int b, /*float*/int divisor)
{
return ((((a >> 4) & 0x0F0F0F) * (16-divisor) + (((b >> 4) & 0x0F0F0F) * divisor)));
}

// render function
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout. this is
// used when you want to do something that you'd otherwise need to make a copy of the framebuffer.
// w and h are the width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
#ifdef LASER
  w=h=1024;
#endif
  int i=w*h;
  int NX,NY;
  int c;

  if (!enabled) return 0;

  if (onbeat && isBeat)
	{
	CurrentSpeed = spdBeat;
	incBeat = (WarpSpeed - CurrentSpeed) / (float)durFrames;
	nc = durFrames;
	}

  if (Width != w || Height != h)
	{
	  Width = w;
	  Height = h;
	  Xoff = Width/2;
	  Yoff = Height/2;
	  InitializeStars();
	}
  if (isBeat&0x80000000) return 0;

  for (i=0;i<MaxStars;i++)
	{
    if ((int)Stars[i].Z > 0)
		{
		NX = ((Stars[i].X << 7) / (int)Stars[i].Z) + Xoff;
        NY = ((Stars[i].Y << 7) / (int)Stars[i].Z) + Yoff;
		if ((NX > 0) && (NX < w) && (NY > 0) && (NY < h))
			{
			c = (int)((255-(int)Stars[i].Z)*Stars[i].Speed);
			if (color != 0xFFFFFF) c = BLEND_ADAPT((c|(c<<8)|(c<<16)), color, c>>4); else c = (c|(c<<8)|(c<<16));
#ifdef LASER
      LineType l;
      l.color=c;
      l.mode=1;
      l.x1=(float)NX/512.0f - 1.0f;
      l.y1=(float)NY/512.0f - 1.0f;
      g_laser_linelist->AddLine(&l);      
#else
			framebuffer[NY*w+NX] = blend ? BLEND(framebuffer[NY*w+NX], c) : blendavg ? BLEND_AVG(framebuffer[NY*w+NX], c) : c;
#endif
      Stars[i].OX = NX;
			Stars[i].OY = NY;
			Stars[i].Z-=Stars[i].Speed*CurrentSpeed;
			}
		else
			CreateStar(i);
		}
	else
		CreateStar(i);
	}

  if (!nc)
		CurrentSpeed = WarpSpeed;
  else
		{
		CurrentSpeed = max(0, CurrentSpeed + incBeat);
		nc--;
		}

  return 0;
}


// configuration dialog stuff


static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
switch (uMsg)
	{
	case WM_INITDIALOG:
#ifdef LASER
    ShowWindow(GetDlgItem(hwndDlg,IDC_ADDITIVE),SW_HIDE);
    ShowWindow(GetDlgItem(hwndDlg,IDC_5050),SW_HIDE);
    ShowWindow(GetDlgItem(hwndDlg,IDC_REPLACE),SW_HIDE);
#endif
		SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETTICFREQ, 10, 0);
		SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETRANGE, TRUE, MAKELONG(1, 5000));
		SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETPOS, TRUE, (int)(g_ConfigThis->WarpSpeed*100));
		SendDlgItemMessage(hwndDlg, IDC_NUMSTARS, TBM_SETTICFREQ, 100, 0);
		SendDlgItemMessage(hwndDlg, IDC_NUMSTARS, TBM_SETRANGE, TRUE, MAKELONG(100, 4095));
		SendDlgItemMessage(hwndDlg, IDC_NUMSTARS, TBM_SETPOS, TRUE, g_ConfigThis->MaxStars_set);
		SendDlgItemMessage(hwndDlg, IDC_SPDCHG, TBM_SETTICFREQ, 10, 0);
		SendDlgItemMessage(hwndDlg, IDC_SPDCHG, TBM_SETRANGE, TRUE, MAKELONG(1, 5000));
		SendDlgItemMessage(hwndDlg, IDC_SPDCHG, TBM_SETPOS, TRUE, (int)(g_ConfigThis->spdBeat*100));
		SendDlgItemMessage(hwndDlg, IDC_SPDDUR, TBM_SETTICFREQ, 10, 0);
		SendDlgItemMessage(hwndDlg, IDC_SPDDUR, TBM_SETRANGE, TRUE, MAKELONG(1, 100));
		SendDlgItemMessage(hwndDlg, IDC_SPDDUR, TBM_SETPOS, TRUE, (int)(g_ConfigThis->durFrames));
        if (g_ConfigThis->enabled) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
        if (g_ConfigThis->onbeat) CheckDlgButton(hwndDlg,IDC_ONBEAT2,BST_CHECKED);
        if (g_ConfigThis->blend) CheckDlgButton(hwndDlg,IDC_ADDITIVE,BST_CHECKED);
        if (g_ConfigThis->blendavg) CheckDlgButton(hwndDlg,IDC_5050,BST_CHECKED);
        if (!g_ConfigThis->blend && !g_ConfigThis->blendavg)
		   CheckDlgButton(hwndDlg,IDC_REPLACE,BST_CHECKED);
		return 1;
	case WM_NOTIFY:
		{
		if (LOWORD(wParam) == IDC_SPDCHG)
			g_ConfigThis->spdBeat = (float)SendDlgItemMessage(hwndDlg, IDC_SPDCHG, TBM_GETPOS, 0, 0)/100;
		if (LOWORD(wParam) == IDC_SPDDUR)
			g_ConfigThis->durFrames = SendDlgItemMessage(hwndDlg, IDC_SPDDUR, TBM_GETPOS, 0, 0);
		if (LOWORD(wParam) == IDC_SPEED)
			{
			g_ConfigThis->WarpSpeed = (float)SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_GETPOS, 0, 0)/100;
			g_ConfigThis->CurrentSpeed = (float)SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_GETPOS, 0, 0)/100;
			}

		if (LOWORD(wParam) == IDC_NUMSTARS)
			{
#ifndef LASER
  		int a = g_ConfigThis->MaxStars_set;
#endif
			g_ConfigThis->MaxStars_set = SendDlgItemMessage(hwndDlg, IDC_NUMSTARS, TBM_GETPOS, 0, 0);
#ifndef LASER
			if (g_ConfigThis->MaxStars_set > a)
#endif
				if (g_ConfigThis->Width && g_ConfigThis->Height) g_ConfigThis->InitializeStars();
      }
			return 0;
		}
	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT *)lParam;
			if (di->CtlID == IDC_DEFCOL) // paint nifty color button
			{
        int w=di->rcItem.right-di->rcItem.left;
        int _color=g_ConfigThis->color;
          _color = ((_color>>16)&0xff)|(_color&0xff00)|((_color<<16)&0xff0000);

	        HPEN hPen,hOldPen;
	        HBRUSH hBrush,hOldBrush;
	        LOGBRUSH lb={ (COLORREF)BS_SOLID,(COLORREF)_color,(COLORREF)0};
	        hPen = (HPEN)CreatePen(PS_SOLID,0,_color);
	        hBrush = CreateBrushIndirect(&lb);
	        hOldPen=(HPEN)SelectObject(di->hDC,hPen);
	        hOldBrush=(HBRUSH)SelectObject(di->hDC,hBrush);
	        Rectangle(di->hDC,di->rcItem.left,di->rcItem.top,di->rcItem.right,di->rcItem.bottom);
	        SelectObject(di->hDC,hOldPen);
	        SelectObject(di->hDC,hOldBrush);
	        DeleteObject(hBrush);
	        DeleteObject(hPen);
      }
	  }
		return 0;
	case WM_COMMAND:
	  if ((LOWORD(wParam) == IDC_CHECK1) ||
	     (LOWORD(wParam) == IDC_ONBEAT2) ||
	     (LOWORD(wParam) == IDC_ADDITIVE) ||
	     (LOWORD(wParam) == IDC_REPLACE) ||
	     (LOWORD(wParam) == IDC_5050) )
			{
			g_ConfigThis->enabled=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
			g_ConfigThis->onbeat=IsDlgButtonChecked(hwndDlg,IDC_ONBEAT2)?1:0;
			g_ConfigThis->blend=IsDlgButtonChecked(hwndDlg,IDC_ADDITIVE)?1:0;
			g_ConfigThis->blendavg=IsDlgButtonChecked(hwndDlg,IDC_5050)?1:0;
			}
      if (LOWORD(wParam) == IDC_DEFCOL) // handle clicks to nifty color button
      {
      int *a=&(g_ConfigThis->color);
      static COLORREF custcolors[16];
      CHOOSECOLOR cs;
      cs.lStructSize = sizeof(cs);
      cs.hwndOwner = hwndDlg;
      cs.hInstance = 0;
      cs.rgbResult=((*a>>16)&0xff)|(*a&0xff00)|((*a<<16)&0xff0000);
      cs.lpCustColors = custcolors;
      cs.Flags = CC_RGBINIT|CC_FULLOPEN;
      if (ChooseColor(&cs))
	        {
		        *a = ((cs.rgbResult>>16)&0xff)|(cs.rgbResult&0xff00)|((cs.rgbResult<<16)&0xff0000);
	        	g_ConfigThis->color = *a;
			}
      InvalidateRect(GetDlgItem(hwndDlg,IDC_DEFCOL),NULL,TRUE);            
      }
	}
return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_ConfigThis = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_STARFIELD,hwndParent,g_DlgProc);
}



// export stuff

C_RBASE *R_StarField(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}

