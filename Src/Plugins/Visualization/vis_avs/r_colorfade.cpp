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

#define C_THISCLASS C_ColorFadeClass
#define MOD_NAME "Trans / Colorfade"

class C_THISCLASS : public C_RBASE2 {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_COLORFADE,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);


    virtual int smp_getflags() { return 1; }
		virtual int smp_begin(int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); 
    virtual void smp_render(int this_thread, int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); 
    virtual int smp_finish(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); // return value is that of render() for fbstuff etc
    static int ft[4][3];


    int enabled;
    int faders[3];
    int beatfaders[3];
    int faderpos[3];
    unsigned char c_tab[512][512];
    unsigned char clip[256+40+40];
};
int C_THISCLASS::ft[4][3];

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { faders[0]=GET_INT(); pos+=4; }
	if (len-pos >= 4) { faders[1]=GET_INT(); pos+=4; }
	if (len-pos >= 4) { faders[2]=GET_INT(); pos+=4; }
  memcpy(beatfaders,faders,3*sizeof(int));
	if (len-pos >= 4) { beatfaders[0]=GET_INT(); pos+=4; }
	if (len-pos >= 4) { beatfaders[1]=GET_INT(); pos+=4; }
	if (len-pos >= 4) { beatfaders[2]=GET_INT(); pos+=4; }
  memcpy(faderpos,faders,3*sizeof(int));
}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
	PUT_INT(enabled); pos+=4;
  PUT_INT(faders[0]); pos+=4;
  PUT_INT(faders[1]); pos+=4;
  PUT_INT(faders[2]); pos+=4;
  PUT_INT(beatfaders[0]); pos+=4;
  PUT_INT(beatfaders[1]); pos+=4;
  PUT_INT(beatfaders[2]); pos+=4;
	return pos;
}




C_THISCLASS::C_THISCLASS()
{
  int x,y;
  enabled=1;
  faders[0]=8;
  faders[1]=faders[2]=-8;
  memcpy(beatfaders,faders,3*sizeof(int));
  memcpy(faderpos,faders,3*sizeof(int));
  for (x = 0; x < 512; x ++)
  {
    for (y = 0; y < 512; y ++)
    {
      int xp=x-255;
      int yp=y-255;
      if (xp > 0  /* g-b > 0, or g > b */  &&  xp > -yp   /* g-b > r-b, or g > r */ ) c_tab[x][y]=0;
      else if (yp < 0 /* b-r < 0 or r > b */ &&  xp < -yp /* g-b < r-b, or g < r */ ) c_tab[x][y]=1;
      else if (xp < 0 && yp > 0) c_tab[x][y]=2;
      else c_tab[x][y]=3;
    }
  }
  for (x = 0; x < 256+40+40; x ++)
    clip[x]=min(max(x-40,0),255);
}

C_THISCLASS::~C_THISCLASS()
{
}
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  smp_begin(1,visdata,isBeat,framebuffer,fbout,w,h);
  if (isBeat & 0x80000000) return 0;

  smp_render(0,1,visdata,isBeat,framebuffer,fbout,w,h);
  return smp_finish(visdata,isBeat,framebuffer,fbout,w,h);
}

int C_THISCLASS::smp_begin(int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (!enabled || (isBeat&0x80000000)) return 0;

  if (faderpos[0] < faders[0]) faderpos[0]++;
  if (faderpos[1] < faders[2]) faderpos[1]++;
  if (faderpos[2] < faders[1]) faderpos[2]++;
  if (faderpos[0] > faders[0]) faderpos[0]--;
  if (faderpos[1] > faders[2]) faderpos[1]--;
  if (faderpos[2] > faders[1]) faderpos[2]--;

  if (!(enabled&4))
  {
    faderpos[0]=faders[0];
    faderpos[1]=faders[1];
    faderpos[2]=faders[2];
  }
  else if (isBeat && (enabled&2))
  {
    faderpos[0]=(rand()%32)-6;
    faderpos[1]=(rand()%64)-32;
    if (faderpos[1] < 0 && faderpos[1] > -16) faderpos[1]=-32;
    if (faderpos[1] >= 0 && faderpos[1] < 16) faderpos[1]=32;
    faderpos[2]=(rand()%32)-6;
  }
  else if (isBeat)
  {
    faderpos[0]=beatfaders[0];
    faderpos[1]=beatfaders[1];
    faderpos[2]=beatfaders[2];
  }

  {
    int fs1,fs2,fs3;
    fs1=faderpos[0];
    fs2=faderpos[1];
    fs3=faderpos[2];

    ft[0][0]=fs3;
    ft[0][1]=fs2;
    ft[0][2]=fs1;

    ft[1][0]=fs2;
    ft[1][1]=fs1;
    ft[1][2]=fs3;

    ft[2][0]=fs1;
    ft[2][1]=fs3;
    ft[2][2]=fs2;

    ft[3][0]=fs3;
    ft[3][1]=fs3;
    ft[3][2]=fs3;
  }

  return max_threads;
}

