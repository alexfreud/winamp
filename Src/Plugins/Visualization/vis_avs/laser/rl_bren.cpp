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

#include <math.h>

#define C_THISCLASS CLASER_BrenClass
#define MOD_NAME "Render / Brennan\'s Effect"

typedef struct {
  float x, y;
} PT;

typedef struct {
  PT pt[4];
} segment;

#define BOUND (0.6f)
#define NSEG 48
#define VISSEG 6

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
  
    segment seg[NSEG];
    PT d[4];
    float phase;
    int step;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0,x=0;
	return pos;
}

float frandom() {
  return (float)rand() / (float)RAND_MAX;
}

C_THISCLASS::C_THISCLASS()
{
  int i, j;

  for (j = 0; j < NSEG; j++) {
    for (i = 0; i < 4; i++) {
//      seg[j].pt[i].x = frandom() * 2.0 - 1.0;
 //     seg[j].pt[i].y = frandom() * 2.0 - 1.0;
      seg[j].pt[i].x = 0.f;
      seg[j].pt[i].y = 0.f;
    }
  }
  for (i = 0; i < 4; i++) {
    d[i].x = frandom() * 0.015 + 0.005;
    d[i].y = frandom() * 0.015 + 0.005;
  }


  phase = 0.0;
  step = 0;
}

C_THISCLASS::~C_THISCLASS()
{
}



#define PI 3.14159

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h) // returns 1 if fbout has dest
{
  if (isBeat&0x80000000) return 0;
  LineType l;
  int i, j;
  float s;

  s = sin(phase * 2 * PI)*0.10+0.9;
  //s = 1.0;
  //s = 0.8 + isBeat * 0.2;

  for (i = 0; i < 4; i++) {
    seg[0].pt[i].x += d[i].x * s;
    seg[0].pt[i].y += d[i].y * s;
    if (seg[0].pt[i].x < -BOUND || seg[0].pt[i].x > BOUND) d[i].x = -d[i].x;
    if (seg[0].pt[i].y < -BOUND || seg[0].pt[i].y > BOUND) d[i].y = -d[i].y;
  }

  for (j = 0; j < NSEG; j++) {
    static PT p;
    if ((step++ % (NSEG/VISSEG)) != 0) continue;
    for (i = 0; i < 4; i++) {
      l.mode=0;
      l.color=RGB(0, 0, 255);
      l.x1= seg[j].pt[i].x * s;
      l.y1= seg[j].pt[i].y * s;
      l.x2= (p.x)*s;
      l.y2= (p.y)*s;
      p = seg[j].pt[i];

      g_laser_linelist->AddLine(&l);
      l.x1 = l.x2;
      l.y1 = l.y2;
      l.mode=1;
      l.color=RGB(0, 255, 0);
      g_laser_linelist->AddLine(&l);
    }
  }

  for (i = NSEG-1; i > 0; i--) {
    seg[i] = seg[i-1];
  }

  phase += 0.005f;
  return 0;
}

C_RBASE *RLASER_Bren(char *desc)
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}


static C_THISCLASS *g_this;

static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return 0;//WASABI_API_CREATEDIALOG(IDD_CFG_LINE,hwndParent,g_DlgProc);
}
#endif