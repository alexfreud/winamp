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
#define M_PI 3.14159265358979323846

#include <windows.h>
#include <commctrl.h>
#include "r_defs.h"
#include "resource.h"

#include "timing.h"
#include "avs_eelif.h"
#include "../Agave/Language/api_language.h"
#include "../nu/AutoWide.h"
#include <math.h>


#ifndef LASER

#define C_THISCLASS C_TransTabClass
#define MOD_NAME "Trans / Movement"

#define REFFECT_MIN 3
#define REFFECT_MAX 23

typedef struct {
//    int index;       // Just here for descriptive purposes, makes it easy to match stuff up.
    int list_desc; // The string to show in the listbox.
    char *eval_desc; // The optional string to display in the evaluation editor.
    char uses_eval;   // If this is true, the preset engages the eval library and there is NULL in the radial_effects array for its entry
    char uses_rect;   // This value sets the checkbox for rectangular calculation
} Description;

static Description descriptions[]=
{
  {/* 0,*/ IDS_NONE, "", 0, 0},
  {/* 1,*/ IDS_SLIGHT_FUZZIFY, "", 0, 0},
  {/* 2,*/ IDS_SHIFT_ROTATE_LEFT, "x=x+1/32; // use wrap for this one", 0, 1},
  {/* 3,*/ IDS_BIG_SWIRL_OUT, "r = r + (0.1 - (0.2 * d));\r\nd = d * 0.96;", 0, 0},
  {/* 4,*/ IDS_MEDIUM_SWIRL, "d = d * (0.99 * (1.0 - sin(r-$PI*0.5) / 32.0));\r\nr = r + (0.03 * sin(d * $PI * 4));", 0, 0},
  {/* 5,*/ IDS_SUNBURSTER, "d = d * (0.94 + (cos((r-$PI*0.5) * 32.0) * 0.06));", 0, 0},
  {/* 6,*/ IDS_SWIRL_TO_CENTER, "d = d * (1.01 + (cos((r-$PI*0.5) * 4) * 0.04));\r\nr = r + (0.03 * sin(d * $PI * 4));", 0, 0},
  {/* 7,*/ IDS_BLOCKY_PARTIAL_OUT, "", 0, 0},
  {/* 8,*/ IDS_SWIRLING_AROUND_BOTH_WAYS, "r = r + (0.1 * sin(d * $PI * 5));", 0, 0},
  {/* 9,*/ IDS_BUBBLING_OUTWARD, "t = sin(d * $PI);\r\nd = d - (8*t*t*t*t*t)/sqrt((sw*sw+sh*sh)/4);", 0, 0},
  {/*10,*/ IDS_BUBBLING_OUTWARD_WITH_SWIRL, "t = sin(d * $PI);\r\nd = d - (8*t*t*t*t*t)/sqrt((sw*sw+sh*sh)/4);\r\nt=cos(d*$PI/2.0);\r\nr= r + 0.1*t*t*t;", 0, 0},
  {/*11,*/ IDS_5_POINTED_DISTRO, "d = d * (0.95 + (cos(((r-$PI*0.5) * 5.0) - ($PI / 2.50)) * 0.03));", 0, 0},
  {/*12,*/ IDS_TUNNELING, "r = r + 0.04;\r\nd = d * (0.96 + cos(d * $PI) * 0.05);", 0, 0},
  {/*13,*/ IDS_BLEEDIN, "t = cos(d * $PI);\r\nr = r + (0.07 * t);\r\nd = d * (0.98 + t * 0.10);", 0, 0},
  {/*14,*/ IDS_SHIFTED_BIG_SWIRL_OUT, "// this is a very bad approximation in script. fixme.\r\nd=sqrt(x*x+y*y); r=atan2(y,x);\r\nr=r+0.1-0.2*d; d=d*0.96;\r\nx=cos(r)*d + 8/128; y=sin(r)*d;", 0, 1},
  {/*15,*/ IDS_PSYCHOTIC_BEAMING_OUTWARD, "d = 0.15", 0, 0},
  {/*16,*/ IDS_COSINE_RADIAL_3_WAY, "r = cos(r * 3)", 0, 0},
  {/*17,*/ IDS_SPINNY_TUBE, "d = d * (1 - ((d - .35) * .5));\r\nr = r + .1;", 0, 0},
  {/*18,*/ IDS_RADIAL_SWIRLIES, "d = d * (1 - (sin((r-$PI*0.5) * 7) * .03));\r\nr = r + (cos(d * 12) * .03);", 1, 0},
  {/*19,*/ IDS_SWILL, "d = d * (1 - (sin((r - $PI*0.5) * 12) * .05));\r\nr = r + (cos(d * 18) * .05);\r\nd = d * (1-((d - .4) * .03));\r\nr = r + ((d - .4) * .13)", 1, 0},
  {/*20,*/ IDS_GRIDLEY, "x = x + (cos(y * 18) * .02);\r\ny = y + (sin(x * 14) * .03);", 1, 1},
  {/*21,*/ IDS_GRAPEVINE, "x = x + (cos(abs(y-.5) * 8) * .02);\r\ny = y + (sin(abs(x-.5) * 8) * .05);\r\nx = x * .95;\r\ny = y * .95;", 1, 1},
  {/*22,*/ IDS_QUADRANT, "y = y * ( 1 + (sin(r + $PI/2) * .3) );\r\nx = x * ( 1 + (cos(r + $PI/2) * .3) );\r\nx = x * .995;\r\ny = y * .995;", 1, 1},
  {/*23,*/ IDS_6_WAY_KALAEIDA, "y = (r*6)/($PI); x = d;", 1, 1},
};

