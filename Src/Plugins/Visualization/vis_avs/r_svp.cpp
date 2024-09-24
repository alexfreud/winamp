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
#include "r_defs.h"
#include "resource.h"
#include "../Agave/Language/api_language.h"

extern HINSTANCE g_hInstance;

#include "svp_vis.h"

#ifndef LASER

#define C_THISCLASS C_SVPClass
#define MOD_NAME "Render / SVP Loader"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_RENDER_SVP_LOADER,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);


    char m_library[MAX_PATH];
    void SetLibrary();
    HMODULE hLibrary;
    CRITICAL_SECTION cs;
    VisInfo *vi;
    VisData vd;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
  if (pos+MAX_PATH <= len)
  {
    memcpy(m_library,data+pos,MAX_PATH);
    pos+=MAX_PATH;
    SetLibrary();
  }
}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
  memcpy(data+pos,m_library,MAX_PATH);
  pos+=MAX_PATH;
	return pos;
}



void C_THISCLASS::SetLibrary()
{
  EnterCriticalSection(&cs);
  if (hLibrary)
  {
    if (vi) vi->SaveSettings("avs.ini");
    vi=NULL;
    FreeLibrary(hLibrary);
    hLibrary=NULL;
  }

  if (!m_library[0]) { LeaveCriticalSection(&cs); return; }

  char buf1[MAX_PATH];
  strcpy(buf1,g_path);
  strcat(buf1,"\\");
  strcat(buf1,m_library);

  vi=NULL;
  hLibrary = LoadLibrary(buf1);
  if (hLibrary)
  {
    VisInfo* (*qm)(void);
    qm=(struct _VisInfo *(__cdecl *)(void))GetProcAddress(hLibrary,"QueryModule");
    if (!qm)
      qm=(struct _VisInfo *(__cdecl *)(void))GetProcAddress(hLibrary,"?QueryModule@@YAPAUUltraVisInfo@@XZ");

    if (!qm || !(vi=qm()))
    {
      vi=NULL;
      FreeLibrary(hLibrary);
      hLibrary=NULL;
    }
  }
  if (vi) { 
    vi->OpenSettings("avs.ini"); vi->Initialize(); 
  }
  LeaveCriticalSection(&cs); 
}


C_THISCLASS::C_THISCLASS()
{
  InitializeCriticalSection(&cs);
  m_library[0]=0;
  hLibrary=0;
  vi=NULL;
}

C_THISCLASS::~C_THISCLASS()
{
  if (vi) vi->SaveSettings("avs.ini");
  if (hLibrary) FreeLibrary(hLibrary);
  DeleteCriticalSection(&cs);
}
	
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (isBeat&0x80000000) return 0;
  EnterCriticalSection(&cs);
  if (vi)
  {
 //   if (vi->lRequired & VI_WAVEFORM)
    {
      int ch,p;
      for (ch = 0; ch < 2; ch ++)
      {
        unsigned char *v=(unsigned char *)visdata[1][ch];
        for (p = 0; p < 512; p ++)
          vd.Waveform[ch][p]=v[p];
      }
    }
   // if (vi->lRequired & VI_SPECTRUM)
    {
      int ch,p;
      for (ch = 0; ch < 2; ch ++)
      {
        unsigned char *v=(unsigned char *) visdata[0][ch];
        for (p = 0; p < 256; p ++)
          vd.Spectrum[ch][p]=v[p*2]/2+v[p*2+1]/2;
      }
    }
   
    vd.MillSec=GetTickCount();
    vi->Render((unsigned long *)framebuffer,w,h,w,&vd);
  }
  LeaveCriticalSection(&cs);
  return 0;
}

C_RBASE *R_SVP(char *desc)
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
      loadComboBox(GetDlgItem(hwndDlg,IDC_COMBO1), "*.SVP", g_this->m_library);
      loadComboBox(GetDlgItem(hwndDlg,IDC_COMBO1), "*.UVS", g_this->m_library);
 			return 1;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_COMBO1:
          {
            int a=SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
            if (a != CB_ERR)
            {
              if (SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETLBTEXT,a,(LPARAM)g_this->m_library) == CB_ERR)
              {
                g_this->m_library[0]=0;
              }
            }
            else 
              g_this->m_library[0]=0;
            g_this->SetLibrary();
          }
          return 0;
      }
    return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDC_CFG_SVP,hwndParent,g_DlgProc);
}
#else
C_RBASE *R_SVP(char *desc)
{ return NULL; }
#endif