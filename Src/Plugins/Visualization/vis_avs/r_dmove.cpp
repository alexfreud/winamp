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
#include "r_list.h"

#include "timing.h"
#include "../Agave/Language/api_language.h"


#if 0 
static void __docheck(int xp, int yp, int m_lastw, int m_lasth, int d_x, int d_y)
{
  xp >>= 16;
  yp >>= 16;
  if (xp < 0 || xp >= m_lastw || yp < 0 || yp >= m_lasth)
  {
    char buf[512];
    wsprintf(buf,"@ %d,%d on %d,%d (dx,dy=%d,%d)\n",xp,yp,m_lastw,m_lasth,d_x,d_y);
    OutputDebugString(buf);
  }
}
#endif


#ifndef LASER

#define C_THISCLASS C_DMoveClass
#define MOD_NAME "Trans / Dynamic Movement"

class C_THISCLASS : public C_RBASE2 {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);

    virtual int smp_getflags() { return 1; }
		virtual int smp_begin(int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); 
    virtual void smp_render(int this_thread, int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); 
    virtual int smp_finish(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); // return value is that of render() for fbstuff etc

		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_DYNAMIC_MOVEMENT,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
    RString effect_exp[4];

    int m_lastw,m_lasth;
    int m_lastxres, m_lastyres, m_xres, m_yres;
    int *m_wmul;
    int *m_tab;
    NSEEL_VMCTX AVS_EEL_CONTEXTNAME;
    double *var_d, *var_b, *var_r, *var_x, *var_y, *var_w, *var_h, *var_alpha;
		int inited;
    NSEEL_CODEHANDLE codehandle[4];
    int need_recompile;
    int buffern;
    int subpixel,rectcoords,blend,wrap, nomove;
    CRITICAL_SECTION rcs;


    // smp stuff
    int __subpixel,__rectcoords,__blend,__wrap, __nomove;
    int w_adj;
    int h_adj;
    int XRES;
    int YRES;

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
    char buf[1025];
    if (len-pos >= 1024)
    {
      memcpy(buf,data+pos,1024);
      pos+=1024;
      buf[1024]=0;
      effect_exp[3].assign(buf+768);
      buf[768]=0;
      effect_exp[2].assign(buf+512);
      buf[512]=0;
      effect_exp[1].assign(buf+256);
      buf[256]=0;
      effect_exp[0].assign(buf);
    }
  }
	if (len-pos >= 4) { subpixel=GET_INT(); pos+=4; }
	if (len-pos >= 4) { rectcoords=GET_INT(); pos+=4; }
	if (len-pos >= 4) { m_xres=GET_INT(); pos+=4; }
	if (len-pos >= 4) { m_yres=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
	if (len-pos >= 4) { wrap=GET_INT(); pos+=4; }
	if (len-pos >= 4) { buffern=GET_INT(); pos+=4; }
  else buffern=0;
	if (len-pos >= 4) { nomove=GET_INT(); pos+=4; }
  else nomove=0;

}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
  data[pos++]=1;
  save_string(data,pos,effect_exp[0]);
  save_string(data,pos,effect_exp[1]);
  save_string(data,pos,effect_exp[2]);
  save_string(data,pos,effect_exp[3]);
	PUT_INT(subpixel); pos+=4;
	PUT_INT(rectcoords); pos+=4;
	PUT_INT(m_xres); pos+=4;
	PUT_INT(m_yres); pos+=4;
	PUT_INT(blend); pos+=4;
	PUT_INT(wrap); pos+=4;
	PUT_INT(buffern); pos+=4;
	PUT_INT(nomove); pos+=4;
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
  effect_exp[0].assign("");
  effect_exp[1].assign("");
  effect_exp[2].assign("");
  effect_exp[3].assign("");

  m_lastxres=m_lastyres=0;
  m_xres=16;
  m_yres=16;
  var_b=0;
  subpixel=1;
  rectcoords=0;
  blend=0;
  wrap=0;
  buffern=0;
  nomove=0;
}

C_THISCLASS::~C_THISCLASS()
{
  int x;
  for (x = 0; x < 4; x ++) 
  {
    freeCode(codehandle[x]);
    codehandle[x]=0;
  }
  AVS_EEL_QUITINST();
  if (m_wmul) GlobalFree(m_wmul);
  if (m_tab) GlobalFree(m_tab);

  m_tab=0;
  m_wmul=0;
  DeleteCriticalSection(&rcs);
}

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  smp_begin(1,visdata,isBeat,framebuffer,fbout,w,h);
  if (isBeat & 0x80000000) return 0;

  smp_render(0,1,visdata,isBeat,framebuffer,fbout,w,h);
  return smp_finish(visdata,isBeat,framebuffer,fbout,w,h);
}

