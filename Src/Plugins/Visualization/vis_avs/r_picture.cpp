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
#include <commctrl.h>
#include "resource.h"
#include "r_defs.h"
#include "../Agave/Language/api_language.h"


#ifndef LASER


#define MOD_NAME "Render / Picture"
#define C_THISCLASS C_PictureClass

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_RENDER_PICTURE,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
		void loadPicture(char *name);
		void freePicture();

    int enabled;
		int width,height;
    HBITMAP hOldBitmap; 
		HBITMAP hb;
		HBITMAP hb2;
    HDC hBitmapDC; 
    HDC hBitmapDC2; 
		int lastWidth,lastHeight;
		int blend, blendavg, adapt, persist;
		int ratio,axis_ratio;
		char ascName[MAX_PATH];
  	int persistCount;
};

static C_THISCLASS *g_ConfigThis; // global configuration dialog pointer 
static HINSTANCE g_hDllInstance; // global DLL instance pointer (not needed in this example, but could be useful)

// configuration read/write

C_THISCLASS::C_THISCLASS() // set up default configuration
{
  persistCount=0;
  enabled=1;
	blend=0;
  adapt=0;
  blendavg=1;
  persist=6;
	strcpy(ascName,"");
	hb=0; ratio=0; axis_ratio=0;
	hBitmapDC=0; hBitmapDC2=0;
}
C_THISCLASS::~C_THISCLASS()
{
	freePicture();
}

void C_THISCLASS::loadPicture(char *name)
{
	freePicture();

	char longName[MAX_PATH];
	wsprintf(longName,"%s\\%s",g_path,name);
	hb=(HBITMAP)LoadImage(0,longName,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
  
	BITMAP bm;
	GetObject(hb, sizeof(bm), (LPSTR)&bm);
  width=bm.bmWidth;
  height=bm.bmHeight;

	lastWidth=lastHeight=0;
}
void C_THISCLASS::freePicture()
{
	if(hb)
		{
		DeleteObject(hb);
		hb=0;
		}
}

#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	int pos=0;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blendavg=GET_INT(); pos+=4; }
	if (len-pos >= 4) { adapt=GET_INT(); pos+=4; }
	if (len-pos >= 4) { persist=GET_INT(); pos+=4; }

	char *p=ascName;
	while (data[pos] && len-pos > 0) *p++=data[pos++];
	*p=0; pos++;

	if (len-pos >= 4) { ratio=GET_INT(); pos+=4; }
	if (len-pos >= 4) { axis_ratio=GET_INT(); pos+=4; }

	if (*ascName)
		loadPicture(ascName);
}

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
int  C_THISCLASS::save_config(unsigned char *data) // write configuration to data, return length. config data should not exceed 64k.
{
	int pos=0;
	PUT_INT(enabled); pos+=4;
  PUT_INT(blend); pos+=4;
  PUT_INT(blendavg); pos+=4;
  PUT_INT(adapt); pos+=4;
  PUT_INT(persist); pos+=4;
  strcpy((char *)data+pos, ascName);
  pos+=strlen(ascName)+1;
  PUT_INT(ratio); pos+=4;
  PUT_INT(axis_ratio); pos+=4;
	return pos;
}

