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
#include <process.h>
#include "draw.h"
#include "resource.h"
#include "cfgwnd.h"
#include "r_defs.h"
#include "r_unkn.h"
#include "r_transition.h"
#include "render.h"
#include <math.h>
#include "../Agave/Language/api_language.h"

extern char *scanstr_back(char *str, char *toscan, char *defval);

int transitionmodes[] = 
{
	IDS_RANDOM,
	IDS_CROSS_DISSOLVE,
	IDS_L_R_PUSH,
	IDS_R_L_PUSH,
	IDS_T_B_PUSH,
	IDS_B_T_PUSH,
	IDS_9_RANDOM_BLOCKS,
	IDS_SPLIT_L_R_PUSH,
	IDS_L_R_CENTER_PUSH,
	IDS_L_R_CENTER_SQUEEZE,
	IDS_L_R_WIPE,
	IDS_R_L_WIPE,
	IDS_T_B_WIPE,
	IDS_B_T_WIPE,
	IDS_DOT_DISSOLVE,
};

static C_RenderTransitionClass *g_this;
C_RenderTransitionClass::C_RenderTransitionClass()
{
  last_file[0]=0;
	l_w=l_h=0;
  memset(fbs,0,sizeof(fbs));
	enabled=0;
  start_time=0;
  _dotransitionflag=0;
  initThread=0;
}

C_RenderTransitionClass::~C_RenderTransitionClass()
{
  int x;
  if (initThread)
  {
    WaitForSingleObject(initThread,INFINITE);
    CloseHandle(initThread);
    initThread=0;
  }  for (x = 0; x < 4; x ++)
  {
    if (fbs[x]) GlobalFree(fbs[x]);
    fbs[x]=NULL;
  }
}

unsigned int WINAPI C_RenderTransitionClass::m_initThread(LPVOID p)
{
  C_RenderTransitionClass *_this=(C_RenderTransitionClass*)p;
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  srand(ft.dwLowDateTime|ft.dwHighDateTime^GetCurrentThreadId());
  if (cfg_transitions2&32)
  {
    extern HANDLE g_hThread;
    int d=GetThreadPriority(g_hThread);
    if (d == THREAD_PRIORITY_TIME_CRITICAL) d=THREAD_PRIORITY_HIGHEST;
    else if (d == THREAD_PRIORITY_HIGHEST) d=THREAD_PRIORITY_ABOVE_NORMAL;
    else if (d == THREAD_PRIORITY_ABOVE_NORMAL) d=THREAD_PRIORITY_NORMAL;
    else if (d == THREAD_PRIORITY_NORMAL) d=THREAD_PRIORITY_BELOW_NORMAL;
    else if (d == THREAD_PRIORITY_BELOW_NORMAL) d=THREAD_PRIORITY_LOWEST;
    else if (d == THREAD_PRIORITY_LOWEST) d=THREAD_PRIORITY_IDLE;
    SetThreadPriority(GetCurrentThread(),d);
  }
  int *fb=(int *)GlobalAlloc(GPTR,_this->l_w*_this->l_h*sizeof(int));
  char last_visdata[2][2][576]={0,};
  g_render_effects2->render(last_visdata,0x80000000,fb,fb,_this->l_w,_this->l_h);
  GlobalFree((HGLOBAL)fb);

  _this->_dotransitionflag=2;

  _endthreadex(0);
  return 0;
}


