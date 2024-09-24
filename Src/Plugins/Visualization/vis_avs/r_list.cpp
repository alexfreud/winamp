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
#include <stdio.h>
#include <commctrl.h>
#include "resource.h"
#include "r_defs.h"
#include "r_unkn.h"
#include "r_list.h"
#include "render.h"
#include "undo.h"

#include "avs_eelif.h"
#include "../Agave/Language/api_language.h"

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))

char *C_RenderListClass::get_desc() { static char desc[128]; return WASABI_API_LNGSTRING_BUF(isroot?IDS_MAIN:IDS_EFFECT_LIST,desc,128); }

int g_config_seh=1;
extern int g_config_smp_mt,g_config_smp;

static char extsigstr[]="AVS 2.8+ Effect List Config";

void C_RenderListClass::load_config_code(unsigned char *data, int len)
{
  int pos=0;
  if (len-pos >= 4)
  {
    use_code=GET_INT(); pos+=4;
    load_string(effect_exp[0],data,pos,len);
    load_string(effect_exp[1],data,pos,len);
  }
  
}

void C_RenderListClass::load_config(unsigned char *data, int len)
{
  int pos=0,ext;
  if (pos<len) { mode=data[pos++]; }
  if (mode&0x80)
  {
    mode&=~0x80;
    mode=mode|GET_INT(); pos+=4;
  }
  ext = get_extended_datasize()+5;
  if (ext > 5)
  { 
    if (pos<ext) { inblendval=GET_INT(); pos+=4; }
    if (pos<ext) { outblendval=GET_INT(); pos+=4; }
    if (pos<ext) { bufferin=GET_INT(); pos+=4; }
    if (pos<ext) { bufferout=GET_INT(); pos+=4; }
    if (pos<ext) { ininvert=GET_INT(); pos+=4; }
    if (pos<ext) { outinvert=GET_INT(); pos+=4; }
    if (pos<ext-4) { beat_render=GET_INT(); pos+=4; }	//BU
    if (pos<ext-4) { beat_render_frames=GET_INT(); pos+=4; }
  }
  use_code=0;
  effect_exp[0].assign("");
  effect_exp[1].assign("");
  while (pos < len)
  {
    char s[33];
    T_RenderListType t;
    int l_len;
    t.effect_index=GET_INT(); pos+=4;
    if (t.effect_index >= DLLRENDERBASE)
    {
      if (pos+32 > len) break;
      memcpy(s,data+pos,32);
      s[32]=0;
      t.effect_index=(int)s;
      pos+=32;
    }
    if (pos+4 > len) break;
    l_len=GET_INT(); pos+=4;
    if (pos+l_len > len || l_len < 0) break;

    if (ext>5 && t.effect_index >= DLLRENDERBASE && !memcmp(s,extsigstr,strlen(extsigstr)+1))
    {
      load_config_code(data+pos,l_len);
    }
    else
    {
      t.render=g_render_library->CreateRenderer(&t.effect_index,&t.has_rbase2);
      if (t.render) 
      {
        t.render->load_config(data+pos,l_len);
        insertRender(&t,-1);
      }
    }
    pos+=l_len;
  }
}

int  C_RenderListClass::save_config_code(unsigned char *data)
{
  int pos=0;
  PUT_INT(use_code); pos+=4;
  save_string(data,pos,effect_exp[0]);
  save_string(data,pos,effect_exp[1]);
  return pos;
}

int  C_RenderListClass::save_config(unsigned char *data)
{
  return save_config_ex(data,0);
}
int  C_RenderListClass::save_config_ex(unsigned char *data, int rootsave)
{
	int pos=0;
	int x;
  if (!rootsave)
  {
    set_extended_datasize(36); // size of extended data + 4 cause we fucked up
    data[pos++]=(mode&0xff)|0x80;
    PUT_INT(mode); pos+=4;
    // extended_data
    PUT_INT(inblendval); pos+=4;
    PUT_INT(outblendval); pos+=4;
    PUT_INT(bufferin); pos+=4;
    PUT_INT(bufferout); pos+=4;
    PUT_INT(ininvert); pos+=4;
    PUT_INT(outinvert); pos+=4;
    PUT_INT(beat_render); pos+=4;
    PUT_INT(beat_render_frames); pos+=4;
    // end extended data
  }
  else data[pos++] = mode;

  if (!rootsave)
  {
    // write in our ext field
    PUT_INT(DLLRENDERBASE); pos+=4;
    char s[33];
    strncpy(s,extsigstr,32);
    memcpy(data+pos,s,32);
    pos+=32;
    int t=save_config_code(data+pos+4);
    PUT_INT(t);
    pos+=4+t;
  }


  for (x = 0; x < num_renders; x ++)
  {
    int t;
    int idx=renders[x].effect_index;
    if (idx==UNKN_ID)
    {
      C_UnknClass *r=(C_UnknClass *)renders[x].render;
      if (!r->idString[0]) { PUT_INT(r->id); pos+=4; }
      else
      {
        PUT_INT(r->id); pos+=4;
        memcpy(data+pos,r->idString,32);
        pos+=32;
      }
    }
    else
    {
  		PUT_INT(idx);	pos+=4;
      if (idx >= DLLRENDERBASE)
      {
        char s[33];
        strncpy(s,(char*)idx,32);
        memcpy(data+pos,s,32);
        pos+=32;
      }
    }

    t=renders[x].render->save_config(data+pos+4);
    PUT_INT(t);
    pos+=4+t;
  }



  return pos;
}