#define MAKE_REFFECT(n,x) void _ref##n(double &r, double &d, double max_d, int &xo, int &yo) { x }

typedef void t_reffect(double &r, double &d, double max_d, int &xo, int &yo);

MAKE_REFFECT(3, r+=0.1-0.2*(d/max_d); d*=0.96;)
MAKE_REFFECT(4, d*=0.99*(1.0-sin(r)/32.0); r+=0.03*sin(d/max_d * M_PI * 4);)
MAKE_REFFECT(5, d*=0.94+(cos(r*32.0)*0.06);)
MAKE_REFFECT(6, d*=1.01+(cos(r*4.0)*0.04); r+=0.03*sin(d/max_d * M_PI * 4);)
MAKE_REFFECT(8, r+=0.1*sin(d/max_d*M_PI*5);)
MAKE_REFFECT(9, double t; t=sin(d/max_d*M_PI); d-=8*t*t*t*t*t; )
MAKE_REFFECT(10,double t; t=sin(d/max_d*M_PI); d-=8*t*t*t*t*t; t=cos((d/max_d)*M_PI/2.0); r+=0.1*t*t*t; )
MAKE_REFFECT(11, d*=0.95+(cos(r*5.0 - M_PI/2.50)*0.03); )
MAKE_REFFECT(12, r+=0.04; d*=0.96+cos(d/max_d*M_PI)*0.05; )
MAKE_REFFECT(13, double t; t=cos(d/max_d*M_PI); r+=0.07*t; d*=0.98+t*0.10; )
MAKE_REFFECT(14, r+=0.1-0.2*(d/max_d); d*=0.96; xo=8; )
MAKE_REFFECT(15, d=max_d*0.15;)
MAKE_REFFECT(16, r=cos(r*3);)
MAKE_REFFECT(17, d*=(1-(((d/max_d)-.35)*.5)); r+=.1;)

static t_reffect *radial_effects[REFFECT_MAX-REFFECT_MIN+1]=
{
  _ref3, _ref4, _ref5, _ref6, NULL, _ref8, _ref9, _ref10, _ref11, _ref12, _ref13, 
  _ref14, _ref15, _ref16, _ref17, NULL/*18*/, NULL/*19*/, NULL/*20*/, NULL/*21*/, 
  NULL/*22*/, NULL/*23*/,
};

