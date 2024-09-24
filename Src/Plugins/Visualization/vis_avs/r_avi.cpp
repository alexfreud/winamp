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
#include <vfw.h>
#include <commctrl.h>
#include "resource.h"
#include "r_defs.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER



#define MOD_NAME "Render / AVI"
#define C_THISCLASS C_AVIClass

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		void reinit(int, int);
		void loadAvi(char *name);
		void closeAvi(void);
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_RENDER_AVI,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
    int enabled;
	char ascName[MAX_PATH];
	int lastWidth, lastHeight;
    HDRAWDIB hDrawDib; 
    PAVISTREAM PAVIVideo; 
    PGETFRAME PgetFrame; 
    HBITMAP hRetBitmap; 
    HBITMAP hOldBitmap; 
    HDC hDesktopDC; 
    HDC hBitmapDC; 
    LPBITMAPINFOHEADER lpFrame; 
	BITMAPINFO bi;
	int blend, blendavg, adapt, persist;
	int loaded, rendering;
	int lFrameIndex;
	int length;
	unsigned int speed;
	unsigned int lastspeed;
	int *old_image,old_image_w,old_image_h;
	};


static C_THISCLASS *g_ConfigThis; // global configuration dialog pointer 
static HINSTANCE g_hDllInstance; // global DLL instance pointer (not needed in this example, but could be useful)

// configuration read/write

C_THISCLASS::C_THISCLASS() // set up default configuration
{
  AVIFileInit ( ) ; 
  hDrawDib = DrawDibOpen ( ) ; 
  lastWidth=0;
  lastHeight=0;
  lFrameIndex=0;
  loaded=0;
  rendering=0;
  length=0;
  blend=0;
  adapt=0;
  blendavg=1;
  persist=6;
  enabled=1;
  speed=0;
  lastspeed=0;
  old_image=NULL; old_image_h=0; old_image_w=0;
}

C_THISCLASS::~C_THISCLASS()
{
   closeAvi();
   SelectObject (hBitmapDC, hOldBitmap); 
   DeleteDC (hBitmapDC); 
   ReleaseDC (NULL, hDesktopDC); 
   AVIFileExit ( ); 
   DrawDibClose (hDrawDib); 
   if(old_image) {
	   GlobalFree(old_image);
	   old_image=NULL;
	   old_image_h=old_image_w=0;
   }
}

void C_THISCLASS::loadAvi(char *name)
{
  char pathfile[MAX_PATH];

  if (loaded) closeAvi();

  wsprintf(pathfile,"%s\\%s",g_path, name);

  if (AVIStreamOpenFromFile ((PAVISTREAM FAR *) &PAVIVideo, pathfile, streamtypeVIDEO, 0, OF_READ | OF_SHARE_EXCLUSIVE, NULL) != 0)
  {
  //	MessageBox(NULL, "An error occured while trying to open a file. Effect is disabled", "Warning", 0);
	    return;
	}
  PgetFrame = AVIStreamGetFrameOpen (PAVIVideo, NULL); 
  length = AVIStreamLength(PAVIVideo);
  lFrameIndex = 0;

  lpFrame = (LPBITMAPINFOHEADER) AVIStreamGetFrame (PgetFrame, 0); 

  reinit(lastWidth, lastHeight);

  loaded=1;
}

void C_THISCLASS::closeAvi(void)
{
if (loaded)
	{
	while (rendering);
    loaded=0;
    AVIStreamGetFrameClose (PgetFrame); 
    AVIStreamRelease (PAVIVideo); 
	}
}

#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	int pos=0;
	char *p=ascName;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blendavg=GET_INT(); pos+=4; }
	while (data[pos] && len-pos > 0 && p-ascName < sizeof(ascName)-1) *p++=data[pos++];
	*p=0; pos++;
	if (len-pos >= 4) { adapt=GET_INT(); pos+=4; }
	if (len-pos >= 4) { persist=GET_INT(); pos+=4; }
	if (len-pos >= 4) { speed=GET_INT(); pos+=4; }

	if (*ascName)
		loadAvi(ascName);
}

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
int  C_THISCLASS::save_config(unsigned char *data) // write configuration to data, return length. config data should not exceed 64k.
{
  int pos=0;
  PUT_INT(enabled); pos+=4;
  PUT_INT(blend); pos+=4;
  PUT_INT(blendavg); pos+=4;
  strcpy((char *)data+pos, ascName);
  pos+=strlen(ascName)+1;
  PUT_INT(adapt); pos+=4;
  PUT_INT(persist); pos+=4;
  PUT_INT(speed); pos+=4;
  return pos;
}


void C_THISCLASS::reinit(int w, int h)
{
if (lastWidth || lastHeight)
   {
   SelectObject (hBitmapDC, hOldBitmap); 
   DeleteDC (hBitmapDC); 
   ReleaseDC (NULL, hDesktopDC); 
   }

   hDesktopDC = GetDC (NULL); 
   hRetBitmap = CreateCompatibleBitmap (hDesktopDC, w, h);
   hBitmapDC = CreateCompatibleDC (hDesktopDC); 
   hOldBitmap = (HBITMAP) SelectObject (hBitmapDC, hRetBitmap); 
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
}

