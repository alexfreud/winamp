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
#include <math.h>
#include "resource.h"
#include "r_defs.h"
#include "../Agave/Language/api_language.h"

#define MOD_NAME "Misc / Custom BPM"
#define C_THISCLASS C_BpmClass

#define SET_BEAT		0x10000000
#define CLR_BEAT		0x20000000

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_MISC_CUSTOM_BPM,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
		void SliderStep(int Ctl, int *slide);
    int enabled;
	int arbitrary, skip, invert;		// Type of action, adapt = detect beat
	int arbVal, skipVal;				// Values of arbitrary beat and beat skip
	DWORD arbLastTC;					// Last tick count used for arbitrary beat
	int skipCount;						// Beat counter used by beat skipper
	int inInc, outInc;					// +1/-1, Used by the nifty beatsynced sliders
	int inSlide, outSlide;				// Positions of sliders
	int oldInSlide, oldOutSlide;		// Used by timer to detect changes in sliders
	int skipfirst;
	int count;
	};

static C_THISCLASS *g_ConfigThis; // global configuration dialog pointer 
static HINSTANCE g_hDllInstance; // global DLL instance pointer (not needed in this example, but could be useful)

C_THISCLASS::~C_THISCLASS() 
{
}

// configuration read/write

C_THISCLASS::C_THISCLASS() // set up default configuration
{
	skipfirst=0;
  inSlide=0;
  outSlide=0;
  oldInSlide=0;
  oldOutSlide=0;
  enabled = 1;
  arbLastTC = GetTickCount();
  arbitrary=1;
  arbVal = 500;
  skipVal = 1;
  skipCount = 0;
  skip=0;
  invert=0;
}

#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	int pos=0;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { arbitrary=GET_INT(); pos+=4; }
	if (len-pos >= 4) { skip=GET_INT(); pos+=4; }
	if (len-pos >= 4) { invert=GET_INT(); pos+=4; }
	if (len-pos >= 4) { arbVal=GET_INT(); pos+=4; }
	if (len-pos >= 4) { skipVal=GET_INT(); pos+=4; }
	if (len-pos >= 4) { skipfirst=GET_INT(); pos+=4; }
}

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
int  C_THISCLASS::save_config(unsigned char *data) // write configuration to data, return length. config data should not exceed 64k.
{
  int pos=0;
  PUT_INT(enabled); pos+=4;
  PUT_INT(arbitrary); pos+=4;
  PUT_INT(skip); pos+=4;
  PUT_INT(invert); pos+=4;
  PUT_INT(arbVal); pos+=4;
  PUT_INT(skipVal); pos+=4;
  PUT_INT(skipfirst); pos+=4;
  return pos;
}



