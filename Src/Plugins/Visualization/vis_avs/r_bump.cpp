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
#include <stdio.h>
#include "resource.h"
#include "r_defs.h"
#include "r_stack.h"
#include "avs_eelif.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

#define MOD_NAME "Trans / Bump"
#define C_THISCLASS C_BumpClass

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		float GET_FLOAT(unsigned char *data, int pos);
		void PUT_FLOAT(float f, unsigned char *data, int pos);
		void InitializeStars(int Start);
		void CreateStar(int A);
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_BUMP,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
		int __inline depthof(int c, int i);
    int enabled;
	int depth;
	int depth2;
	int onbeat;
	int durFrames;
	int thisDepth;
	int blend;
	int blendavg;
	int nF;
	NSEEL_CODEHANDLE codeHandle;
	NSEEL_CODEHANDLE codeHandleBeat;
	NSEEL_CODEHANDLE codeHandleInit;
	double *var_x;
	double *var_y;
	double *var_isBeat;
	double *var_isLongBeat;
	double *var_bi;
	RString code1,code2,code3;
	int need_recompile;
	int showlight;
  int initted;
	int invert;
  NSEEL_VMCTX AVS_EEL_CONTEXTNAME;
	int oldstyle;
	int buffern;
    CRITICAL_SECTION rcs;
	};


static C_THISCLASS *g_ConfigThis; // global configuration dialog pointer 
static HINSTANCE g_hDllInstance; // global DLL instance pointer (not needed in this example, but could be useful)


C_THISCLASS::~C_THISCLASS() 
{
	freeCode(codeHandle);
	freeCode(codeHandleBeat);
	freeCode(codeHandleInit);
  DeleteCriticalSection(&rcs);
  AVS_EEL_QUITINST();
}

// configuration read/write

C_THISCLASS::C_THISCLASS() // set up default configuration
{
  AVS_EEL_INITINST();
  InitializeCriticalSection(&rcs);
	buffern=0;
	oldstyle=0;
	invert=0;
  enabled=1;
  onbeat = 0;
  durFrames = 15;
  depth=30;
  depth2=100;
  nF=0;
  showlight=0;
  thisDepth=depth;
  blend = 0;
  blendavg = 0;
  code1.assign("x=0.5+cos(t)*0.3;\r\ny=0.5+sin(t)*0.3;\r\nt=t+0.1;");
  code2.assign(""); 
  code3.assign("t=0;");
  codeHandle=0;
  codeHandleBeat=0;
  codeHandleInit=0;
  var_bi=0;
  initted=0;
  
  need_recompile=1;
}

#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))


void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { onbeat=GET_INT(); pos+=4; }
	if (len-pos >= 4) { durFrames=GET_INT(); pos+=4; }
	if (len-pos >= 4) { depth=GET_INT(); pos+=4; }
	if (len-pos >= 4) { depth2=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blendavg=GET_INT(); pos+=4; }
	load_string(code1,data,pos,len);
	load_string(code2,data,pos,len);
	load_string(code3,data,pos,len);
	if (len-pos >= 4) { showlight=GET_INT(); pos+=4; }
	if (len-pos >= 4) { invert=GET_INT(); pos+=4; }
	if (len-pos >= 4) { oldstyle=GET_INT(); pos+=4; } else oldstyle=1;
	if (len-pos >= 4) { buffern=GET_INT(); pos+=4; }
	thisDepth=depth;
	nF=0;

	need_recompile=1;
}

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255


int  C_THISCLASS::save_config(unsigned char *data) // write configuration to data, return length. config data should not exceed 64k.
{
  int pos=0;
  PUT_INT(enabled); pos+=4;
  PUT_INT(onbeat); pos+=4;
  PUT_INT(durFrames); pos+=4;
  PUT_INT(depth); pos+=4;
  PUT_INT(depth2); pos+=4;
  PUT_INT(blend); pos+=4;
  PUT_INT(blendavg); pos+=4;
  save_string(data, pos, code1);
  save_string(data, pos, code2);
  save_string(data, pos, code3);
  PUT_INT(showlight); pos+=4;
  PUT_INT(invert); pos+=4;
  PUT_INT(oldstyle); pos+=4;
  PUT_INT(buffern); pos+=4;
  return pos;
}

