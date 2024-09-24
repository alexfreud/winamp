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

#include "timing.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

#define C_THISCLASS C_FadeOutClass
#define MOD_NAME "Trans / Fadeout"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_FADEOUT,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

    void maketab(void);

		unsigned char fadtab[3][256];
		int fadelen, color;
};

void C_THISCLASS::maketab(void)
{
  int rseek=color&0xff;
  int gseek=(color>>8)&0xff;
  int bseek=(color>>16)&0xff;
  int x;
  for (x = 0; x < 256; x ++)
  {
    int r=x;
    int g=x;
    int b=x;
    if (r <= rseek-fadelen) r+=fadelen;
    else if (r >= rseek+fadelen) r-=fadelen;
    else r=rseek;

    if (g <= gseek-fadelen) g+=fadelen;
    else if (g >= gseek+fadelen) g-=fadelen;
    else g=gseek;
    if (b <= bseek-fadelen) b+=fadelen;
    else if (b >= bseek+fadelen) b-=fadelen;
    else b=bseek;

    fadtab[0][x]=r;
    fadtab[1][x]=g;
    fadtab[2][x]=b;
  }
}

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { fadelen=GET_INT(); pos+=4; }
	if (len-pos >= 4) { color=GET_INT(); pos+=4; }
  maketab();
}

int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
	PUT_INT(fadelen); pos+=4;
	PUT_INT(color); pos+=4;
	return pos;
}



C_THISCLASS::C_THISCLASS()
{
  color=0;
	fadelen=16;
  maketab();
}

C_THISCLASS::~C_THISCLASS()
{
}
	
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (isBeat&0x80000000) return 0;
  if (!fadelen) return 0;
	timingEnter(1);
	if (
#ifdef NO_MMX
    1
#else   
    color
#endif
    )
	{
	  unsigned char *t=(unsigned char *)framebuffer;
	  int x=w*h;
	  while (x--)
	  {
		  t[0]=fadtab[0][t[0]];
		  t[1]=fadtab[1][t[1]];
		  t[2]=fadtab[2][t[2]];
		  t+=4;
	  }
  }
#ifndef NO_MMX
  else
  {
		int l=(w*h);
		char fadj[8];
		int x;
    unsigned char *t=fadtab[0];
		for (x = 0; x < 8; x ++) fadj[x]=this->fadelen;
		__asm 
		{
			mov edx, l
			mov edi, framebuffer
			movq mm7, [fadj]
			shr edx, 3
      align 16
		_l1:
			movq mm0, [edi]

			movq mm1, [edi+8]

			movq mm2, [edi+16]
			psubusb mm0, mm7

			movq mm3, [edi+24]
			psubusb mm1, mm7

			movq [edi], mm0
			psubusb mm2, mm7

			movq [edi+8], mm1
			psubusb mm3, mm7

			movq [edi+16], mm2

			movq [edi+24], mm3

			add edi, 8*4

			dec edx
			jnz _l1

			mov edx, l
			sub eax, eax

			and edx, 7
			jz _l3

			sub ebx, ebx
			sub ecx, ecx

			mov esi, t
		_l2:
			mov al, [edi]
			mov bl, [edi+1]
			mov cl, [edi+2]
			sub al, [esi+eax]
			sub bl, [esi+ebx]
			sub cl, [esi+ecx]
			mov [edi], al
			mov [edi+1], bl
			mov [edi+2], cl
			add edi, 4
			dec edx
			jnz _l2
		_l3:
			emms
		}
  }
#endif
	timingLeave(1);
  return 0;
}

C_RBASE *R_FadeOut(char *desc)
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
						GR_DrawColoredButton(di,g_this->color);
					break;
				}
			}
		return 0;
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,92);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,g_this->fadelen);
			
			return 1;

		case WM_HSCROLL:
			{
				HWND swnd = (HWND) lParam;
				int t = (int) SendMessage(swnd,TBM_GETPOS,0,0);
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
				{
					g_this->fadelen=t;
          g_this->maketab();
				}
			}
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
				case IDC_LC:
					GR_SelectColor(hwndDlg,&g_this->color);
					InvalidateRect(GetDlgItem(hwndDlg,LOWORD(wParam)),NULL,FALSE);
          g_this->maketab();
				return 0;
      }
    return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_FADE,hwndParent,g_DlgProc);
}

#else
C_RBASE *R_FadeOut(char *desc) { return NULL; }
#endif