C_RenderListClass::C_RenderListClass(int iroot)
{
  AVS_EEL_INITINST();
  isstart=0;
#ifndef LASER
  nsaved=0;
  memset(nbw_save,0,sizeof(nbw_save));
  memset(nbw_save2,0,sizeof(nbw_save2));
  memset(nbh_save,0,sizeof(nbh_save));
  memset(nbh_save2,0,sizeof(nbh_save2));
  memset(nb_save,0,sizeof(nb_save));
  memset(nb_save2,0,sizeof(nb_save2));
#endif
	inblendval=128;
	outblendval=128;
	ininvert=0;

  InitializeCriticalSection(&rcs);
  use_code=0;
  inited=0;
  need_recompile=1;
  memset(codehandle,0,sizeof(codehandle));
  var_beat=0;


  effect_exp[0].assign("");
  effect_exp[1].assign("");
	outinvert=0;
	bufferin=0;
	bufferout=0;
  isroot=iroot;
  num_renders=0;
  num_renders_alloc=0;
  renders=NULL;
  thisfb=NULL;
  l_w=l_h=0;
  mode=0;
  beat_render = 0;
  beat_render_frames = 1;
  fake_enabled = 0;
#ifdef LASER
  if (!iroot) line_save=createLineList();
  else line_save=NULL;
#endif
}

extern int g_n_buffers_w[NBUF],g_n_buffers_h[NBUF];
extern void *g_n_buffers[NBUF];

#ifndef LASER
void C_RenderListClass::set_n_Context()
{
  if (!isroot) return;
  if (nsaved) return;
  nsaved=1;
  memcpy(nbw_save2,g_n_buffers_w,sizeof(nbw_save2));
  memcpy(nbh_save2,g_n_buffers_h,sizeof(nbh_save2));
  memcpy(nb_save2,g_n_buffers,sizeof(nb_save2));

  memcpy(g_n_buffers_w,nbw_save,sizeof(nbw_save));
  memcpy(g_n_buffers_h,nbh_save,sizeof(nbh_save));
  memcpy(g_n_buffers,nb_save,sizeof(nb_save));
}

void C_RenderListClass::unset_n_Context()
{
  if (!isroot) return;
  if (!nsaved) return;
  nsaved=0;

  memcpy(nbw_save,g_n_buffers_w,sizeof(nbw_save));
  memcpy(nbh_save,g_n_buffers_h,sizeof(nbh_save));
  memcpy(nb_save,g_n_buffers,sizeof(nb_save));
  
  memcpy(g_n_buffers_w,nbw_save2,sizeof(nbw_save2));
  memcpy(g_n_buffers_h,nbh_save2,sizeof(nbh_save2));
  memcpy(g_n_buffers,nb_save2,sizeof(nb_save2));

}
#endif

void C_RenderListClass::smp_cleanupthreads()
{

  if (smp_parms.threadTop>0)
  {
    if (smp_parms.hQuitHandle) SetEvent(smp_parms.hQuitHandle);

    WaitForMultipleObjects(smp_parms.threadTop,smp_parms.hThreads,TRUE,INFINITE);
    int x;
    for (x = 0; x < smp_parms.threadTop; x ++)
    {
      CloseHandle(smp_parms.hThreads[x]);
      CloseHandle(smp_parms.hThreadSignalsDone[x]);
      CloseHandle(smp_parms.hThreadSignalsStart[x]);
    }
  }

  if (smp_parms.hQuitHandle) CloseHandle(smp_parms.hQuitHandle);

  memset(&smp_parms,0,sizeof(smp_parms));
}

void C_RenderListClass::freeBuffers()
{
#ifndef LASER
  if (isroot)
  {
    int x;
    for (x = 0; x < NBUF; x ++)
    {
      if (nb_save[x]) GlobalFree(nb_save[x]);
      nb_save[x]=NULL;
      nbw_save[x]=nbh_save[x]=0;
    }
  }
#endif
}

C_RenderListClass::~C_RenderListClass()
{
#ifdef LASER
  if (line_save) delete line_save;
#endif
  clearRenders();

  // free nb_save
  freeBuffers();

  int x;
  for (x = 0; x < 2; x ++) 
  {
    freeCode(codehandle[x]);
    codehandle[x]=0;
  }
  AVS_EEL_QUITINST();
  DeleteCriticalSection(&rcs);
}