int C_THISCLASS::smp_finish(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h) // return value is that of render() for fbstuff etc
{
  return !__nomove;
}

int C_THISCLASS::smp_begin(int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  __subpixel=subpixel;
  __rectcoords=rectcoords;
  __blend=blend;
  __wrap=wrap;
  __nomove=nomove;

  w_adj=(w-2)<<16;
  h_adj=(h-2)<<16;
  XRES=m_xres+1;
  YRES=m_yres+1;

  if (XRES < 2) XRES=2;
  if (XRES > 256) XRES=256;
  if (YRES < 2) YRES=2;
  if (YRES > 256) YRES=256;

  if (m_lasth != h || m_lastw != w || !m_tab || !m_wmul || 
    m_lastxres != XRES || m_lastyres != YRES)
  {
    int y;
    m_lastxres = XRES;
    m_lastyres = YRES;
    m_lastw=w;
    m_lasth=h;
    if (m_wmul) GlobalFree(m_wmul);
    m_wmul=(int*)GlobalAlloc(GMEM_FIXED,sizeof(int)*h);
    for (y = 0; y < h; y ++) m_wmul[y]=y*w;
    if (m_tab) GlobalFree(m_tab);

    m_tab=(int*)GlobalAlloc(GMEM_FIXED,(XRES*YRES*3 + (XRES*6 + 6)*MAX_SMP_THREADS)*sizeof(int));
  }

  if (!__subpixel)
  {
    w_adj=(w-1)<<16;
    h_adj=(h-1)<<16;
  }

  if (need_recompile)
  {
    int x;
    int err=0;
    EnterCriticalSection(&rcs);
    if (!var_b || g_reset_vars_on_recompile)
    {
      clearVars();
      var_d = registerVar("d");
      var_b = registerVar("b");
      var_r = registerVar("r");
      var_x = registerVar("x");
      var_y = registerVar("y");
      var_w = registerVar("w");
      var_h = registerVar("h");
      var_alpha = registerVar("alpha");
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
  int *fbin = !buffern ? framebuffer : (int *)getGlobalBuffer(w,h,buffern-1,0);
	if (!fbin) return 0;

  *var_w=(double)w;
  *var_h=(double)h;
  *var_b=isBeat?1.0:0.0;
  *var_alpha=0.5;
  if (codehandle[3] && !inited) { executeCode(codehandle[3],visdata); inited=1; }
  executeCode(codehandle[1],visdata);
  if (isBeat) executeCode(codehandle[2],visdata);
  {
    int x;
    int y;
    int *tabptr=m_tab;

    double xsc=2.0/w,ysc=2.0/h;
    double dw2=((double)w*32768.0);
    double dh2=((double)h*32768.0);
    double max_screen_d=sqrt((double)(w*w+h*h))*0.5;
    
    double divmax_d=1.0/max_screen_d;

    max_screen_d *= 65536.0;

    int yc_pos, yc_dpos, xc_pos, xc_dpos;
    yc_pos=0;
    xc_dpos = (w<<16)/(XRES-1);
    yc_dpos = (h<<16)/(YRES-1);
    for (y = 0; y < YRES; y ++)
    {
      xc_pos=0;
      for (x = 0; x < XRES; x ++)
      {
        double xd,yd;
        
        xd=((double)xc_pos-dw2)*(1.0/65536.0);
        yd=((double)yc_pos-dh2)*(1.0/65536.0);

        xc_pos+=xc_dpos;

        *var_x=xd*xsc;
        *var_y=yd*ysc;
        *var_d=sqrt(xd*xd+yd*yd)*divmax_d;
        *var_r=atan2(yd,xd) + M_PI*0.5;

        executeCode(codehandle[0],visdata);

        int tmp1,tmp2;
        if (!__rectcoords)
        {
          *var_d *= max_screen_d;
          *var_r -= M_PI*0.5;
          tmp1=(int) (dw2 + cos(*var_r) * *var_d);
          tmp2=(int) (dh2 + sin(*var_r) * *var_d);
        }
        else
        {
          tmp1=(int) ((*var_x+1.0)*dw2);
          tmp2=(int) ((*var_y+1.0)*dh2);
        }
        if (!__wrap)
        {
          if (tmp1 < 0) tmp1=0;
          if (tmp1 > w_adj) tmp1=w_adj;
          if (tmp2 < 0) tmp2=0;
          if (tmp2 > h_adj) tmp2=h_adj;
        }
        *tabptr++ = tmp1;
        *tabptr++ = tmp2;
        double va=*var_alpha;
        if (va < 0.0) va=0.0;
        else if (va > 1.0) va=1.0;
        int a=(int)(va*255.0*65536.0);
        *tabptr++ = a;
      }
      yc_pos+=yc_dpos;
    }
  }

  return max_threads;
}


void C_THISCLASS::smp_render(int this_thread, int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (max_threads < 1) max_threads=1;

  int start_l = ( this_thread * h ) / max_threads;
  int end_l;
  int ypos=0;

  if (this_thread >= max_threads - 1) end_l = h;
  else end_l = ( (this_thread+1) * h ) / max_threads;  

  int outh=end_l-start_l;
  if (outh<1) return;

  int *fbin = !buffern ? framebuffer : (int *)getGlobalBuffer(w,h,buffern-1,0);
	if (!fbin) return;
  // yay, the table is generated. now we do a fixed point 
  // interpolation of the whole thing and pray.

  {
    int *interptab=m_tab+XRES*YRES*3 + this_thread * (XRES*6+6);
    int *rdtab=m_tab;
    unsigned int *in=(unsigned int *)fbin;
    unsigned int *blendin=(unsigned int *)framebuffer;
    unsigned int *out=(unsigned int *)fbout;
    int yseek=1;
    int xc_dpos, yc_pos=0, yc_dpos;
    xc_dpos=(w<<16)/(XRES-1);
    yc_dpos=(h<<16)/(YRES-1);
    int lypos=0;
    int yl=end_l;
    while (yl>0)
    {
      yc_pos+=yc_dpos;   
      yseek=(yc_pos>>16)-lypos;
      if (!yseek) 
      {
        #ifndef NO_MMX
          __asm emms;
        #endif
        return;
      }
      lypos=yc_pos>>16;
      int l=XRES;
      int *stab=interptab;
      int xr3=XRES*3;
      while (l--)
      {
        int tmp1, tmp2,tmp3;
        tmp1=rdtab[0];
        tmp2=rdtab[1];
        tmp3=rdtab[2];
        stab[0]=tmp1;
        stab[1]=tmp2;
        stab[2]=(rdtab[xr3]-tmp1)/yseek;
        stab[3]=(rdtab[xr3+1]-tmp2)/yseek;
        stab[4]=tmp3;
        stab[5]=(rdtab[xr3+2]-tmp3)/yseek;
        rdtab+=3;
        stab+=6;
      }

      if (yseek > yl) yseek=yl;
      yl-=yseek;

      if (yseek > 0) while (yseek--)
      {
        int d_x;
        int d_y;
        int d_a;
        int ap;
        int seek;
        int *seektab=interptab;
        int xp,yp;
        int l=w;
        int lpos=0;
        int xc_pos=0;
        ypos++;
        {
          while (l>0)
          {
            xc_pos+=xc_dpos;
            seek=(xc_pos>>16)-lpos;
            if (!seek) 
            {
              #ifndef NO_MMX
                __asm emms;
              #endif
              return;
            }
            lpos=xc_pos>>16;
            xp=seektab[0];
            yp=seektab[1];
            ap=seektab[4];
            d_a=(seektab[10]-ap)/(seek);
            d_x=(seektab[6]-xp)/(seek);
            d_y=(seektab[7]-yp)/(seek);
            seektab[0] += seektab[2];
            seektab[1] += seektab[3];
            seektab[4] += seektab[5];
            seektab+=6;
        
            if (seek>l) seek=l;
            l-=seek;
            if (seek > 0 && ypos <= start_l)
            {
              blendin+=seek;
              if (__nomove) in+=seek;
              else out+=seek;

              seek=0;
            }
            if (seek>0)
            {

  #define CHECK
              //__docheck(xp,yp,m_lastw,m_lasth,d_x,d_y);
  // normal loop
  #define NORMAL_LOOP(Z) while ((seek--)) { Z; xp+=d_x; yp+=d_y; }

  #if 0 
              // this would be faster, but seems like it might be less reliable:
  #define WRAPPING_LOOPS(Z) \
    if (d_x <= 0 && d_y <= 0) NORMAL_LOOP(if (xp < 0) xp += w_adj; if (yp < 0) yp += h_adj; Z) \
    else if (d_x <= 0) NORMAL_LOOP(if (xp < 0) xp += w_adj; if (yp >= h_adj) yp-=h_adj; Z) \
    else if (d_y <= 0) NORMAL_LOOP(if (xp >= w_adj) xp-=w_adj; if (yp < 0) yp += h_adj; Z) \
    else NORMAL_LOOP(if (xp >= w_adj) xp-=w_adj; if (yp >= h_adj) yp-=h_adj; Z)

  #define CLAMPED_LOOPS(Z) \
    if (d_x <= 0 && d_y <= 0) NORMAL_LOOP(if (xp < 0) xp=0; if (yp < 0) yp=0; Z) \
    else if (d_x <= 0) NORMAL_LOOP(if (xp < 0) xp=0; if (yp >= h_adj) yp=h_adj-1; Z) \
    else if (d_y <= 0) NORMAL_LOOP(if (xp >= w_adj) xp=w_adj-1; if (yp < 0) yp=0; Z) \
    else NORMAL_LOOP(if (xp >= w_adj) xp=w_adj-1; if (yp >= h_adj) yp=h_adj-1; Z)

  #else     // slower, more reliable loops

  // wrapping loop
  #define WRAPPING_LOOPS(Z) \
    NORMAL_LOOP(if (xp < 0) xp += w_adj;  \
                else if (xp >= w_adj) xp-=w_adj;  \
                if (yp < 0) yp += h_adj;  \
                else if (yp >= h_adj) yp-=h_adj;  \
                Z)

  #define CLAMPED_LOOPS(Z) \
    NORMAL_LOOP(if (xp < 0) xp=0; \
                else if (xp >= w_adj) xp=w_adj-1; \
                if (yp < 0) yp=0; \
                else if (yp >= h_adj) yp=h_adj-1; \
                Z)

  #endif

  #define LOOPS(DO)  \
                if (__blend && __subpixel) DO(CHECK *out++=BLEND_ADJ(BLEND4_16(in+(xp>>16)+(m_wmul[yp>>16]),w,xp,yp),*blendin++,ap>>16); ap+=d_a) \
                else if (__blend) DO(CHECK *out++=BLEND_ADJ(in[(xp>>16)+(m_wmul[yp>>16])],*blendin++,ap>>16); ap+=d_a) \
                else if (__subpixel) DO(CHECK *out++=BLEND4_16(in+(xp>>16)+(m_wmul[yp>>16]),w,xp,yp)) \
                else DO(CHECK *out++=in[(xp>>16)+(m_wmul[yp>>16])])

              if (__nomove)
              {
                if (fbin != framebuffer) while (seek--)
                {
                  *blendin=BLEND_ADJ(*in++,*blendin,ap>>16); ap+=d_a;
                  blendin++;
                }
                else while (seek--)
                {
                  *blendin=BLEND_ADJ(0,*blendin,ap>>16); ap+=d_a;
                  blendin++;
                }
              }
              else if (!__wrap)
              {
                // this might not really be necessary b/c of the clamping in the loop, but I'm sick of crashes
                if (xp < 0) xp=0;
                else if (xp >= w_adj) xp=w_adj-1;
                if (yp < 0) yp=0;
                else if (yp >= h_adj) yp=h_adj-1;

                LOOPS(CLAMPED_LOOPS)
              }
              else // __wrap
              {
                xp %= w_adj; 
                yp %= h_adj;
                if (xp < 0) xp+=w_adj; 
                if (yp < 0) yp+=h_adj;

                if (d_x <= -w_adj) d_x=-w_adj+1;
                else if (d_x >= w_adj) d_x=w_adj-1; 

                if (d_y <= -h_adj) d_y=-h_adj+1;
                else if (d_y >= h_adj) d_y=h_adj-1;

                LOOPS(WRAPPING_LOOPS)
              }
            } // if seek>0
          }
          // adjust final (rightmost elem) part of seektab
          seektab[0] += seektab[2];
          seektab[1] += seektab[3];
          seektab[4] += seektab[5];
        }
      }
    }
  }


#ifndef NO_MMX
  __asm emms;
#endif
  
}

C_RBASE *R_DMove(char *desc)
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}

typedef struct 
{
  char *name;
  int rect;
  int wrap;
  int grid1;
  int grid2;
  char *init;
  char *point;
  char *frame;
  char *beat;
} presetType;

static presetType presets[]=
{
  {"Random Rotate", 0, 1, 2, 2, "","r = r + dr;","","dr = (rand(100) / 100) * $PI;\r\nd = d * .95;"},
  {"Random Direction", 1, 1, 2, 2, "speed=.05;dr = (rand(200) / 100) * $PI;","x = x + dx;\r\ny = y + dy;","dx = cos(dr) * speed;\r\ndy = sin(dr) * speed;","dr = (rand(200) / 100) * $PI;"},
  {"In and Out", 0, 1, 2, 2, "speed=.2;c=0;","d = d * dd;","","c = c + ($PI/2);\r\ndd = 1 - (sin(c) * speed);"},
  {"Unspun Kaleida", 0, 1, 33, 33, "c=200;f=0;dt=0;dl=0;beatdiv=8","r=cos(r*dr);","f = f + 1;\r\nt = ((f * $pi * 2)/c)/beatdiv;\r\ndt = dl + t;\r\ndr = 4+(cos(dt)*2);","c=f;f=0;dl=dt"},
  {"Roiling Gridley", 1, 1, 32, 32, "c=200;f=0;dt=0;dl=0;beatdiv=8","x=x+(sin(y*dx) * .03);\r\ny=y-(cos(x*dy) * .03);","f = f + 1;\r\nt = ((f * $pi * 2)/c)/beatdiv;\r\ndt = dl + t;\r\ndx = 14+(cos(dt)*8);\r\ndy = 10+(sin(dt*2)*4);","c=f;f=0;dl=dt"},
  {"6-Way Outswirl", 0, 0, 32, 32, "c=200;f=0;dt=0;dl=0;beatdiv=8","d=d*(1+(cos(r*6) * .05));\r\nr=r-(sin(d*dr) * .05);\r\nd = d * .98;","f = f + 1;\r\nt = ((f * $pi * 2)/c)/beatdiv;\r\ndt = dl + t;\r\ndr = 18+(cos(dt)*12);","c=f;f=0;dl=dt"},
  {"Wavy", 1, 1, 6, 6, "c=200;f=0;dx=0;dl=0;beatdiv=16;speed=.05","y = y + ((sin((x+dx) * $PI))*speed);\r\nx = x + .025","f = f + 1;\r\nt = ( (f * 2 * 3.1415) / c ) / beatdiv;\r\ndx = dl + t;","c = f;\r\nf = 0;\r\ndl = dx;"},
  {"Smooth Rotoblitter", 0, 1, 2, 2, "c=200;f=0;dt=0;dl=0;beatdiv=4;speed=.15","r = r + dr;\r\nd = d * dd;","f = f + 1;\r\nt = ((f * $pi * 2)/c)/beatdiv;\r\ndt = dl + t;\r\ndr = cos(dt)*speed*2;\r\ndd = 1 - (sin(dt)*speed);","c=f;f=0;dl=dt"},
};


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
      if (g_this->blend)
        CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
      if (g_this->subpixel)
        CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);
      if (g_this->rectcoords)
        CheckDlgButton(hwndDlg,IDC_CHECK3,BST_CHECKED);
      if (g_this->wrap)
        CheckDlgButton(hwndDlg,IDC_WRAP,BST_CHECKED);
      if (g_this->nomove)
        CheckDlgButton(hwndDlg,IDC_NOMOVEMENT,BST_CHECKED);

      SendDlgItemMessage(hwndDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRING(IDS_CURRENT));
  		{
				int i=0;
				char txt[64];
				for (i=0;i<NBUF;i++)
        {
					 wsprintf(txt, WASABI_API_LNGSTRING(IDS_BUFFER_X), i+1);
					 SendDlgItemMessage(hwndDlg, IDC_COMBO1, CB_ADDSTRING, 0, (int)txt);
				}
      }
		  SendDlgItemMessage(hwndDlg, IDC_COMBO1, CB_SETCURSEL, (WPARAM) g_this->buffern, 0);

      SetDlgItemInt(hwndDlg,IDC_EDIT5,g_this->m_xres,FALSE);
      SetDlgItemInt(hwndDlg,IDC_EDIT6,g_this->m_yres,FALSE);
      isstart=0;

    return 1;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDC_BUTTON1) 
      {
        char text[4096];
	WASABI_API_LNGSTRING_BUF(IDS_DYNAMIC_MOVEMENT,text,4096);
	int titlelen = lstrlen(text)+1;
	lstrcpyn(text+titlelen,GetTextResource(IDR_DYNAMIC_MOVEMENT),4095-titlelen);
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
      if (LOWORD(wParam)==IDC_CHECK3)
      {
        g_this->rectcoords=IsDlgButtonChecked(hwndDlg,IDC_CHECK3)?1:0;
      }
      if (LOWORD(wParam)==IDC_WRAP)
      {
        g_this->wrap=IsDlgButtonChecked(hwndDlg,IDC_WRAP)?1:0;
      }
      if (LOWORD(wParam)==IDC_NOMOVEMENT)
      {
        g_this->nomove=IsDlgButtonChecked(hwndDlg,IDC_NOMOVEMENT)?1:0;
      }
      // Load preset examples from the examples table.
      if (LOWORD(wParam) == IDC_BUTTON4)
      {
        RECT r;
        HMENU hMenu;
        MENUITEMINFO i={sizeof(i),};
        hMenu=CreatePopupMenu();
        int x;
        for (x = 0; x < sizeof(presets)/sizeof(presets[0]); x ++)
        {
            i.fMask=MIIM_TYPE|MIIM_DATA|MIIM_ID;
            i.fType=MFT_STRING;
            i.wID = x+16;
            i.dwTypeData=presets[x].name;
            i.cch=strlen(presets[x].name);
            InsertMenuItem(hMenu,x,TRUE,&i);
        }
        GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON1),&r);
        x=TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,r.right,r.top,0,hwndDlg,NULL);
        if (x >= 16 && x < 16+sizeof(presets)/sizeof(presets[0]))
        {
          SetDlgItemText(hwndDlg,IDC_EDIT1,presets[x-16].point);
          SetDlgItemText(hwndDlg,IDC_EDIT2,presets[x-16].frame);
          SetDlgItemText(hwndDlg,IDC_EDIT3,presets[x-16].beat);
          SetDlgItemText(hwndDlg,IDC_EDIT4,presets[x-16].init);
          SetDlgItemInt(hwndDlg,IDC_EDIT5,presets[x-16].grid1,FALSE);
          SetDlgItemInt(hwndDlg,IDC_EDIT6,presets[x-16].grid2,FALSE);
          if (presets[x-16].rect)
          {
            g_this->rectcoords = 1;
            CheckDlgButton(hwndDlg,IDC_CHECK3,BST_CHECKED);
          }
          else
          {
            g_this->rectcoords = 0;
            CheckDlgButton(hwndDlg,IDC_CHECK3,0);
          }
          if (presets[x-16].wrap)
          {
            g_this->wrap = 1;
            CheckDlgButton(hwndDlg,IDC_WRAP,BST_CHECKED);
          }
          else
          {
            g_this->wrap = 0;
            CheckDlgButton(hwndDlg,IDC_WRAP,0);
          }
          SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_EDIT4,EN_CHANGE),0);
        }
        DestroyMenu(hMenu);
      }

  	  if (!isstart && HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMBO1) // handle clicks to combo box
	  	  g_this->buffern = SendDlgItemMessage(hwndDlg, IDC_COMBO1, CB_GETCURSEL, 0, 0);
 
      if (!isstart && HIWORD(wParam) == EN_CHANGE)
      {
        if (LOWORD(wParam) == IDC_EDIT5 || LOWORD(wParam) == IDC_EDIT6)
        {
          BOOL t;
          g_this->m_xres=GetDlgItemInt(hwndDlg,IDC_EDIT5,&t,0);
          g_this->m_yres=GetDlgItemInt(hwndDlg,IDC_EDIT6,&t,0);
        }
      
        if (LOWORD(wParam) == IDC_EDIT1||LOWORD(wParam) == IDC_EDIT2||LOWORD(wParam) == IDC_EDIT3||LOWORD(wParam) == IDC_EDIT4)
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
      }
    return 0;
  }
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_DMOVE,hwndParent,g_DlgProc);
}

#else
C_RBASE *R_DMove(char *desc) { return NULL; }
#endif