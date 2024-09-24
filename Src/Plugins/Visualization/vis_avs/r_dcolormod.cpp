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

#define C_THISCLASS C_DColorModClass
#define MOD_NAME "Trans / Color Modifier"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_COLOR_MODIFIER,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
    RString effect_exp[4];

    int m_recompute;
    
    int m_tab_valid;
    unsigned char m_tab[768];
    NSEEL_VMCTX AVS_EEL_CONTEXTNAME;
    double *var_r, *var_g, *var_b, *var_beat;
		int inited;
    NSEEL_CODEHANDLE codehandle[4];
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
	if (len-pos >= 4) { m_recompute=GET_INT(); pos+=4; }

}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
  data[pos++]=1;
  save_string(data,pos,effect_exp[0]);
  save_string(data,pos,effect_exp[1]);
  save_string(data,pos,effect_exp[2]);
  save_string(data,pos,effect_exp[3]);
	PUT_INT(m_recompute); pos+=4;
	return pos;
}



C_THISCLASS::C_THISCLASS()
{
  AVS_EEL_INITINST();
  InitializeCriticalSection(&rcs);
  need_recompile=1;
  m_recompute=1;
  memset(codehandle,0,sizeof(codehandle));
  effect_exp[0].assign("");
  effect_exp[1].assign("");
  effect_exp[2].assign("");
  effect_exp[3].assign("");

  var_beat=0;
  m_tab_valid=0;
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

  DeleteCriticalSection(&rcs);
}


int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (need_recompile)
  {
    EnterCriticalSection(&rcs);
    if (!var_beat || g_reset_vars_on_recompile)
    {
      clearVars();
      var_r = registerVar("red");
      var_g = registerVar("green");
      var_b = registerVar("blue");
      var_beat = registerVar("beat");
      inited=0;
    }
    need_recompile=0;
    int x;
    for (x = 0; x < 4; x ++) 
    {
      freeCode(codehandle[x]);
      codehandle[x]=compileCode(effect_exp[x].get());
    }
    LeaveCriticalSection(&rcs);
  }
  if (isBeat&0x80000000) return 0;

  *var_beat=isBeat?1.0:0.0;

  if (codehandle[3] && !inited) { executeCode(codehandle[3],visdata); inited=1; }
  executeCode(codehandle[1],visdata);

  if (isBeat) executeCode(codehandle[2],visdata);

  if (m_recompute || !m_tab_valid)
  {
    int x;
    unsigned char *t=m_tab;
    for (x = 0; x < 256; x ++)
    {
      *var_r=*var_b=*var_g=x/255.0;
      executeCode(codehandle[0],visdata);
      int r=(int) (*var_r*255.0 + 0.5);
      int g=(int) (*var_g*255.0 + 0.5);
      int b=(int) (*var_b*255.0 + 0.5);
      if (r < 0) r=0;
      else if (r > 255)r=255;
      if (g < 0) g=0;
      else if (g > 255)g=255;
      if (b < 0) b=0;
      else if (b > 255)b=255;
      t[512]=r;
      t[256]=g;
      t[0]=b;
      t++;
    }
    m_tab_valid=1;
  }

  unsigned char *fb=(unsigned char *)framebuffer;
  int l=w*h;
  while (l--)
  {
    fb[0]=m_tab[fb[0]];
    fb[1]=m_tab[(int)fb[1]+256];
    fb[2]=m_tab[(int)fb[2]+512];
    fb+=4;
  }


  return 0;
}

C_RBASE *R_DColorMod(char *desc)
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}

typedef struct 
{
  char *name;
  char *init;
  char *point;
  char *frame;
  char *beat;
  int recompute;
} presetType;