// render function
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout. this is
// used when you want to do something that you'd otherwise need to make a copy of the framebuffer.
// w and h are the width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{

  if (!enabled) return 0;

  if (!width || !height) return 0;


	if (lastWidth != w || lastHeight != h) 
  {
		lastWidth = w;
		lastHeight = h;

		if(hBitmapDC2) {
			DeleteDC(hBitmapDC2);
			DeleteObject(hb2);
			hBitmapDC2=0;
		}

		// Copy the bitmap from hBitmapDC to hBitmapDC2 and stretch it
	  hBitmapDC = CreateCompatibleDC (NULL); 
	  hOldBitmap = (HBITMAP) SelectObject (hBitmapDC, hb); 
		hBitmapDC2 = CreateCompatibleDC (NULL);
		hb2=CreateCompatibleBitmap (hBitmapDC, w, h);
		SelectObject(hBitmapDC2,hb2);
		{
			HBRUSH b=CreateSolidBrush(0);
			HPEN p=CreatePen(PS_SOLID,0,0);
			HBRUSH bold;
			HPEN pold;
			bold=(HBRUSH)SelectObject(hBitmapDC2,b);
			pold=(HPEN)SelectObject(hBitmapDC2,p);
			Rectangle(hBitmapDC2,0,0,w,h);
			SelectObject(hBitmapDC2,bold);
			SelectObject(hBitmapDC2,pold);
			DeleteObject(b);
			DeleteObject(p);
		}
		SetStretchBltMode(hBitmapDC2,COLORONCOLOR);
		int final_height=h,start_height=0;
		int final_width=w,start_width=0;
		if (ratio)
    {
			if(axis_ratio==0) {
				// ratio on X axis
				final_height=height*w/width;
				start_height=(h/2)-(final_height/2);
			} else {
				// ratio on Y axis
				final_width=width*h/height;
				start_width=(w/2)-(final_width/2);
			}
    }
		StretchBlt(hBitmapDC2,start_width,start_height,final_width,final_height,hBitmapDC,0,0,width,height,SRCCOPY);
		DeleteDC(hBitmapDC);
		hBitmapDC=0;
	}
  if (isBeat&0x80000000) return 0;

	// Copy the stretched bitmap to fbout
	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = w;
  bi.bmiHeader.biHeight = h;
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 32;
  bi.bmiHeader.biCompression = BI_RGB;
  bi.bmiHeader.biSizeImage = 0;
  bi.bmiHeader.biXPelsPerMeter = 0;
  bi.bmiHeader.biYPelsPerMeter = 0;
  bi.bmiHeader.biClrUsed = 0;
  bi.bmiHeader.biClrImportant = 0;
	GetDIBits(hBitmapDC2, hb2, 0, h, (void *)fbout, &bi, DIB_RGB_COLORS);

	// Copy the bitmap from fbout to framebuffer applying replace/blend/etc...
	if (isBeat)
		persistCount=persist;
  else
    if (persistCount>0) persistCount--;

	int *p,*d;
	int i,j;

  p = fbout;
  d = framebuffer+w*(h-1);
  if (blend || (adapt && (isBeat || persistCount)))
   for (i=0;i<h;i++)
	{
	for (j=0;j<w;j++)
		{
		*d=BLEND(*p, *d);
		d++;
		p++;
		}
	d -= w*2;
	}
  else
  if (blendavg || adapt)
   for (i=0;i<h;i++)
	{
	for (j=0;j<w;j++)
		{
		*d=BLEND_AVG(*p, *d);
		d++;
		p++;
		}
	d -= w*2;
	}
  else
   for (i=0;i<h;i++)
	{
	memcpy(d, p, w*4);
	p+=w;
	d-=w;
	}

  return 0;
}


// configuration dialog stuff

static void EnableWindows(HWND hwndDlg)
{
	EnableWindow(GetDlgItem(hwndDlg,IDC_PERSIST),g_ConfigThis->adapt);
	EnableWindow(GetDlgItem(hwndDlg,IDC_PERSIST_TITLE),g_ConfigThis->adapt);
	EnableWindow(GetDlgItem(hwndDlg,IDC_PERSIST_TEXT1),g_ConfigThis->adapt);
	EnableWindow(GetDlgItem(hwndDlg,IDC_PERSIST_TEXT2),g_ConfigThis->adapt);
	EnableWindow(GetDlgItem(hwndDlg,IDC_X_RATIO),g_ConfigThis->ratio);
	EnableWindow(GetDlgItem(hwndDlg,IDC_Y_RATIO),g_ConfigThis->ratio);
}