void C_THISCLASS::SliderStep(int Ctl, int *slide)
{
*slide += Ctl == IDC_IN ? inInc : outInc;
if (!*slide || *slide == 8) (Ctl == IDC_IN ? inInc : outInc) *= -1;
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
  if (isBeat&0x80000000) return 0;

	if (isBeat) // Show the beat received from AVS
		{
		SliderStep(IDC_IN, &inSlide);
		count++;
		}

	if (skipfirst != 0 && count <= skipfirst)
			return isBeat ? CLR_BEAT : 0;

  if (arbitrary)
	{
	DWORD TCNow = GetTickCount();
	if (TCNow > arbLastTC + arbVal)
		{
		arbLastTC = TCNow;
		SliderStep(IDC_OUT, &outSlide);
		return SET_BEAT;
		}
	return CLR_BEAT;
	}

  if (skip)
	{
	if (isBeat && ++skipCount >= skipVal+1)
		{
		skipCount = 0;
		SliderStep(IDC_OUT, &outSlide);
		return SET_BEAT;
		}
	return CLR_BEAT;
	}

  if (invert)
	{
	if (isBeat)
		return CLR_BEAT;
	else
		{
		SliderStep(IDC_OUT, &outSlide);
		return SET_BEAT;
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
		{
		char txt[40], beat[16], beats[16];
		g_ConfigThis->inInc = 1;
		g_ConfigThis->outInc = 1;
		g_ConfigThis->inSlide = 0;
		g_ConfigThis->outSlide = 0;
        if (g_ConfigThis->enabled) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
        if (g_ConfigThis->arbitrary) CheckDlgButton(hwndDlg,IDC_ARBITRARY,BST_CHECKED);
        if (g_ConfigThis->skip) CheckDlgButton(hwndDlg,IDC_SKIP,BST_CHECKED);
        if (g_ConfigThis->invert) CheckDlgButton(hwndDlg,IDC_INVERT,BST_CHECKED);
		SendDlgItemMessage(hwndDlg, IDC_ARBVAL, TBM_SETTICFREQ, 100, 0);
		SendDlgItemMessage(hwndDlg, IDC_SKIPVAL, TBM_SETTICFREQ, 1, 0);
		SendDlgItemMessage(hwndDlg, IDC_ARBVAL, TBM_SETRANGE, TRUE, MAKELONG(200, 10000));
		SendDlgItemMessage(hwndDlg, IDC_SKIPVAL, TBM_SETRANGE, TRUE, MAKELONG(1, 16));
		SendDlgItemMessage(hwndDlg, IDC_ARBVAL, TBM_SETPOS, TRUE, g_ConfigThis->arbVal);
		SendDlgItemMessage(hwndDlg, IDC_SKIPVAL, TBM_SETPOS, TRUE, g_ConfigThis->skipVal);
		SendDlgItemMessage(hwndDlg, IDC_IN, TBM_SETTICFREQ, 1, 0);
		SendDlgItemMessage(hwndDlg, IDC_IN, TBM_SETRANGE, TRUE, MAKELONG(0, 8));
		SendDlgItemMessage(hwndDlg, IDC_OUT, TBM_SETTICFREQ, 1, 0);
		SendDlgItemMessage(hwndDlg, IDC_OUT, TBM_SETRANGE, TRUE, MAKELONG(0, 8));
		SendDlgItemMessage(hwndDlg, IDC_SKIPFIRST, TBM_SETTICFREQ, 1, 0);
		SendDlgItemMessage(hwndDlg, IDC_SKIPFIRST, TBM_SETRANGE, TRUE, MAKELONG(0, 64));
		SendDlgItemMessage(hwndDlg, IDC_SKIPFIRST, TBM_SETPOS, TRUE, g_ConfigThis->skipfirst);
		wsprintf(txt, "%d bpm", 60000 / g_ConfigThis->arbVal);
		SetDlgItemText(hwndDlg, IDC_ARBTXT, txt);
		WASABI_API_LNGSTRING_BUF(IDS_BEAT,beat,16);
		WASABI_API_LNGSTRING_BUF(IDS_BEATS,beats,16);
		wsprintf(txt, "%d %s", g_ConfigThis->skipVal, g_ConfigThis->skipVal > 1 ? beats : beat);
		SetDlgItemText(hwndDlg, IDC_SKIPTXT, txt);
		wsprintf(txt, "%d %s", g_ConfigThis->skipfirst, g_ConfigThis->skipfirst > 1 ? beats : beat);
		SetDlgItemText(hwndDlg, IDC_SKIPFIRSTTXT, txt);
		SetTimer(hwndDlg, 0, 50, NULL);
		}
		return 1;
	case WM_TIMER:
		{
		if (g_ConfigThis->oldInSlide != g_ConfigThis->inSlide) {
			SendDlgItemMessage(hwndDlg, IDC_IN, TBM_SETPOS, TRUE, g_ConfigThis->inSlide); g_ConfigThis->oldInSlide=g_ConfigThis->inSlide; }
		if (g_ConfigThis->oldOutSlide != g_ConfigThis->outSlide) {
			SendDlgItemMessage(hwndDlg, IDC_OUT, TBM_SETPOS, TRUE, g_ConfigThis->outSlide); g_ConfigThis->oldOutSlide=g_ConfigThis->outSlide; }
		}
		return 0;
	case WM_NOTIFY:
		{
		char txt[40], beat[16], beats[16];
		if (LOWORD(wParam) == IDC_ARBVAL)
			g_ConfigThis->arbVal = SendDlgItemMessage(hwndDlg, IDC_ARBVAL, TBM_GETPOS, 0, 0);
		if (LOWORD(wParam) == IDC_SKIPVAL)
			g_ConfigThis->skipVal = SendDlgItemMessage(hwndDlg, IDC_SKIPVAL, TBM_GETPOS, 0, 0);
		if (LOWORD(wParam) == IDC_SKIPFIRST)
			g_ConfigThis->skipfirst = SendDlgItemMessage(hwndDlg, IDC_SKIPFIRST, TBM_GETPOS, 0, 0);
		wsprintf(txt, "%d bpm", 60000 / g_ConfigThis->arbVal);
		SetDlgItemText(hwndDlg, IDC_ARBTXT, txt);
		WASABI_API_LNGSTRING_BUF(IDS_BEAT,beat,16);
		WASABI_API_LNGSTRING_BUF(IDS_BEATS,beats,16);
		wsprintf(txt, "%d %s", g_ConfigThis->skipVal, g_ConfigThis->skipVal > 1 ? beats : beat);
		SetDlgItemText(hwndDlg, IDC_SKIPTXT, txt);
		wsprintf(txt, "%d %s", g_ConfigThis->skipfirst, g_ConfigThis->skipfirst > 1 ? beats : beat);
		SetDlgItemText(hwndDlg, IDC_SKIPFIRSTTXT, txt);
		return 0;
		}
	case WM_COMMAND:
      if ((LOWORD(wParam) == IDC_CHECK1) ||
		  (LOWORD(wParam) == IDC_ARBITRARY) ||
		  (LOWORD(wParam) == IDC_SKIP) ||
		  (LOWORD(wParam) == IDC_INVERT))
		  {
			g_ConfigThis->enabled=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
			g_ConfigThis->arbitrary=IsDlgButtonChecked(hwndDlg,IDC_ARBITRARY)?1:0;
			g_ConfigThis->skip=IsDlgButtonChecked(hwndDlg,IDC_SKIP)?1:0;
			g_ConfigThis->invert=IsDlgButtonChecked(hwndDlg,IDC_INVERT)?1:0;
		  }
	  return 0;
	case WM_DESTROY:
		KillTimer(hwndDlg, 0);
		return 0;
	}
return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_ConfigThis = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_BPM,hwndParent,g_DlgProc);
}

C_RBASE *R_Bpm(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}
