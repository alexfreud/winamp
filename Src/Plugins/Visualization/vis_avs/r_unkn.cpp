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

#define MOD_NAME "Unknown Render Object"

#include "r_unkn.h"

char *C_UnknClass::get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_UNKNOWN_RENDER_OBJECT,desc,128):desc); }
void C_UnknClass::SetID(int d, char *dString) { id=d; memset(idString,0,sizeof(idString)); strcpy(idString,dString); }


#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_UnknClass::load_config(unsigned char *data, int len)
{
  if (configdata) GlobalFree(configdata);
  configdata=(char *)GlobalAlloc(GMEM_FIXED,len);
  memcpy(configdata,data,len);
  configdata_len=len;
//  char s[1024]="";
//  for (int x = 0 ;x  < configdata_len; x ++)
//    wsprintf(s+strlen(s),"%X,",configdata[x]);
//  MessageBox(NULL,s,"loaded config",0);

}
int  C_UnknClass::save_config(unsigned char *data)
{
	int pos=0;
//  char s[1024]="";
//  for (int x = 0 ;x  < configdata_len; x ++)
//    wsprintf(s+strlen(s),"%X,",configdata[x]);
//  MessageBox(NULL,s,"saving config",0);
  memcpy(data+pos,configdata,configdata_len);
  pos+=configdata_len;
	return pos;
}


C_UnknClass::C_UnknClass()
{
  configdata=0;
  configdata_len=0;
  id=0;
  idString[0]=0;
}

C_UnknClass::~C_UnknClass()
{
  if (configdata) GlobalFree(configdata);
  configdata=0;
}
	
int C_UnknClass::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  return 0;
}

static C_UnknClass *g_this;

BOOL CALLBACK C_UnknClass::g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
      {
        char s[512]="";
        if (g_this->idString[0]) wsprintf(s,"APE: %s\r\n",g_this->idString);
        else wsprintf(s,WASABI_API_LNGSTRING(IDS_BUILTIN_ID),g_this->id);
        wsprintf(s+strlen(s),WASABI_API_LNGSTRING(IDS_CONFIG_SIZE),g_this->configdata_len);
        SetDlgItemText(hwndDlg,IDC_EDIT1,s);
      }
 			return 1;
	}
	return 0;
}


HWND C_UnknClass::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_UNKN,hwndParent,g_DlgProc);
}