static int __inline depthof(int c, int i)
{
  int r= max(max((c & 0xFF), ((c & 0xFF00)>>8)), (c & 0xFF0000)>>16);
  return i ? 255 - r : r;
}


	
int C_RenderListClass::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  int is_preinit = (isBeat&0x80000000);

  if (isBeat && beat_render)
    fake_enabled = beat_render_frames;

  int use_enabled=enabled();
  int use_inblendval = inblendval;
  int use_outblendval = outblendval;
  int use_clear=clearfb();

  if (!isroot && use_code)
  { 
    if (need_recompile)
    {
      EnterCriticalSection(&rcs);

      if (!var_beat || g_reset_vars_on_recompile) 
      {
        clearVars();
        var_beat = registerVar("beat");
        var_alphain = registerVar("alphain");
        var_alphaout = registerVar("alphaout");
        var_enabled = registerVar("enabled");
        var_clear = registerVar("clear");
        var_w = registerVar("w");
        var_h = registerVar("h");
        inited=0;
      }
  
      need_recompile=0;
      int x;
      for (x = 0; x < 2; x ++) 
      {
        freeCode(codehandle[x]);
        codehandle[x]=compileCode(effect_exp[x].get());
      }

      LeaveCriticalSection(&rcs);
    }

    *var_beat = ((isBeat&1) && !is_preinit) ?1.0:0.0;
    *var_enabled=use_enabled ? 1.0:0.0;
    *var_w=(double)w;
    *var_h=(double)h;
    *var_clear=use_clear ? 1.0:0.0;
    *var_alphain=use_inblendval/255.0;
    *var_alphaout=use_outblendval/255.0;
    if (codehandle[0] && !inited) { executeCode(codehandle[0],visdata); inited=1; }
    executeCode(codehandle[1],visdata);

    if (!is_preinit)
    {
      isBeat = *var_beat > 0.1 || *var_beat < -0.1;
    }
    use_inblendval = (int) (*var_alphain * 255.0);
    if (use_inblendval < 0) use_inblendval=0;
    else if (use_inblendval>255) use_inblendval=255;
    use_outblendval = (int) (*var_alphaout * 255.0);
    if (use_outblendval < 0) use_outblendval=0;
    else if (use_outblendval>255) use_outblendval=255;

    use_enabled = *var_enabled > 0.1 || *var_enabled < -0.1;
    use_clear = *var_clear > 0.1 || *var_clear < -0.1;

    // code execute
  }

#ifndef LASER
  // root/replaceinout special cases
  if (isroot || (use_enabled && blendin()==1 && blendout()==1)) 
#endif
  {
    int s=0,x;
#ifndef LASER
    int line_blend_mode_save=g_line_blend_mode;
    if (thisfb) GlobalFree((HGLOBAL)thisfb); 
    thisfb=NULL;
    if (use_clear&&(isroot||blendin()!=1)) 
      memset(framebuffer,0,w*h*sizeof(int));
    if (!is_preinit) 
    {
      g_line_blend_mode=0;
      set_n_Context();
    }
#else
    if (isroot || use_enabled) 
    {
      void LineListSwap(C_LineListBase *item1, C_LineListBase *item2);
      if (!isroot) 
      {
        line_save->ClearLineList();
        LineListSwap(line_save,g_laser_linelist);
      }
#endif
      for (x = 0; x < num_renders; x ++)
      {     
        int t=0;
        int smp_max_threads = g_config_smp ? g_config_smp_mt : 0;
        C_RBASE2 *rb2 = (C_RBASE2*)renders[x].render;

        if (renders[x].has_rbase2 && smp_max_threads > 1 && (rb2->smp_getflags()&1))
        {
          if (smp_max_threads>MAX_SMP_THREADS) smp_max_threads=MAX_SMP_THREADS;

          int nt=smp_max_threads;
          nt=rb2->smp_begin(nt,visdata,isBeat,s?fbout:framebuffer,s?framebuffer:fbout,w,h);
          if (!is_preinit && nt>0)
          {
            if (nt>smp_max_threads)nt=smp_max_threads;

            // launch threads
            smp_Render(nt,rb2,visdata,isBeat,s?fbout:framebuffer,s?framebuffer:fbout,w,h);


            t=rb2->smp_finish(visdata,isBeat,s?fbout:framebuffer,s?framebuffer:fbout,w,h);
          }

        }
        else
        {
          if (g_config_seh && renders[x].effect_index != LIST_ID)
          {
            __try 
            {
              t=renders[x].render->render(visdata,isBeat,s?fbout:framebuffer,s?framebuffer:fbout,w,h);
            }
  		      __except(EXCEPTION_EXECUTE_HANDLER)
	  	      {
              t=0;
            }
          }
          else
          {
            t=renders[x].render->render(visdata,isBeat,s?fbout:framebuffer,s?framebuffer:fbout,w,h);
          }
        }


        if (t&1) s^=1;
        if (!is_preinit) 
        {
          if (t&0x10000000) isBeat=1;
          if (t&0x20000000) isBeat=0;
        }
      }
#ifdef LASER
      if (!isroot) 
      {
        LineListSwap(line_save,g_laser_linelist);
        int us=g_laser_linelist->GetUsedLines();
        int n=g_laser_linelist->GetMaxLines()-us;
        if (line_save->GetUsedLines() < n) n=line_save->GetUsedLines();
        g_laser_linelist->SetLines(line_save->GetLineList(),us,n);
        g_laser_linelist->SetUsedLines(us+n);
      }
    }
#else
    if (!is_preinit)
    {
      g_line_blend_mode=line_blend_mode_save;
      unset_n_Context();
    }
#endif
    fake_enabled--;
    return s;
  }
  
