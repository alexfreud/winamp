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

#define C_THISCLASS C_BlurClass
#define MOD_NAME "Trans / Blur"

static const int zero=0;

class C_THISCLASS : public C_RBASE2 {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_BLUR,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

    virtual int smp_getflags() { return 1; }
		virtual int smp_begin(int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); 
    virtual void smp_render(int this_thread, int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); 
    virtual int smp_finish(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); // return value is that of render() for fbstuff etc

    int enabled;

    int roundmode;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { roundmode=GET_INT(); pos+=4; }
  else roundmode=0;
}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
	PUT_INT(enabled); pos+=4;
	PUT_INT(roundmode); pos+=4;
	return pos;
}




C_THISCLASS::C_THISCLASS()
{
  roundmode=0;
  enabled=1;
}

C_THISCLASS::~C_THISCLASS()
{
}

#define MASK_SH1 (~(((1<<7)|(1<<15)|(1<<23))<<1))
#define MASK_SH2 (~(((3<<6)|(3<<14)|(3<<22))<<2))
#define MASK_SH3 (~(((7<<5)|(7<<13)|(7<<21))<<3))
#define MASK_SH4 (~(((15<<4)|(15<<12)|(15<<20))<<4))
static unsigned int mmx_mask1[2]={MASK_SH1,MASK_SH1};
static unsigned int mmx_mask2[2]={MASK_SH2,MASK_SH2};
static unsigned int mmx_mask3[2]={MASK_SH3,MASK_SH3};
static unsigned int mmx_mask4[2]={MASK_SH4,MASK_SH4};

#define DIV_2(x) ((( x ) & MASK_SH1)>>1)
#define DIV_4(x) ((( x ) & MASK_SH2)>>2)
#define DIV_8(x) ((( x ) & MASK_SH3)>>3)
#define DIV_16(x) ((( x ) & MASK_SH4)>>4)


