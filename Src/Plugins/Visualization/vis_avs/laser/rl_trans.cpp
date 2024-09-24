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
#ifdef LASER
#include <windows.h>
#include <commctrl.h>
#include "../r_defs.h"
#include "../resource.h"
#include "../evallib/eval.h"
#include "../evallib/compiler.h"
#include <math.h>

#define C_THISCLASS CLASER_Transform
#define MOD_NAME "Misc / Transform"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); // returns 1 if fbout has dest
		virtual char *get_desc() { return MOD_NAME; }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

		void load_string(RString &s,unsigned char *data, int &pos, int len);
		void save_string(unsigned char *data, int &pos, RString &text);

    void dopoint(float &x, float &y, char visdata[2][2][576], int &color);

    varType vars[EVAL_MAX_VARS];
    NSEEL_CODEHANDLE codehandle,codehandle_b,codehandle_i,codehandle_f;
    double *v_d;
    double *v_r;
    double *v_x;
    double *v_y;
    double *v_b;
    double *v_red;
    double *v_green;
    double *v_blue;

    int rectangular;
    RString effect_exp[4];
    int effect_exp_ch;
    CRITICAL_SECTION rcs;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	rectangular=GET_INT(); pos+=4;
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
  effect_exp_ch=1;
}

void C_THISCLASS::load_string(RString &s,unsigned char *data, int &pos, int len) // read configuration of max length "len" from data.
{
  int size=GET_INT(); pos += 4;
  if (size > 0 && len-pos >= size)
	{
    s.resize(size);
	  memcpy(s.get(), data+pos, size);
	  pos+=size;
	}
  else 
  {
    s.resize(1);
    s.get()[0]=0;
  }
}

int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
  PUT_INT(rectangular); pos+=4;
  data[pos++]=1;
  save_string(data,pos,effect_exp[0]);
  save_string(data,pos,effect_exp[1]);
  save_string(data,pos,effect_exp[2]);
  save_string(data,pos,effect_exp[3]);
	return pos;
}

// special version that encodes with a 1 at the start.
void C_THISCLASS::save_string(unsigned char *data, int &pos, RString &text)
{
  if (text.get() && text.get()[0])
	{
    int l=(strlen(text.get())+1);
	  PUT_INT(l); pos+=4;
	  memcpy(data+pos, text.get(), strlen(text.get())+1);
	  pos+=strlen(text.get())+1;
	}
  else
  { 
    PUT_INT(0); 
    pos+=4;
  }
}

C_THISCLASS::C_THISCLASS()
{
  rectangular=0;
  codehandle=0;
  codehandle_b=0;
  codehandle_i=0;
  codehandle_f=0;
  memset(vars,0,sizeof(vars));
  resetVars(vars);
  v_d = registerVar("d");
  v_r = registerVar("r");
  v_x = registerVar("x");
  v_y = registerVar("y");
  v_b = registerVar("b");
  v_red = registerVar("red");
  v_green = registerVar("green");
  v_blue = registerVar("blue");
  resetVars(NULL);
  effect_exp[0].assign("d=0.5;");
  effect_exp[1].assign("");
  effect_exp[2].assign("");
  effect_exp[3].assign("");
  effect_exp_ch=1;
  InitializeCriticalSection(&rcs);
}

C_THISCLASS::~C_THISCLASS()
{
  freeCode(codehandle);
  freeCode(codehandle_b);
  freeCode(codehandle_i);
  freeCode(codehandle_f);
  codehandle=0;
  codehandle_b=0;
  codehandle_i=0;
  codehandle_f=0;
  DeleteCriticalSection(&rcs);
}

void C_THISCLASS::dopoint(float &x, float &y, char visdata[2][2][576], int &color)
{
  *v_d=sqrt(x*x+y*y);
  *v_r=atan2(y,x) + 3.14159*0.5;
  *v_x=x;
  *v_y=y;
  *v_blue=(color&0xff)/255.0;
  *v_green=((color>>8)&0xff)/255.0;
  *v_red=((color>>16)&0xff)/255.0;

  executeCode(codehandle,visdata);
  if (rectangular)
  {
    x=*v_x;
    y=*v_y;
  }
  else
  {
    *v_r -= 3.14159*0.5;
    if (*v_r < 0.0) *v_r += 3.14159*2.0;
    if (*v_r >= 3.14159*2.0) *v_r -= 3.14159*2.0;
    x=cos(*v_r)* *v_d;
    y=sin(*v_r)* *v_d;
  }

  int a=(int)(*v_blue * 255.0);
  if (a>255) a=255;
  if (a<0)a=0;
  color=a;
  a=(int)(*v_green * 255.0);
  if (a>255) a=255;
  if (a<0)a=0;
  color|=a<<8;
  a=(int)(*v_red * 255.0);
  if (a>255) a=255;
  if (a<0)a=0;
  color|=a<<16;
  
}