static inline bool effect_uses_eval(int t)
{
  bool retval = false;
  if ((t >= REFFECT_MIN) && (t <= REFFECT_MAX))
  {
    if (descriptions[t].uses_eval)
    {
      retval = true;
    }
  }
  return retval;
}


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

		virtual char *get_desc() { return MOD_NAME; }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

    int *trans_tab, trans_tab_w, trans_tab_h, trans_tab_subpixel;
    int trans_effect;
    RString effect_exp;
    int effect_exp_ch;
    int effect,blend;
    int sourcemapped;
    int rectangular;
    int subpixel;
    int wrap;
    CRITICAL_SECTION rcs;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { effect=GET_INT(); pos+=4; }
  if (effect == 32767)
  {
    if (!memcmp(data+pos,"!rect ",6))
    {
      pos+=6;
      rectangular=1;
    }
    if (data[pos]==1)
    {
      pos++;
      int l=GET_INT(); pos+=4;
      if (l > 0 && len-pos >= l)
      {
        effect_exp.resize(l);
	      memcpy(effect_exp.get(), data+pos, l);
	      pos+=l;
      }
      else
      {
        effect_exp.resize(1);
        effect_exp.get()[0]=0;
      }
    }
    else if (len-pos >= 256)
    {
      char buf[257];
      int l=256-(rectangular?6:0);
      memcpy(buf,data+pos,l);
      buf[l]=0;
      effect_exp.assign(buf);
      pos+=l;
    }
  }
	if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
	if (len-pos >= 4) { sourcemapped=GET_INT(); pos+=4; }
	if (len-pos >= 4) { rectangular=GET_INT(); pos+=4; }
	if (len-pos >= 4) { subpixel=GET_INT(); pos+=4; }
  else subpixel=0;
	if (len-pos >= 4) { wrap=GET_INT(); pos+=4; }
  else wrap=0;
	if (!effect && len-pos >= 4) 
  { 
    effect=GET_INT(); pos+=4; 
  }

  if (effect != 32767 && effect > REFFECT_MAX || effect < 0)
    effect=0;

  // Once we know what our _real_ effect value is, if it uses the evaluator, allocate and stuff the string, here.
  /*
  if (effect_uses_eval(effect))
  {
    const char *eval_string = descriptions[effect].eval_desc;
    int length = strlen(eval_string);
    effect_exp.resize(length+1);
    strcpy(effect_exp.get(), eval_string);
  }
  */

  effect_exp_ch=1;
}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;

  // the reason this is 15, and not REFFECT_MAX, is because old versions of AVS barf
  // if effect is > 15, so we put 0, and at the end put the new value
  if (effect > 15 && effect != 32767)
  {
    PUT_INT(0); 
    pos += 4;
  }
	else
  {
    PUT_INT(effect); 
    pos+=4;
  }
  if (effect == 32767)
  {
    int l=strlen(effect_exp.get())+1;
    data[pos++]=1;
    PUT_INT(l);
    pos+=4;
    memcpy(data+pos,effect_exp.get(),strlen(effect_exp.get())+1);
    pos+=strlen(effect_exp.get())+1;
  }
	PUT_INT(blend); pos+=4;
	PUT_INT(sourcemapped); pos+=4;
  PUT_INT(rectangular); pos+=4;
  PUT_INT(subpixel); pos+=4;
  PUT_INT(wrap); pos+=4;

  // see note on '15' above =)
  if (effect > 15 && effect != 32767)
  {
    PUT_INT(effect);
    pos+=4;
  }
	return pos;
}




C_THISCLASS::C_THISCLASS()
{
  InitializeCriticalSection(&rcs);
  sourcemapped=0;
  trans_tab=NULL;
  trans_tab_w=trans_tab_h=0;
  effect=1;
  trans_effect=0;
  blend=0;
  effect_exp.assign("");
  rectangular=0;
  subpixel=1;
  wrap=0;
  trans_tab_subpixel=0;
  effect_exp_ch=1;
}

C_THISCLASS::~C_THISCLASS()
{
  if (trans_tab) GlobalFree(trans_tab);
  trans_tab=NULL;
  trans_tab_w=trans_tab_h=0;
  trans_effect=0;
  DeleteCriticalSection(&rcs);
}