int C_RenderTransitionClass::LoadPreset(char *file, int which, C_UndoItem *item)
{
  if (initThread)
  {
    if (WaitForSingleObject(initThread,0)==WAIT_TIMEOUT) 
    {
      DDraw_SetStatusText("loading [wait]...",1000*100);
      return 2;
    }
    CloseHandle(initThread);
    initThread=0;
  }


  EnterCriticalSection(&g_render_cs);
  if (enabled)
  {
    enabled=0;
  }

	int r = 0;
  
  if (item) 
  {
    g_render_effects2->__LoadPresetFromUndo(*item,1);
    last_which=which;
    _dotransitionflag=2;
  }
  else
  {
    lstrcpyn(last_file,file,sizeof(last_file));
    if (file[0]) r=g_render_effects2->__LoadPreset(file,1);
    else 
    {
      g_render_effects2->clearRenders();
    }
    if (!r && l_w && l_h && (cfg_transitions2&which) && ((cfg_transitions2&128)||DDraw_IsFullScreen()))
    {
      DWORD id;
      last_which=which;
      _dotransitionflag=1;
      initThread=(HANDLE)_beginthreadex(NULL,0,m_initThread,(LPVOID)this,0,(unsigned int*)&id);
      DDraw_SetStatusText("loading...",1000*100);
    }
    else
    {
      last_which=which;
      _dotransitionflag=2;
    }

    if (r)
    {
      char s[MAX_PATH*2];
      wsprintf(s,WASABI_API_LNGSTRING(IDS_ERROR_LOADING_X),scanstr_back(last_file,"\\",last_file-1)+1);
      DDraw_SetStatusText(s);
      _dotransitionflag=3;
    }
    C_UndoStack::clear();
    C_UndoStack::saveundo(1);
    C_UndoStack::cleardirty();
  }
  LeaveCriticalSection(&g_render_cs);

  return !!r;
}

#define PI 3.14159265358979323846
//264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651328230664709384460955058223172535940812848...

extern int g_rnd_cnt;

