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
#include "r_defs.h"
#include "resource.h"
#include "r_stack.h"

#include "timing.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

#define MOD_NAME "Misc / Buffer Save"

#define C_THISCLASS C_StackClass

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_StackClass();
		virtual ~C_StackClass();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc();
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

    int clear;
    volatile int dir_ch;

    int blend;
    int dir;
    int which;
    int adjblend_val;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { dir=GET_INT(); pos+=4; }
	if (len-pos >= 4) { which=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
	if (len-pos >= 4) { adjblend_val=GET_INT(); pos+=4; }
  
  if (which < 0) which=0;
  if (which >= NBUF) which=NBUF-1;
}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
  PUT_INT(dir); pos+=4;
  PUT_INT(which); pos+=4;
  PUT_INT(blend); pos+=4;
  PUT_INT(adjblend_val); pos+=4;
	return pos;
}

char *C_THISCLASS::get_desc()
{
static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_MISC_BUFFER_SAVE,desc,128):desc);
}


C_THISCLASS::C_THISCLASS()
{
  adjblend_val=128;
  dir_ch=0;
  which=0;
  dir=0;
  blend=0;
  clear=0;
}

C_THISCLASS::~C_THISCLASS()
{
}

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  void *thisbufptr;
  if (isBeat&0x80000000) return 0;
  if (!(thisbufptr=getGlobalBuffer(w,h,which,dir!=1)))
  {
    return 0;
  }

  if (clear)
  {
    clear=0;
    memset(thisbufptr,0,sizeof(int)*w*h);
  }

  {
    int t_dir;
    t_dir=(dir<2)?dir:((dir&1)^dir_ch);
    dir_ch^=1;
    int *fbin=(int*)(t_dir==0?framebuffer:thisbufptr);
    int *fbout=(int*)(t_dir!=0?framebuffer:thisbufptr);
    if (blend == 1)
    {
      mmx_avgblend_block(fbout,fbin,w*h);
    }
    else if (blend == 2)
    {
      mmx_addblend_block(fbout,fbin,w*h);
    }
    else if (blend == 3)
    {
      int r=0;
      int y=h;
      int *bf=fbin;
      while (y-- > 0)
      {
        int *out, *in;
        int x=w/2;
        out=fbout+r;
        in=bf+r;
        r^=1;
        while (x-- > 0)
        {
          *out=*in;
          out+=2;
          in+=2;
        }
        fbout+=w;
        bf+=w;
      }
    }
    else if (blend == 4)
    {
      int x=w*h;
      int *bf=fbin;
      while (x-- > 0)
      {
        *fbout=BLEND_SUB(*fbout,*bf);
        fbout++;
        bf++;
      }
    }
    else if (blend == 5)
    {
      int y=h/2;
      while (y-- > 0)
      {
        memcpy(fbout,fbin,w*sizeof(int));
        fbout+=w*2;
        fbin+=w*2;
      }
    }
    else if (blend == 6)
    {
      int x=w*h;
      while (x-- > 0)
      {
        *fbout=*fbout ^ *fbin;
        fbout++;
        fbin++;
      }
    }
    else if (blend == 7)
    {
      int x=w*h;
      int *bf=fbin;
      while (x-- > 0)
      {
        *fbout=BLEND_MAX(*fbout,*bf);
        fbout++;
        bf++;
      }
    }
    else if (blend == 8)
    {
      int x=w*h;
      int *bf=fbin;
      while (x-- > 0)
      {
        *fbout=BLEND_MIN(*fbout,*bf);
        fbout++;
        bf++;
      }
    }
    else if (blend == 9)
    {
      int x=w*h;
      int *bf=fbin;
      while (x-- > 0)
      {
        *fbout=BLEND_SUB(*bf,*fbout);
        fbout++;
        bf++;
      }
    }
    else if (blend == 10)
    {
      mmx_mulblend_block(fbout,fbin,w*h);
    }
    else if (blend == 11)
    {
      mmx_adjblend_block(fbout,fbin,fbout,w*h,adjblend_val);
    }
    else memcpy(fbout,fbin,w*h*sizeof(int));
    return 0;
  }

  return 0;
}

C_RBASE *R_Stack(char *desc)
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}


static C_THISCLASS *g_this;

