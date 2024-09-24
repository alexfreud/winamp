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
#include <stdlib.h>
#include <vfw.h>
#include <commctrl.h>
#include <math.h>
#include "resource.h"
#include "r_defs.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

#define MOD_NAME "Trans / Water Bump"
#define C_THISCLASS C_WaterBumpClass

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_WATER_BUMP,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
		void SineBlob(int x, int y, int radius, int height, int page);
		void CalcWater(int npage, int density);
		void CalcWaterSludge(int npage, int density);
		void HeightBlob(int x, int y, int radius, int height, int page);

    int enabled;
	int *buffers[2];
	int buffer_w,buffer_h;
	int page;
	int density;
	int depth;
	int random_drop;
	int drop_position_x;
	int drop_position_y;
	int drop_radius;
	int method;
};


static C_THISCLASS *g_ConfigThis; // global configuration dialog pointer 
static HINSTANCE g_hDllInstance; // global DLL instance pointer (not needed in this example, but could be useful)



// configuration read/write

C_THISCLASS::C_THISCLASS() // set up default configuration
{
  int i;
  enabled=1; density=6; depth=600; random_drop=0; drop_position_x=1; drop_position_y=1; drop_radius=40; method=0;
  buffer_w=0; buffer_h=0; 
  for(i=0;i<2;i++)
	  buffers[i]=NULL;
  page=0;
}
C_THISCLASS::~C_THISCLASS()
{
  int i;
  for(i=0;i<2;i++) {
	  if(buffers[i]) GlobalFree(buffers[i]);
	  buffers[i]=NULL;
  }
}

#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	int pos=0;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { density=GET_INT(); pos+=4; }
	if (len-pos >= 4) { depth=GET_INT(); pos+=4; }
	if (len-pos >= 4) { random_drop=GET_INT(); pos+=4; }
	if (len-pos >= 4) { drop_position_x=GET_INT(); pos+=4; }
	if (len-pos >= 4) { drop_position_y=GET_INT(); pos+=4; }
	if (len-pos >= 4) { drop_radius=GET_INT(); pos+=4; }
	if (len-pos >= 4) { method=GET_INT(); pos+=4; }
}

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
int  C_THISCLASS::save_config(unsigned char *data) // write configuration to data, return length. config data should not exceed 64k.
{
	int pos=0;
	PUT_INT(enabled); pos+=4;
	PUT_INT(density); pos+=4;
	PUT_INT(depth); pos+=4;
	PUT_INT(random_drop); pos+=4;
	PUT_INT(drop_position_x); pos+=4;
	PUT_INT(drop_position_y); pos+=4;
	PUT_INT(drop_radius); pos+=4;
	PUT_INT(method); pos+=4;
	return pos;
}


void C_THISCLASS::SineBlob(int x, int y, int radius, int height, int page)
{
  int cx, cy;
  int left,top,right,bottom;
  int square;
  double dist;
  int radsquare = radius * radius;
  double length = (1024.0/(float)radius)*(1024.0/(float)radius);

  if(x<0) x = 1+radius+ rand()%(buffer_w-2*radius-1);
  if(y<0) y = 1+radius+ rand()%(buffer_h-2*radius-1);


  radsquare = (radius*radius);

  left=-radius; right = radius;
  top=-radius; bottom = radius;

  // Perform edge clipping...
  if(x - radius < 1) left -= (x-radius-1);
  if(y - radius < 1) top  -= (y-radius-1);
  if(x + radius > buffer_w-1) right -= (x+radius-buffer_w+1);
  if(y + radius > buffer_h-1) bottom-= (y+radius-buffer_h+1);

  for(cy = top; cy < bottom; cy++)
  {
    for(cx = left; cx < right; cx++)
    {
      square = cy*cy + cx*cx;
      if(square < radsquare)
      {
        dist = sqrt(square*length);
        buffers[page][buffer_w*(cy+y) + cx+x]
          += (int)((cos(dist)+0xffff)*(height)) >> 19;
      }
    }
  }
}

