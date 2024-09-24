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
#include <math.h>
#include "r_defs.h"
#include "resource.h"
#include "avs_eelif.h"

#include "timing.h"
#include "../Agave/Language/api_language.h"


#ifndef LASER

#define C_THISCLASS C_ShiftClass
#define MOD_NAME "Trans / Dynamic Shift"


class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_DYNAMIC_SHIFT,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

    RString effect_exp[3];
    int blend,subpixel;

    int m_lastw,m_lasth;
    NSEEL_VMCTX AVS_EEL_CONTEXTNAME;
    double *var_x, *var_y, *var_w, *var_h, *var_b, *var_alpha;
    double max_d;
		int inited;
    NSEEL_CODEHANDLE codehandle[3];
    int need_recompile;
    CRITICAL_SECTION rcs;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
  if (data[pos] == 1)
  {
    pos++;
    load_string(effect_exp[0],data,pos,len);
    load_string(effect_exp[1],data,pos,len);
    load_string(effect_exp[2],data,pos,len);
  }
  else
  {
    char buf[769];
    if (len-pos >= 768)
    {
      memcpy(buf,data+pos,768);
      pos+=768;
      buf[768]=0;
      effect_exp[2].assign(buf+512);
      buf[512]=0;
      effect_exp[1].assign(buf+256);
      buf[256]=0;
      effect_exp[0].assign(buf);
    }
  }
	if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
	if (len-pos >= 4) { subpixel=GET_INT(); pos+=4; }

}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
  data[pos++]=1;
  save_string(data,pos,effect_exp[0]);
  save_string(data,pos,effect_exp[1]);
  save_string(data,pos,effect_exp[2]);
	PUT_INT(blend); pos+=4;
	PUT_INT(subpixel); pos+=4;
	return pos;
}



C_THISCLASS::C_THISCLASS()
{
  InitializeCriticalSection(&rcs);
  AVS_EEL_INITINST();

  memset(codehandle,0,sizeof(codehandle));
  m_lasth=m_lastw=0;

  effect_exp[0].assign("d=0;"); // init
  effect_exp[1].assign("x=sin(d)*1.4; y=1.4*cos(d); d=d+0.01;"); // frame
  effect_exp[2].assign("d=d+2.0");
  blend=0;
  subpixel=1;

  need_recompile=1;
  var_b=0;
}

C_THISCLASS::~C_THISCLASS()
{
  int x;
  for (x = 0; x < 3; x ++) 
  {
    freeCode(codehandle[x]);
  }
  AVS_EEL_QUITINST();

  DeleteCriticalSection(&rcs);
}

	
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  //pow(sin(d),dpos)*1.7
  if (need_recompile)
  {
    int err=0;
    int x;
    EnterCriticalSection(&rcs);
    if (!var_b || g_reset_vars_on_recompile)
    {
      clearVars();
      var_x = registerVar("x");
      var_y = registerVar("y");
      var_w = registerVar("w");
      var_h = registerVar("h");
      var_b = registerVar("b");
      var_alpha = registerVar("alpha");
      inited=0;
    }
    need_recompile=0;
    for (x = 0; x < 3; x ++) 
    {
      freeCode(codehandle[x]);
      codehandle[x]=compileCode(effect_exp[x].get());
    }
    LeaveCriticalSection(&rcs);
  }
  *var_w=w;
  *var_h=h;
  *var_b=isBeat?1.0:0.0;

  if (isBeat&0x80000000) return 0;

  if (codehandle[0] && (!inited || m_lasth != h || m_lastw != w)) 
  { 
    m_lastw=w;
    m_lasth=h;
    *var_x=0; *var_y=0; *var_alpha=0.5; 
    executeCode(codehandle[0],visdata); 
    inited=1; 
  }

  executeCode(codehandle[1],visdata);
  if (isBeat) executeCode(codehandle[2],visdata);

  int doblend=blend;
  int ialpha=127;
  if (doblend)
  {
    ialpha=(int)(*var_alpha*255.0);
    if (ialpha <= 0) return 0;
    if (ialpha >= 255) doblend=0;
  }
  int *inptr=framebuffer;
  int *blendptr=framebuffer;
  int *outptr=fbout;
  int xa=(int)*var_x;
  int ya=(int)*var_y;
  
  // var_x, var_y at this point tell us how to shift, and blend also tell us what to do.
  if (!subpixel) 
  {
    int endy=h+ya;
    int endx=w+xa;
    int x,y;
    if (endx > w) endx=w;
    if (endy > h) endy=h;
    if (ya < 0) inptr += -ya*w;
    if (ya > h) ya=h;
    if (xa > w) xa=w;
    for (y = 0; y < ya; y ++)
    {
      x=w; 
      if (!doblend) while (x--) *outptr++ = 0;
      else while (x--) *outptr++ = BLEND_ADJ(0,*blendptr++,ialpha);
    }
    for (; y < endy; y ++)
    {
      int *ip=inptr;
      if (xa < 0) inptr += -xa;
      if (!doblend)
      {
        for (x = 0; x < xa; x ++) *outptr++=0;
        for (; x < endx; x ++) *outptr++=*inptr++;
        for (; x < w; x ++) *outptr++=0;
      }
      else
      {
        for (x = 0; x < xa; x ++) *outptr++ = BLEND_ADJ(0,*blendptr++,ialpha);
        for (; x < endx; x ++) *outptr++ = BLEND_ADJ(*inptr++,*blendptr++,ialpha);
        for (; x < w; x ++) *outptr++ = BLEND_ADJ(0,*blendptr++,ialpha);
      }
      inptr=ip+w;
    }
    for (; y < h; y ++)
    {   
      x=w; 
      if (!doblend) while (x--) *outptr++ = 0;
      else while (x--) *outptr++ = BLEND_ADJ(0,*blendptr++,ialpha);
    }
  }
  else // bilinear filtering version
  {
    
    int xpart,ypart;

    {
      double vx=*var_x;
      double vy=*var_y;
      xpart=(int) ((vx - (int)vx)*255.0);
      if (xpart < 0) xpart=-xpart;
      else { xa++; xpart=255-xpart; }
      if (xpart < 0) xpart=0;
      if (xpart > 255) xpart=255;

      ypart=(int) ((vy - (int)vy)*255.0);
      if (ypart < 0) ypart=-ypart;
      else { ya++; ypart=255-ypart; }
      if (ypart < 0) ypart=0;
      if (ypart > 255) ypart=255;
    }

    int x,y;
    if (ya < 1-h) ya=1-h;
    if (xa < 1-w) xa=1-w;
    if (ya > h-1) ya=h-1;
    if (xa > w-1) xa=w-1;
    if (ya < 0) inptr += -ya*w;
    int endy=h-1+ya;
    int endx=w-1+xa;
    if (endx > w-1) endx=w-1;
    if (endy > h-1) endy=h-1;
    if (endx < 0) endx=0;
    if (endy < 0) endy=0;
    for (y = 0; y < ya; y ++)
    {
      x=w; 
      if (!doblend) while (x--) *outptr++ = 0;
      else while (x--) *outptr++ = BLEND_ADJ(0,*blendptr++,ialpha);
    }
    for (; y < endy; y ++)
    {
      int *ip=inptr;
      if (xa < 0) inptr += -xa;
      if (!doblend)
      {
        for (x = 0; x < xa; x ++) *outptr++=0;
        for (; x < endx; x ++) *outptr++=BLEND4((unsigned int *)inptr++,w,xpart,ypart);
        for (; x < w; x ++) *outptr++=0;
      }
      else
      {
        for (x = 0; x < xa; x ++) *outptr++ = BLEND_ADJ(0,*blendptr++,ialpha);
        for (; x < endx; x ++) *outptr++ = BLEND_ADJ(BLEND4((unsigned int *)inptr++,w,xpart,ypart),*blendptr++,ialpha);
        for (; x < w; x ++) *outptr++ = BLEND_ADJ(0,*blendptr++,ialpha);
      }
      inptr=ip+w;
    }
    for (; y < h; y ++)
    {   
      x=w; 
      if (!doblend) while (x--) *outptr++ = 0;
      else while (x--) *outptr++ = BLEND_ADJ(0,*blendptr++,ialpha);
    }
  }
  #ifndef NO_MMX
    __asm emms;
  #endif

  return 1;
}

