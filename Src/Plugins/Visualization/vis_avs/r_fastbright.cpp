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
#include <commctrl.h>
#include "r_defs.h"
#include "resource.h"

#include "timing.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

#define C_THISCLASS C_FastBright
#define MOD_NAME "Trans / Fast Brightness"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_FAST_BRIGHTNESS,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
#ifdef NO_MMX 
    int tab[3][256];
#endif
    
    int dir;
};


#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	int pos=0;
  dir=0;
	if (len-pos >= 4) { dir=GET_INT(); pos+=4; }
}

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
int  C_THISCLASS::save_config(unsigned char *data) // write configuration to data, return length. config data should not exceed 64k.
{
  int pos=0;
  PUT_INT(dir); pos+=4;
  return pos;
}


C_THISCLASS::C_THISCLASS()
{
#ifdef NO_MMX 
  int x;
  for (x = 0; x < 128; x ++)
  {
    tab[0][x]=x+x;
    tab[1][x]=x<<9;
    tab[2][x]=x<<17;
  }
  for (; x < 256; x ++)
  {
    tab[0][x]=255;
    tab[1][x]=255<<8;
    tab[2][x]=255<<16;
  }
#endif
  dir=0;
}

C_THISCLASS::~C_THISCLASS()
{
}
	
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (isBeat&0x80000000) return 0;
#ifdef NO_MMX // the non mmx x2 version really isn't any , in terms faster than normal brightness with no exclusions turned on
	{
	  unsigned int *t=(unsigned int *)framebuffer;
	  int x;
    unsigned int mask = 0x7F7F7F7F;

    x=w*h/2;
	  if (dir == 0)
      while (x--)
	    {
        unsigned int v1=t[0];
        unsigned int v2=t[1];
        v1=tab[0][v1&0xff]|tab[1][(v1>>8)&0xff]|tab[2][(v1>>16)&0xff]|(v1&0xff000000);
        v2=tab[0][v2&0xff]|tab[1][(v2>>8)&0xff]|tab[2][(v2>>16)&0xff]|(v2&0xff000000);
        t[0]=v1;
        t[1]=v2;
        t+=2;
	    }
    else if (dir == 1)
      while (x--)
	    {
        unsigned int v1=t[0]>>1;
        unsigned int v2=t[1]>>1;
        t[0]=v1&mask;
        t[1]=v2&mask;
        t+=2;
	    }
  }
#else
  int mask[2] =
  {
    0x7F7F7F7F,
    0x7F7F7F7F,
  };
  int l=(w*h);
  if (dir == 0) __asm 
		{
			mov edx, l
			mov edi, framebuffer
      shr edx, 3 // 8 pixels at a time
      align 16
		_l1:
			movq mm0, [edi]
			movq mm1, [edi+8]
			movq mm2, [edi+16]
      paddusb mm0, mm0
			movq mm3, [edi+24]
      paddusb mm1, mm1
      paddusb mm2, mm2
      movq [edi], mm0
      paddusb mm3, mm3
      movq [edi+8], mm1
      movq [edi+16], mm2
      movq [edi+24], mm3

      add edi, 32

			dec edx
			jnz _l1

			mov edx, l
			and edx, 7
      shr edx, 1 // up the last 7 pixels (two at a time)
			jz _l3

		_l2:
			movq mm3, [edi]
      paddusb mm3, mm3
      movq [edi], mm3
			add edi, 8
			dec edx
			jnz _l2
		_l3:
			emms
		}
  else if (dir == 1) __asm 
		{
			mov edx, l
      movq mm7, [mask]
			mov edi, framebuffer
      shr edx, 3 // 8 pixels at a time
      align 16
		_lr1:
			movq mm0, [edi]
			movq mm1, [edi+8]

			movq mm2, [edi+16]
      psrl mm0, 1

			movq mm3, [edi+24]
      pand mm0, mm7
      
      psrl mm1, 1
      movq [edi], mm0

      psrl mm2, 1
      pand mm1, mm7

      movq [edi+8], mm1
      pand mm2, mm7

      psrl mm3, 1
      movq [edi+16], mm2

      pand mm3, mm7
      movq [edi+24], mm3

      add edi, 32

			dec edx
			jnz _lr1

			mov edx, l
			and edx, 7
      shr edx, 1 // up the last 7 pixels (two at a time)
			jz _lr3

		_lr2:
			movq mm3, [edi]
      psrl mm3, 1
      pand mm3, mm7
      movq [edi], mm3
			add edi, 8
			dec edx
			jnz _lr2
		_lr3:
			emms
		}
#endif
  return 0;
}

C_RBASE *R_FastBright(char *desc)
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
      if (g_this->dir==0) CheckDlgButton(hwndDlg,IDC_RADIO1,BST_CHECKED);
      else if (g_this->dir == 1) CheckDlgButton(hwndDlg,IDC_RADIO2,BST_CHECKED);
      else CheckDlgButton(hwndDlg,IDC_RADIO3,BST_CHECKED);
			return 1;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDC_RADIO1)
        if (IsDlgButtonChecked(hwndDlg,IDC_RADIO1))
          g_this->dir=0;
      if (LOWORD(wParam) == IDC_RADIO2)
        if (IsDlgButtonChecked(hwndDlg,IDC_RADIO2))
          g_this->dir=1;
      if (LOWORD(wParam) == IDC_RADIO3)
        if (IsDlgButtonChecked(hwndDlg,IDC_RADIO3))
          g_this->dir=2;
    return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_FASTBRIGHT,hwndParent,g_DlgProc);
}
#else
C_RBASE *R_FastBright(char *desc) { return NULL; }

#endif