// render function
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout. this is
// used when you want to do something that you'd otherwise need to make a copy of the framebuffer.
// w and h are the width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  int *p,*d;
  int i,j;
  static int persistCount=0;

  if (!enabled || !loaded) return 0;

  if(h!=old_image_h||w!=old_image_w) {
	  if(old_image)
		  GlobalFree(old_image);
	  old_image=(int *)GlobalAlloc(GMEM_FIXED,sizeof(int)*w*h);
	  old_image_h=h; old_image_w=w;
  }

  if((lastspeed+speed)>GetTickCount()) {
	  memcpy(fbout,old_image,sizeof(int)*w*h);
  } else {
	  lastspeed=GetTickCount();

	  rendering=1;

	if (lastWidth != w || lastHeight != h)
		{
		lastWidth = w;
		lastHeight = h;
		reinit(w, h);
		}
  if (isBeat&0x80000000) return 0;

  if (!length) return 0;

	lFrameIndex %= length;
	lpFrame = (LPBITMAPINFOHEADER) AVIStreamGetFrame (PgetFrame, lFrameIndex++); 
	DrawDibDraw (hDrawDib, hBitmapDC, 0, 0, lastWidth, lastHeight, lpFrame,
	NULL, 0, 0, (int) lpFrame ->biWidth, (int) lpFrame ->biHeight, 0); 
	GetDIBits(hBitmapDC, hRetBitmap, 0, h, (void *)fbout, &bi, DIB_RGB_COLORS);
	rendering=0;
	memcpy(old_image,fbout,sizeof(int)*w*h);
  }

  if (isBeat)
	persistCount=persist;
  else
    if (persistCount>0) persistCount--;

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
}

static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
switch (uMsg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hwndDlg, IDC_PERSIST, TBM_SETRANGE, TRUE, MAKELONG(0, 32));
		SendDlgItemMessage(hwndDlg, IDC_PERSIST, TBM_SETPOS, TRUE, g_ConfigThis->persist);
		SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETTICFREQ, 50, 0);
		SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETRANGE, TRUE, MAKELONG(0, 1000));
		SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETPOS, TRUE, g_ConfigThis->speed);
    if (g_ConfigThis->enabled) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
    if (g_ConfigThis->blend) CheckDlgButton(hwndDlg,IDC_ADDITIVE,BST_CHECKED);
    if (g_ConfigThis->blendavg) CheckDlgButton(hwndDlg,IDC_5050,BST_CHECKED);
    if (g_ConfigThis->adapt) CheckDlgButton(hwndDlg,IDC_ADAPT,BST_CHECKED);
    if (!g_ConfigThis->adapt && !g_ConfigThis->blend && !g_ConfigThis->blendavg)
		CheckDlgButton(hwndDlg,IDC_REPLACE,BST_CHECKED);
		EnableWindows(hwndDlg);
	  loadComboBox(GetDlgItem(hwndDlg,OBJ_COMBO),"*.AVI",g_ConfigThis->ascName);
		return 1;
	case WM_NOTIFY:
		{
		if (LOWORD(wParam) == IDC_PERSIST)
			g_ConfigThis->persist = SendDlgItemMessage(hwndDlg, IDC_PERSIST, TBM_GETPOS, 0, 0);
		if (LOWORD(wParam) == IDC_SPEED)
			g_ConfigThis->speed = SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_GETPOS, 0, 0);
		}
	case WM_COMMAND:
	  if ((LOWORD(wParam) == IDC_CHECK1) ||
	     (LOWORD(wParam) == IDC_ADDITIVE) ||
	     (LOWORD(wParam) == IDC_REPLACE) ||
	     (LOWORD(wParam) == IDC_ADAPT) ||
	     (LOWORD(wParam) == IDC_5050) )
			{
			g_ConfigThis->enabled=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
			g_ConfigThis->blend=IsDlgButtonChecked(hwndDlg,IDC_ADDITIVE)?1:0;
			g_ConfigThis->blendavg=IsDlgButtonChecked(hwndDlg,IDC_5050)?1:0;
			g_ConfigThis->adapt=IsDlgButtonChecked(hwndDlg,IDC_ADAPT)?1:0;
			EnableWindows(hwndDlg);
			}
	  if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == OBJ_COMBO) // handle clicks to combo box
		  {
		  int sel = SendDlgItemMessage(hwndDlg, OBJ_COMBO, CB_GETCURSEL, 0, 0);
		  if (sel != -1)
				{
				SendDlgItemMessage(hwndDlg, OBJ_COMBO, CB_GETLBTEXT, sel, (LPARAM)g_ConfigThis->ascName);
				if (*(g_ConfigThis->ascName))
					g_ConfigThis->loadAvi(g_ConfigThis->ascName);
				}
		  }
	  return 0;
	}
return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_ConfigThis = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_AVI,hwndParent,g_DlgProc);
}
// export stuff

C_RBASE *R_AVI(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}

#else//!LASER
C_RBASE *R_AVI(char *desc) { return NULL; }
#endif