#ifndef LASER
  // check to see if we're enabled
  if (!use_enabled) 
  {
    if (thisfb) GlobalFree((HGLOBAL)thisfb); 
    thisfb=NULL;
    return 0;
  }

  fake_enabled--;


  // handle resize
  if (l_w != w || l_h != h || !thisfb)
  {
    extern int config_reuseonresize;
    int do_resize=config_reuseonresize && !!thisfb && l_w && l_h && !use_clear;

    int *newfb=(int*)GlobalAlloc(do_resize?GMEM_FIXED:GPTR,w*h*sizeof(int));
    if (newfb && do_resize) 
    {
      int x,y;
      int dxpos=(l_w<<16)/w;
      int ypos=0;
      int dypos=(l_h<<16)/h;
      int *out=newfb;
      for (y = 0; y < h; y++)
      {
        int *p=thisfb + l_w * (ypos>>16);
        int xpos=0;
        for (x = 0; x < w; x ++)
        {
          *out++=p[xpos>>16];
          xpos+=dxpos;
        }
        ypos+=dypos;
      }
    }
    l_w=w;
    l_h=h;
    if (thisfb) GlobalFree((HGLOBAL)thisfb);
    thisfb=newfb;
  }
  // handle clear mode
  if (use_clear) memset(thisfb,0,w*h*sizeof(int));


  // blend parent framebuffer into current, if necessary

  if (!is_preinit)
  {
    int x=w*h;
    int *tfb=framebuffer;
    int *o=thisfb;
    set_n_Context();
    int use_blendin=blendin();
    if (use_blendin == 10 && use_inblendval >= 255)
      use_blendin=1;

    switch (use_blendin)
    {
      case 1:
        memcpy(o,tfb,w*h*sizeof(int));
      break;
      case 2:
        mmx_avgblend_block(o,tfb,x);
      break;
      case 3:
        while (x--)
        {
          *o=BLEND_MAX(*o,*tfb++);
          o++;
        }
      break;
      case 4:
        mmx_addblend_block(o,tfb,x);
      break;
      case 5:
        while (x--)
        {
          *o=BLEND_SUB(*o,*tfb++);
          o++;
        }
      break;
      case 6:
        while (x--)
        {
          *o=BLEND_SUB(*tfb++,*o);
          o++;
        }
      break;
      case 7:
        {
          int y=h/2;
          while (y-- > 0)
          {
            memcpy(o,tfb,w*sizeof(int));
            tfb+=w*2;
            o+=w*2;
          }
        }
      break;
      case 8:
        {
          int r=0;
          int y=h;
          while (y-- > 0)
          {
            int *out, *in;
            int x=w/2;
            out=o+r;
            in=tfb+r;
            r^=1;
            while (x-- > 0)
            {
              *out=*in;
              out+=2;
              in+=2;
            }
            o+=w;
            tfb+=w;
          }
        }
      break;
      case 9:
        while (x--)
        {
          *o=*o^*tfb++;
          o++;
        }
      break;
      case 10:
        mmx_adjblend_block(o,tfb,o,x,use_inblendval);
      break;
      case 11:
          mmx_mulblend_block(o,tfb,x);
      break;
      case 13:
        while (x--)
        {
          *o=BLEND_MIN(*o,*tfb++);
          o++;
        }
      break;
      case 12:
				{
					int *buf=(int*)getGlobalBuffer(w,h,bufferin,0);
					if (!buf) break;
					while (x--)
					{
						*o=BLEND_ADJ(*tfb++,*o, depthof(*buf, ininvert));
						o++;
						buf++;
					}
				}
#ifndef NO_MMX
        __asm emms;
#endif
      break;
      default:
      break;
    }
    unset_n_Context();
  }

  int s=0;
  int x;
  int line_blend_mode_save=g_line_blend_mode;
  if (!is_preinit) g_line_blend_mode=0;

  for (x = 0; x < num_renders; x ++)
  {
    int t=0;
    
    int smp_max_threads;
    C_RBASE2 *rb2;
    
    if (renders[x].has_rbase2 && (smp_max_threads=g_config_smp ? g_config_smp_mt : 0) > 1 && ((rb2 = (C_RBASE2*)renders[x].render)->smp_getflags()&1))
    {
      if (smp_max_threads>MAX_SMP_THREADS) smp_max_threads=MAX_SMP_THREADS;

      int nt=smp_max_threads;
      nt=rb2->smp_begin(nt,visdata,isBeat,s?fbout:framebuffer,s?framebuffer:fbout,w,h);
      if (!is_preinit && nt>0)
      {
        if (nt>smp_max_threads)nt=smp_max_threads;

        // launch threads
        smp_Render(nt,rb2,visdata,isBeat,s?fbout:thisfb,s?thisfb:fbout,w,h);

        t=rb2->smp_finish(visdata,isBeat,s?fbout:framebuffer,s?framebuffer:fbout,w,h);
      }

    }
    else if (g_config_seh && renders[x].effect_index != LIST_ID)
    {
      __try 
      {
        t=renders[x].render->render(visdata,isBeat,s?fbout:thisfb,s?thisfb:fbout,w,h);
      }
  		__except(EXCEPTION_EXECUTE_HANDLER)
	  	{
        t=0;
      }
    }
    else
    {
      t=renders[x].render->render(visdata,isBeat,s?fbout:thisfb,s?thisfb:fbout,w,h);
    }


    if (t&1) s^=1;
    if (!is_preinit) 
    {
      if (t&0x10000000) 
          isBeat=1;
      if (t&0x20000000) 
          isBeat=0;
    }
  }
  if (!is_preinit) g_line_blend_mode=line_blend_mode_save;

  // if s==1 at this point, data we want is in fbout.

  if (!is_preinit) 
  {
    if (s) memcpy(thisfb,fbout,w*h*sizeof(int));

    int *tfb=s?fbout:thisfb;
    int *o=framebuffer;
    x=w*h;
    set_n_Context();

    int use_blendout=blendout();
    if (use_blendout == 10 && use_outblendval >= 255)
      use_blendout=1;
    switch (use_blendout)
    {
      case 1:
        if (s) 
        { 
          unset_n_Context(); 
          return 1; 
        }
        memcpy(o,tfb,x*sizeof(int));
      break;
      case 2:
        mmx_avgblend_block(o,tfb,x);
      break;
      case 3:
        while (x--)
        {
          *o=BLEND_MAX(*o,*tfb++);
          o++;
        }
      break;
      case 4:
            mmx_addblend_block(o,tfb,x);
      break;
      case 5:
        while (x--)
        {
          *o=BLEND_SUB(*o,*tfb++);
          o++;
        }
      break;
      case 6:
        while (x--)
        {
          *o=BLEND_SUB(*tfb++,*o);
          o++;
        }
      break;
      case 7:
        {
          int y=h/2;
          while (y-- > 0)
          {
            memcpy(o,tfb,w*sizeof(int));
            tfb+=w*2;
            o+=w*2;
          }
        }
      break;
      case 8:
        {
          int r=0;
          int y=h;
          while (y-- > 0)
          {
            int *out, *in;
            int x=w/2;
            out=o+r;
            in=tfb+r;
            r^=1;
            while (x-- > 0)
            {
              *out=*in;
              out+=2;
              in+=2;
            }
            o+=w;
            tfb+=w;
          }
        }
      break;
      case 9:
        while (x--)
        {
          *o=*o^*tfb++;
          o++;
        }
      break;
      case 10:
          mmx_adjblend_block(o,tfb,o,x,use_outblendval);
      break;
      case 11:
            mmx_mulblend_block(o,tfb,x);
      break;
      case 13:
        while (x--)
        {
          *o=BLEND_MIN(*o,*tfb++);
          o++;
        }
      break;
      case 12:
			  {
				  int *buf=(int*)getGlobalBuffer(w,h,bufferout,0);
				  if (!buf) break;
				  while (x--)
				  {
					  *o=BLEND_ADJ(*tfb++,*o, depthof(*buf, outinvert));
					  o++;
					  buf++;
				  }
  #ifndef NO_MMX
          __asm emms;
  #endif
			  }
      break;
      default:
      break;
    }
    unset_n_Context();
  }
  return 0;