static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg, IDC_PERSIST, TBM_SETRANGE, TRUE, MAKELONG(0, 32));
			SendDlgItemMessage(hwndDlg, IDC_PERSIST, TBM_SETPOS, TRUE, g_ConfigThis->persist);
      if (g_ConfigThis->enabled) CheckDlgButton(hwndDlg,IDC_ENABLED,BST_CHECKED);
      if (g_ConfigThis->blend) CheckDlgButton(hwndDlg,IDC_ADDITIVE,BST_CHECKED);
      if (g_ConfigThis->blendavg) CheckDlgButton(hwndDlg,IDC_5050,BST_CHECKED);
      if (g_ConfigThis->adapt) CheckDlgButton(hwndDlg,IDC_ADAPT,BST_CHECKED);
      if (!g_ConfigThis->adapt && !g_ConfigThis->blend && !g_ConfigThis->blendavg)
			CheckDlgButton(hwndDlg,IDC_REPLACE,BST_CHECKED);
      if (g_ConfigThis->ratio) CheckDlgButton(hwndDlg,IDC_RATIO,BST_CHECKED);
      if (!g_ConfigThis->axis_ratio) CheckDlgButton(hwndDlg,IDC_X_RATIO,BST_CHECKED);
      if (g_ConfigThis->axis_ratio) CheckDlgButton(hwndDlg,IDC_Y_RATIO,BST_CHECKED);
			EnableWindows(hwndDlg);
	    loadComboBox(GetDlgItem(hwndDlg,OBJ_COMBO),"*.BMP",g_ConfigThis->ascName);
			return 1;
		case WM_NOTIFY:
			if (LOWORD(wParam) == IDC_PERSIST)
				g_ConfigThis->persist = SendDlgItemMessage(hwndDlg, IDC_PERSIST, TBM_GETPOS, 0, 0);
			return 0;
    case WM_COMMAND:
		  if ((LOWORD(wParam) == IDC_ENABLED) ||
	     (LOWORD(wParam) == IDC_ADDITIVE) ||
	     (LOWORD(wParam) == IDC_REPLACE) ||
	     (LOWORD(wParam) == IDC_ADAPT) ||
			 (LOWORD(wParam) == IDC_5050) ) {
				g_ConfigThis->enabled=IsDlgButtonChecked(hwndDlg,IDC_ENABLED)?1:0;
				g_ConfigThis->blend=IsDlgButtonChecked(hwndDlg,IDC_ADDITIVE)?1:0;
				g_ConfigThis->blendavg=IsDlgButtonChecked(hwndDlg,IDC_5050)?1:0;
				g_ConfigThis->adapt=IsDlgButtonChecked(hwndDlg,IDC_ADAPT)?1:0;
				EnableWindows(hwndDlg);
			}
			if(LOWORD(wParam) == IDC_RATIO || LOWORD(wParam) == IDC_X_RATIO || LOWORD(wParam) == IDC_Y_RATIO) {
				g_ConfigThis->ratio=IsDlgButtonChecked(hwndDlg,IDC_RATIO)?1:0;
				g_ConfigThis->axis_ratio=IsDlgButtonChecked(hwndDlg,IDC_Y_RATIO)?1:0;
				g_ConfigThis->lastWidth=-1; g_ConfigThis->lastHeight=-1;
				EnableWindows(hwndDlg);
			}
		  if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == OBJ_COMBO) // handle clicks to combo box
			  {
				int sel = SendDlgItemMessage(hwndDlg, OBJ_COMBO, CB_GETCURSEL, 0, 0);
				if (sel != -1)
					{
					SendDlgItemMessage(hwndDlg, OBJ_COMBO, CB_GETLBTEXT, sel, (LPARAM)g_ConfigThis->ascName);
					if (*(g_ConfigThis->ascName))
						g_ConfigThis->loadPicture(g_ConfigThis->ascName);
					}
				}
			return 0;
		case WM_HSCROLL:
			return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_ConfigThis = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_PICTURE,hwndParent,g_DlgProc);
}



// export stuff

C_RBASE *R_Picture(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}

#else
C_RBASE *R_Picture(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
return NULL;
}
#endif