static presetType presets[]=
{
  // Name, Init, Level, Frame, Beat, Recalc
  {"4x Red Brightness, 2x Green, 1x Blue","","red=4*red; green=2*green;","","",0},
  {"Solarization","","red=(min(1,red*2)-red)*2;\r\ngreen=red; blue=red;","","",0},
  {"Double Solarization","","red=(min(1,red*2)-red)*2;\r\nred=(min(1,red*2)-red)*2;\r\ngreen=red; blue=red;","","",0},
  {"Inverse Solarization (Soft)","","red=abs(red - .5) * 1.5;\r\ngreen=red; blue=red;","","",0},
  {"Big Brightness on Beat","scale=1.0","red=red*scale;\r\ngreen=red; blue=red;","scale=0.07 + (scale*0.93)","scale=16",1},
  {"Big Brightness on Beat (Interpolative)","c = 200; f = 0;","red = red * t;\r\ngreen=red;blue=red;","f = f + 1;\r\nt = (1.025 - (f / c)) * 5;","c = f;f = 0;",1},
  {"Pulsing Brightness (Beat Interpolative)","c = 200; f = 0;","red = red * st;\r\ngreen=red;blue=red;","f = f + 1;\r\nt = (f * 2 * $PI) / c;\r\nst = sin(t) + 1;","c = f;f = 0;",1},
  {"Rolling Solarization (Beat Interpolative)","c = 200; f = 0;","red=(min(1,red*st)-red)*st;\r\nred=(min(1,red*2)-red)*2;\r\ngreen=red; blue=red;","f = f + 1;\r\nt = (f * 2 * $PI) / c;\r\nst = ( sin(t) * .75 ) + 2;","c = f;f = 0;",1},
  {"Rolling Tone (Beat Interpolative)","c = 200; f = 0;","red = red * st;\r\ngreen = green * ct;\r\nblue = (blue * 4 * ti) - red - green;","f = f + 1;\r\nt = (f * 2 * $PI) / c;\r\nti = (f / c);\r\nst = sin(t) + 1.5;\r\nct = cos(t) + 1.5;","c = f;f = 0;",1},
  {"Random Inverse Tone (Switch on Beat)","","dd = red * 1.5;\r\nred = pow(dd, dr);\r\ngreen = pow(dd, dg);\r\nblue = pow(dd, db);","","token = rand(99) % 3;\r\ndr = if (equal(token, 0), -1, 1);\r\ndg = if (equal(token, 1), -1, 1);\r\ndb = if (equal(token, 2), -1, 1);",1},
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
      if (g_this->m_recompute)
        CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);

      isstart=0;

    return 1;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDC_BUTTON1) 
      {
/*		char text[4096];
		WASABI_API_LNGSTRING_BUF(IDS_COLOR_MODIFIER,text,4096);
		int titlelen = lstrlen(text)+1;
		lstrcpyn(text+titlelen,GetTextResource(IDR_COLOR_MODIFIER),4095-titlelen);
*/
        char *text="Color Modifier\0"
          "The color modifier allows you to modify the intensity of each color\r\n"
          "channel with respect to itself. For example, you could reverse the red\r\n"
          "channel, double the green channel, or half the blue channel.\r\n"
          "\r\n"
          "The code in the 'level' section should adjust the variables\r\n"
          "'red', 'green', and 'blue', whose value represent the channel\r\n"
          "intensity (0..1).\r\n"
          "Code in the 'frame' or 'level' sections can also use the variable\r\n"
          "'beat' to detect if it is currently a beat.\r\n"
          "\r\n"
          "Try loading an example via the 'Load Example' button for examples."

          ;
        compilerfunctionlist(hwndDlg,text);
      }

      if (LOWORD(wParam)==IDC_CHECK1)
      {
        g_this->m_recompute=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
      }
 
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
        GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON4),&r);
        x=TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,r.right,r.top,0,hwndDlg,NULL);
        if (x >= 16 && x < 16+sizeof(presets)/sizeof(presets[0]))
        {
          isstart=1;
          SetDlgItemText(hwndDlg,IDC_EDIT1,presets[x-16].point);
          SetDlgItemText(hwndDlg,IDC_EDIT2,presets[x-16].frame);
          SetDlgItemText(hwndDlg,IDC_EDIT3,presets[x-16].beat);
          SetDlgItemText(hwndDlg,IDC_EDIT4,presets[x-16].init);
          g_this->m_recompute=presets[x-16].recompute;
          CheckDlgButton(hwndDlg,IDC_CHECK1,g_this->m_recompute?BST_CHECKED:0);
          isstart=0;

          SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_EDIT4,EN_CHANGE),0);
        }
        DestroyMenu(hMenu);
      }
      if (!isstart && HIWORD(wParam) == EN_CHANGE)
      {      
        if (LOWORD(wParam) == IDC_EDIT1||LOWORD(wParam) == IDC_EDIT2||LOWORD(wParam) == IDC_EDIT3||LOWORD(wParam) == IDC_EDIT4)
        {
          EnterCriticalSection(&g_this->rcs);
          g_this->effect_exp[0].get_from_dlgitem(hwndDlg,IDC_EDIT1);
          g_this->effect_exp[1].get_from_dlgitem(hwndDlg,IDC_EDIT2);
          g_this->effect_exp[2].get_from_dlgitem(hwndDlg,IDC_EDIT3);
          g_this->effect_exp[3].get_from_dlgitem(hwndDlg,IDC_EDIT4);
          g_this->need_recompile=1;
				  if (LOWORD(wParam) == IDC_EDIT4) g_this->inited = 0;
          g_this->m_tab_valid=0;
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
	return WASABI_API_CREATEDIALOG(IDD_CFG_COLORMOD,hwndParent,g_DlgProc);
}

#else
C_RBASE *R_DColorMod(char *desc) { return NULL; }
#endif