#endif // !LASER
}

int C_RenderListClass::getNumRenders(void)
{
  return num_renders;
}

C_RenderListClass::T_RenderListType *C_RenderListClass::getRender(int index)
{
  if (index >= 0 && index < num_renders) return &renders[index];
  return NULL;
}

int C_RenderListClass::findRender(T_RenderListType *r)
{
  int idx;
  if (!r) return -1;
  for (idx=0;idx<num_renders&&renders[idx].render!=r->render; idx++);
  if (idx<num_renders) return idx;
  return -1;
}

int C_RenderListClass::removeRenderFrom(T_RenderListType *r, int del)
{
  int idx;
  if (!r) return 1;
  for (idx=0;idx<num_renders&&renders[idx].render!=r->render; idx++);
  return removeRender(idx,del);
}

int C_RenderListClass::removeRender(int index, int del)
{
  if (index >= 0 && index < num_renders)
  {
    if (del&&renders[index].render) delete renders[index].render;
    num_renders--;
    while (index<num_renders)
    {
      renders[index]=renders[index+1];
      index++;
    }
    if (!num_renders)
    {
      num_renders_alloc=0;
      if (renders) GlobalFree((HGLOBAL)renders);
      renders=NULL;
    }
    return 0;
  }
  return 1;
}
void C_RenderListClass::clearRenders(void)
{
  int x;
  if (renders) 
  {
    for (x = 0; x < num_renders; x ++)
    {
      delete renders[x].render;
    }
    GlobalFree((HGLOBAL)renders);
  }
  num_renders=0;
  num_renders_alloc=0;
  renders=NULL;
  if (thisfb) GlobalFree((HGLOBAL)thisfb);
  thisfb=0;
}

int C_RenderListClass::insertRenderBefore(T_RenderListType *r, T_RenderListType *before)
{
  int idx;
  if (!before) idx=num_renders;
  else for (idx=0;idx<num_renders&&renders[idx].render!=before->render; idx++);
  return insertRender(r,idx);
}

int C_RenderListClass::insertRender(T_RenderListType *r, int index) // index=-1 for add
{
  if (num_renders+1 >= num_renders_alloc || !renders)
  {
    num_renders_alloc=num_renders+16;
    T_RenderListType *newr=(T_RenderListType *) GlobalAlloc(GPTR,num_renders_alloc*sizeof(T_RenderListType));
    if (!newr) return -1;
    if (num_renders&&renders)
    {
      memcpy(newr,renders,num_renders*sizeof(T_RenderListType));
    }
    if (renders)
    {
      GlobalFree((HGLOBAL)renders);
    }
    renders=newr;
  }

  if (index<0 || index>=num_renders)
  {
    renders[num_renders]=*r;
    return num_renders++;
  }
  int x;
  for (x=num_renders++; x > index; x--)
  {
    renders[x]=renders[x-1];
  }
  renders[x]=*r;

  return x;
}

void C_RenderListClass::FillBufferCombo(HWND dlg, int ctl)
{
int i=0;
char txt[64];
for (i=0;i<NBUF;i++)
	{
	wsprintf(txt, WASABI_API_LNGSTRING(IDS_BUFFER_X), i+1);
	SendDlgItemMessage(dlg, ctl, CB_ADDSTRING, 0, (LPARAM)txt);
	}
}


