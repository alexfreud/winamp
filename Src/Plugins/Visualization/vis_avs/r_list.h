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
#ifndef _R_LIST_H_
#define _R_LIST_H_

#include "avs_eelif.h"
#define LIST_ID 0xfffffffe

extern unsigned char blendtable[256][256];
extern BOOL blendtableInited;

class C_RenderTransitionClass;
class C_UndoItem;

class C_RenderListClass : public C_RBASE {
	friend C_RenderTransitionClass;
  public:
    typedef struct 
    {
      C_RBASE *render;
      int effect_index;
      int has_rbase2;
    } T_RenderListType;

	protected:
    static char sig_str[];
    int *thisfb;
    int l_w, l_h;
    int isroot;

    int num_renders,num_renders_alloc;
    T_RenderListType *renders;
		int inblendval, outblendval;
		int bufferin, bufferout;
		int ininvert, outinvert;

    int use_code;
    RString effect_exp[2];

		int inited;
    NSEEL_CODEHANDLE codehandle[4];
    int need_recompile;
    CRITICAL_SECTION rcs;

    NSEEL_VMCTX AVS_EEL_CONTEXTNAME;
    double *var_beat, *var_alphain, *var_alphaout, *var_enabled, *var_clear, *var_w, *var_h;
    int isstart;

    int mode;

    int beat_render, beat_render_frames;
    int fake_enabled;

#ifdef LASER
    C_LineListBase *line_save;
#else
    void set_n_Context();
    void unset_n_Context();

    int nbw_save[NBUF],nbh_save[NBUF]; // these are our framebuffers
    void *nb_save[NBUF];

    int nbw_save2[NBUF],nbh_save2[NBUF]; // this are temp space for saving the global ones
    void *nb_save2[NBUF];
    int nsaved;
#endif

#define MAX_SMP_THREADS 8
    // smp stuff
    void smp_Render(int minthreads, C_RBASE2 *render, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
    typedef struct 
    {
      void *vis_data_ptr;
      int nthreads;
      int isBeat;
      int *framebuffer;
      int *fbout;
      int w;
      int h;
      C_RBASE2 *render;



      HANDLE hQuitHandle;
      HANDLE hThreads[MAX_SMP_THREADS];
      HANDLE hThreadSignalsStart[MAX_SMP_THREADS];
      HANDLE hThreadSignalsDone[MAX_SMP_THREADS];

      int threadTop;

    } _s_smp_parms;

    
    static _s_smp_parms smp_parms;
    static DWORD WINAPI smp_threadProc(LPVOID parm);

	public:

    static void smp_cleanupthreads();

    C_RenderListClass(int iroot=0);
		virtual ~C_RenderListClass();

		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
		virtual char *get_desc();

    int getNumRenders(void);
    T_RenderListType *getRender(int index);
    int findRender(T_RenderListType *r);
    int removeRender(int index, int del); // return 0 on success
    int removeRenderFrom(T_RenderListType *r, int del); // return 0 on success
    int insertRender(T_RenderListType *r, int index); // return -1 on failure, actual position on success
    int insertRenderBefore(T_RenderListType *r, T_RenderListType *before); // return -1 on failure, actual position on success
    void clearRenders(void);
    void freeBuffers();

    int __SavePreset(char *filename);
    int __LoadPreset(char *filename, int clear);

    int __SavePresetToUndo(C_UndoItem &item);
    int __LoadPresetFromUndo(C_UndoItem &item, int clear);

    int clearfb() { return mode&1; }
    void set_clearfb(int v) { if (v) mode|=1; else mode&=~1; }

    int blendin() { return ((mode>>8)&31); }
    void set_blendin(int v) { mode&=~(31<<8); mode|=(v&31)<<8; }
    int blendout() { return ((mode>>16)&31)^1; }
    void set_blendout(int v) { mode&=~(31<<16); mode|=((v^1)&31)<<16; }
		unsigned char get_extended_datasize(void) { return ((mode & 0xFF000000) >> 24); }
		void set_extended_datasize(unsigned char s) { mode &= 0xFFFFFF; mode |= s << 24; }

    int enabled() { return ((mode&2)^2) || fake_enabled>0; }
    void set_enabled(int v) { if (!v) mode|=2; else mode&=~2; }
    static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
    static BOOL CALLBACK g_DlgProcRoot(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
		int save_config_ex(unsigned char *data, int rootsave);
		void load_config_code(unsigned char *data, int len);
		int  save_config_code(unsigned char *data);    
		void FillBufferCombo(HWND dlg, int ctl);
};

#endif // _R_LIST_H_