int C_THISCLASS::smp_begin(int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (!effect) return 0;

  if (!trans_tab || trans_tab_w != w || trans_tab_h != h || effect != trans_effect || 
       effect_exp_ch)
  {
    int p;
    int *transp,x;
    if (trans_tab) GlobalFree(trans_tab);
    trans_tab_w=w; 
    trans_tab_h=h;
    trans_tab=(int*)GlobalAlloc(GMEM_FIXED,trans_tab_w*trans_tab_h*sizeof(int));
    trans_effect=effect;
    trans_tab_subpixel=(subpixel && trans_tab_w*trans_tab_h < (1<<22) &&
                  ((trans_effect >= REFFECT_MIN && trans_effect <= REFFECT_MAX
                  && trans_effect != 1 && trans_effect != 2 && trans_effect != 7
                  )||trans_effect ==32767));

    /* generate trans_tab */
    transp=trans_tab;
    x=w*h;
    p=0;

	  if (trans_effect == 1)
	  {
		  while (x--)
		  {
		    int r=(p++)+(rand()%3)-1 + ((rand()%3)-1)*w;
		    *transp++ = min(w*h-1,max(r,0));
		  }
	  }
	  else if (trans_effect == 2)
	  {
		  int y=h;
		  while (y--)
		  {
		    int x=w;
		    int lp=w/64;
		    while (x--)
		    {
			  *transp++ = p+lp++;
			  if (lp >= w) lp-=w;
		    }
		    p+=w;
		  }
    }
    else if (trans_effect == 7)
    {
      int y;
      for (y = 0; y < h; y ++)
      {
        for (x = 0; x < w; x ++)
        {
          if (x&2 || y&2)
          {
            *transp++ = x+y*w;
          }
          else
          {
            int xp=w/2+(((x&~1)-w/2)*7)/8;
            int yp=h/2+(((y&~1)-h/2)*7)/8;
            *transp++=xp+yp*w;
          }
        }
      }
    }
    else if (trans_effect >= REFFECT_MIN && trans_effect <= REFFECT_MAX && !effect_uses_eval(trans_effect))
    {
      double max_d=sqrt((w*w+h*h)/4.0);
      int y;
      t_reffect *ref=radial_effects[trans_effect-REFFECT_MIN];
      if (ref) for (y = 0; y < h; y ++)
      {
        for (x = 0; x < w; x ++)
        {
          double r,d;
          double xd,yd;
          int ow,oh,xo=0,yo=0;
          xd=x-(w/2);
          yd=y-(h/2);
          d=sqrt(xd*xd+yd*yd);
          r=atan2(yd,xd);

          ref(r,d,max_d,xo,yo);

          double tmp1,tmp2;
          tmp1= ((h/2) + sin(r)*d + 0.5) + (yo*h)*(1.0/256.0);
          tmp2= ((w/2) + cos(r)*d + 0.5) + (xo*w)*(1.0/256.0);
          oh=(int)tmp1;
          ow=(int)tmp2;
          if (trans_tab_subpixel)
          {
            int xpartial=(int)(32.0*(tmp2-ow));
            int ypartial=(int)(32.0*(tmp1-oh));
            if (wrap)
            {
              ow%=(w-1);
              oh%=(h-1);
              if (ow<0)ow+=w-1;
              if (oh<0)oh+=h-1;
            }
            else
            {
              if (ow < 0) { xpartial=0; ow=0; }
              if (ow >= w-1) { xpartial=31; ow=w-2; }
              if (oh < 0) { ypartial=0; oh=0; }
              if (oh >= h-1) {ypartial=31; oh=h-2; }
            }
            *transp++ = ow+oh*w | (ypartial<<22) | (xpartial<<27);
          }
          else 
          {
            if (wrap)
            {
              ow%=(w);
              oh%=(h);
              if (ow<0)ow+=w;
              if (oh<0)oh+=h;
            }
            else
            {
              if (ow < 0) ow=0;
              if (ow >= w) ow=w-1;
              if (oh < 0) oh=0;
              if (oh >= h) oh=h-1;
            }
            *transp++ = ow+oh*w;
          }
        }
      }
    }
    else if (trans_effect == 32767 || effect_uses_eval(trans_effect))
    {
      NSEEL_VMCTX AVS_EEL_CONTEXTNAME;
      AVS_EEL_INITINST();
      double max_d=sqrt((double)(w*w+h*h))/2.0;
      double divmax_d=1.0/max_d;
      int y;
      double *d = registerVar("d");
      double *r = registerVar("r");
      double *px = registerVar("x");
      double *py = registerVar("y");
      double *pw = registerVar("sw");
      double *ph = registerVar("sh");
      NSEEL_CODEHANDLE codehandle=0;
      int offs=0;
      int is_rect = trans_effect == 32767 ? rectangular : descriptions[trans_effect].uses_rect;
      *pw=w;
      *ph=h;
      EnterCriticalSection(&rcs);
      codehandle=compileCode(
        trans_effect == 32767 ? effect_exp.get() : descriptions[trans_effect].eval_desc
        );
      LeaveCriticalSection(&rcs);
      if (codehandle)         
      {
        double w2=w/2;
        double h2=h/2;
        double xsc=1.0/w2,ysc=1.0/h2;

        for (y = 0; y < h; y ++)
        {
          for (x = 0; x < w; x ++)
          {
            double xd,yd;
            int ow,oh;
            xd=x-w2;
            yd=y-h2;
            *px=xd*xsc;
            *py=yd*ysc;
            *d=sqrt(xd*xd+yd*yd)*divmax_d;
            *r=atan2(yd,xd) + M_PI*0.5;
    
            executeCode(codehandle,visdata);
        
            double tmp1,tmp2;
            if (!is_rect)
            {
              *d *= max_d;
              *r -= M_PI/2.0;
              tmp1=((h/2) + sin(*r)* *d);
              tmp2=((w/2) + cos(*r)* *d);
            }
            else
            {
              tmp1=((*py+1.0)*h2);
              tmp2=((*px+1.0)*w2);
            }
            if (trans_tab_subpixel)
            {
              oh=(int) tmp1;
              ow=(int) tmp2;
              int xpartial=(int)(32.0*(tmp2-ow));
              int ypartial=(int)(32.0*(tmp1-oh));
              if (wrap)
              {
                ow%=(w-1);
                oh%=(h-1);
                if (ow<0)ow+=w-1;
                if (oh<0)oh+=h-1;
              }
              else
              {
                if (ow < 0) { xpartial=0; ow=0; }
                if (ow >= w-1) { xpartial=31; ow=w-2; }
                if (oh < 0) { ypartial=0; oh=0; }
                if (oh >= h-1) {ypartial=31; oh=h-2; }
              }
              *transp++ = ow+oh*w | (ypartial<<22) | (xpartial<<27);
            }
            else
            {
              tmp1+=0.5;
              tmp2+=0.5;
              oh=(int) tmp1;
              ow=(int) tmp2;
              if (wrap)
              {
                ow%=(w);
                oh%=(h);
                if (ow<0)ow+=w;
                if (oh<0)oh+=h;
              }
              else
              {
                if (ow < 0) ow=0;
                if (ow >= w) ow=w-1;
                if (oh < 0) oh=0;
                if (oh >= h) oh=h-1;
              }
              *transp++ = ow+oh*w;
            }
          }
        }
      }
      else 
      {
        transp=trans_tab;
        trans_tab_subpixel=0;
        for (x = 0; x < w*h; x ++)
          *transp++=x;
      }
      freeCode(codehandle);
      AVS_EEL_QUITINST();
    }
    effect_exp_ch=0;
  }

  if (!(isBeat & 0x80000000))
  {
    if ((sourcemapped&2)&&isBeat) sourcemapped^=1;
    if (sourcemapped&1)
    {
      if (!blend) memset(fbout,0,w*h*sizeof(int));
      else memcpy(fbout,framebuffer,w*h*sizeof(int));
    }
  }
  return max_threads;
}