int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h) // returns 1 if fbout has dest
{
  if (isBeat&0x80000000) return 0;

  if (effect_exp_ch || !codehandle)
  {
    EnterCriticalSection(&rcs);
    memset(vars,0,sizeof(vars));
    resetVars(vars);
    if (codehandle) freeCode(codehandle);
    if (codehandle_b) freeCode(codehandle_b);
    if (codehandle_i) freeCode(codehandle_i);
    if (codehandle_f) freeCode(codehandle_f);
    codehandle=compileCode(effect_exp[0].get());
    codehandle_i=compileCode(effect_exp[1].get());
    codehandle_f=compileCode(effect_exp[2].get());
    codehandle_b=compileCode(effect_exp[3].get());
    effect_exp_ch=0;
    resetVars(NULL);
    LeaveCriticalSection(&rcs);
    executeCode(codehandle_i,visdata);
  }
  executeCode(codehandle_f,visdata);
  if (isBeat) 
    executeCode(codehandle_b,visdata);
  if (codehandle)
  {
    *v_b=isBeat?1.0:0.0;
    int x,num=g_laser_linelist->GetUsedLines();
    LineType *l=g_laser_linelist->GetLineList();
    for (x = 0; x < num; x ++)
    {
      dopoint(l->x1,l->y1,visdata,l->color);
      if (l->mode == 0) dopoint(l->x2,l->y2,visdata,l->color);
      l++;
    }
  }
  return 0;
}

C_RBASE *RLASER_Transform(char *desc)
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}


static C_THISCLASS *g_this;

static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  static int isstart;
	int *a=NULL;
	switch (uMsg)
	{
		case WM_INITDIALOG:      
      isstart=1;
      SetDlgItemText(hwndDlg,IDC_EDIT1,g_this->effect_exp[0].get());
      SetDlgItemText(hwndDlg,IDC_EDIT2,g_this->effect_exp[1].get());
      SetDlgItemText(hwndDlg,IDC_EDIT3,g_this->effect_exp[2].get());
      SetDlgItemText(hwndDlg,IDC_EDIT4,g_this->effect_exp[3].get());
      isstart=0;
      if (g_this->rectangular)
        CheckDlgButton(hwndDlg,IDC_CHECK3,BST_CHECKED);
    return 1;
    case WM_COMMAND:
      if ((
        LOWORD(wParam) == IDC_EDIT1 ||
        LOWORD(wParam) == IDC_EDIT2 ||
        LOWORD(wParam) == IDC_EDIT3 ||
        LOWORD(wParam) == IDC_EDIT4

        )
        && HIWORD(wParam) == EN_CHANGE)
      {
        if (!isstart)
        {
          EnterCriticalSection(&g_this->rcs);
          g_this->effect_exp[0].get_from_dlgitem(hwndDlg,IDC_EDIT1);
          g_this->effect_exp[1].get_from_dlgitem(hwndDlg,IDC_EDIT2);
          g_this->effect_exp[2].get_from_dlgitem(hwndDlg,IDC_EDIT3);
          g_this->effect_exp[3].get_from_dlgitem(hwndDlg,IDC_EDIT4);
          g_this->effect_exp_ch=1;
          LeaveCriticalSection(&g_this->rcs);
        }
      }
      if (LOWORD(wParam) == IDC_CHECK3)
      {
          EnterCriticalSection(&g_this->rcs);
          g_this->rectangular=IsDlgButtonChecked(hwndDlg,IDC_CHECK3)?1:0;
          g_this->effect_exp_ch=1;
          LeaveCriticalSection(&g_this->rcs);
      }
    return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_LASER_TRANSFORM,hwndParent,g_DlgProc);
}
#endif