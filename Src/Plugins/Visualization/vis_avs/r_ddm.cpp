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
#define M_PI 3.14159265358979323846

#include <windows.h>
#include <commctrl.h>
#include <math.h>
#include "r_defs.h"
#include "resource.h"
#include "avs_eelif.h"

#include "timing.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

#define C_THISCLASS C_PulseClass
#define MOD_NAME "Trans / Dynamic Distance Modifier"


// Integer Square Root function

// Uses factoring to find square root
// A 256 entry table used to work out the square root of the 7 or 8 most
// significant bits.  A power of 2 used to approximate the rest.
// Based on an 80386 Assembly implementation by Arne Steinarson

static unsigned const char sq_table[]=
{0, 16, 22, 27, 32, 35, 39, 42, 45, 48, 50, 53, 55, 57, 59, 61, 64, 65, 
67, 69, 71, 73, 75, 76, 78, 80, 81, 83, 84, 86, 87, 89, 90, 91, 93, 94, 
96, 97, 98, 99, 101, 102, 103, 104, 106, 107, 108, 109, 110, 112, 113, 
114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 128, 
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 
142, 143, 144, 144, 145, 146, 147, 148, 149, 150, 150, 151, 152, 153, 
154, 155, 155, 156, 157, 158, 159, 160, 160, 161, 162, 163, 163, 164, 
165, 166, 167, 167, 168, 169, 170, 170, 171, 172, 173, 173, 174, 175, 
176, 176, 177, 178, 178, 179, 180, 181, 181, 182, 183, 183, 184, 185, 
185, 186, 187, 187, 188, 189, 189, 190, 191, 192, 192, 193, 193, 194, 
195, 195, 196, 197, 197, 198, 199, 199, 200, 201, 201, 202, 203, 203, 
204, 204, 205, 206, 206, 207, 208, 208, 209, 209, 210, 211, 211, 212, 
212, 213, 214, 214, 215, 215, 216, 217, 217, 218, 218, 219, 219, 220,
221, 221, 222, 222, 223, 224, 224, 225, 225, 226, 226, 227, 227, 228,
229, 229, 230, 230, 231, 231, 232, 232, 233, 234, 234, 235, 235, 236, 
236, 237, 237, 238, 238, 239, 240, 240, 241, 241, 242, 242, 243, 243, 
244, 244, 245, 245, 246, 246, 247, 247, 248, 248, 249, 249, 250, 250, 
251, 251, 252, 252, 253, 253, 254, 254, 255};

static __inline unsigned long isqrt(unsigned long n)
{
  if (n >= 0x10000)
    if (n >= 0x1000000)
      if (n >= 0x10000000)
        if (n >= 0x40000000) return(sq_table[n >> 24] << 8);
	else                 return(sq_table[n >> 22] << 7);
      else 
	if (n >= 0x4000000)  return(sq_table[n >> 20] << 6);
	else                 return(sq_table[n >> 18] << 5);
    else
      if (n >= 0x100000)
	if (n >= 0x400000)   return(sq_table[n >> 16] << 4);
	else                 return(sq_table[n >> 14] << 3);
      else 
	if (n >= 0x40000)    return(sq_table[n >> 12] << 2);
	else                 return(sq_table[n >> 10] << 1);

  else
   if (n >= 0x100)
      if (n >= 0x1000)
        if (n >= 0x4000)     return(sq_table[n >> 8]);
	else                 return(sq_table[n >> 6] >> 1);
      else 
	if (n >= 0x400)      return(sq_table[n >> 4] >> 2);
	else                 return(sq_table[n >> 2] >> 3);
    else
      if (n >= 0x10)
	if (n >= 0x40)       return(sq_table[n]      >> 4);
	else                 return(sq_table[n << 2] << 5);
      else 
	if (n >= 0x4)        return(sq_table[n >> 4] << 6);
	else                 return(sq_table[n >> 6] << 7);

}