void C_THISCLASS::smp_render(int this_thread, int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (!effect) return;
  
#define OFFSET_MASK ((1<<22)-1)

  unsigned int *inp = (unsigned int *) framebuffer;
  unsigned int *outp;
  int *transp, x;

  if (max_threads < 1) max_threads=1;

  int start_l = ( this_thread * h ) / max_threads;
  int end_l;

  if (this_thread >= max_threads - 1) end_l = h;
  else end_l = ( (this_thread+1) * h ) / max_threads;  

  int outh=end_l-start_l;
  if (outh<1) return;

  int skip_pix=start_l*w;
  transp=trans_tab;

  outp = (unsigned int *) fbout;
  x=(w*outh)/4;
  if (sourcemapped&1)
  {
    inp += skip_pix;
    transp += skip_pix;
    if (trans_tab_subpixel)
    {
      timingEnter(3);
      while (x--) 
      {
        fbout[transp[0]&OFFSET_MASK]=BLEND_MAX(inp[0],fbout[transp[0]&OFFSET_MASK]);
        fbout[transp[1]&OFFSET_MASK]=BLEND_MAX(inp[1],fbout[transp[1]&OFFSET_MASK]);
        fbout[transp[2]&OFFSET_MASK]=BLEND_MAX(inp[2],fbout[transp[2]&OFFSET_MASK]);
        fbout[transp[3]&OFFSET_MASK]=BLEND_MAX(inp[3],fbout[transp[3]&OFFSET_MASK]);
        inp+=4;
        transp+=4;
      }
      timingLeave(3);
      x = (w*outh)&3;
      if (x>0) while (x--)
      {
        fbout[transp[0]&OFFSET_MASK]=BLEND_MAX(inp++[0],fbout[transp[0]&OFFSET_MASK]);
        transp++;
      }
    }
    else
    {
      {
        timingEnter(3);
        while (x--) 
        {
          fbout[transp[0]]=BLEND_MAX(inp[0],fbout[transp[0]]);
          fbout[transp[1]]=BLEND_MAX(inp[1],fbout[transp[1]]);
          fbout[transp[2]]=BLEND_MAX(inp[2],fbout[transp[2]]);
          fbout[transp[3]]=BLEND_MAX(inp[3],fbout[transp[3]]);
          inp+=4;
          transp+=4;
        }
        timingLeave(3);
        x = (w*outh)&3;
        if (x>0) while (x--)
        {
          fbout[transp[0]]=BLEND_MAX(inp++[0],fbout[transp[0]]);
          transp++;
        }
      }
    }
    if (blend)
    {
      framebuffer += skip_pix;
      fbout += skip_pix;
      x=(w*outh)/4;
      while (x--)
      {
        fbout[0]=BLEND_AVG(fbout[0],framebuffer[0]);
        fbout[1]=BLEND_AVG(fbout[1],framebuffer[1]);
        fbout[2]=BLEND_AVG(fbout[2],framebuffer[2]);
        fbout[3]=BLEND_AVG(fbout[3],framebuffer[3]);
        fbout+=4;
        framebuffer+=4;
      }
      x=(w*outh)&3;
      while (x--)
      {
        fbout[0]=BLEND_AVG(fbout[0],framebuffer[0]);
        fbout++;
        framebuffer++;
      }
    }
  }
  else
  {
    inp += skip_pix;
    outp += skip_pix;
    transp += skip_pix;
    if (trans_tab_subpixel&&blend)
    {
      while (x--)
      {
        int offs=transp[0]&OFFSET_MASK;
        outp[0]=BLEND_AVG(inp[0],BLEND4((unsigned int *)framebuffer+offs,w,((transp[0]>>24)&(31<<3)),((transp[0]>>19)&(31<<3))));
        offs=transp[1]&OFFSET_MASK;
        outp[1]=BLEND_AVG(inp[1],BLEND4((unsigned int *)framebuffer+offs,w,((transp[1]>>24)&(31<<3)),((transp[1]>>19)&(31<<3))));
        offs=transp[2]&OFFSET_MASK;
        outp[2]=BLEND_AVG(inp[2],BLEND4((unsigned int *)framebuffer+offs,w,((transp[2]>>24)&(31<<3)),((transp[2]>>19)&(31<<3))));
        offs=transp[3]&OFFSET_MASK;
        outp[3]=BLEND_AVG(inp[3],BLEND4((unsigned int *)framebuffer+offs,w,((transp[3]>>24)&(31<<3)),((transp[3]>>19)&(31<<3))));
        transp+=4;
        outp+=4;
        inp+=4;
      }    
      x=(w*outh)&3;
      while (x--)
      {
        int offs=transp[0]&OFFSET_MASK;
        outp++[0]=BLEND_AVG(inp[0],BLEND4((unsigned int *)framebuffer+offs,w,((transp[0]>>24)&(31<<3)),((transp[0]>>19)&(31<<3))));
        transp++;
        inp++;
      }    
    #ifndef NO_MMX
      __asm emms;
    #endif
    }
    else if (trans_tab_subpixel)
    {
      while (x--)
      {
        int offs=transp[0]&OFFSET_MASK;
        outp[0]=BLEND4((unsigned int *)framebuffer+offs,w,((transp[0]>>24)&(31<<3)),((transp[0]>>19)&(31<<3)));
        offs=transp[1]&OFFSET_MASK;
        outp[1]=BLEND4((unsigned int *)framebuffer+offs,w,((transp[1]>>24)&(31<<3)),((transp[1]>>19)&(31<<3)));
        offs=transp[2]&OFFSET_MASK;
        outp[2]=BLEND4((unsigned int *)framebuffer+offs,w,((transp[2]>>24)&(31<<3)),((transp[2]>>19)&(31<<3)));
        offs=transp[3]&OFFSET_MASK;
        outp[3]=BLEND4((unsigned int *)framebuffer+offs,w,((transp[3]>>24)&(31<<3)),((transp[3]>>19)&(31<<3)));
        transp+=4;
        outp+=4;
      }    
      x=(w*outh)&3;
      while (x--)
      {
        int offs=transp[0]&OFFSET_MASK;
        outp++[0]=BLEND4((unsigned int *)framebuffer+offs,w,((transp[0]>>24)&(31<<3)),((transp[0]>>19)&(31<<3)));
        transp++;
      }    
    #ifndef NO_MMX
      __asm emms;
    #endif
    }
    else if (blend)
    {
      timingEnter(3);
      while (x--) 
      {
        outp[0]=BLEND_AVG(inp[0],framebuffer[transp[0]]);
        outp[1]=BLEND_AVG(inp[1],framebuffer[transp[1]]);
        outp[2]=BLEND_AVG(inp[2],framebuffer[transp[2]]);
        outp[3]=BLEND_AVG(inp[3],framebuffer[transp[3]]);
        outp+=4;
        inp+=4;
        transp+=4;
      }
      timingLeave(3);
      x = (w*outh)&3;
      if (x>0) while (x--)
      {
        outp++[0]=BLEND_AVG(inp++[0],framebuffer[transp++[0]]);
      }
    }
    else
    {
      timingEnter(4);
      while (x--) 
      {
        outp[0]=framebuffer[transp[0]];
        outp[1]=framebuffer[transp[1]];
        outp[2]=framebuffer[transp[2]];
        outp[3]=framebuffer[transp[3]];
        outp+=4;
        transp+=4;
      }
      timingLeave(4);
      x = (w*outh)&3;
      if (x>0) while (x--)
      {
        outp++[0]=framebuffer[transp++[0]];
      }
    }
  }
}

