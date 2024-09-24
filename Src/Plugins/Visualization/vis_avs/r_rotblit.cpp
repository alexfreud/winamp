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

#include <math.h>
#define M_PI 3.1415926536

#define C_THISCLASS C_RotBlitClass
#define MOD_NAME "Trans / Roto Blitter"


class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_ROTO_BLITTER,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

		int zoom_scale, rot_dir, blend, beatch, beatch_speed, zoom_scale2, beatch_scale,scale_fpos;
		int rot_rev;
    int subpixel;
    double rot_rev_pos;

    int l_w, l_h;
    int *w_mul;
};


#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { zoom_scale=GET_INT(); pos+=4; }
	if (len-pos >= 4) { rot_dir=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
	if (len-pos >= 4) { beatch=GET_INT(); pos+=4; }
	if (len-pos >= 4) { beatch_speed=GET_INT(); pos+=4; }
	if (len-pos >= 4) { zoom_scale2=GET_INT(); pos+=4; }
	if (len-pos >= 4) { beatch_scale=GET_INT(); pos+=4; }
	if (len-pos >= 4) { subpixel=GET_INT(); pos+=4; }
  else subpixel=0;
	
  scale_fpos=zoom_scale;
}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
	PUT_INT(zoom_scale); pos+=4;
	PUT_INT(rot_dir); pos+=4;
	PUT_INT(blend); pos+=4;
	PUT_INT(beatch); pos+=4;
	PUT_INT(beatch_speed); pos+=4;
	PUT_INT(zoom_scale2); pos+=4;
	PUT_INT(beatch_scale); pos+=4;
	PUT_INT(subpixel); pos+=4;
	return pos;
}


C_THISCLASS::C_THISCLASS()
{
	zoom_scale=31;
	rot_dir=31;
	blend=0;
  rot_rev=1;
  rot_rev_pos=1.0;
	beatch=0;
  l_w=l_h=0;
  w_mul=NULL;
  beatch_speed=0;
  beatch_scale=0;
  zoom_scale2=31;
  scale_fpos=zoom_scale;
  subpixel=1;
}

C_THISCLASS::~C_THISCLASS()
{
  if (w_mul) GlobalFree(w_mul);
  w_mul=NULL;
  l_w=l_h=0;
}
	
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
	int y;

  if (l_w != w || l_h != h || !w_mul) // generate width table
  {
    int x;
    if (w_mul) GlobalFree(w_mul);
    l_w=w;
    l_h=h;
    w_mul=(int *)GlobalAlloc(GMEM_FIXED,sizeof(int)*h);
    for (x = 0; x < h; x ++)
      w_mul[x]=x*w;
  }
  if (isBeat&0x80000000) return 0;

	unsigned int *dest=(unsigned int *) fbout;
  unsigned int *src=(unsigned int *) framebuffer;
  unsigned int *bdest=(unsigned int *) framebuffer;

	if (isBeat && beatch) 
	{
		rot_rev=-rot_rev;
	}
  
  if (!beatch) rot_rev=1;
	
  rot_rev_pos+=(1.0/(1+beatch_speed*4))*(rot_rev-rot_rev_pos);

  if (rot_rev_pos > rot_rev && rot_rev>0) rot_rev_pos=rot_rev;
  if (rot_rev_pos < rot_rev && rot_rev<0) rot_rev_pos=rot_rev;


	if (isBeat && beatch_scale) 
	{
		scale_fpos=zoom_scale2;
	}

  int f_val;
	if (zoom_scale < zoom_scale2) 
	{
		f_val=max(scale_fpos,zoom_scale); 
		if (scale_fpos > zoom_scale) scale_fpos -= 3;
	}
	else 
	{
		f_val=min(scale_fpos,zoom_scale);
		if (scale_fpos < zoom_scale) scale_fpos+=3;
	}
  
  double zoom = 1.0 + (f_val-31)/31.0;
	
  double theta=((rot_dir-32))*rot_rev_pos;
	double temp;
	int ds_dx, dt_dx, ds_dy, dt_dy, s, t, sstart, tstart;
	int x, offset=0;

	temp = cos((theta)*M_PI/180.0)*zoom;
	ds_dx = (int) (temp*65536.0);
	dt_dy = (int) (temp*65536.0);
	temp = sin((theta)*M_PI/180.0)*zoom;
	ds_dy = - (int) (temp*65536.0);
	dt_dx = (int) (temp*65536.0);
  
	s = sstart = -(((w-1)/2)*ds_dx + ((h-1)/2)*ds_dy) + (w-1)*(32768 + (1<<20));
	t = tstart = -(((w-1)/2)*dt_dx + ((h-1)/2)*dt_dy) + (h-1)*(32768 + (1<<20));
	int ds, dt;
	ds = (w-1)<<16;
	dt = (h-1)<<16;
	y = h;

	if (ds_dx <= -ds || ds_dx >= ds || dt_dx <= -dt || dt_dx >= dt);
	else while (y--) 
  {
		if (ds) s %= ds;
		if (dt) t %= dt;
    if (s < 0) s+=ds; if (t < 0) t+=dt;
		x = w;
		offset = y*w;


#define DO_LOOP(Z) while (x--) { Z; s += ds_dx; t += dt_dx; }
#define DO_LOOPS(Z) \
		    if (ds_dx <= 0 && dt_dx <= 0) DO_LOOP(if (t < 0) t += dt; if (s < 0) s += ds; Z)  \
        else if (ds_dx <= 0) DO_LOOP(if (t >= dt) t -= dt; if (s < 0) s += ds; Z) \
		    else if (dt_dx <= 0) DO_LOOP(if (t < 0) t += dt; if (s >= ds) s -= ds; Z) \
        else DO_LOOP(if (t >= dt) t -= dt; if (s >= ds) s -= ds; Z)


    if (subpixel && blend) DO_LOOPS(*dest++ = BLEND_AVG(*bdest++,BLEND4_16(src+(s>>16)+w_mul[t>>16],w,s,t)))
    else if (subpixel) DO_LOOPS(*dest++ = BLEND4_16(src+(s>>16)+w_mul[t>>16],w,s,t))
    else if (!blend) DO_LOOPS(*dest++ = src[(s>>16)+w_mul[t>>16]])
    else DO_LOOPS(*dest++ = BLEND_AVG(*bdest++,src[(s>>16)+w_mul[t>>16]]))

		s = (sstart += ds_dy);
		t = (tstart += dt_dy);
	}