class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_DYNAMIC_DISTANCE_MODIFIER,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
    RString effect_exp[4];
    int blend;

    int m_wt;
    int m_lastw,m_lasth;
    int *m_wmul;
    int *m_tab;
    NSEEL_VMCTX AVS_EEL_CONTEXTNAME;
    double *var_d, *var_b;
    double max_d;
		int inited;
    NSEEL_CODEHANDLE codehandle[4];
    int need_recompile;
    int subpixel;
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
    load_string(effect_exp[3],data,pos,len);
  }
  else
  {
    char buf[513];
    if (len-pos >= 256*2)
    {
      memcpy(buf,data+pos,256*2);
      pos+=256*2;
      buf[512]=0;
      effect_exp[1].assign(buf+256);
      buf[256]=0;
      effect_exp[0].assign(buf);
    }
    if (len-pos >= 256*2)
    {
      memcpy(buf,data+pos,256*2);
      pos+=256*2;
      buf[512]=0;
      effect_exp[3].assign(buf+256);
      buf[256]=0;
      effect_exp[2].assign(buf);
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
  save_string(data,pos,effect_exp[3]);
	PUT_INT(blend); pos+=4;
	PUT_INT(subpixel); pos+=4;
	return pos;
}



C_THISCLASS::C_THISCLASS()
{
  AVS_EEL_INITINST();
  InitializeCriticalSection(&rcs);
  need_recompile=1;
  memset(codehandle,0,sizeof(codehandle));
  m_lasth=m_lastw=0;
  m_wmul=0;
  m_tab=0;
  m_wt=0;
  effect_exp[0].assign("d=d-sigmoid((t-50)/100,2)");
  effect_exp[3].assign("u=1;t=0");
  effect_exp[1].assign("t=t+u;t=min(100,t);t=max(0,t);u=if(equal(t,100),-1,u);u=if(equal(t,0),1,u)");
  effect_exp[2].assign("");

  blend=0;
  subpixel=0;

  var_b=0;
}

C_THISCLASS::~C_THISCLASS()
{
  int x;
  for (x = 0; x < 4; x ++) 
  {
    freeCode(codehandle[x]);
    codehandle[x]=0;
  }
  if (m_wmul) GlobalFree(m_wmul);
  if (m_tab) GlobalFree(m_tab);
  AVS_EEL_QUITINST();

  m_tab=0;
  m_wmul=0;
  DeleteCriticalSection(&rcs);
}

	
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  int *fbin=framebuffer;
  if (m_lasth != h || m_lastw != w || !m_tab || !m_wmul)
  {
    int y;
    m_lastw=w; // jf 121100 - added (oops)
    m_lasth=h;
    max_d=sqrt((w*w+h*h)/4.0);
    if (m_wmul) GlobalFree(m_wmul);
    m_wmul=(int*)GlobalAlloc(GMEM_FIXED,sizeof(int)*h);
    for (y = 0; y < h; y ++) m_wmul[y]=y*w;
    if (m_tab) GlobalFree(m_tab);
    m_tab=0;
  }
  int imax_d=(int)(max_d + 32.9);

  if (imax_d < 33) imax_d=33;

  if (!m_tab)
    m_tab=(int*)GlobalAlloc(GMEM_FIXED,sizeof(int)*imax_d);

  int x;

  //pow(sin(d),dpos)*1.7
  if (need_recompile)
  {
    EnterCriticalSection(&rcs);
    if (!var_b || g_reset_vars_on_recompile)
    {
      clearVars();
      var_d = registerVar("d");
      var_b = registerVar("b");
      inited=0;
    }
    need_recompile=0;
    for (x = 0; x < 4; x ++) 
    {
      freeCode(codehandle[x]);
      codehandle[x]=compileCode(effect_exp[x].get());
    }
    LeaveCriticalSection(&rcs);
  }
  if (isBeat&0x80000000) return 0;

  *var_b=isBeat?1.0:0.0;

  if (codehandle[3] && !inited) { executeCode(codehandle[3],visdata); inited=1; }
  executeCode(codehandle[1],visdata);
  if (isBeat) executeCode(codehandle[2],visdata);
  if (codehandle[0]) 
  {
    for (x = 0; x < imax_d-32; x ++)
    {
      *var_d=x/(max_d-1);
      executeCode(codehandle[0],visdata);
      m_tab[x]=(int) (*var_d*256.0*max_d/(x+1));
    }
    for (; x < imax_d; x ++)
    {
      m_tab[x]=m_tab[x-1];
    }
  }
  else for (x = 0; x < imax_d; x ++) m_tab[x]=0;

  m_wt++;
  m_wt&=63;

  {
    int w2=w/2;
    int h2=h/2;
    int y;
    for (y = 0; y < h; y ++)
    {
      int ty=y-h2;
      int x2=w2*w2+w2+ty*ty+256;
      int dx2=-2*w2;
      int yysc=ty;
      int xxsc=-w2;
      int x=w;
      if (subpixel)
      {
        if (blend)
          while (x--)
          {
            int qd=m_tab[isqrt(x2)];
            int ow,oh;
            int xpart,ypart;
            x2+=dx2;
            dx2+=2;
            xpart=(qd*xxsc+128);
            ypart=(qd*yysc+128);
            ow = w2 + (xpart>>8);
            oh = h2 + (ypart>>8);
            xpart&=0xff;
            ypart&=0xff;
            xxsc++;
        
            if (ow < 0) ow=0;
            else if (ow >= w-1) ow=w-2;
            if (oh < 0) oh=0;
            else if (oh >= h-1) oh=h-2;

            *fbout++=BLEND_AVG(BLEND4((unsigned int *)framebuffer+ow+m_wmul[oh],w,xpart,ypart),*fbin++);
          }
        else
          while (x--)
          {
            int qd=m_tab[isqrt(x2)];
            int ow,oh;
            int xpart,ypart;
            x2+=dx2;
            dx2+=2;
            xpart=(qd*xxsc+128);
            ypart=(qd*yysc+128);
            ow = w2 + (xpart>>8);
            oh = h2 + (ypart>>8);
            xpart&=0xff;
            ypart&=0xff;
            xxsc++;
        
            if (ow < 0) ow=0;
            else if (ow >= w-1) ow=w-2;
            if (oh < 0) oh=0;
            else if (oh >= h-1) oh=h-2;

            *fbout++=BLEND4((unsigned int *)framebuffer+ow+m_wmul[oh],w,xpart,ypart);
          }
      }
      else
      {
        if (blend)
          while (x--)
          {
            int qd=m_tab[isqrt(x2)];
            int ow,oh;
            x2+=dx2;
            dx2+=2;
            ow = w2 + ((qd*xxsc+128)>>8);
            xxsc++;
            oh = h2 + ((qd*yysc+128)>>8);
        
            if (ow < 0) ow=0;
            else if (ow >= w) ow=w-1;
            if (oh < 0) oh=0;
            else if (oh >= h) oh=h-1;

            *fbout++=BLEND_AVG(framebuffer[ow+m_wmul[oh]],*fbin++);
          }
        else
          while (x--)
          {
            int qd=m_tab[isqrt(x2)];
            int ow,oh;
            x2+=dx2;
            dx2+=2;
            ow = w2 + ((qd*xxsc+128)>>8);
            xxsc++;
            oh = h2 + ((qd*yysc+128)>>8);
        
            if (ow < 0) ow=0;
            else if (ow >= w) ow=w-1;
            if (oh < 0) oh=0;
            else if (oh >= h) oh=h-1;

            *fbout++=framebuffer[ow+m_wmul[oh]];
          }
      }
    }
  }
#ifndef NO_MMX
  if (subpixel) __asm emms;
#endif
  return 1;
}

