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
#include "render.h"
#include "timing.h"
#include "undo.h"

#ifdef LASER
C_LineListBase *g_laser_linelist;
#endif

C_RenderListClass *g_render_effects;
C_RenderListClass *g_render_effects2;
C_RenderTransitionClass *g_render_transition;
C_RLibrary *g_render_library;

int is_mmx(void) {	
	DWORD retval1,retval2;	
	__try { 		
		_asm {
			mov eax, 1		// set up CPUID to return processor version and features
										//	0 = vendor string, 1 = version info, 2 = cache info
			_emit 0x0f	// code bytes = 0fh,  0a2h
			_emit 0xa2
			mov retval1, eax		
			mov retval2, edx
		}	
	} __except(EXCEPTION_EXECUTE_HANDLER) { retval1 = retval2= 0;}
	if (!retval1) return 0;
	return (retval2&0x800000)?1:0;
}


unsigned char g_blendtable[256][256];
unsigned int const mmx_blend4_revn[2]={0xff00ff,0xff00ff};//{0x1000100,0x1000100}; <<- this is actually more correct, but we're going for consistency vs. the non-mmx ver-jf
int const mmx_blendadj_mask[2] = { 0xff00ff,0xff00ff};
int const mmx_blend4_zero=0;

void Render_Init(HINSTANCE hDllInstance)
{
#ifdef LASER
  laser_connect();
  g_laser_linelist=createLineList();
#endif
	timingInit();
  {
    int i,j;
    for (j=0;j<256;j++)
  	  for (i=0;i<256;i++)
	  	  g_blendtable[i][j] = (unsigned char)((i / 255.0) * (float)j);
  }

  g_render_library=new C_RLibrary();
  g_render_effects=new C_RenderListClass(1);
  g_render_effects2=new C_RenderListClass(1);
  g_render_transition=new C_RenderTransitionClass();

	char INI_FILE[MAX_PATH];
	char *p=INI_FILE;
	GetModuleFileName(hDllInstance,INI_FILE,sizeof(INI_FILE));
	if (p[0]) while (p[1]) p++;
	while (p >= INI_FILE && *p != '\\') p--;
#ifdef LASER
	strcpy(p,"\\vis_avs_laser.dat");
#else
	strcpy(p,"\\vis_avs.dat");
#endif
  extern int g_saved_preset_dirty;
  // clear the undo stack before loading a file.
  C_UndoStack::clear();
  g_render_effects->__LoadPreset(INI_FILE,1);
  // then add the new load to the undo stack but mark it clean if it is supposed to be
  C_UndoStack::saveundo();
  if (!g_saved_preset_dirty)
  {
    C_UndoStack::cleardirty();
  }
}

void Render_Quit(HINSTANCE hDllInstance)
{
  if (g_render_transition) delete g_render_transition;
  g_render_transition=NULL;
	if (g_render_effects)
  {
		char INI_FILE[MAX_PATH];
		char *p=INI_FILE;
		GetModuleFileName(hDllInstance,INI_FILE,sizeof(INI_FILE));
		if (p[0]) while (p[1]) p++;
		while (p >= INI_FILE && *p != '\\') p--;
  #ifdef LASER
	  strcpy(p,"\\vis_avs_laser.dat");
  #else
	  strcpy(p,"\\vis_avs.dat");
  #endif

    g_render_effects->__SavePreset(INI_FILE);
	}

  if (g_render_effects) delete g_render_effects;
  g_render_effects=NULL;
  if (g_render_effects2) delete g_render_effects2;
  g_render_effects2=NULL;

  if (g_render_library) delete g_render_library;
  g_render_library=NULL;

	timingPrint();
#ifdef LASER
  if (g_laser_linelist) delete g_laser_linelist;
  g_laser_linelist=0;
  laser_disconnect();
#endif
}