static C_RenderListClass *g_this;
int blendmodes[] = 
{
  IDS_IGNORE,
  IDS_REPLACE,
  IDS_50_50,
  IDS_MAXIMUM,
  IDS_ADDITIVE,
  IDS_SUBTRACTIVE_1,
  IDS_SUBTRACTIVE_2,
  IDS_EVERY_OTHER_LINE,
  IDS_EVERY_OTHER_PIXEL,
  IDS_XOR,
  IDS_ADJUSTABLE,
  IDS_MULTIPLY,
  IDS_BUFFER,
  IDS_MINIMUM,
};



BOOL CALLBACK C_RenderListClass::g_DlgProcRoot(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
#ifdef LASER
      ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK1),SW_HIDE);
#endif
      {
#ifndef LASER
        if (g_this->clearfb()) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
#endif
      }
 		return 1;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_CHECK1:
          g_this->set_clearfb(IsDlgButtonChecked(hwndDlg,IDC_CHECK1));
        break;
      }
    break;
	}
	return 0;
}


BOOL CALLBACK C_RenderListClass::g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
#ifdef LASER
      ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK1),SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_INSLIDE), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_OUTSLIDE), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_CBBUF1), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_CBBUF2), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_INVERT1), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_INVERT2), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_COMBO1), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_COMBO2), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_FR1), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_FR2), SW_HIDE);
#endif
      {
#ifndef LASER
        {
          int x;
          for (x = 0; x < sizeof(blendmodes)/sizeof(blendmodes[0]); x ++)
          {
            char blendStr[64];
            SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRING_BUF(blendmodes[x],blendStr,64));
            SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)blendStr);
          }
          SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,(WPARAM)g_this->blendout(),0);
          SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETCURSEL,(WPARAM)g_this->blendin(),0);
					ShowWindow(GetDlgItem(hwndDlg, IDC_INSLIDE), (g_this->blendin() == 10) ? SW_NORMAL : SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_OUTSLIDE), (g_this->blendout() == 10) ? SW_NORMAL : SW_HIDE);
					SendDlgItemMessage(hwndDlg, IDC_INSLIDE, TBM_SETRANGE, TRUE, MAKELONG(0, 255));
					SendDlgItemMessage(hwndDlg, IDC_INSLIDE, TBM_SETPOS, TRUE, (int)(g_this->inblendval));
					SendDlgItemMessage(hwndDlg, IDC_OUTSLIDE, TBM_SETRANGE, TRUE, MAKELONG(0, 255));
					SendDlgItemMessage(hwndDlg, IDC_OUTSLIDE, TBM_SETPOS, TRUE, (int)(g_this->outblendval));
					ShowWindow(GetDlgItem(hwndDlg, IDC_CBBUF1), (g_this->blendin() == 12) ? SW_NORMAL : SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_CBBUF2), (g_this->blendout() == 12) ? SW_NORMAL : SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_INVERT1), (g_this->blendin() == 12) ? SW_NORMAL : SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_INVERT2), (g_this->blendout() == 12) ? SW_NORMAL : SW_HIDE);
					g_this->FillBufferCombo(hwndDlg, IDC_CBBUF1);
					g_this->FillBufferCombo(hwndDlg, IDC_CBBUF2);
					SendDlgItemMessage(hwndDlg, IDC_CBBUF1, CB_SETCURSEL, (WPARAM) g_this->bufferin, 0);
					SendDlgItemMessage(hwndDlg, IDC_CBBUF2, CB_SETCURSEL, (WPARAM) g_this->bufferout, 0);
          if (g_this->ininvert) CheckDlgButton(hwndDlg,IDC_INVERT1,BST_CHECKED);
          if (g_this->outinvert) CheckDlgButton(hwndDlg,IDC_INVERT2,BST_CHECKED);
        }
        if (g_this->clearfb()) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