C_RBASE *R_Shift(char *desc)
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}


static C_THISCLASS *g_this;
static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  static int isstart;
	switch (uMsg)
	{
		case WM_INITDIALOG:
      isstart=1;
      SetDlgItemText(hwndDlg,IDC_EDIT1,g_this->effect_exp[0].get());
      SetDlgItemText(hwndDlg,IDC_EDIT2,g_this->effect_exp[1].get());
      SetDlgItemText(hwndDlg,IDC_EDIT3,g_this->effect_exp[2].get());
      isstart=0;
      if (g_this->blend)
        CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
      if (g_this->subpixel)
        CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);
    return 1;
    case WM_COMMAND:
  	  if (LOWORD(wParam) == IDC_HELPBTN)
	  	{
        char *text="Dynamic Shift\0"
          "better Dynamic shift help goes here (send me some :)\r\n";
          "Variables:\r\n"
           "x,y = amount to shift (in pixels - set these)\r\n"
           "w,h = width, height (in pixels)\r\n"
           "b = isBeat\r\n"
           "alpha = alpha value (0.0-1.0) for blend\r\n"
         ;
        compilerfunctionlist(hwndDlg,text);
      }
      if (LOWORD(wParam)==IDC_CHECK1)
      {
        g_this->blend=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
      }
      if (LOWORD(wParam)==IDC_CHECK2)
      {
        g_this->subpixel=IsDlgButtonChecked(hwndDlg,IDC_CHECK2)?1:0;
      }
      if (!isstart && (LOWORD(wParam) == IDC_EDIT1||LOWORD(wParam) == IDC_EDIT2||LOWORD(wParam) == IDC_EDIT3) && HIWORD(wParam) == EN_CHANGE)
      {
        EnterCriticalSection(&g_this->rcs);
        g_this->effect_exp[0].get_from_dlgitem(hwndDlg,IDC_EDIT1);
        g_this->effect_exp[1].get_from_dlgitem(hwndDlg,IDC_EDIT2);
        g_this->effect_exp[2].get_from_dlgitem(hwndDlg,IDC_EDIT3);
        g_this->need_recompile=1;
				if (LOWORD(wParam) == IDC_EDIT1) g_this->inited = 0;
        LeaveCriticalSection(&g_this->rcs);
      }
    return 0;
  }
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_SHIFT,hwndParent,g_DlgProc);
}

#else
C_RBASE *R_Shift(char *desc)
{
  return NULL;
}
#endif