int __inline C_THISCLASS::depthof(int c, int i)
{
int r= max(max((c & 0xFF), ((c & 0xFF00)>>8)), (c & 0xFF0000)>>16);
return i ? 255 - r : r;
}

static int __inline setdepth(int l, int c)
{
int r;
r=min((c&0xFF)+l, 254);
r|=min(((c&0xFF00))+(l<<8),254<<8);
r|=min(((c&0xFF0000))+(l<<16),254<<16);
return r;
}

static int __inline setdepth0(int c)
{
int r;
r=min((c&0xFF), 254);
r|=min(((c&0xFF00)),254<<8);
r|=min(((c&0xFF0000)),254<<16);
return r;
}

// render function
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout. this is
// used when you want to do something that you'd otherwise need to make a copy of the framebuffer.
// w and h are the width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].
#define abs(x) (( x ) >= 0 ? ( x ) : - ( x ))

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  int cx,cy;
	int curbuf;

	if (!enabled) return 0;

  if (need_recompile) 
  {
    EnterCriticalSection(&rcs);
    if (!var_bi || g_reset_vars_on_recompile)
    {
      clearVars();
      var_x = registerVar("x");
      var_y = registerVar("y");
      var_isBeat = registerVar("isbeat");
      var_isLongBeat = registerVar("islbeat");
      var_bi = registerVar("bi");
      *var_bi = 1.0;
      initted=0;
    }

    need_recompile=0;

    freeCode(codeHandle);
    freeCode(codeHandleBeat);
    freeCode(codeHandleInit);
    codeHandle = compileCode(code1.get());
    codeHandleBeat = compileCode(code2.get());
    codeHandleInit = compileCode(code3.get());

    LeaveCriticalSection(&rcs);
  }
  if (isBeat&0x80000000) return 0;

	int *depthbuffer = !buffern ? framebuffer : (int *)getGlobalBuffer(w,h,buffern-1,0);
	if (!depthbuffer) return 0;

	curbuf = (depthbuffer==framebuffer);

	if (!initted)
  {
		executeCode(codeHandleInit,visdata);
    initted=1;
	}

	executeCode(codeHandle,visdata);
	if (isBeat)	executeCode(codeHandleBeat,visdata);

	if (isBeat)
		*var_isBeat=-1;
	else
		*var_isBeat=1;
	if (nF)
		*var_isLongBeat=-1;
	else
		*var_isLongBeat=1;

    if (onbeat && isBeat)
	{
	thisDepth=depth2;
	nF = durFrames;
	}
    else if (!nF) thisDepth = depth;

	memset(fbout, 0, w*h*4); // previous effects may have left fbout in a mess

	if (oldstyle)
		{
		cx = (int)(*var_x/100.0*w);
		cy = (int)(*var_y/100.0*h);
		}
	else
		{
		cx = (int)(*var_x*w);
		cy = (int)(*var_y*h);
		}
	cx = max(0, min(w, cx));
	cy = max(0, min(h, cy));
	if (showlight) fbout[cx+cy*w]=0xFFFFFF;

	if (var_bi) 
		{
		*var_bi = min(max(*var_bi, 0), 1);
		thisDepth = (int)(thisDepth * *var_bi);
		}

  int thisDepth_scaled=(thisDepth<<8)/100;
	depthbuffer += w+1;
	framebuffer += w+1;
	fbout += w+1;

	int ly=1-cy;
  int i=h-2;
  while (i--)
	{
    int j=w-2;
		int lx=1-cx;
    if (blend)
    {
      while (j--)
		  {
        int m1,p1,mw,pw;
        m1=depthbuffer[-1];
        p1=depthbuffer[1];
        mw=depthbuffer[-w];
        pw=depthbuffer[w];
			  if (!curbuf || (curbuf && (m1||p1||mw||pw)))
			  {
          int coul1,coul2;
				  coul1=depthof(p1, invert)-depthof(m1, invert)-lx;
				  coul2=depthof(pw, invert)-depthof(mw, invert)-ly;
				  coul1=127-abs(coul1);
				  coul2=127-abs(coul2);
				  if (coul1<=0||coul2<=0)
            coul1=setdepth0(framebuffer[0]);
          else 
            coul1=setdepth((coul1*coul2*thisDepth_scaled)>>(8+6), framebuffer[0]);
				  fbout[0]=BLEND(framebuffer[0], coul1);
			  }
			  depthbuffer++;
			  framebuffer++;
        fbout++;
			  lx++;
		  }
    }
    else if (blendavg)
    {
      while (j--)
		  {
        int m1,p1,mw,pw;
        m1=depthbuffer[-1];
        p1=depthbuffer[1];
        mw=depthbuffer[-w];
        pw=depthbuffer[w];
			  if (!curbuf || (curbuf && (m1||p1||mw||pw)))
			  {
          int coul1,coul2;
				  coul1=depthof(p1, invert)-depthof(m1, invert)-lx;
				  coul2=depthof(pw, invert)-depthof(mw, invert)-ly;
				  coul1=127-abs(coul1);
				  coul2=127-abs(coul2);
				  if (coul1<=0||coul2<=0)
            coul1=setdepth0(framebuffer[0]);
          else 
            coul1=setdepth((coul1*coul2*thisDepth_scaled)>>(8+6), framebuffer[0]);
				  fbout[0]=BLEND_AVG(framebuffer[0], coul1);
			  }
			  depthbuffer++;
			  framebuffer++;
        fbout++;
			  lx++;
		  }
    }
    else
    {
      while (j--)
		  {
        int m1,p1,mw,pw;
        m1=depthbuffer[-1];
        p1=depthbuffer[1];
        mw=depthbuffer[-w];
        pw=depthbuffer[w];
			  if (!curbuf || (curbuf && (m1||p1||mw||pw)))
			  {
          int coul1,coul2;
				  coul1=depthof(p1, invert)-depthof(m1, invert)-lx;
				  coul2=depthof(pw, invert)-depthof(mw, invert)-ly;
				  coul1=127-abs(coul1);
				  coul2=127-abs(coul2);
				  if (coul1<=0||coul2<=0)
            coul1=setdepth0(framebuffer[0]);
          else 
            coul1=setdepth((coul1*coul2*thisDepth_scaled)>>(8+6), framebuffer[0]);
				  fbout[0]=coul1;
			  }
			  depthbuffer++;
			  framebuffer++;
        fbout++;
			  lx++;
		  }
    }
		depthbuffer+=2;
		framebuffer+=2;
    fbout+=2;
		ly++;
	}

  if (nF)
	{
	  nF--;
	  if (nF)
		{
		  int a = abs(depth - depth2) / durFrames;
		  thisDepth += a * (depth2 > depth ? -1 : 1);
		}
	}

 return 1;
}