int C_THISCLASS::smp_finish(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h) // return value is that of render() for fbstuff etc
{
  return 0;
}
	

void C_THISCLASS::smp_render(int this_thread, int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (!enabled) return;

  if (max_threads < 1) max_threads=1;

  int start_l = ( this_thread * h ) / max_threads;
  int end_l;

  if (this_thread >= max_threads - 1) end_l = h;
  else end_l = ( (this_thread+1) * h ) / max_threads;  

  int outh=end_l-start_l;
  if (outh<1) return;

  unsigned char *q=(unsigned char *)(framebuffer + start_l*w);

  unsigned char *ctab_ptr=(unsigned char *)c_tab[0]+255+(255<<9);
  unsigned char *clip_ptr=(unsigned char *)clip+40;

  int x=w*outh;

  if (enabled) 
  {
    int lx=x&1;
    x>>=1;
    while (x--)
    {
      int r=q[0];
      int g=q[1];
      int b=q[2];
      int r2=q[4];
      int g2=q[5];
      int b2=q[6];
      int i=((g-b)<<9) + b - r;
      int i2=((g2-b2)<<9) + b2 - r2;
      int p=ctab_ptr[i];
      int p2=ctab_ptr[i2];

      q[0]=clip_ptr[r+ft[p][0]];
      q[1]=clip_ptr[g+ft[p][1]];
      q[2]=clip_ptr[b+ft[p][2]];
      q[4]=clip_ptr[r2+ft[p2][0]];
      q[5]=clip_ptr[g2+ft[p2][1]];
      q[6]=clip_ptr[b2+ft[p2][2]];
      q+=8;
    }
    if (lx)
    {
      int r=q[0];
      int g=q[1];
      int b=q[2];
      int i=((g-b)<<9) + b - r;
      int p=ctab_ptr[i];
      q[0]=clip_ptr[r+ft[p][0]];
      q[1]=clip_ptr[g+ft[p][1]];
      q[2]=clip_ptr[b+ft[p][2]];
    }
  }
}

C_RBASE *R_ColorFade(char *desc)
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
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,64);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,g_this->faders[0]+32);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETRANGEMAX,0,64);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETPOS,1,g_this->faders[1]+32);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER3,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER3,TBM_SETRANGEMAX,0,64);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER3,TBM_SETPOS,1,g_this->faders[2]+32);

			SendDlgItemMessage(hwndDlg,IDC_SLIDER4,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER4,TBM_SETRANGEMAX,0,64);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER4,TBM_SETPOS,1,g_this->beatfaders[0]+32);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER5,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER5,TBM_SETRANGEMAX,0,64);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER5,TBM_SETPOS,1,g_this->beatfaders[1]+32);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER6,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER6,TBM_SETRANGEMAX,0,64);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER6,TBM_SETPOS,1,g_this->beatfaders[2]+32);

      if (g_this->enabled&1) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
      if (g_this->enabled&2) CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);
      if (g_this->enabled&4) CheckDlgButton(hwndDlg,IDC_CHECK3,BST_CHECKED);
			return 1;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_CHECK1:
        case IDC_CHECK2:
        case IDC_CHECK3:
          g_this->enabled=(IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0)|
                          (IsDlgButtonChecked(hwndDlg,IDC_CHECK2)?2:0)|
                          (IsDlgButtonChecked(hwndDlg,IDC_CHECK3)?4:0);
          return 0;

      }
    return 0;
		case WM_HSCROLL:
			{
				HWND swnd = (HWND) lParam;
				int t = (int) SendMessage(swnd,TBM_GETPOS,0,0);
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
				{
					g_this->faders[0]=t-32;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER2))
				{
					g_this->faders[1]=t-32;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER3))
				{
					g_this->faders[2]=t-32;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER4))
				{
					g_this->beatfaders[0]=t-32;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER5))
				{
					g_this->beatfaders[1]=t-32;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER6))
				{
					g_this->beatfaders[2]=t-32;
				}
			}
    return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_COLORFADE,hwndParent,g_DlgProc);
}
#else
C_RBASE *R_ColorFade(char *desc) { return NULL; }
#endif