void C_THISCLASS::smp_render(int this_thread, int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (!enabled) return;

  timingEnter(0);

	unsigned int *f = (unsigned int *) framebuffer;
  unsigned int *of = (unsigned int *) fbout;

  if (max_threads < 1) max_threads=1;

  int start_l = ( this_thread * h ) / max_threads;
  int end_l;

  if (this_thread >= max_threads - 1) end_l = h;
  else end_l = ( (this_thread+1) * h ) / max_threads;  

  int outh=end_l-start_l;
  if (outh<1) return;

  int skip_pix=start_l*w;

  f += skip_pix;
  of+= skip_pix;

  int at_top=0, at_bottom=0;

  if (!this_thread) at_top=1;
  if (this_thread >= max_threads - 1) at_bottom=1;

  if (enabled == 2) 
  {
    // top line
    if (at_top)
    {
      unsigned int *f2=f+w;
      int x;
      int adj_tl=0, adj_tl2=0;
      if (roundmode) { adj_tl = 0x03030303; adj_tl2 = 0x04040404; }
      // top left
      *of++=DIV_2(f[0])+DIV_4(f[0])+DIV_8(f[1])+DIV_8(f2[0]) + adj_tl; f++; f2++;
	    // top center
      x=(w-2)/4;
	    while (x--)
	    {
		    of[0]=DIV_2(f[0]) + DIV_8(f[0]) + DIV_8(f[1]) + DIV_8(f[-1]) + DIV_8(f2[0]) + adj_tl2;
		    of[1]=DIV_2(f[1]) + DIV_8(f[1]) + DIV_8(f[2]) + DIV_8(f[0]) + DIV_8(f2[1]) + adj_tl2;
		    of[2]=DIV_2(f[2]) + DIV_8(f[2]) + DIV_8(f[3]) + DIV_8(f[1]) + DIV_8(f2[2]) + adj_tl2;
		    of[3]=DIV_2(f[3]) + DIV_8(f[3]) + DIV_8(f[4]) + DIV_8(f[2]) + DIV_8(f2[3]) + adj_tl2;
        f+=4;
        f2+=4;
        of+=4;
	    }
      x=(w-2)&3;
	    while (x--)
	    {
		    *of++=DIV_2(f[0]) + DIV_8(f[0]) + DIV_8(f[1]) + DIV_8(f[-1]) + DIV_8(f2[0]) + adj_tl2;
        f++;
        f2++;
      }
      // top right
      *of++=DIV_2(f[0])+DIV_4(f[0]) + DIV_8(f[-1])+DIV_8(f2[0]) + adj_tl; f++; f2++;
    }


	  // middle block
    {
      int y=outh-at_top-at_bottom;
      unsigned int adj_tl1=0,adj_tl2=0;
      unsigned __int64 adj2=0;
      if (roundmode) { adj_tl1=0x04040404; adj_tl2=0x05050505; adj2=0x0505050505050505i64; }
      while (y--)
      {
        int x;
        unsigned int *f2=f+w;
        unsigned int *f3=f-w;
      
        // left edge
        *of++=DIV_2(f[0])+DIV_8(f[0])+DIV_8(f[1])+DIV_8(f2[0])+DIV_8(f3[0])+adj_tl1; f++; f2++; f3++;

        // middle of line
#ifdef NO_MMX
        x=(w-2)/4;
        if (roundmode)
        {
	        while (x--)
	        {
		        of[0]=DIV_2(f[0]) + DIV_4(f[0]) + DIV_16(f[1]) + DIV_16(f[-1]) + DIV_16(f2[0]) + DIV_16(f3[0]) + 0x05050505;
		        of[1]=DIV_2(f[1]) + DIV_4(f[1]) + DIV_16(f[2]) + DIV_16(f[0]) + DIV_16(f2[1]) + DIV_16(f3[1]) + 0x05050505;
		        of[2]=DIV_2(f[2]) + DIV_4(f[2]) + DIV_16(f[3]) + DIV_16(f[1]) + DIV_16(f2[2]) + DIV_16(f3[2]) + 0x05050505;
		        of[3]=DIV_2(f[3]) + DIV_4(f[3]) + DIV_16(f[4]) + DIV_16(f[2]) + DIV_16(f2[3]) + DIV_16(f3[3]) + 0x05050505;
            f+=4;
            f2+=4;
            f3+=4;
            of+=4;
          }
        }
        else
        {
	        while (x--)
	        {
		        of[0]=DIV_2(f[0]) + DIV_4(f[0]) + DIV_16(f[1]) + DIV_16(f[-1]) + DIV_16(f2[0]) + DIV_16(f3[0]);
		        of[1]=DIV_2(f[1]) + DIV_4(f[1]) + DIV_16(f[2]) + DIV_16(f[0]) + DIV_16(f2[1]) + DIV_16(f3[1]);
		        of[2]=DIV_2(f[2]) + DIV_4(f[2]) + DIV_16(f[3]) + DIV_16(f[1]) + DIV_16(f2[2]) + DIV_16(f3[2]);
		        of[3]=DIV_2(f[3]) + DIV_4(f[3]) + DIV_16(f[4]) + DIV_16(f[2]) + DIV_16(f2[3]) + DIV_16(f3[3]);
            f+=4;
            f2+=4;
            f3+=4;
            of+=4;
          }
        }
#else
        {
          __asm 
          {
            mov ecx, w
            mov edx, ecx
            mov ebx, edx
            neg ebx
            mov esi, f
            mov edi, of
            sub ecx, 2
            shr ecx, 2
            movq mm1, [esi-4]
            align 16
mmx_light_blur_loop:
            movq mm0, [esi]

            movq mm2, [esi+4]
            pand mm0, mmx_mask1

            movq mm5, mm2
            psrl mm0, 1

            movq mm7, [esi+8]
            movq mm4, mm0

            pand mm1, mmx_mask4
            pand mm4, mmx_mask1

            movq mm3, [esi+edx*4]
            psrl mm4, 1
           
            paddb mm0, mm4
            pand mm2, mmx_mask4

            movq mm4, [esi+ebx*4]
            pand mm3, mmx_mask4

            pand mm4, mmx_mask4

            psrl mm1, 4
            pand mm7, mmx_mask1

            movq mm6, [esi+12]

            psrl mm2, 4
            add esi, 16

            psrl mm3, 4

            paddb mm0, mm1
            psrl mm4, 4

            movq mm1, mm6
            psrl mm7, 1

            paddb mm2, mm3
            paddb mm0, mm4

            movq mm3, [esi+edx*4-8]
            paddb mm0, mm2

            movq mm4, [esi+ebx*4-8]
            paddb mm0, [adj2]

            pand mm6, mmx_mask4

            movq [edi],mm0
            pand mm5, mmx_mask4

            movq mm0, mm7
            pand mm3, mmx_mask4

            psrl mm6, 4
            pand mm0, mmx_mask1

            pand mm4, mmx_mask4
            psrl mm5, 4

            psrl mm0, 1
            paddb mm7, mm6

            paddb mm7, mm0
            add edi, 16

            psrl mm3, 4

            psrl mm4, 4
            paddb mm5, mm3

            paddb mm7, mm4
            dec ecx

            paddb mm7, mm5
            paddb mm7, [adj2]

            movq [edi-8],mm7
            
            jnz mmx_light_blur_loop
            mov of, edi
            mov f, esi           
          };
          f2=f+w; // update these bitches
          f3=f-w;
        }
#endif
        x=(w-2)&3;
	      while (x--)
	      {
		      *of++=DIV_2(f[0]) + DIV_4(f[0]) + DIV_16(f[1]) + DIV_16(f[-1]) + DIV_16(f2[0]) + DIV_16(f3[0]) + adj_tl2;
          f++;
          f2++;
          f3++;
        }

        // right block
        *of++=DIV_2(f[0])+DIV_8(f[0])+DIV_8(f[-1])+DIV_8(f2[0])+DIV_8(f3[0])+adj_tl1; f++;
	    }
    }
    // bottom block
    if (at_bottom)
    {
      unsigned int *f2=f-w;
      int x;
      int adj_tl=0, adj_tl2=0;
      if (roundmode) { adj_tl = 0x03030303; adj_tl2 = 0x04040404; }
      // bottom left
      *of++=DIV_2(f[0])+DIV_4(f[0])+DIV_8(f[1])+DIV_8(f2[0]) + adj_tl; f++; f2++;
	    // bottom center
      x=(w-2)/4;
	    while (x--)
	    {
		    of[0]=DIV_2(f[0]) + DIV_8(f[0]) + DIV_8(f[1]) + DIV_8(f[-1]) + DIV_8(f2[0]) + adj_tl2;
		    of[1]=DIV_2(f[1]) + DIV_8(f[1]) + DIV_8(f[2]) + DIV_8(f[0]) + DIV_8(f2[1]) + adj_tl2;
		    of[2]=DIV_2(f[2]) + DIV_8(f[2]) + DIV_8(f[3]) + DIV_8(f[1]) + DIV_8(f2[2])+adj_tl2;
		    of[3]=DIV_2(f[3]) + DIV_8(f[3]) + DIV_8(f[4]) + DIV_8(f[2]) + DIV_8(f2[3])+adj_tl2;
        f+=4;
        f2+=4;
        of+=4;
	    }
      x=(w-2)&3;
	    while (x--)
	    {
		    *of++=DIV_2(f[0]) + DIV_8(f[0]) + DIV_8(f[1]) + DIV_8(f[-1]) + DIV_8(f2[0])+adj_tl2;
        f++;
        f2++;
      }
      // bottom right
      *of++=DIV_2(f[0])+DIV_4(f[0]) + DIV_8(f[-1])+DIV_8(f2[0])+adj_tl; f++; f2++;
    }
  }
  else if (enabled == 3) // more blur
  {
    // top line
    if (at_top) {
      unsigned int *f2=f+w;
      int x;
      int adj_tl=0, adj_tl2=0;
      if (roundmode) { adj_tl = 0x02020202; adj_tl2 = 0x01010101; }
      // top left
      *of++=DIV_2(f[1])+DIV_2(f2[0]) + adj_tl2; f++; f2++;
	    // top center
      x=(w-2)/4;
	    while (x--)
	    {
		    of[0]=DIV_4(f[1]) + DIV_4(f[-1]) + DIV_2(f2[0]) + adj_tl;
		    of[1]=DIV_4(f[2]) + DIV_4(f[0]) + DIV_2(f2[1]) +adj_tl;
		    of[2]=DIV_4(f[3]) + DIV_4(f[1]) + DIV_2(f2[2]) + adj_tl;
		    of[3]=DIV_4(f[4]) + DIV_4(f[2]) + DIV_2(f2[3]) + adj_tl;
        f+=4;
        f2+=4;
        of+=4;
	    }
      x=(w-2)&3;
	    while (x--)
	    {
		    *of++=DIV_4(f[1]) + DIV_4(f[-1]) + DIV_2(f2[0])+adj_tl;
        f++;
        f2++;
      }
      // top right
      *of++=DIV_2(f[-1])+DIV_2(f2[0])+adj_tl2; f++; f2++;
    }


	  // middle block
    {
      int y=outh-at_top-at_bottom;
      int adj_tl1=0,adj_tl2=0;
      unsigned __int64 adj2=0;
      if (roundmode) { adj_tl1=0x02020202; adj_tl2=0x03030303; adj2=0x0303030303030303i64; }

      while (y--)
      {
        int x;
        unsigned int *f2=f+w;
        unsigned int *f3=f-w;
      
        // left edge
        *of++=DIV_2(f[1])+DIV_4(f2[0])+DIV_4(f3[0]) + adj_tl1; f++; f2++; f3++;

        // middle of line
#ifdef NO_MMX
        x=(w-2)/4;
        if (roundmode)
        {
	        while (x--)
	        {
		        of[0]=DIV_4(f[1]) + DIV_4(f[-1]) + DIV_4(f2[0]) + DIV_4(f3[0]) + 0x03030303;
		        of[1]=DIV_4(f[2]) + DIV_4(f[0]) + DIV_4(f2[1]) + DIV_4(f3[1]) + 0x03030303;
		        of[2]=DIV_4(f[3]) + DIV_4(f[1]) + DIV_4(f2[2]) + DIV_4(f3[2]) + 0x03030303;
		        of[3]=DIV_4(f[4]) + DIV_4(f[2]) + DIV_4(f2[3]) + DIV_4(f3[3]) + 0x03030303;
            f+=4; f2+=4; f3+=4; of+=4;
          }
        }
        else
        {
	        while (x--)
	        {
		        of[0]=DIV_4(f[1]) + DIV_4(f[-1]) + DIV_4(f2[0]) + DIV_4(f3[0]);
		        of[1]=DIV_4(f[2]) + DIV_4(f[0]) + DIV_4(f2[1]) + DIV_4(f3[1]);
		        of[2]=DIV_4(f[3]) + DIV_4(f[1]) + DIV_4(f2[2]) + DIV_4(f3[2]);
		        of[3]=DIV_4(f[4]) + DIV_4(f[2]) + DIV_4(f2[3]) + DIV_4(f3[3]);
            f+=4; f2+=4; f3+=4; of+=4;
          }
        }
#else
        {
          __asm 
          {
            mov ecx, w
            mov edx, ecx
            mov ebx, edx
            neg ebx
            mov esi, f
            mov edi, of
            sub ecx, 2
            shr ecx, 2
            movq mm1, [esi-4]
            align 16
mmx_heavy_blur_loop:
            movq mm2, [esi+4]
            pxor mm0, mm0

            movq mm5, mm2
            pxor mm7, mm7

            movq mm3, [esi+edx*4]
            pand mm1, mmx_mask2

            movq mm4, [esi+ebx*4]
            pand mm2, mmx_mask2

            pand mm3, mmx_mask2
            pand mm4, mmx_mask2

            psrl mm1, 2

            movq mm6, [esi+12]
            psrl mm2, 2

            psrl mm3, 2

            paddb mm0, mm1
            psrl mm4, 2

            movq mm1, mm6

            paddb mm2, mm3
            paddb mm0, mm4

            movq mm3, [esi+edx*4+8]
            paddb mm0, mm2

            movq mm4, [esi+ebx*4+8]
            paddb mm0, [adj2]

            pand mm6, mmx_mask2

            movq [edi],mm0
            pand mm5, mmx_mask2

            pand mm3, mmx_mask2
            add esi, 16

            psrl mm6, 2
            pand mm4, mmx_mask2

            psrl mm5, 2
            paddb mm7, mm6

            psrl mm3, 2
            add edi, 16

            psrl mm4, 2
            paddb mm5, mm3

            paddb mm7, mm4

            paddb mm7, mm5
            paddb mm7, [adj2]

            movq [edi-8],mm7
                        
            dec ecx
            jnz mmx_heavy_blur_loop
            mov of, edi
            mov f, esi           
          };
          f2=f+w; // update these bitches
          f3=f-w;
        }
#endif
        x=(w-2)&3;
	      while (x--)
	      {
		      *of++=DIV_4(f[1]) + DIV_4(f[-1]) + DIV_4(f2[0]) + DIV_4(f3[0]) + adj_tl2;
          f++;
          f2++;
          f3++;
        }

        // right block
        *of++=DIV_2(f[-1])+DIV_4(f2[0])+DIV_4(f3[0]) + adj_tl1; f++;
	    }
    }

    // bottom block
    if (at_bottom)
    {
      unsigned int *f2=f-w;
      int x;
      int adj_tl=0, adj_tl2=0;
      if (roundmode) { adj_tl = 0x02020202; adj_tl2 = 0x01010101; }
      // bottom left
      *of++=DIV_2(f[1])+DIV_2(f2[0]) + adj_tl2; f++; f2++;
	    // bottom center
      x=(w-2)/4;
	    while (x--)
	    {
		    of[0]=DIV_4(f[1]) + DIV_4(f[-1]) + DIV_2(f2[0])+adj_tl;
		    of[1]=DIV_4(f[2]) + DIV_4(f[0]) + DIV_2(f2[1])+adj_tl;
		    of[2]=DIV_4(f[3]) + DIV_4(f[1]) + DIV_2(f2[2])+adj_tl;
		    of[3]=DIV_4(f[4]) + DIV_4(f[2]) + DIV_2(f2[3])+adj_tl;
        f+=4;
        f2+=4;
        of+=4;
	    }
      x=(w-2)&3;
	    while (x--)
	    {
		    *of++=DIV_4(f[1]) + DIV_4(f[-1]) + DIV_2(f2[0])+adj_tl;
        f++;
        f2++;
      }
      // bottom right
      *of++=DIV_2(f[-1])+DIV_2(f2[0])+adj_tl2; f++; f2++;
    }
  }
  else
  {
    // top line
    if (at_top) 
    {
      unsigned int *f2=f+w;
      int x;
      int adj_tl=0, adj_tl2=0;
      if (roundmode) { adj_tl = 0x02020202; adj_tl2 = 0x03030303; }
      // top left
      *of++=DIV_2(f[0])+DIV_4(f[1])+DIV_4(f2[0]) + adj_tl; f++; f2++;
	    // top center
      x=(w-2)/4;
	    while (x--)
	    {
		    of[0]=DIV_4(f[0]) + DIV_4(f[1]) + DIV_4(f[-1]) + DIV_4(f2[0]) + adj_tl2;
		    of[1]=DIV_4(f[1]) + DIV_4(f[2]) + DIV_4(f[0]) + DIV_4(f2[1]) + adj_tl2;
		    of[2]=DIV_4(f[2]) + DIV_4(f[3]) + DIV_4(f[1]) + DIV_4(f2[2]) + adj_tl2;
		    of[3]=DIV_4(f[3]) + DIV_4(f[4]) + DIV_4(f[2]) + DIV_4(f2[3]) + adj_tl2;
        f+=4;
        f2+=4;
        of+=4;
	    }
      x=(w-2)&3;
	    while (x--)
	    {
		    *of++=DIV_4(f[0]) + DIV_4(f[1]) + DIV_4(f[-1]) + DIV_4(f2[0]) + adj_tl2;
        f++;
        f2++;
      }
      // top right
      *of++=DIV_2(f[0])+DIV_4(f[-1])+DIV_4(f2[0]) + adj_tl; f++; f2++;
    }


	  // middle block
    {
      int y=outh-at_top-at_bottom;
      int adj_tl1=0,adj_tl2=0;
      unsigned __int64 adj2=0;
      if (roundmode) { adj_tl1=0x03030303; adj_tl2=0x04040404; adj2=0x0404040404040404i64; }
      while (y--)
      {
        int x;
        unsigned int *f2=f+w;
        unsigned int *f3=f-w;
      
        // left edge
        *of++=DIV_4(f[0])+DIV_4(f[1])+DIV_4(f2[0])+DIV_4(f3[0])+adj_tl1; f++; f2++; f3++;

        // middle of line
#ifdef NO_MMX
        x=(w-2)/4;
        if (roundmode)
        {
	        while (x--)
	        {
		        of[0]=DIV_2(f[0]) + DIV_8(f[1]) + DIV_8(f[-1]) + DIV_8(f2[0]) + DIV_8(f3[0]) + 0x04040404;
		        of[1]=DIV_2(f[1]) + DIV_8(f[2]) + DIV_8(f[0]) + DIV_8(f2[1]) + DIV_8(f3[1]) + 0x04040404;
		        of[2]=DIV_2(f[2]) + DIV_8(f[3]) + DIV_8(f[1]) + DIV_8(f2[2]) + DIV_8(f3[2]) + 0x04040404;
		        of[3]=DIV_2(f[3]) + DIV_8(f[4]) + DIV_8(f[2]) + DIV_8(f2[3]) + DIV_8(f3[3]) + 0x04040404;
            f+=4; f2+=4; f3+=4; of+=4;
          }
        }
        else
        {
	        while (x--)
	        {
		        of[0]=DIV_2(f[0]) + DIV_8(f[1]) + DIV_8(f[-1]) + DIV_8(f2[0]) + DIV_8(f3[0]);
		        of[1]=DIV_2(f[1]) + DIV_8(f[2]) + DIV_8(f[0]) + DIV_8(f2[1]) + DIV_8(f3[1]);
		        of[2]=DIV_2(f[2]) + DIV_8(f[3]) + DIV_8(f[1]) + DIV_8(f2[2]) + DIV_8(f3[2]);
		        of[3]=DIV_2(f[3]) + DIV_8(f[4]) + DIV_8(f[2]) + DIV_8(f2[3]) + DIV_8(f3[3]);
            f+=4; f2+=4; f3+=4; of+=4;
          }
        }
#else
        {
          __asm 
          {
            mov ecx, w
            mov edx, ecx
            mov ebx, edx
            neg ebx
            mov esi, f
            mov edi, of
            sub ecx, 2
            shr ecx, 2
            movq mm1, [esi-4]
            align 16
mmx_normal_blur_loop:
            movq mm0, [esi]

            movq mm2, [esi+4]
            pand mm0, mmx_mask1

            movq mm5, mm2

            movq mm7, [esi+8]
            pand mm1, mmx_mask3

            movq mm3, [esi+edx*4]
            pand mm2, mmx_mask3

            movq mm4, [esi+ebx*4]
            pand mm3, mmx_mask3

            psrl mm0, 1
            pand mm4, mmx_mask3

            psrl mm1, 3
            pand mm7, mmx_mask1

            movq mm6, [esi+12]
            psrl mm2, 3

            add esi, 16
            psrl mm3, 3

            paddb mm0, mm1
            psrl mm4, 3

            movq mm1, mm6

            paddb mm2, mm3
            paddb mm0, mm4

            movq mm3, [esi+edx*4-8]
            paddb mm0, mm2

            movq mm4, [esi+ebx*4-8]
            paddb mm0, [adj2]

            pand mm6, mmx_mask3

            movq [edi],mm0
            pand mm5, mmx_mask3

            psrl mm7, 1
            pand mm3, mmx_mask3

            psrl mm6, 3
            pand mm4, mmx_mask3

            psrl mm5, 3
            paddb mm7, mm6

            add edi, 16
            psrl mm3, 3

            psrl mm4, 3
            paddb mm5, mm3

            paddb mm7, mm4
            dec ecx

            paddb mm7, mm5
            paddb mm7, [adj2]

            movq [edi-8],mm7
            
            jnz mmx_normal_blur_loop
            mov of, edi
            mov f, esi           
          };
          f2=f+w; // update these bitches
          f3=f-w;
        }
#endif
        x=(w-2)&3;
	      while (x--)
	      {
		      *of++=DIV_2(f[0]) + DIV_8(f[1]) + DIV_8(f[-1]) + DIV_8(f2[0]) + DIV_8(f3[0]) + adj_tl2;
          f++;
          f2++;
          f3++;
        }

        // right block
        *of++=DIV_4(f[0])+DIV_4(f[-1])+DIV_4(f2[0])+DIV_4(f3[0]) + adj_tl1; f++;
	    }
    }

    // bottom block
    if (at_bottom)
    {
      unsigned int *f2=f-w;
      int adj_tl=0, adj_tl2=0;
      if (roundmode) { adj_tl = 0x02020202; adj_tl2 = 0x03030303; }
      int x;
      // bottom left
      *of++=DIV_2(f[0])+DIV_4(f[1])+DIV_4(f2[0]) + adj_tl; f++; f2++;
	    // bottom center
      x=(w-2)/4;
	    while (x--)
	    {
		    of[0]=DIV_4(f[0]) + DIV_4(f[1]) + DIV_4(f[-1]) + DIV_4(f2[0]) + adj_tl2;
		    of[1]=DIV_4(f[1]) + DIV_4(f[2]) + DIV_4(f[0]) + DIV_4(f2[1]) + adj_tl2;
		    of[2]=DIV_4(f[2]) + DIV_4(f[3]) + DIV_4(f[1]) + DIV_4(f2[2]) + adj_tl2;
		    of[3]=DIV_4(f[3]) + DIV_4(f[4]) + DIV_4(f[2]) + DIV_4(f2[3]) + adj_tl2;
        f+=4;
        f2+=4;
        of+=4;
	    }
      x=(w-2)&3;
	    while (x--)
	    {
		    *of++=DIV_4(f[0]) + DIV_4(f[1]) + DIV_4(f[-1]) + DIV_4(f2[0]) + adj_tl2;
        f++;
        f2++;
      }
      // bottom right
      *of++=DIV_2(f[0])+DIV_4(f[-1])+DIV_4(f2[0]) + adj_tl; f++; f2++;
    }
  }

#ifndef NO_MMX
  __asm emms;
#endif
  
	timingLeave(0);
}