#endif
        g_this->isstart=1;
        SetDlgItemText(hwndDlg,IDC_EDIT4,g_this->effect_exp[0].get());
        SetDlgItemText(hwndDlg,IDC_EDIT5,g_this->effect_exp[1].get());
        g_this->isstart=0;


        if (((g_this->mode&2)^2)) 
          CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);
        if (g_this->beat_render) CheckDlgButton(hwndDlg,IDC_CHECK3,BST_CHECKED);
        else EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT1), FALSE);
        if (g_this->use_code) CheckDlgButton(hwndDlg,IDC_CHECK4,BST_CHECKED);
        char buf[999];
        wsprintf(buf, "%d", g_this->beat_render_frames);
        SetDlgItemText(hwndDlg,IDC_EDIT1,buf);
      }
 		return 1;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_EDIT4:
        case IDC_EDIT5:
          if (!g_this->isstart && HIWORD(wParam) == EN_CHANGE)
          {
            EnterCriticalSection(&g_this->rcs);
            g_this->effect_exp[0].get_from_dlgitem(hwndDlg,IDC_EDIT4);
            g_this->effect_exp[1].get_from_dlgitem(hwndDlg,IDC_EDIT5);
            g_this->need_recompile=1;
				    if (LOWORD(wParam) == IDC_EDIT4) g_this->inited = 0;
            LeaveCriticalSection(&g_this->rcs);
          }
        break;
        case IDC_COMBO1:
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            int r=SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
            if (r!=CB_ERR) 
							{
							g_this->set_blendout(r);
							ShowWindow(GetDlgItem(hwndDlg, IDC_OUTSLIDE), (r == 10) ? SW_NORMAL : SW_HIDE);
							ShowWindow(GetDlgItem(hwndDlg, IDC_CBBUF2), (g_this->blendout() == 12) ? SW_NORMAL : SW_HIDE);
							ShowWindow(GetDlgItem(hwndDlg, IDC_INVERT2), (g_this->blendout() == 12) ? SW_NORMAL : SW_HIDE);
							}
          }
        break;
        case IDC_COMBO2:
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            int r=SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCURSEL,0,0);
            if (r!=CB_ERR) 
							{
							g_this->set_blendin(r);
							ShowWindow(GetDlgItem(hwndDlg, IDC_INSLIDE), (r == 10) ? SW_NORMAL : SW_HIDE);
							ShowWindow(GetDlgItem(hwndDlg, IDC_CBBUF1), (g_this->blendin() == 12) ? SW_NORMAL : SW_HIDE);
							ShowWindow(GetDlgItem(hwndDlg, IDC_INVERT1), (g_this->blendin() == 12) ? SW_NORMAL : SW_HIDE);
							}
          }
        break;
				case IDC_CBBUF1:
          if (HIWORD(wParam) == CBN_SELCHANGE)
						g_this->bufferin = SendDlgItemMessage(hwndDlg, IDC_CBBUF1, CB_GETCURSEL, 0, 0);
				break;
				case IDC_CBBUF2:
          if (HIWORD(wParam) == CBN_SELCHANGE)
						g_this->bufferout = SendDlgItemMessage(hwndDlg, IDC_CBBUF2, CB_GETCURSEL, 0, 0);
				break;
        case IDC_CHECK1:
          g_this->set_clearfb(IsDlgButtonChecked(hwndDlg,IDC_CHECK1));
        break;
        case IDC_CHECK2:
          g_this->set_enabled(IsDlgButtonChecked(hwndDlg,IDC_CHECK2));
        break;
        case IDC_INVERT1:
					g_this->ininvert = IsDlgButtonChecked(hwndDlg,IDC_INVERT1);
					break;
        case IDC_INVERT2:
          g_this->outinvert = IsDlgButtonChecked(hwndDlg,IDC_INVERT2);
					break;
	      case IDC_CHECK3:
          g_this->beat_render = IsDlgButtonChecked(hwndDlg,IDC_CHECK3);
          EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT1), g_this->beat_render);
        break;
        case IDC_CHECK4:
          g_this->use_code = !!IsDlgButtonChecked(hwndDlg,IDC_CHECK4);
        break;
        case IDC_EDIT1:
          if (HIWORD(wParam) == EN_CHANGE) {
            char buf[999]="1";
            GetDlgItemText(hwndDlg, IDC_EDIT1, buf, 999);
            buf[998] = 0;
            g_this->beat_render_frames = atoi(buf);
          }
        break;
        case IDC_BUTTON2:
        {
/*        char text[4096];
          WASABI_API_LNGSTRING_BUF(IDS_EFFECT_LIST,text,4096);
          int titlelen = lstrlen(text)+1;
          lstrcpyn(text+titlelen,GetTextResource(IDR_EFFECT_LIST),4095-titlelen);
*/
          char *text="Effect List\0"
                     "Read/write 'enabled' to get/set whether the effect list is enabled for this frame\r\n"
                     "Read/write 'beat' to get/set whether there is currently a beat\r\n"
                     "Read/write 'clear' to get/set whether to clear the framebuffer\r\n"
                     "If the input blend is set to adjustable, 'alphain' can be set from 0.0-1.0\r\n"
                     "If the output blend is set to adjustable, 'alphaout' can be set from 0.0-1.0\r\n"
                     "'w' and 'h' are set with the current width and height of the frame\r\n"
            ;

          compilerfunctionlist(hwndDlg,text);
        }
        break;
      }
    break;
	case WM_NOTIFY:
		if (LOWORD(wParam) == IDC_INSLIDE)
			g_this->inblendval = SendDlgItemMessage(hwndDlg, IDC_INSLIDE, TBM_GETPOS, 0, 0);
		if (LOWORD(wParam) == IDC_OUTSLIDE)
			g_this->outblendval = SendDlgItemMessage(hwndDlg, IDC_OUTSLIDE, TBM_GETPOS, 0, 0);
		break;
	}
	return 0;
}


HWND C_RenderListClass::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
  return WASABI_API_CREATEDIALOG(isroot?IDD_CFG_LISTROOT:IDD_CFG_LIST,hwndParent,(WNDPROC)(isroot?g_DlgProcRoot:g_DlgProc));
//  return NULL;
}

char C_RenderListClass::sig_str[] = "Nullsoft AVS Preset 0.2\x1a";

int C_RenderListClass::__SavePreset(char *filename)
{
  EnterCriticalSection(&g_render_cs);
	unsigned char *data = (unsigned char *) GlobalAlloc(GPTR,1124*1024);
  int success=-1;
  if (data)
  {
    int pos=0;
  	memcpy(data+pos,sig_str,strlen(sig_str)); pos += strlen(sig_str);
    pos+=save_config_ex(data+pos,1);
    if (pos < 1024*1024) 
    {
      HANDLE fp=CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	    if (fp!=INVALID_HANDLE_VALUE)
      {
        DWORD dw;
        success=0;
        WriteFile(fp,data,pos,&dw,NULL);
        CloseHandle(fp);
      }
      else success=2;
    }
    else success=1;
    GlobalFree((HGLOBAL)data);
  }
  LeaveCriticalSection(&g_render_cs);
	return success;
}

