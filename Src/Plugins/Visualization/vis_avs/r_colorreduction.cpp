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
#include "resource.h"
#include "r_defs.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER


// this will be the directory and APE name displayed in the AVS Editor
#define MOD_NAME "Trans / Color Reduction"
#define C_THISCLASS C_ColorReduction

typedef struct {
	char fname[MAX_PATH];
	int	levels;
} apeconfig;

class C_THISCLASS : public C_RBASE 
{
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat,	int *framebuffer, int *fbout, int w, int h);		
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual char *get_desc();
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

		apeconfig config;

		HWND hwndDlg;
};

// global configuration dialog pointer 
static C_THISCLASS *g_ConfigThis; 
static HINSTANCE g_hDllInstance; 


// this is where we deal with the configuration screen
static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_HSCROLL:
			{
				if (LOWORD(wParam) == TB_ENDTRACK)
					g_ConfigThis->config.levels = SendMessage(GetDlgItem(hwndDlg, IDC_LEVELS), TBM_GETPOS, 0, 0);
				{
					char buf[4];
					int	a,b;
					a = 8-g_ConfigThis->config.levels;
					b = 0x100;
					while (a--) b>>=1;
					wsprintf(buf, "%d", b);
					SetDlgItemText(hwndDlg, IDC_LEVELTEXT, buf);
				}
			}
			return 1;

		case WM_INITDIALOG:
			g_ConfigThis->hwndDlg = hwndDlg;
 
			SendMessage(GetDlgItem(hwndDlg, IDC_LEVELS), TBM_SETRANGE, TRUE, MAKELONG(1, 8));
			SendMessage(GetDlgItem(hwndDlg, IDC_LEVELS), TBM_SETPOS, TRUE, g_ConfigThis->config.levels); 
 			SetFocus(GetDlgItem(hwndDlg, IDC_LEVELS));
			{
				char buf[4];
				int a,b;
				a = 8-g_ConfigThis->config.levels;
				b = 0x100;
				while (a--) b>>=1;				
				wsprintf(buf, "%d", b);
				SetDlgItemText(hwndDlg, IDC_LEVELTEXT, buf);
			}
			return 1;

		case WM_DESTROY:
			KillTimer(hwndDlg, 1);
			return 1;
	}
	return 0;
}

// set up default configuration 
C_THISCLASS::C_THISCLASS() 
{
	memset(&config, 0, sizeof(apeconfig));
	config.levels = 7;
}

// virtual destructor
C_THISCLASS::~C_THISCLASS() 
{
}


int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (isBeat&0x80000000) return 0;

	int a,b,c;
	a = 8-config.levels;
	b = 0xFF;
	while (a--) b=(b<<1)&0xFF;
	b |= (b<<16) | (b<<8);
	c = w*h;
	__asm {
		mov ebx, framebuffer;
		mov ecx, c;
		mov edx, b;
		lp:
		sub ecx, 4;
		test ecx, ecx;
		jz end;
		and dword ptr [ebx+ecx*4], edx;
		and dword ptr [ebx+ecx*4+4], edx;
		and dword ptr [ebx+ecx*4+8], edx;
		and dword ptr [ebx+ecx*4+12], edx;
		jmp lp;
		end:
	}
	return 0;
}

HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) 
{
	g_ConfigThis = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_COLORREDUCTION, hwndParent, (DLGPROC)g_DlgProc);
}


char *C_THISCLASS::get_desc(void)
{ 
	static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_COLOR_REDUCTION,desc,128):desc); 
}

void C_THISCLASS::load_config(unsigned char *data, int len) 
{
	if (len == sizeof(apeconfig))
		memcpy(&this->config, data, len);
	else
		memset(&this->config, 0, sizeof(apeconfig));
}


int  C_THISCLASS::save_config(unsigned char *data) 
{
	memcpy(data, &this->config, sizeof(apeconfig));
	return sizeof(apeconfig);
}

C_RBASE *R_ColorReduction(char *desc) 
{
	if (desc) { 
		strcpy(desc,MOD_NAME); 
		return NULL; 
	}
	return (C_RBASE *) new C_THISCLASS();
}


#endif