int C_RenderTransitionClass::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (_dotransitionflag||enabled) g_rnd_cnt=0;
  if (_dotransitionflag==2 || _dotransitionflag == 3)
  {
    int notext=_dotransitionflag==3;
    _dotransitionflag=0;
    if (cfg_transitions&last_which)
    {
      curtrans = (cfg_transition_mode&0x7fff) ? (cfg_transition_mode&0x7fff) : (rand() % ((sizeof(transitionmodes)/sizeof(transitionmodes[0]))-1))+1;
      if (cfg_transition_mode&0x8000) curtrans|=0x8000;
      ep[0]=0;
      ep[1]=2;
      mask=0;
      start_time=0;
      enabled=1;
    }
    C_RenderListClass *temp=g_render_effects;
    g_render_effects=g_render_effects2;
    g_render_effects2=temp;
    extern int need_repop;
    extern char *extension(char *fn);
    need_repop=1;
    PostMessage(g_hwndDlg,WM_USER+20,0,0);
    if (!notext && stricmp("aph",extension(last_file)))
    {
      char buf[512];
      strncpy(buf,scanstr_back(last_file,"\\",last_file-1)+1,510);
      buf[510]=0;
      scanstr_back(buf,".",buf+strlen(buf))[0]=0;
      strcat(buf," ");
      DDraw_SetStatusText(buf);
    }
  }

	if (!enabled) 
  {
    int x;
    l_w=w;
    l_h=h;
    if (fbs[0]) for (x = 0; x < 4; x ++)
    {
      if (fbs[x]) 
        {
        GlobalFree(fbs[x]);
        fbs[x]=NULL;
      }
    }
    if (!initThread && g_render_effects2->getNumRenders()) 
    {
      g_render_effects2->clearRenders();
      g_render_effects2->freeBuffers();
    }
    return g_render_effects->render(visdata,isBeat,framebuffer,fbout,w,h);
  }

	// handle resize
  if (l_w != w || l_h != h || !fbs[0])
  {
    l_w=w;
    l_h=h;
    int x;
    for (x = 0; x < 4; x ++)
    {
      if (fbs[x]) GlobalFree(fbs[x]);
      fbs[x]=(int*)GlobalAlloc(GPTR,l_w*l_h*sizeof(int));
    }
  }

	if (start_time == 0)
	{
		memcpy(fbs[ep[0]],framebuffer,sizeof(int)*l_w*l_h);
        memcpy(fbs[ep[1]],framebuffer,sizeof(int)*l_w*l_h);
	}


	// maybe there's a faster way than using 3 more buffers without screwing
	// any effect... justin ?
  if (curtrans&0x8000)
	  ep[1]^=g_render_effects2->render(visdata,isBeat,fbs[ep[1]],fbs[ep[1]^1],w,h)&1;
  ep[0]^=g_render_effects->render(visdata,isBeat,fbs[ep[0]],fbs[ep[0]^1],w,h)&1;

	int *p = fbs[ep[1]];
  int *d = fbs[ep[0]];
	int *o = framebuffer;
  int x=w*h;

  int ttime=250*cfg_transitions_speed;
  if (ttime<100) ttime=100;

  int n;
  if (!start_time) { n=0; start_time=GetTickCount(); }
  else n=MulDiv(GetTickCount()-start_time,256,ttime);

  if (n >= 255) n=255;

	float sintrans = (float)(sin(((float)n/255)*PI-PI/2)/2+0.5); // used for smoothing transitions
																															 // now sintrans does a smooth curve
																															 // from 0 to 1
  switch (curtrans&0x7fff)
  {
    case 1: // Crossfade
      mmx_adjblend_block(o,d,p,x,n);
    break;
    case 2: // Left to right push
				{
				  int i = (int)(sintrans*w);
				  int j;
				  for (j=0;j<h;j++)
          {
					  memcpy(framebuffer+(j*w), d+(j*w)+(w-i), i*4);
					  memcpy(framebuffer+(j*w)+i, p+(j*w), (w-i)*4);
          }
				}
	    break;
    case 3: // Right to left push
				{
				  int i = (int)(sintrans*w);
				  int j;
				  for (j=0;j<h;j++)
          {
						memcpy(framebuffer+(j*w), p+(i+j*w), (w-i)*4);
						memcpy(framebuffer+(j*w)+(w-i), d+(j*w), i*4);          
          }
				}
	    break;
		case 4: // Top to bottom push
				{
				int i = (int)(sintrans*h);
				memcpy(framebuffer, d+(h-i)*w, w*i*4);
				memcpy(framebuffer+w*i, p, w*(h-i)*4);
				}
	    break;
		case 5: // Bottom to Top push
				{
				int i = (int)(sintrans*h);
				memcpy(framebuffer,p+i*w, w*(h-i)*4);
				memcpy(framebuffer+w*(h-i), d, w*i*4);
				}
	    break;
    case 6: // 9 random blocks
				{
          if (!(mask&(1<<(10+n/28))))
					{
						int r=0;
						if ((mask & 0x1ff) != 0x1ff) 
            {
              do 
              {
							  r = rand()%9; 
              }
							while ((1 << r) & mask);
            }
						mask |= (1<<r)|(1<<(10+n/28));
					}
				  int j;
				  int tw=w/3, th=h/3;
				  int twr=w-2*tw;
				  memcpy(framebuffer, p, w*h*4);
				  int i;
				  for (i=0;i<9;i++)
					{
					  if (mask & (1<<i))
            {
              int end=i/3*th+th;
              if (i > 5) end=h;
						  for (j=i/3*th;j<end;j++)
							  memcpy(framebuffer+(j*w)+(i%3)*tw, d+(j*w)+(i%3)*tw, (i%3==2) ? twr*4 : tw*4);
            }
          }
				}
	    break;
		case 7: // Left/Right to Right/Left
				{
				int i = (int)(sintrans*w);
				int j;
				for (j=0;j<h/2;j++)
        {
						memcpy(framebuffer+(i+j*w), p+(j*w), (w-i)*4);
						memcpy(framebuffer+(j*w), d+((j+1)*w)-i, i*4);
        }
				for (j=h/2;j<h;j++)
        {
						memcpy(framebuffer+(j*w), p+(i+j*w), (w-i)*4);
						memcpy(framebuffer+(j*w)+(w-i), d+(j*w), i*4);
        }
				}
	    break;
		case 8: // Left/Right to Center
				{
				int i = (int)(sintrans*w/2);
				int j;
				for (j=0;j<h;j++)
        {
						memcpy(framebuffer+(j*w), d+((j+1)*w-i-w/2), i*4);
						memcpy(framebuffer+((j+1)*w-i), d+(j*w+w/2), i*4);
						memcpy(framebuffer+(j*w)+i, p+(j*w)+i, (w-i*2)*4);
        }
				}
	    break;
		case 9: // Left/Right to Center, squeeze
				{
	  			int i = (int)(sintrans*w/2);
		  		int j;
			  	for (j=0;j<h;j++)
          {
            if (i) 
            {
              int xl=i;
              int xp=0;
              int dxp=((w/2)<<16)/xl;
              int *ot=framebuffer+(j*w);
              int *it=d+(j*w);
              while (xl--)
              {
                *ot++=it[xp>>16];
                xp+=dxp;
              }
            }

            if (i*2 != w) 
            {
              int xl=w-i*2;
              int xp=0;
              int dxp=(w<<16)/xl;
              int *ot=framebuffer+(j*w)+i;
              int *it=p+(j*w);
              while (xl--)
              {
                *ot++=it[xp>>16];
                xp+=dxp;
              }
            }
            if (i) 
            {
              int xl=i;
              int xp=0;
              int dxp=((w/2)<<16)/xl;
              int *ot=framebuffer+(j*w)+w-i;
              int *it=d+(j*w)+w/2;
              while (xl--)
              {
                *ot++=it[xp>>16];
                xp+=dxp;
              }
            }


          }
				}
	    break;
    case 10: // Left to right wipe
				{
				int i = (int)(sintrans*w);
				int j;
				for (j=0;j<h;j++)
        {
						memcpy(framebuffer+(i+j*w), p+(j*w)+i, (w-i)*4);
						memcpy(framebuffer+(j*w), d+(j*w), i*4);
        }
				}
	    break;
    case 11: // Right to left wipe
				{
				int i = (int)(sintrans*w);
				int j;
				for (j=0;j<h;j++)
        {
						memcpy(framebuffer+(j*w), p+(j*w), (w-i)*4);
						memcpy(framebuffer+(j*w)+(w-i), d+(j*w)+(w-i), i*4);
        }
				}
	    break;
    case 12: // Top to bottom wipe
				{
				int i = (int)(sintrans*h);
				memcpy(framebuffer, d, w*i*4);
				memcpy(framebuffer+w*i, p+w*i, w*(h-i)*4);
				}
	    break;
    case 13: // Bottom to top wipe
				{
				int i = (int)(sintrans*h);
				memcpy(framebuffer, p, w*(h-i)*4);
				memcpy(framebuffer+w*(h-i), d+w*(h-i), w*i*4);
				}
	    break;
    case 14: // dot dissolve
      {
        int i=((int)(sintrans*5))-5;
        int j;
        int t=0;
        int dir=1;
        
        if (i < 0)
        {
          dir=!dir;
          i++;
          i=-i;
        }
        i=1<<i;
        for (j = 0; j < h; j ++)
        {
          if (t++==i)
          {
            int x=w;
            t=0;
            int t2=0;
            int *of=framebuffer+j*w;
            int *p2=(dir?p:d)+j*w;
            int *d2=(dir?d:p)+j*w;
            while (x--)
            {
              if (t2++==i)
              {
                of[0]=p2[0];
                t2=0;
              }
              else of[0]=d2[0];
              p2++;
              d2++;
              of++;            
            }
          }
          else
            memcpy(framebuffer+j*w,(dir?d:p)+j*w,w*sizeof(int));
        }
      }
    break;
    default:
		  break;
  }

	if (n == 255)
  {
    int x;
    enabled=0;
    start_time=0;
    for (x = 0; x < 4; x ++)
    {
      if (fbs[x]) GlobalFree(fbs[x]);
      fbs[x]=NULL;
    }
    g_render_effects2->clearRenders();
    g_render_effects2->freeBuffers();
  }
  return 0;
}