int C_THISCLASS::smp_finish(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h) // return value is that of render() for fbstuff etc
{
  return !!effect;
}
	
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  smp_begin(1,visdata,isBeat,framebuffer,fbout,w,h);
  if (isBeat & 0x80000000) return 0;

  smp_render(0,1,visdata,isBeat,framebuffer,fbout,w,h);
  return smp_finish(visdata,isBeat,framebuffer,fbout,w,h);
}

C_RBASE *R_Trans(char *desc)
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
      {
        int x;
        for (x = 0; x < sizeof(descriptions)/sizeof(descriptions[0]); x ++) 
        {
            SendDlgItemMessage( hwndDlg, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)(const wchar_t *)AutoWide( WASABI_API_LNGSTRING( descriptions[ x ].list_desc ), CP_UTF8 ) );
        }

        isstart=1;
        SetDlgItemText(hwndDlg,IDC_EDIT1,g_this->effect_exp.get());
        // After we set whatever value into the edit box, that's the new saved value (ie: don't change the save format)
        isstart=0;

        if (g_this->blend)
          CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
        if (g_this->subpixel)
          CheckDlgButton(hwndDlg,IDC_CHECK4,BST_CHECKED);
        if (g_this->wrap)
          CheckDlgButton(hwndDlg,IDC_WRAP,BST_CHECKED);
        if (g_this->rectangular)
          CheckDlgButton(hwndDlg,IDC_CHECK3,BST_CHECKED);
        if (g_this->sourcemapped&2)
          CheckDlgButton(hwndDlg,IDC_CHECK2,BST_INDETERMINATE);
        else if (g_this->sourcemapped&1)
          CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);

        SendDlgItemMessage( hwndDlg, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)(const wchar_t *)AutoWide( WASABI_API_LNGSTRING( IDS_USER_DEFINED ), CP_UTF8 ) );
        SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_SETCURSEL,(g_this->effect==32767)?sizeof(descriptions)/sizeof(descriptions[0]):g_this->effect,0);
        if (g_this->effect == 32767)
        {
          EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),1);
          EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK3),1);
          EnableWindow(GetDlgItem(hwndDlg,IDC_LABEL1),1);          
        }
        else if (g_this->effect >= 0 && g_this->effect <= REFFECT_MAX)
        {
          if (strlen(descriptions[g_this->effect].eval_desc) > 0)
          {
            SetDlgItemTextA(hwndDlg,IDC_EDIT1,descriptions[g_this->effect].eval_desc);
            CheckDlgButton(hwndDlg,IDC_CHECK3,descriptions[g_this->effect].uses_rect?BST_CHECKED:0);
            EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),1);
            EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK3),1);
            EnableWindow(GetDlgItem(hwndDlg,IDC_LABEL1),1);
          }
          else
          {
            EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),0);
            EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK3),0);
            EnableWindow(GetDlgItem(hwndDlg,IDC_LABEL1),0);
          }
        }
        else
        {
          EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),0);
          EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK3),0);
          EnableWindow(GetDlgItem(hwndDlg,IDC_LABEL1),0);
        }
      }
			return 1;
    case WM_TIMER:
      if (wParam == 1)
      {
        KillTimer(hwndDlg,1);
        EnterCriticalSection(&g_this->rcs);
        g_this->effect=32767;
        g_this->effect_exp.get_from_dlgitem(hwndDlg,IDC_EDIT1);
        g_this->effect_exp_ch=1;
        LeaveCriticalSection(&g_this->rcs);
      }
    return 0;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDC_EDIT1 && HIWORD(wParam) == EN_CHANGE)
      {
        KillTimer(hwndDlg,1);
        if (!isstart) SetTimer(hwndDlg,1,1000,NULL);

        // If someone edits the editwnd, force the "(user defined)" to be the new selection.
        if (SendDlgItemMessageA(hwndDlg,IDC_LIST1,LB_GETCURSEL,0,0) != sizeof(descriptions)/sizeof(descriptions[0]))
        {
          g_this->rectangular=IsDlgButtonChecked(hwndDlg,IDC_CHECK3)?1:0;
          SendDlgItemMessageA(hwndDlg,IDC_LIST1,LB_SETCURSEL,sizeof(descriptions)/sizeof(descriptions[0]),0);
        }
      }
      if (LOWORD(wParam)==IDC_LIST1 && HIWORD(wParam)==LBN_SELCHANGE)
      {
        int t;
        t=SendDlgItemMessageA(hwndDlg,IDC_LIST1,LB_GETCURSEL,0,0);
        if (t == sizeof(descriptions)/sizeof(descriptions[0]))
        {
          g_this->effect=32767;
          EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),1);
          EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK3),1);
          EnableWindow(GetDlgItem(hwndDlg,IDC_LABEL1),1);

          SetDlgItemText(hwndDlg,IDC_EDIT1,g_this->effect_exp.get());
          CheckDlgButton(hwndDlg,IDC_CHECK3,g_this->rectangular);

          // always reinit =)
          {
            EnterCriticalSection(&g_this->rcs);
            g_this->effect_exp.get_from_dlgitem(hwndDlg,IDC_EDIT1);
            g_this->effect_exp_ch=1;
            LeaveCriticalSection(&g_this->rcs);
          }
        }
        else 
        {
          g_this->effect=t;

          // If there is a string to stuff in the eval box,
          if (strlen(descriptions[t].eval_desc) > 0)
          {
            // save the value to be able to restore it later
            // stuff it and make sure the boxes are editable
            SetDlgItemText(hwndDlg,IDC_EDIT1,descriptions[t].eval_desc);
            EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),1);
            EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK3),1);
            EnableWindow(GetDlgItem(hwndDlg,IDC_LABEL1),1);
            CheckDlgButton(hwndDlg,IDC_CHECK3,descriptions[t].uses_rect?BST_CHECKED:0);
          }
          else
          {
            // otherwise, they're not editable.
            CheckDlgButton(hwndDlg,IDC_CHECK3,g_this->rectangular?BST_CHECKED:0);
            SetDlgItemText(hwndDlg,IDC_EDIT1,"");
            EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),0);
            EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK3),0);
            EnableWindow(GetDlgItem(hwndDlg,IDC_LABEL1),0);
          }
        }

      }
      if (LOWORD(wParam)==IDC_CHECK1)
      {
        g_this->blend=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
      }
      if (LOWORD(wParam)==IDC_CHECK4)
      {
        g_this->subpixel=IsDlgButtonChecked(hwndDlg,IDC_CHECK4)?1:0;
        g_this->effect_exp_ch=1;
      }
      if (LOWORD(wParam)==IDC_WRAP)
      {
        g_this->wrap=IsDlgButtonChecked(hwndDlg,IDC_WRAP)?1:0;
        g_this->effect_exp_ch=1;
      }
      if (LOWORD(wParam)==IDC_CHECK2)
      {
        int a=IsDlgButtonChecked(hwndDlg,IDC_CHECK2);
        if (a == BST_INDETERMINATE) 
          g_this->sourcemapped=2;
        else if (a == BST_CHECKED) 
          g_this->sourcemapped=1;
        else
          g_this->sourcemapped=0;
      }
      if (LOWORD(wParam) == IDC_CHECK3)
      {
          g_this->rectangular=IsDlgButtonChecked(hwndDlg,IDC_CHECK3)?1:0;
          if (SendDlgItemMessageA(hwndDlg,IDC_LIST1,LB_GETCURSEL,0,0) != sizeof(descriptions)/sizeof(descriptions[0]))
          {
            EnterCriticalSection(&g_this->rcs);
            g_this->effect=32767;
            g_this->effect_exp.get_from_dlgitem(hwndDlg,IDC_EDIT1);
            g_this->effect_exp_ch=1;
            LeaveCriticalSection(&g_this->rcs);
            SendDlgItemMessageA(hwndDlg,IDC_LIST1,LB_SETCURSEL,sizeof(descriptions)/sizeof(descriptions[0]),0);
          }
          else
            g_this->effect_exp_ch=1;
      }
      if (LOWORD(wParam) == IDC_BUTTON2)
      {
/*
		char text[4096];
		WASABI_API_LNGSTRING_BUF(IDS_MOVEMENT,text,4096);
		int titlelen = lstrlen(text)+1;
		lstrcpyn(text+titlelen,GetTextResource(IDR_MOVEMENT),4095-titlelen);
*/

        char *text="Movement\0"
          "Movement help:\r\n"
"To use the custom table, modify r,d,x or y.\r\n"
"Rect coords: x,y are in (-1..1) . Otherwise: d is (0..1) and r is (0..2PI).\r\n"
"You can also access 'sw' and 'sh' for screen dimensions in pixels (might be useful)\r\n"
          ;
        compilerfunctionlist(hwndDlg,text);
      }
      return 0;
    return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_TRANS,hwndParent,g_DlgProc);
}

#else
C_RBASE *R_Trans(char *desc){return NULL; }
#endif