// configuration dialog stuff
static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
switch (uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg, IDC_CODE1, g_ConfigThis->code1.get());
		SetDlgItemText(hwndDlg, IDC_CODE2, g_ConfigThis->code2.get());
		SetDlgItemText(hwndDlg, IDC_CODE3, g_ConfigThis->code3.get());
		SendDlgItemMessage(hwndDlg, IDC_DEPTH, TBM_SETTICFREQ, 10, 0);
		SendDlgItemMessage(hwndDlg, IDC_DEPTH, TBM_SETRANGE, TRUE, MAKELONG(1, 100));
		SendDlgItemMessage(hwndDlg, IDC_DEPTH, TBM_SETPOS, TRUE, g_ConfigThis->depth);
		SendDlgItemMessage(hwndDlg, IDC_DEPTH2, TBM_SETTICFREQ, 10, 0);
		SendDlgItemMessage(hwndDlg, IDC_DEPTH2, TBM_SETRANGE, TRUE, MAKELONG(1, 100));
		SendDlgItemMessage(hwndDlg, IDC_DEPTH2, TBM_SETPOS, TRUE, g_ConfigThis->depth2);
		SendDlgItemMessage(hwndDlg, IDC_BEATDUR, TBM_SETTICFREQ, 10, 0);
		SendDlgItemMessage(hwndDlg, IDC_BEATDUR, TBM_SETRANGE, TRUE, MAKELONG(1, 100));
		SendDlgItemMessage(hwndDlg, IDC_BEATDUR, TBM_SETPOS, TRUE, g_ConfigThis->durFrames);
        if (g_ConfigThis->enabled) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
        if (g_ConfigThis->invert) CheckDlgButton(hwndDlg,IDC_INVERTDEPTH,BST_CHECKED);
        if (g_ConfigThis->onbeat) CheckDlgButton(hwndDlg,IDC_ONBEAT,BST_CHECKED);
        if (g_ConfigThis->blend) CheckDlgButton(hwndDlg,IDC_ADDITIVE,BST_CHECKED);
        if (g_ConfigThis->blendavg) CheckDlgButton(hwndDlg,IDC_5050,BST_CHECKED);
        if (g_ConfigThis->showlight) CheckDlgButton(hwndDlg,IDC_DOT,BST_CHECKED);
        if (!g_ConfigThis->blend && !g_ConfigThis->blendavg)
		    CheckDlgButton(hwndDlg,IDC_REPLACE,BST_CHECKED);
				SendDlgItemMessage(hwndDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRING(IDS_CURRENT));
				{
				int i=0;
				char txt[64];
				for (i=0;i<NBUF;i++)
					 {
					 wsprintf(txt, WASABI_API_LNGSTRING(IDS_BUFFER_X), i+1);
					 SendDlgItemMessage(hwndDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)txt);
					 }
				}
		SendDlgItemMessage(hwndDlg, IDC_COMBO1, CB_SETCURSEL, (WPARAM) g_ConfigThis->buffern, 0);
		return 1;
	case WM_NOTIFY:
		{
		if (LOWORD(wParam) == IDC_DEPTH)
			g_ConfigThis->depth = SendDlgItemMessage(hwndDlg, IDC_DEPTH, TBM_GETPOS, 0, 0);
		if (LOWORD(wParam) == IDC_DEPTH2)
			g_ConfigThis->depth2 = SendDlgItemMessage(hwndDlg, IDC_DEPTH2, TBM_GETPOS, 0, 0);
		if (LOWORD(wParam) == IDC_BEATDUR)
			g_ConfigThis->durFrames = SendDlgItemMessage(hwndDlg, IDC_BEATDUR, TBM_GETPOS, 0, 0);
		}
	return 0;
	case WM_COMMAND:
	  if (LOWORD(wParam) == IDC_HELPBTN)
		{
/*		char text[4096];
		WASABI_API_LNGSTRING_BUF(IDS_BUMP_LIGHT_POSITION,text,4096);
		int titlelen = lstrlen(text)+1;
		lstrcpyn(text+titlelen,GetTextResource(IDR_BUMP_LIGHT_POSITION),4095-titlelen);
*/
        char *text="Bump Light Position\0"
											"How to use the custom light position evaluator:\r\n"
										  " * Init code will be executed each time the window size is changed\r\n"
                      "   or when the effect loads\r\n"
										  " * Frame code is executed before rendering a new frame\r\n"
										  " * Beat code is executed when a beat is detected\r\n"
										  "\r\n"
										  "Predefined variables:\r\n"
										  " x : Light x position, ranges from 0 (left) to 1 (right) (0.5 = center)\r\n"
										  " y : Light y position, ranges from 0 (top) to 1 (bottom) (0.5 = center)\r\n"
										  " isBeat : 1 if no beat, -1 if beat (weird, but old)\r\n"
										  " isLBeat: same as isBeat but persists according to 'shorter/longer' settings\r\n"
										  "          (usable only with OnBeat checked)\r\n"
											" bi:	Bump intensity, ranges	from 0 (flat) to 1 (max specified bump, default)\r\n"
										  " You may also use temporary variables accross code segments\r\n"
										  "\r\n"
										  "Some examples:\r\n"
										  "   Circular move\r\n"
										  "      Init : t=0\r\n"
										  "      Frame: x=0.5+cos(t)*0.3; y=0.5+sin(t)*0.3; t=t+0.1;\r\n"
										  "   Nice motion:\r\n"
										  "      Init : t=0;u=0\r\n"
										  "      Frame: x=0.5+cos(t)*0.3; y=0.5+cos(u)*0.3; t=t+0.1; u=u+0.012;\r\n"
                      ;
        compilerfunctionlist(hwndDlg,text);
			return 0;
    }
	  if ((LOWORD(wParam) == IDC_CHECK1) ||
	     (LOWORD(wParam) == IDC_ONBEAT) ||
	     (LOWORD(wParam) == IDC_ADDITIVE) ||
	     (LOWORD(wParam) == IDC_REPLACE) ||
	     (LOWORD(wParam) == IDC_DOT) ||
			 (LOWORD(wParam) == IDC_INVERTDEPTH) || 
	     (LOWORD(wParam) == IDC_5050) )
			{
			g_ConfigThis->enabled=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
			g_ConfigThis->onbeat=IsDlgButtonChecked(hwndDlg,IDC_ONBEAT)?1:0;
			g_ConfigThis->blend=IsDlgButtonChecked(hwndDlg,IDC_ADDITIVE)?1:0;
			g_ConfigThis->blendavg=IsDlgButtonChecked(hwndDlg,IDC_5050)?1:0;
			g_ConfigThis->showlight=IsDlgButtonChecked(hwndDlg,IDC_DOT)?1:0;
			g_ConfigThis->invert=IsDlgButtonChecked(hwndDlg,IDC_INVERTDEPTH)?1:0;
			}
	  if (LOWORD(wParam) == IDC_CODE1 && HIWORD(wParam) == EN_CHANGE)
			{
        EnterCriticalSection(&g_ConfigThis->rcs);
        g_ConfigThis->code1.get_from_dlgitem(hwndDlg,IDC_CODE1);
				g_ConfigThis->need_recompile=1;
        LeaveCriticalSection(&g_ConfigThis->rcs);
			}
	  if (LOWORD(wParam) == IDC_CODE2 && HIWORD(wParam) == EN_CHANGE)
			{
        EnterCriticalSection(&g_ConfigThis->rcs);
        g_ConfigThis->code2.get_from_dlgitem(hwndDlg,IDC_CODE2);
				g_ConfigThis->need_recompile=1;
        LeaveCriticalSection(&g_ConfigThis->rcs);
			}
	  if (LOWORD(wParam) == IDC_CODE3 && HIWORD(wParam) == EN_CHANGE)
			{
        EnterCriticalSection(&g_ConfigThis->rcs);
        g_ConfigThis->code3.get_from_dlgitem(hwndDlg,IDC_CODE3);
				g_ConfigThis->need_recompile=1;
        g_ConfigThis->initted=0;
        LeaveCriticalSection(&g_ConfigThis->rcs);
			}
	  if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMBO1) // handle clicks to combo box
		  g_ConfigThis->buffern = SendDlgItemMessage(hwndDlg, IDC_COMBO1, CB_GETCURSEL, 0, 0);
		return 0;
	}
return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_ConfigThis = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_BUMP,hwndParent,g_DlgProc);
}



// export stuff

C_RBASE *R_Bump(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}

#else
C_RBASE *R_Bump(char *desc) { return NULL; }
#endif