BOOL CALLBACK C_RenderTransitionClass::g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	  case WM_INITDIALOG:
	      {
	      int x;
	      for (x = 0; x < sizeof(transitionmodes)/sizeof(transitionmodes[0]); x ++)
	      SendDlgItemMessage(hwndDlg,IDC_TRANSITION,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRING(transitionmodes[x]));
	      SendDlgItemMessage(hwndDlg,IDC_TRANSITION,CB_SETCURSEL,(WPARAM)cfg_transition_mode&0x7fff,0);
	      SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETRANGE, TRUE, MAKELONG(1, 32));
	      SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETPOS, TRUE, cfg_transitions_speed);
	      if (cfg_transition_mode&0x8000) CheckDlgButton(hwndDlg,IDC_CHECK9,BST_CHECKED);
	      if (cfg_transitions&1) CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);
	      if (cfg_transitions&2) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
	      if (cfg_transitions&4) CheckDlgButton(hwndDlg,IDC_CHECK8,BST_CHECKED);
	      if (cfg_transitions2&1) CheckDlgButton(hwndDlg,IDC_CHECK10,BST_CHECKED);
	      if (cfg_transitions2&2) CheckDlgButton(hwndDlg,IDC_CHECK11,BST_CHECKED);
	      if (cfg_transitions2&4) CheckDlgButton(hwndDlg,IDC_CHECK3,BST_CHECKED);
	      if (cfg_transitions2&32) CheckDlgButton(hwndDlg,IDC_CHECK4,BST_CHECKED);
	      if (!(cfg_transitions2&128)) CheckDlgButton(hwndDlg,IDC_CHECK5,BST_CHECKED);

	      }
 	      return 1;
	case WM_COMMAND:
	switch (LOWORD(wParam))
        {
        case IDC_TRANSITION:
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            int r=SendDlgItemMessage(hwndDlg,IDC_TRANSITION,CB_GETCURSEL,0,0);
            if (r!=CB_ERR) 
            {
              cfg_transition_mode&=~0x7fff;
							cfg_transition_mode |= r;
            }
          }
	        break;
        case IDC_CHECK9:
          cfg_transition_mode&=0x7fff;
          cfg_transition_mode |= IsDlgButtonChecked(hwndDlg,IDC_CHECK9)?0x8000:0;
		      break;
        case IDC_CHECK2:
          cfg_transitions &= ~1;
          cfg_transitions |= IsDlgButtonChecked(hwndDlg,IDC_CHECK2)?1:0;
		      break;
        case IDC_CHECK1:
          cfg_transitions &= ~2;
          cfg_transitions |= IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?2:0;
		      break;
        case IDC_CHECK8:
          cfg_transitions &= ~4;
          cfg_transitions |= IsDlgButtonChecked(hwndDlg,IDC_CHECK8)?4:0;
		      break;
        case IDC_CHECK10:
          cfg_transitions2 &= ~1;
          cfg_transitions2 |= IsDlgButtonChecked(hwndDlg,IDC_CHECK10)?1:0;
		      break;
        case IDC_CHECK11:
          cfg_transitions2 &= ~2;
          cfg_transitions2 |= IsDlgButtonChecked(hwndDlg,IDC_CHECK11)?2:0;
		      break;
        case IDC_CHECK3:
          cfg_transitions2 &= ~4;
          cfg_transitions2 |= IsDlgButtonChecked(hwndDlg,IDC_CHECK3)?4:0;
		      break;
        case IDC_CHECK4:
          cfg_transitions2 &= ~32;
          cfg_transitions2 |= IsDlgButtonChecked(hwndDlg,IDC_CHECK4)?32:0;
		      break;
        case IDC_CHECK5:
          cfg_transitions2 &= ~128;
          cfg_transitions2 |= IsDlgButtonChecked(hwndDlg,IDC_CHECK5)?0:128;
		      break;
			}
	    break;
	case WM_NOTIFY:
		if (LOWORD(wParam) == IDC_SPEED)
			cfg_transitions_speed = SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_GETPOS, 0, 0);
		break;
	}
	return 0;
}


HWND C_RenderTransitionClass::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
  return WASABI_API_CREATEDIALOG(IDD_GCFG_TRANSITIONS,hwndParent,g_DlgProc);
}