void C_THISCLASS::HeightBlob(int x, int y, int radius, int height, int page)
{
  int rquad;
  int cx, cy, cyq;
  int left, top, right, bottom;

  rquad = radius * radius;

  // Make a randomly-placed blob...
  if(x<0) x = 1+radius+ rand()%(buffer_w-2*radius-1);
  if(y<0) y = 1+radius+ rand()%(buffer_h-2*radius-1);

  left=-radius; right = radius;
  top=-radius; bottom = radius;

  // Perform edge clipping...
  if(x - radius < 1) left -= (x-radius-1);
  if(y - radius < 1) top  -= (y-radius-1);
  if(x + radius > buffer_w-1) right -= (x+radius-buffer_w+1);
  if(y + radius > buffer_h-1) bottom-= (y+radius-buffer_h+1);


  for(cy = top; cy < bottom; cy++)
  {
    cyq = cy*cy;
    for(cx = left; cx < right; cx++)
    {
      if(cx*cx + cyq < rquad)
        buffers[page][buffer_w*(cy+y) + (cx+x)] += height;
    }
  }

}


void C_THISCLASS::CalcWater(int npage, int density)
{
  int newh;
  int count = buffer_w + 1;

  int *newptr = buffers[npage];
  int *oldptr = buffers[!npage];

  int x, y;

  for (y = (buffer_h-1)*buffer_w; count < y; count += 2)
  {
    for (x = count+buffer_w-2; count < x; count++)
    {
// This does the eight-pixel method.  It looks much better.

      newh          = ((oldptr[count + buffer_w]
                      + oldptr[count - buffer_w]
                      + oldptr[count + 1]
                      + oldptr[count - 1]
                      + oldptr[count - buffer_w - 1]
                      + oldptr[count - buffer_w + 1]
                      + oldptr[count + buffer_w - 1]
                      + oldptr[count + buffer_w + 1]
                       ) >> 2 )
                      - newptr[count];


      newptr[count] =  newh - (newh >> density);
    }
  }
}

/*
void C_THISCLASS::CalcWaterSludge(int npage, int density)
{
  int newh;
  int count = buffer_w + 1;

  int *newptr = buffers[npage];
  int *oldptr = buffers[!npage];

  int x, y;

  for (y = (buffer_h-1)*buffer_w; count < y; count += 2)
  {
    for (x = count+buffer_w-2; count < x; count++)
    {
// This is the "sludge" method...
      newh = (oldptr[count]<<2)
           +  oldptr[count-1-buffer_w]
           +  oldptr[count+1-buffer_w]
           +  oldptr[count-1+buffer_w]
           +  oldptr[count+1+buffer_w]
           + ((oldptr[count-1]
           +   oldptr[count+1]
           +   oldptr[count-buffer_w]
           +   oldptr[count+buffer_w])<<1);

      newptr[count] = (newh-(newh>>6)) >> density;
    }
  }
}
*/
// render function
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout. this is
// used when you want to do something that you'd otherwise need to make a copy of the framebuffer.
// w and h are the width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  if (!enabled) return 0;
  int l,i;
  l=w*h;

  if(buffer_w!=w||buffer_h!=h) {
	  for(i=0;i<2;i++) {
		  if(buffers[i])GlobalFree(buffers[i]);
		  buffers[i]=NULL;
	  }
  }
  if(buffers[0]==NULL) {
	  for(i=0;i<2;i++) {
		buffers[i]=(int *)GlobalAlloc(GPTR,w*h*sizeof(int));
		}
	  buffer_w=w;
	  buffer_h=h;
  }
  if (isBeat&0x80000000) return 0;

  if(isBeat) {
	  if(random_drop) {
		int max=w;
		if(h>w) max=h;
		SineBlob(-1,-1,drop_radius*max/100,-depth,page);
	  } else {
		int x,y;
		switch(drop_position_x) {
			case 0: x=w/4; break;
			case 1: x=w/2; break;
			case 2: x=w*3/4; break;
		}
		switch(drop_position_y) {
			case 0: y=h/4; break;
			case 1: y=h/2; break;
			case 2: y=h*3/4; break;
		}
		SineBlob(x,y,drop_radius,-depth,page);
	  }
//	HeightBlob(-1,-1,80/2,1400,page);
  }

{
  int dx, dy;
  int x, y;
  int ofs,len=buffer_h*buffer_w;

  int offset=buffer_w + 1;

  int *ptr = buffers[page];

  for (y = (buffer_h-1)*buffer_w; offset < y; offset += 2)
  {
    for (x = offset+buffer_w-2; offset < x; offset++)
    {
      dx = ptr[offset] - ptr[offset+1];
      dy = ptr[offset] - ptr[offset+buffer_w];
	  ofs=offset + buffer_w*(dy>>3) + (dx>>3);
	  if((ofs<len)&&(ofs>-1))
		fbout[offset] = framebuffer[ofs];
	  else
		fbout[offset] = framebuffer[offset];

	  offset++;
      dx = ptr[offset] - ptr[offset+1];
      dy = ptr[offset] - ptr[offset+buffer_w];
	  ofs=offset + buffer_w*(dy>>3) + (dx>>3);
	  if((ofs<len)&&(ofs>-1))
		fbout[offset] = framebuffer[ofs];
	  else
		fbout[offset] = framebuffer[offset];
 
    }
  }
}

  CalcWater(!page,density);

  page=!page;

  return 1;
}