int C_RenderListClass::__LoadPreset(char *filename, int clear)
{
  EnterCriticalSection(&g_render_cs);
	unsigned char *data = (unsigned char *) GlobalAlloc(GPTR,1024*1024);
  int success=1;
  if (clear) clearRenders();
  if (data)
  {
  //  OutputDebugString(filename);
    HANDLE fp=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (fp!=INVALID_HANDLE_VALUE)
    {
      DWORD len=GetFileSize(fp,NULL);
      if (len == 0xffffffff) len=0;
      if (!ReadFile(fp,data,min(len,1024*1024),&len,NULL)) len=0;
		  CloseHandle(fp);
	    if (len>strlen(sig_str)+2 && !memcmp(data,sig_str,strlen(sig_str)-2) &&
           data[strlen(sig_str)-2] >= '1' &&
           data[strlen(sig_str)-2] <= '2' &&
           data[strlen(sig_str)-1] == '\x1a')
	    {
	      load_config(data+strlen(sig_str),len-strlen(sig_str));
        success=0;
      }
      else 
      {
   //     if (len<=strlen(sig_str)+2) MessageBox(NULL,"Error loading preset: len",filename,MB_OK);
 //       else MessageBox(NULL,"Error loading preset: signature",filename,MB_OK);
      }
    }
  //  else MessageBox(NULL,"Error loading preset: fopen",filename,MB_OK);
    GlobalFree((HGLOBAL)data);
  }
//  else MessageBox(NULL,"Error loading preset: MALLOC",filename,MB_OK);
  LeaveCriticalSection(&g_render_cs);
  return success;
}

int C_RenderListClass::__SavePresetToUndo(C_UndoItem &item)
{
  EnterCriticalSection(&g_render_cs);
	unsigned char *data = (unsigned char *) GlobalAlloc(GPTR,1124*1024);
  int success=-1;
  if (data)
  {
    // Do whatever the file saving stuff did
    int pos=0;
  	memcpy(data+pos,sig_str,strlen(sig_str)); pos += strlen(sig_str);
    pos+=save_config_ex(data+pos,1);

    // And then set the data into the undo object.
    if (pos < 1024*1024) 
    {
      item.set(data, pos, true); // all undo items start dirty.
    }

    else success=1;
    GlobalFree((HGLOBAL)data);
  }
  LeaveCriticalSection(&g_render_cs);
	return success;
}

int C_RenderListClass::__LoadPresetFromUndo(C_UndoItem &item, int clear)
{
  EnterCriticalSection(&g_render_cs);
	unsigned char *data = (unsigned char *) GlobalAlloc(GPTR,1024*1024);
  int success=1;
  if (clear) clearRenders();
  if (data)
  {
    if (item.size() < 1024*1024)
    {
      // Get the data from the undo object.
      DWORD len = item.size();
      if (len == 0xffffffff) len=0;
      memcpy(data, item.get(), item.size());

      // And then do whatever the file loading stuff did.
	    if (!memcmp(data,sig_str,strlen(sig_str)-2) &&
           data[strlen(sig_str)-2] >= '1' &&
           data[strlen(sig_str)-2] <= '2' &&
           data[strlen(sig_str)-1] == '\x1a')
	    {
	      load_config(data+strlen(sig_str),len-strlen(sig_str));
        success=0;
      }
    }
    GlobalFree((HGLOBAL)data);
  }
  LeaveCriticalSection(&g_render_cs);
  return success;
}




/// smp fun

void C_RenderListClass::smp_Render(int minthreads, C_RBASE2 *render, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  int x;
  smp_parms.nthreads=minthreads;
  if (!smp_parms.hQuitHandle) smp_parms.hQuitHandle=CreateEvent(NULL,TRUE,FALSE,NULL);

  smp_parms.vis_data_ptr=visdata;
  smp_parms.isBeat=isBeat;
  smp_parms.framebuffer=framebuffer;
  smp_parms.fbout=fbout;
  smp_parms.w=w;
  smp_parms.h=h;
  smp_parms.render=render;
  for (x = 0; x < minthreads; x ++)
  {
    if (x >= smp_parms.threadTop)
    {
      DWORD id;
      smp_parms.hThreadSignalsStart[x]=CreateEvent(NULL,FALSE,TRUE,NULL);
      smp_parms.hThreadSignalsDone[x]=CreateEvent(NULL,FALSE,FALSE,NULL);

      smp_parms.hThreads[x]=CreateThread(NULL,0,smp_threadProc,(LPVOID)(x),0,&id);
      smp_parms.threadTop=x+1;
    }
    else
      SetEvent(smp_parms.hThreadSignalsStart[x]);
  }
  WaitForMultipleObjects(smp_parms.nthreads,smp_parms.hThreadSignalsDone,TRUE,INFINITE);
}

DWORD WINAPI C_RenderListClass::smp_threadProc(LPVOID parm)
{
  int which=(int)parm;
  HANDLE hdls[2]={smp_parms.hThreadSignalsStart[which],smp_parms.hQuitHandle};
  for (;;)
  {
    if (WaitForMultipleObjects(2,hdls,FALSE,INFINITE) == WAIT_OBJECT_0 + 1) return 0;

    smp_parms.render->smp_render(which,smp_parms.nthreads,
      *(char (*)[2][2][576])smp_parms.vis_data_ptr,
      
      smp_parms.isBeat,smp_parms.framebuffer,smp_parms.fbout,smp_parms.w,smp_parms.h);
    SetEvent(smp_parms.hThreadSignalsDone[which]);
  }
}

C_RenderListClass::_s_smp_parms C_RenderListClass::smp_parms;