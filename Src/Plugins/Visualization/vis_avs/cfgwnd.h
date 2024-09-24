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
void CfgWnd_Create(struct winampVisModule *this_mod);
void CfgWnd_Destroy(void);

void CfgWnd_Populate(int force=0);
void CfgWnd_Unpopulate(int force=0);
void CfgWnd_RePopIfNeeded(void);

extern int cfg_fs_w,cfg_fs_h,cfg_fs_d,cfg_fs_bpp,cfg_fs_fps,cfg_fs_rnd,cfg_fs_flip,cfg_fs_height,cfg_speed,cfg_fs_rnd_time;
extern int cfg_cfgwnd_x,cfg_cfgwnd_y,cfg_cfgwnd_open;
extern int cfg_trans,cfg_trans_amount;
extern int cfg_dont_min_avs;
extern int cfg_smartbeat, cfg_smartbeatsticky, cfg_smartbeatresetnewsong, cfg_smartbeatonlysticky;
extern int cfg_transitions, cfg_transitions2, cfg_transitions_speed, cfg_transition_mode;
extern int cfg_bkgnd_render,cfg_bkgnd_render_color;
extern int cfg_render_prio;

extern char config_pres_subdir[MAX_PATH];
extern HWND g_hwndDlg;