C_RBASE *R_DDM(char *desc)
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
      SetDlgItemText(hwndDlg,IDC_EDIT4,g_this->effect_exp[3].get());
      isstart=0;
      if (g_this->blend)
        CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
      if (g_this->subpixel)
        CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);
    return 1;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDC_BUTTON1) 
      {
/*		char text[4096];
		WASABI_API_LNGSTRING_BUF(IDS_DYNAMIC_DISTANCE_MODIFIER,text,4096);
		int titlelen = lstrlen(text)+1;
		lstrcpyn(text+titlelen,GetTextResource(IDR_DYNAMIC_DISTANCE_MODIFIER),4095-titlelen);
*/
        char *text="Dynamic Distance Modifier\0"
          "The dynamic distance modifier allows you to dynamically (once per frame)\r\n"
          "change the source pixels for each ring of pixels out from the center.\r\n"
          "In the 'pixel' code section, 'd' represents the distance in pixels\r\n"
          "the current ring is from the center, and code can modify it to\r\n"
          "change the distance from the center where the source pixels for\r\n"
          "that ring would be read. This is a terrible explanation, and if\r\n"
          "you want to make a better one send it to me. \r\n"
          "\r\n"
          "Examples:\r\n"
          "Zoom in: 'd=d*0.9'\r\n"
          "Zoom out: 'd=d*1.1'\r\n"
          "Back and forth: pixel='d=d*(1.0+0.1*cos(t));', frame='t=t+0.1'\r\n"
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
      if (!isstart && (LOWORD(wParam) == IDC_EDIT1||LOWORD(wParam) == IDC_EDIT2||LOWORD(wParam) == IDC_EDIT3||LOWORD(wParam) == IDC_EDIT4) && HIWORD(wParam) == EN_CHANGE)
      {
        EnterCriticalSection(&g_this->rcs);
        g_this->effect_exp[0].get_from_dlgitem(hwndDlg,IDC_EDIT1);
        g_this->effect_exp[1].get_from_dlgitem(hwndDlg,IDC_EDIT2);
        g_this->effect_exp[2].get_from_dlgitem(hwndDlg,IDC_EDIT3);
        g_this->effect_exp[3].get_from_dlgitem(hwndDlg,IDC_EDIT4);
        g_this->need_recompile=1;
				if (LOWORD(wParam) == IDC_EDIT4) g_this->inited = 0;
        LeaveCriticalSection(&g_this->rcs);
      }
    return 0;
  }
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_DDM,hwndParent,g_DlgProc);
}
#else
C_RBASE *R_DDM(char *desc) { return NULL; }
#endif