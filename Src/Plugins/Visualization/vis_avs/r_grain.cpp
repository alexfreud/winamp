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

#define MOD_NAME "Trans / Grain"
#define C_THISCLASS C_GrainClass

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_GRAIN,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
    unsigned char __inline fastrandbyte(void);
		void reinit(int w, int h);
    int enabled;
	int blend, blendavg, smax;
	unsigned char *depthBuffer;
	int oldx, oldy;
	int staticgrain;
  unsigned char randtab[491];
  int randtab_pos;
};


static C_THISCLASS *g_ConfigThis; // global configuration dialog pointer 
static HINSTANCE g_hDllInstance; // global DLL instance pointer (not needed in this example, but could be useful)


C_THISCLASS::~C_THISCLASS() // set up default configuration
{
if (depthBuffer)
	GlobalFree(depthBuffer);
}

// configuration read/write

C_THISCLASS::C_THISCLASS() // set up default configuration
{
  blend=0;
  blendavg=0;
  smax=100;
  enabled=1;
  oldx=0;
  oldy=0;
  staticgrain=0;
  depthBuffer=NULL;
  int x;
  for (x = 0 ;x < sizeof(randtab); x ++)
    randtab[x]=rand()&255;
  randtab_pos=rand()%sizeof(randtab);
}

unsigned char __inline C_THISCLASS::fastrandbyte(void)
{
  unsigned char r=randtab[randtab_pos];
  randtab_pos++;
  if (!(randtab_pos&15))
  {
    randtab_pos+=rand()%73;
  }
  if (randtab_pos >= 491) randtab_pos-=491;
  return r;
}

#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	int pos=0;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blendavg=GET_INT(); pos+=4; }
	if (len-pos >= 4) { smax=GET_INT(); pos+=4; }
	if (len-pos >= 4) { staticgrain=GET_INT(); pos+=4; }
}

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
int  C_THISCLASS::save_config(unsigned char *data) // write configuration to data, return length. config data should not exceed 64k.
{
  int pos=0;
  PUT_INT(enabled); pos+=4;
  PUT_INT(blend); pos+=4;
  PUT_INT(blendavg); pos+=4;
  PUT_INT(smax); pos+=4;
  PUT_INT(staticgrain); pos+=4;
  return pos;
}

void C_THISCLASS::reinit(int w, int h)
{
int x,y;
unsigned char *p;
if (depthBuffer)
	GlobalFree(depthBuffer);
depthBuffer = (unsigned char *)GlobalAlloc(GMEM_FIXED,w*h*2);
p = depthBuffer;
if (p)
	for (y=0;y<h;y++)
		for (x=0;x<w;x++)
			{
			*p++ = (rand()%255);
			*p++ = (rand()%100);
			}

}