#ifndef NO_MMX
  __asm emms;
#endif
 
  return 1;
}

C_RBASE *R_RotBlit(char *desc)
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
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,256);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,g_this->zoom_scale);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER6,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER6,TBM_SETRANGEMAX,0,256);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER6,TBM_SETPOS,1,g_this->zoom_scale2);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER5,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER5,TBM_SETRANGEMAX,0,8);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER5,TBM_SETPOS,1,g_this->beatch_speed);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETRANGEMAX,0,64);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETPOS,1,g_this->rot_dir);
			if (g_this->subpixel) CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);			
			else if (g_this->blend==1) CheckDlgButton(hwndDlg,IDC_BLEND,BST_CHECKED);			
			if (g_this->beatch==1) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);			
      if (g_this->beatch_scale==1) CheckDlgButton(hwndDlg,IDC_CHECK6,BST_CHECKED);			
			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BLEND:
					if (IsDlgButtonChecked(hwndDlg,IDC_BLEND))
          {
						g_this->blend=1;
          }
					else g_this->blend=0;
				break;
				case IDC_CHECK1:
					if (IsDlgButtonChecked(hwndDlg,IDC_CHECK1))
						g_this->beatch=1;
					else g_this->beatch=0;
				break;
				case IDC_CHECK2:
					if (IsDlgButtonChecked(hwndDlg,IDC_CHECK2))
          {
						g_this->subpixel=1;
          }
					else g_this->subpixel=0;
				break;
				case IDC_CHECK6:
					if (IsDlgButtonChecked(hwndDlg,IDC_CHECK6))
						g_this->beatch_scale=1;
					else g_this->beatch_scale=0;
				break;
			}
		return 0;

		case WM_HSCROLL:
			{
				HWND swnd = (HWND) lParam;
				int t = (int) SendMessage(swnd,TBM_GETPOS,0,0);
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
				{
					g_this->zoom_scale=t;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER6))
				{
					g_this->zoom_scale2=t;
          g_this->scale_fpos=t;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER2))
				{
					g_this->rot_dir=t;
				}
        if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER5))
        {
          g_this->beatch_speed=t;
        }
			}
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_ROTBLT,hwndParent,g_DlgProc);
}

#else
C_RBASE *R_RotBlit(char *desc)
{ return NULL; }
#endif