static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
      {
        int x;
        for (x = 1; x <= NBUF; x ++)
        {
          char s[32];
          wsprintf(s,WASABI_API_LNGSTRING(IDS_BUFFER_X),x);
          SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)s);
        }
      }
      SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,g_this->which,0);
      if (g_this->dir==1) CheckDlgButton(hwndDlg,IDC_RESTFB,BST_CHECKED);
      else if (g_this->dir == 2) CheckDlgButton(hwndDlg,IDC_RADIO1,BST_CHECKED);
      else if (g_this->dir == 3) CheckDlgButton(hwndDlg,IDC_RADIO2,BST_CHECKED);
      else CheckDlgButton(hwndDlg,IDC_SAVEFB,BST_CHECKED);
			SendDlgItemMessage(hwndDlg, IDC_BLENDSLIDE, TBM_SETRANGE, TRUE, MAKELONG(0, 255));
			SendDlgItemMessage(hwndDlg, IDC_BLENDSLIDE, TBM_SETPOS, TRUE, (int)(g_this->adjblend_val));
      if (g_this->blend==0) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND1,BST_CHECKED);
      if (g_this->blend==1) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND2,BST_CHECKED);
      if (g_this->blend==2) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND3,BST_CHECKED);
      if (g_this->blend==3) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND4,BST_CHECKED);
      if (g_this->blend==4) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND5,BST_CHECKED);
      if (g_this->blend==5) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND6,BST_CHECKED);
      if (g_this->blend==6) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND7,BST_CHECKED);
      if (g_this->blend==7) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND8,BST_CHECKED);
      if (g_this->blend==8) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND9,BST_CHECKED);
      if (g_this->blend==9) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND10,BST_CHECKED);
      if (g_this->blend==10) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND11,BST_CHECKED);
      if (g_this->blend==11) CheckDlgButton(hwndDlg,IDC_RSTACK_BLEND12,BST_CHECKED);
			return 1;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDC_BUTTON2)
      {
        g_this->dir_ch^=1;
      }
      if (LOWORD(wParam) == IDC_BUTTON1)
      {
        g_this->clear=1;
      }
      if (LOWORD(wParam) == IDC_SAVEFB || LOWORD(wParam) == IDC_RESTFB || LOWORD(wParam) == IDC_RADIO1 || LOWORD(wParam) == IDC_RADIO2)
      {
        if (IsDlgButtonChecked(hwndDlg,IDC_SAVEFB)) g_this->dir=0;
        else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO2)) g_this->dir=3;
        else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO1)) g_this->dir=2;
        else if (IsDlgButtonChecked(hwndDlg,IDC_RESTFB)) g_this->dir=1;
      }
      if (IsDlgButtonChecked(hwndDlg,LOWORD(wParam)))
      {
        if (LOWORD(wParam) == IDC_RSTACK_BLEND1) g_this->blend=0;
        if (LOWORD(wParam) == IDC_RSTACK_BLEND2) g_this->blend=1;
        if (LOWORD(wParam) == IDC_RSTACK_BLEND3) g_this->blend=2;
        if (LOWORD(wParam) == IDC_RSTACK_BLEND4) g_this->blend=3;
        if (LOWORD(wParam) == IDC_RSTACK_BLEND5) g_this->blend=4;
        if (LOWORD(wParam) == IDC_RSTACK_BLEND6) g_this->blend=5;
        if (LOWORD(wParam) == IDC_RSTACK_BLEND7) g_this->blend=6;
        if (LOWORD(wParam) == IDC_RSTACK_BLEND8) g_this->blend=7;
        if (LOWORD(wParam) == IDC_RSTACK_BLEND9) g_this->blend=8;
        if (LOWORD(wParam) == IDC_RSTACK_BLEND10) g_this->blend=9;
        if (LOWORD(wParam) == IDC_RSTACK_BLEND11) g_this->blend=10;
        if (LOWORD(wParam) == IDC_RSTACK_BLEND12) g_this->blend=11;
      }
      if (LOWORD(wParam) == IDC_COMBO1 && HIWORD(wParam) == CBN_SELCHANGE)
      {
        int i=SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
        if (i != CB_ERR)
          g_this->which=i;
      }
      return 0;
    return 0;
	  case WM_NOTIFY:
		  if (LOWORD(wParam) == IDC_BLENDSLIDE)
			  g_this->adjblend_val = SendDlgItemMessage(hwndDlg, IDC_BLENDSLIDE, TBM_GETPOS, 0, 0);
    return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_STACK,hwndParent,g_DlgProc);
}
#else
C_RBASE *R_Stack(char *desc)
{ return NULL; }
#endif