// render function
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout. this is
// used when you want to do something that you'd otherwise need to make a copy of the framebuffer.
// w and h are the width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (isBeat&0x80000000) return 0;
  if (!enabled) return 0;

  int smax_sc = (smax*255)/100;
  int *p;
  unsigned char *q;
  int l=w*h;

  if (w != oldx || h != oldy)
	{
	  reinit(w, h);
	  oldx = w;
	  oldy = h;
	}
  randtab_pos+=rand()%300;
  if (randtab_pos >= 491) randtab_pos-=491;

  p = framebuffer;
  q = depthBuffer;
  if (staticgrain)
  {
    if (blend)
    {
      while (l--)
	    {
		    if (*p)
		    {
			    int c=0;
			    if (q[1] < smax_sc) 
			    {
    		    int s = q[0];
            int r=(((p[0]&0xff0000)*s)>>8);
            if (r > 0xff0000) r=0xff0000;
            c|=r&0xff0000;
            r=(((p[0]&0xff00)*s)>>8);
            if (r > 0xff00) r=0xff00;
            c|=r&0xff00;
            r=(((p[0]&0xff)*s)>>8);
            if (r > 0xff) r=0xff;
            c|=r;
			    }
			    *p = BLEND(*p, c);
		    }
		    p++;
		    q+=2;
	    }
    }
    else if (blendavg)
    {
      while (l--)
	    {
		    if (*p)
		    {
			    int c=0;
			    if (q[1] < smax_sc) 
			    {
    		    int s = q[0];
            int r=(((p[0]&0xff0000)*s)>>8);
            if (r > 0xff0000) r=0xff0000;
            c|=r&0xff0000;
            r=(((p[0]&0xff00)*s)>>8);
            if (r > 0xff00) r=0xff00;
            c|=r&0xff00;
            r=(((p[0]&0xff)*s)>>8);
            if (r > 0xff) r=0xff;
            c|=r;
			    }
			    *p = BLEND_AVG(*p, c);
		    }
		    p++;
		    q+=2;
	    }
    }
    else
    {
      while (l--)
	    {
		    if (*p)
		    {
			    int c=0;
			    if (q[1] < smax_sc) 
			    {
    		    int s = q[0];
            int r=(((p[0]&0xff0000)*s)>>8);
            if (r > 0xff0000) r=0xff0000;
            c|=r&0xff0000;
            r=(((p[0]&0xff00)*s)>>8);
            if (r > 0xff00) r=0xff00;
            c|=r&0xff00;
            r=(((p[0]&0xff)*s)>>8);
            if (r > 0xff) r=0xff;
            c|=r;
			    }
			    *p = c;
		    }
		    p++;
		    q+=2;
	    }
    }
  }
  else
  {
    if (blend)
    {
      while (l--)
	    {
		    if (*p)
		    {
			    int c=0;
			    if (fastrandbyte() < smax_sc) 
			    {
            int s = fastrandbyte();
            int r=(((p[0]&0xff0000)*s)>>8);
            if (r > 0xff0000) r=0xff0000;
            c|=r&0xff0000;
            r=(((p[0]&0xff00)*s)>>8);
            if (r > 0xff00) r=0xff00;
            c|=r&0xff00;
            r=(((p[0]&0xff)*s)>>8);
            if (r > 0xff) r=0xff;
            c|=r;
			    }
			    *p = BLEND(*p, c);
		    }
		    p++;
		    q+=2;
	    }
    }
    else if (blendavg)
    {
      while (l--)
	    {
		    if (*p)
		    {
			    int c=0;
			    if (fastrandbyte() < smax_sc) 
			    {
            int s = fastrandbyte();
            int r=(((p[0]&0xff0000)*s)>>8);
            if (r > 0xff0000) r=0xff0000;
            c|=r&0xff0000;
            r=(((p[0]&0xff00)*s)>>8);
            if (r > 0xff00) r=0xff00;
            c|=r&0xff00;
            r=(((p[0]&0xff)*s)>>8);
            if (r > 0xff) r=0xff;
            c|=r;
			    }
			    *p = BLEND_AVG(*p, c);
		    }
		    p++;
		    q+=2;
	    }
    }
    else
    {
      while (l--)
	    {
		    if (*p)
		    {
			    int c=0;
			    if (fastrandbyte() < smax_sc) 
			    {
            int s = fastrandbyte();
            int r=(((p[0]&0xff0000)*s)>>8);
            if (r > 0xff0000) r=0xff0000;
            c|=r&0xff0000;
            r=(((p[0]&0xff00)*s)>>8);
            if (r > 0xff00) r=0xff00;
            c|=r&0xff00;
            r=(((p[0]&0xff)*s)>>8);
            if (r > 0xff) r=0xff;
            c|=r;
			    }
			    *p = c;
		    }
		    p++;
		    q+=2;
	    }
    }
  }
  return 0;
}


// configuration dialog stuff


static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
switch (uMsg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hwndDlg, IDC_MAX, TBM_SETTICFREQ, 10, 0);
		SendDlgItemMessage(hwndDlg, IDC_MAX, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
		SendDlgItemMessage(hwndDlg, IDC_MAX, TBM_SETPOS, TRUE, g_ConfigThis->smax);
        if (g_ConfigThis->enabled) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
        if (g_ConfigThis->staticgrain) CheckDlgButton(hwndDlg,IDC_STATGRAIN,BST_CHECKED);
        if (g_ConfigThis->blend) CheckDlgButton(hwndDlg,IDC_ADDITIVE,BST_CHECKED);
        if (g_ConfigThis->blendavg) CheckDlgButton(hwndDlg,IDC_5050,BST_CHECKED);
        if (!g_ConfigThis->blend && !g_ConfigThis->blendavg)
		   CheckDlgButton(hwndDlg,IDC_REPLACE,BST_CHECKED);
		return 1;
	case WM_NOTIFY:
		{
		if (LOWORD(wParam) == IDC_MAX)
			g_ConfigThis->smax = SendDlgItemMessage(hwndDlg, IDC_MAX, TBM_GETPOS, 0, 0);
			return 0;
		}
	case WM_COMMAND:
	  if ((LOWORD(wParam) == IDC_CHECK1) ||
	     (LOWORD(wParam) == IDC_STATGRAIN) ||
	     (LOWORD(wParam) == IDC_ADDITIVE) ||
	     (LOWORD(wParam) == IDC_REPLACE) ||
	     (LOWORD(wParam) == IDC_5050) )
			{
			g_ConfigThis->enabled=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
			g_ConfigThis->blend=IsDlgButtonChecked(hwndDlg,IDC_ADDITIVE)?1:0;
			g_ConfigThis->blendavg=IsDlgButtonChecked(hwndDlg,IDC_5050)?1:0;
			g_ConfigThis->staticgrain=IsDlgButtonChecked(hwndDlg,IDC_STATGRAIN)?1:0;
			}
	  return 0;
	}
return 0;
}




HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_ConfigThis = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_GRAIN,hwndParent,g_DlgProc);
}



// export stuff

C_RBASE *R_Grain(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}


#else
C_RBASE *R_Grain(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
  return NULL;
}
#endif