int C_THISCLASS::smp_begin(int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (!enabled) return 0;
  return max_threads;
}


int C_THISCLASS::smp_finish(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h) // return value is that of render() for fbstuff etc
{
  return !!enabled;
}

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  smp_begin(1,visdata,isBeat,framebuffer,fbout,w,h);
  if (isBeat & 0x80000000) return 0;

  smp_render(0,1,visdata,isBeat,framebuffer,fbout,w,h);
  return smp_finish(visdata,isBeat,framebuffer,fbout,w,h);
}


C_RBASE *R_Blur(char *desc)
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
      if (g_this->enabled==2) CheckDlgButton(hwndDlg,IDC_RADIO3,BST_CHECKED);
      else if (g_this->enabled==3) CheckDlgButton(hwndDlg,IDC_RADIO4,BST_CHECKED);
      else if (g_this->enabled) CheckDlgButton(hwndDlg,IDC_RADIO2,BST_CHECKED);
      else CheckDlgButton(hwndDlg,IDC_RADIO1,BST_CHECKED);
      if (g_this->roundmode==0) CheckDlgButton(hwndDlg,IDC_ROUNDDOWN,BST_CHECKED);
      else CheckDlgButton(hwndDlg,IDC_ROUNDUP,BST_CHECKED);
			return 1;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDC_RADIO1)
        if (IsDlgButtonChecked(hwndDlg,IDC_RADIO1))
          g_this->enabled=0;
      if (LOWORD(wParam) == IDC_RADIO2)
        if (IsDlgButtonChecked(hwndDlg,IDC_RADIO2))
          g_this->enabled=1;
      if (LOWORD(wParam) == IDC_RADIO3)
        if (IsDlgButtonChecked(hwndDlg,IDC_RADIO3))
          g_this->enabled=2;
      if (LOWORD(wParam) == IDC_RADIO4)
        if (IsDlgButtonChecked(hwndDlg,IDC_RADIO4))
          g_this->enabled=3;
      if (LOWORD(wParam) == IDC_ROUNDUP)
        if (IsDlgButtonChecked(hwndDlg,IDC_ROUNDUP))
          g_this->roundmode=1;
      if (LOWORD(wParam) == IDC_ROUNDDOWN)
        if (IsDlgButtonChecked(hwndDlg,IDC_ROUNDDOWN))
          g_this->roundmode=0;
      return 0;
    return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_BLUR,hwndParent,g_DlgProc);
}

#else 
C_RBASE *R_Blur(char *desc) { return NULL; }
#endif