// configuration dialog stuff


static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			if (g_ConfigThis->enabled) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
			SendDlgItemMessage(hwndDlg,IDC_DAMP,TBM_SETRANGEMIN,0,2);
			SendDlgItemMessage(hwndDlg,IDC_DAMP,TBM_SETRANGEMAX,0,10);
			SendDlgItemMessage(hwndDlg,IDC_DAMP,TBM_SETPOS,1,g_ConfigThis->density);
			SendDlgItemMessage(hwndDlg,IDC_DEPTH,TBM_SETRANGEMIN,0,100);
			SendDlgItemMessage(hwndDlg,IDC_DEPTH,TBM_SETRANGEMAX,0,2000);
			SendDlgItemMessage(hwndDlg,IDC_DEPTH,TBM_SETPOS,1,g_ConfigThis->depth);
			SendDlgItemMessage(hwndDlg,IDC_RADIUS,TBM_SETRANGEMIN,0,10);
			SendDlgItemMessage(hwndDlg,IDC_RADIUS,TBM_SETRANGEMAX,0,100);
			SendDlgItemMessage(hwndDlg,IDC_RADIUS,TBM_SETPOS,1,g_ConfigThis->drop_radius);
			CheckDlgButton(hwndDlg,IDC_RANDOM_DROP,g_ConfigThis->random_drop);
			CheckDlgButton(hwndDlg,IDC_DROP_LEFT,g_ConfigThis->drop_position_x==0);
			CheckDlgButton(hwndDlg,IDC_DROP_CENTER,g_ConfigThis->drop_position_x==1);
			CheckDlgButton(hwndDlg,IDC_DROP_RIGHT,g_ConfigThis->drop_position_x==2);
			CheckDlgButton(hwndDlg,IDC_DROP_TOP,g_ConfigThis->drop_position_y==0);
			CheckDlgButton(hwndDlg,IDC_DROP_MIDDLE,g_ConfigThis->drop_position_y==1);
			CheckDlgButton(hwndDlg,IDC_DROP_BOTTOM,g_ConfigThis->drop_position_y==2);
			return 1;
		case WM_DRAWITEM:
		return 0;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDC_CHECK1)
        g_ConfigThis->enabled=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
      if (LOWORD(wParam) == IDC_RANDOM_DROP)
        g_ConfigThis->random_drop=IsDlgButtonChecked(hwndDlg,IDC_RANDOM_DROP);
      if (LOWORD(wParam) == IDC_DROP_LEFT)
        g_ConfigThis->drop_position_x=0;
      if (LOWORD(wParam) == IDC_DROP_CENTER)
        g_ConfigThis->drop_position_x=1;
      if (LOWORD(wParam) == IDC_DROP_RIGHT)
        g_ConfigThis->drop_position_x=2;
      if (LOWORD(wParam) == IDC_DROP_TOP)
        g_ConfigThis->drop_position_y=0;
      if (LOWORD(wParam) == IDC_DROP_MIDDLE)
        g_ConfigThis->drop_position_y=1;
      if (LOWORD(wParam) == IDC_DROP_BOTTOM)
        g_ConfigThis->drop_position_y=2;
      return 0;
	case WM_HSCROLL:
		{
			HWND swnd = (HWND) lParam;
			int t = (int) SendMessage(swnd,TBM_GETPOS,0,0);
			if (swnd == GetDlgItem(hwndDlg,IDC_DAMP))
				g_ConfigThis->density=t;
			if (swnd == GetDlgItem(hwndDlg,IDC_DEPTH))
				g_ConfigThis->depth=t;
			if (swnd == GetDlgItem(hwndDlg,IDC_RADIUS))
				g_ConfigThis->drop_radius=t;
		}
		return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_ConfigThis = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_WATERBUMP,hwndParent,g_DlgProc);
}



// export stuff

C_RBASE *R_WaterBump(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}

#else
C_RBASE *R_WaterBump(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{return NULL; }
#endif