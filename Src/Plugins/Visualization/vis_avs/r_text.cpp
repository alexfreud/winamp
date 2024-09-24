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
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <commctrl.h>
#include "resource.h"
#include "r_defs.h"

#include "avs_eelif.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER


#define MOD_NAME "Render / Text"
#define C_THISCLASS C_TextClass

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		void getWord(int n, char *buf, int max);
		void reinit(int w, int h);
		float GET_FLOAT(unsigned char *data, int pos);
		void PUT_FLOAT(float f, unsigned char *data, int pos);
		void InitializeStars(int Start);
		void CreateStar(int A);
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_RENDER_TEXT,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
	CHOOSEFONT cf;
	LOGFONT lf;
    HBITMAP hRetBitmap; 
    HBITMAP hOldBitmap; 
	HFONT hOldFont;
	HFONT myFont;
    HDC hDesktopDC; 
    HDC hBitmapDC; 
	BITMAPINFO bi;
    int enabled;
    int color;
	int blend;
	int blendavg;
	int onbeat;
	int insertBlank;
	int randomPos;
	int valign;
	int halign;
	int onbeatSpeed;
	int normSpeed;
	char *text;
	int lw,lh;
	bool updating;
	int outline;
	int shadow;
	int outlinecolor;
	int outlinesize;
	int curword;
	int nb;
	int forceBeat;
	int xshift, yshift;
	int _xshift, _yshift;
	int forceshift;
	int forcealign;
	int _valign, _halign;
	int oddeven;
	int nf;
	RECT r;
	int *myBuffer;
	int forceredraw;
    int old_valign, old_halign, old_outline, oldshadow;
    int old_curword, old_clipcolor;
    char oldtxt[256];
    int old_blend1, old_blend2, old_blend3;
	int oldxshift, oldyshift;
	int randomword;
	int shiftinit;
	};


static C_THISCLASS *g_ConfigThis; // global configuration dialog pointer 
static HINSTANCE g_hDllInstance; // global DLL instance pointer (not needed in this example, but could be useful)

// Reinit bitmap buffer since size changed
void C_THISCLASS::reinit(int w, int h)
{
// Free anything if needed
if (lw || lh)
   {
   SelectObject (hBitmapDC, hOldBitmap); 
   if (hOldFont) SelectObject(hBitmapDC, hOldFont);
   DeleteDC (hBitmapDC); 
   ReleaseDC (NULL, hDesktopDC); 
   if (myBuffer) GlobalFree(myBuffer);
   }

// Alloc buffers, select objects, init structures
   myBuffer = (int *)GlobalAlloc(GMEM_FIXED,w*h*4);
   hDesktopDC = GetDC (NULL); 
   hRetBitmap = CreateCompatibleBitmap (hDesktopDC, w, h);
   hBitmapDC = CreateCompatibleDC (hDesktopDC); 
   hOldBitmap = (HBITMAP) SelectObject (hBitmapDC, hRetBitmap); 
   SetTextColor(hBitmapDC, ((color & 0xFF0000) >> 16) | (color & 0xFF00) | (color&0xFF)<<16);
   SetBkMode(hBitmapDC, TRANSPARENT);
   SetBkColor(hBitmapDC, 0);
   if (myFont)
	 hOldFont = (HFONT) SelectObject(hBitmapDC, myFont);
	else
	 hOldFont = NULL;
   bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bi.bmiHeader.biWidth = w;
   bi.bmiHeader.biHeight = h;
   bi.bmiHeader.biPlanes = 1;
   bi.bmiHeader.biBitCount = 32;
   bi.bmiHeader.biCompression = BI_RGB;
   bi.bmiHeader.biSizeImage = 0;
   bi.bmiHeader.biXPelsPerMeter = 0;
   bi.bmiHeader.biYPelsPerMeter = 0;
   bi.bmiHeader.biClrUsed = 0;
   bi.bmiHeader.biClrImportant = 0;
}


C_THISCLASS::~C_THISCLASS() 
{
// Free up everything
if (lw || lh)
	{
    SelectObject (hBitmapDC, hOldBitmap); 
    if (hOldFont) SelectObject(hBitmapDC, hOldFont);
	DeleteDC (hBitmapDC); 
	ReleaseDC (NULL, hDesktopDC); 
	if (myBuffer) GlobalFree(myBuffer);
	}
if (text) GlobalFree(text);
if (myFont)
	DeleteObject(myFont);
}

// configuration read/write

C_THISCLASS::C_THISCLASS() // set up default configuration
{
// Init all
  randomword=0;
  shadow=0;
  old_valign=0;
  old_halign=0;
  old_outline=-1;
  old_curword=-1;
  old_clipcolor=-1;
  oldshadow=-1;
  *oldtxt=0;
  old_blend1=0;
  old_blend2=0;
  old_blend3=0;
  _xshift=0;
  _yshift=0;
  forceredraw=0;
  myBuffer = NULL;
  curword=0;
  forceshift=0;
  forceBeat=0;
  forcealign=1;
  xshift=0;
  yshift=0;
  outlinecolor = 0;
  outline=0;
  outline=0;
  color = 0xFFFFFF;
  enabled = 1;
  blend = 0;
  blendavg = 0;
  onbeat = 0;
  insertBlank = 0;
  randomPos = 0;
  halign = DT_CENTER;
  valign = DT_VCENTER;
  onbeatSpeed=15;
  normSpeed=15;
  myFont = NULL;
  memset(&cf, 0, sizeof(CHOOSEFONT));
  cf.lStructSize = sizeof(CHOOSEFONT);
  cf.lpLogFont = &lf;
  cf.Flags = CF_EFFECTS|CF_SCREENFONTS |CF_FORCEFONTEXIST|CF_INITTOLOGFONTSTRUCT;
  cf.rgbColors = color;
  memset(&lf, 0, sizeof(LOGFONT));
  text = NULL;
  lw=lh=0;
  updating=false;
  nb=0;
  outlinesize=1;
  oddeven=0;
  nf=0;
  shiftinit=1;
}

#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))

float C_THISCLASS::GET_FLOAT(unsigned char *data, int pos)
{
int a = data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24);
float f = *(float *)&a;
return f;
}

void C_THISCLASS::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	int pos=0;
	int size;
	updating=true;
	forceredraw=1;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { color=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blend=GET_INT(); pos+=4; }
	if (len-pos >= 4) { blendavg=GET_INT(); pos+=4; }
	if (len-pos >= 4) { onbeat=GET_INT(); pos+=4; }
	if (len-pos >= 4) { insertBlank=GET_INT(); pos+=4; }
	if (len-pos >= 4) { randomPos=GET_INT(); pos+=4; }
	if (len-pos >= 4) { valign=GET_INT(); pos+=4; }
	if (len-pos >= 4) { halign=GET_INT(); pos+=4; }
	if (len-pos >= 4) { onbeatSpeed=GET_INT(); pos+=4; }
	if (len-pos >= 4) { normSpeed=GET_INT(); pos+=4; }
	if (len-pos >= sizeof(cf)) { memcpy(&cf, data+pos, sizeof(cf)); pos+=sizeof(cf);}
	cf.lpLogFont = &lf;
	if (len-pos >= sizeof(lf)) { memcpy(&lf, data+pos, sizeof(lf)); pos+=sizeof(lf);}
	if (len-pos >= 4) { size=GET_INT(); pos+=4; }
	if (size > 0 && len-pos >= size)
		{
		if (text) GlobalFree(text);
		text = (char *)GlobalAlloc(GMEM_FIXED,size+1);
		memcpy(text, data+pos, size);
		pos+=size;
		}
	else
		{
		if (text) GlobalFree(text);
		text = NULL;
		}
	myFont = CreateFontIndirect(&lf);
	if (len-pos >= 4) { outline=GET_INT(); pos+=4; }
	if (len-pos >= 4) { outlinecolor=GET_INT(); pos+=4; }
	if (len-pos >= 4) { xshift=GET_INT(); pos+=4; }
	if (len-pos >= 4) { yshift=GET_INT(); pos+=4; }
	if (len-pos >= 4) { outlinesize=GET_INT(); pos+=4; }
	if (len-pos >= 4) { randomword=GET_INT(); pos+=4; }
	if (len-pos >= 4) { shadow=GET_INT(); pos+=4; }
	forcealign=1;
	forceredraw=1;
	forceshift=1;
	shiftinit=1;
	updating=false;
}

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
void C_THISCLASS::PUT_FLOAT(float f, unsigned char *data, int pos)
{
int y = *(int *)&f;
data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255;
}
extern HWND hwnd_WinampParent;


int  C_THISCLASS::save_config(unsigned char *data) // write configuration to data, return length. config data should not exceed 64k.
{
  int pos=0;	   
  PUT_INT(enabled); pos+=4;
  PUT_INT(color); pos+=4;
  PUT_INT(blend); pos+=4;
  PUT_INT(blendavg); pos+=4;
  PUT_INT(onbeat); pos+=4;
  PUT_INT(insertBlank); pos+=4;
  PUT_INT(randomPos); pos+=4;
  PUT_INT(valign); pos+=4;
  PUT_INT(halign); pos+=4;
  PUT_INT(onbeatSpeed); pos+=4;
  PUT_INT(normSpeed); pos+=4;
  memcpy(data+pos, &cf, sizeof(cf));
  pos+=sizeof(cf);
  memcpy(data+pos, &lf, sizeof(lf));
  pos+=sizeof(lf);
  if (text)
	{
  int l=strlen(text)+1;
	PUT_INT(l); pos+=4;
	memcpy(data+pos, text, strlen(text)+1);
	pos+=strlen(text)+1;
	}
  else
    { PUT_INT(0); pos+=4;}
  PUT_INT(outline); pos+=4;
  PUT_INT(outlinecolor); pos+=4;
  PUT_INT(xshift); pos+=4;
  PUT_INT(yshift); pos+=4;
  PUT_INT(outlinesize); pos+=4;
  PUT_INT(randomword); pos+=4;
  PUT_INT(shadow); pos+=4;
	
  return pos;
}
// Parse text buffer for a specified word
void C_THISCLASS::getWord(int n, char *buf, int maxlen)
{
  int w=0;
  char *p=text;
  char *d=buf;
  *d=0;
  if (!p) return;
  while (w<n && *p)
	  {
	  if (*p==';')
		  w++; 
	  p++;
	  }

  maxlen--; // null terminator
  while (*p && *p != ';' && maxlen>0)
  {
    char *endp;
    if ((!strnicmp(p,"$(playpos",9) || !strnicmp(p,"$(playlen",9)) && (endp = strstr(p+9,")")))
    {
      char buf[128];
      int islen=strnicmp(p,"$(playpos",9);
      int add_dig=0;
      if (p[9]=='.')
        add_dig=atoi(p+10);
      
      if (add_dig > 3) add_dig=3;

      int pos=0;

      extern HWND hwnd_WinampParent;
	    if (IsWindow(hwnd_WinampParent)) 
      {
        if (!SendMessageTimeout( hwnd_WinampParent, WM_USER,(WPARAM)!!islen,(LPARAM)105,SMTO_BLOCK,50,(LPDWORD)&pos)) 
          pos=0;
      }
      if (islen) pos*=1000;
      wsprintf(buf,"%d:%02d",pos/60000,(pos/1000)%60);
      if (add_dig>0)
      {
        char fmt[16];       
        wsprintf(fmt,".%%%02dd",add_dig);
        int div=1;
        int x;
        for (x = 0; x < 3-add_dig; x ++)
        {
          div*=10;
        }
        wsprintf(buf+strlen(buf),fmt,(pos%1000) / div);
      }

      int l=strlen(buf);
      if (l > maxlen) l=maxlen;
      memcpy(d,buf,l);
      maxlen-=l;
      d += l;

      p=endp+1;
    }
    else if (!strnicmp(p,"$(title",7) && (endp = strstr(p+7,")")))
    {
      static char this_title[256];
      static unsigned int ltg;

      unsigned int now=GetTickCount();

      if (!this_title[0] || now-ltg > 1000 || now < ltg)
      {
        ltg=now;

        char *tpp;
	      if (IsWindow(hwnd_WinampParent)) 
        {
          DWORD id;
          if (!SendMessageTimeout( hwnd_WinampParent,WM_GETTEXT,(WPARAM)sizeof(this_title),(LPARAM)this_title,SMTO_BLOCK,50,&id) || !id) 
          {
            this_title[0]=0;
          }
        }
	      tpp = this_title+strlen(this_title);
	      while (tpp >= this_title)
	      {
		      char buf[9];
		      memcpy(buf,tpp,8);
		      buf[8]=0;
		      if (!lstrcmpi(buf,"- Winamp")) break;
		      tpp--;
	      }
	      if (tpp >= this_title) tpp--;
	      while (tpp >= this_title && *tpp == ' ') tpp--;
	      *++tpp=0;
      }

      int skipnum=1,max_fmtlen=0;
      char *titleptr=this_title;

      if (p[7] == ':')
      {
        char *ptr=p+8;
        if (*ptr == 'n') { ptr++; skipnum=0; }

        max_fmtlen=atoi(ptr);
      }

      // use: $(reg00), $(reg00:4.5), $(title), $(title:32), $(title:n32)

      if (skipnum && *titleptr >= '0' && *titleptr <= '9')
      {
        while (*titleptr >= '0' && *titleptr <= '9') titleptr++;
        if (*titleptr == '.') 
        {
          titleptr++;
          if (*titleptr == ' ') 
          {
            titleptr++;
          }
          else titleptr=this_title;
        }
        else titleptr=this_title;
      }
      int n=strlen(titleptr);
      if (n > maxlen)n=maxlen;
      if (max_fmtlen >0 && max_fmtlen < n) n=max_fmtlen;

      memcpy(d,titleptr,n);
      maxlen-=n;
      d+=n;
      p=endp+1;
    }
    else if (!strnicmp(p,"$(reg",5) && p[5] >= '0' && p[5] <= '9' &&
                                  p[6] >= '0' && p[6] <= '9' && (endp = strstr(p+7,")")))
    {
      char buf[128];
      char fmt[32];
      int wr=atoi(p+5);
      if (wr <0)wr=0;
      if (wr>99)wr=99;
      p+=7;
      char *fmtptr=fmt;
      *fmtptr++='%';
      if (*p == ':')
      {
        p++;
        while (fmtptr-fmt < 16 && ((*p >= '0' && *p <= '9') || *p == '.'))
          *fmtptr++=*p++;
      }
      *fmtptr++='f';
      *fmtptr++=0;

      int l=sprintf(buf,fmt,NSEEL_getglobalregs()[wr]);
      if (l > maxlen) l=maxlen;
      memcpy(d,buf,l);
      maxlen-=l;
      d += l;
      p = endp+1;
    }
    else 
    {
      *d++=*p++;
      maxlen--;
    }
  }
  *d=0;
}

// Returns number of words in buffer
static int getNWords(char *buf)
{
char *p=buf;
int n=0;
while (p && *p)
	{
	if (*p==';')
		n++;
	p++;
	}
return n;	
}

// render function
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout. this is
// used when you want to do something that you'd otherwise need to make a copy of the framebuffer.
// w and h are the width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
  int i,j;
  int *p,*d;
  int clipcolor;
  char thisText[256];

  if (updating) return 0;
  if (!enabled) return 0;
  if (isBeat&0x80000000) return 0;

  if (forcealign)
	{
	forcealign=false;
	_halign = halign;
	_valign = valign;
	}
  if (shiftinit)
	{
	shiftinit=0;
	_xshift = xshift;
	_yshift = yshift;
	}

  // If not beat sensitive and time is up for this word
  // OR if beat sensitive and this frame is a beat and time is up for last beat
  if ((!onbeat && nf >= normSpeed) || (onbeat && isBeat && !nb))
  { // Then choose which word to show 
	if (!(insertBlank && !(oddeven % 2))) 
		{
		if (randomword)
			curword=rand()%(getNWords(text)+1);
		else
			{
			curword++;
			curword%=(getNWords(text)+1);
			}
		}
    oddeven++;
    oddeven%=2;
	}

  if (forceBeat) 
	{
	isBeat = 1;
	forceBeat=0;
	}

  // If beat sensitive and frame is a beat and last beat expired, start frame timer for this beat
  if (onbeat && isBeat && !nb)
	nb = onbeatSpeed;

  // Get the word(s) to show
  getWord(curword, thisText, 256);
  if (insertBlank && !oddeven)
	*thisText=0;

  // Same test as above but takes care of nb init
  if ((!onbeat && nf >= normSpeed) || (onbeat && isBeat && nb == onbeatSpeed))
  {
	nf=0;
	if (randomPos && w && h) // Handle random position
		{
		SIZE size={0,0};
		GetTextExtentPoint32(hBitmapDC, thisText, strlen(thisText), &size); // Don't write outside the screen
		_halign = DT_LEFT;
		if (size.cx < w)
			_xshift = rand() % (int)(((float)(w-size.cx) / (float)w) * 100.0F);
		_valign = DT_TOP;
		if (size.cy < h)
			_yshift = rand() % (int)(((float)(h-size.cy) / (float)h) * 100.0F);
		forceshift=1;	
		}
	else
		{ // Reset position to what is specified
		_halign = halign;
		_valign = valign;
		_xshift = xshift;
		_yshift = yshift;
		}
  }

  // Choose cliping color
  if (color != 0 && outlinecolor != 0)
    clipcolor = 0;
  else
    if (color != 0x000008 && outlinecolor != 0x000008)
		clipcolor = 8;
	else
		if (color != 0x0000010 && outlinecolor != 0x0000010)
			clipcolor = 10;
			
  // If size changed or if we're forced to shift the buffer
  if ((lw != w || lh != h) || forceshift)
	{
	if (lw != w || lh != h) // only if size changed then reinit buffer, not if its only a forcedshifting
		reinit(w, h);
	forceshift=0;
	forceredraw=1; // do redraw!
	// remember last state
	lw = w;
	lh = h;
	// Compute buffer position
	r.left = 0;
	r.right = w;
	r.top = 0;
	r.bottom = h;
	r.left += (int)((float)_xshift * (float)w / 100.0F);
	r.right += (int)((float)_xshift * (float)w / 100.0F);
	r.top += (int)((float)_yshift * (float)h / 100.0F);
	r.bottom += (int)((float)_yshift * (float)h / 100.0F);
	forceredraw=1;
	}

	// Check if we need to redraw the buffer
	if (forceredraw || 
	    old_halign != _halign ||
		old_valign != _valign ||
		curword != old_curword ||
        (*thisText && strcmp(thisText, oldtxt)) ||
		old_clipcolor != clipcolor ||
		(old_blend1 != (blend && !(onbeat && !nb))) ||
		(old_blend2 != (blendavg && !(onbeat && !nb))) ||
		(old_blend3 != (!(onbeat && !nb))) ||
		old_outline != outline ||
		oldshadow != shadow ||
		_xshift != oldxshift ||
		_yshift != oldyshift)
		{
		forceredraw=0;
		old_halign = _halign;
		old_valign = _valign;
		old_curword = curword;
		if (thisText) strcpy(oldtxt, thisText);
		old_clipcolor = clipcolor;
		old_blend1 = (blend && !(onbeat && !nb));
		old_blend2 = (blendavg && !(onbeat && !nb));
		old_blend3 = !(onbeat && !nb);
		old_outline = outline;
		oldshadow = shadow;
		oldxshift = _xshift;
		oldyshift = _yshift;

	// Draw everything
	p = myBuffer;
	while (p < myBuffer+h*w) { *p = clipcolor; p++; }
	SetDIBits(hBitmapDC, hRetBitmap, 0, h, (void *)myBuffer, &bi, DIB_RGB_COLORS);
    if (*thisText)
		{
		if (outline)
			{
			SetTextColor(hBitmapDC, ((outlinecolor&0xFF0000) >> 16) | (outlinecolor& 0xFF00) | (outlinecolor&0xFF)<<16);
			r.left-=outlinesize; r.right-=outlinesize; r.top-=outlinesize; r.bottom-=outlinesize;
    		DrawText(hBitmapDC, thisText, strlen(thisText), &r, _valign | _halign | DT_NOCLIP | DT_SINGLELINE);
			r.left+=outlinesize; r.right+=outlinesize;
    		DrawText(hBitmapDC, thisText, strlen(thisText), &r, _valign | _halign | DT_NOCLIP | DT_SINGLELINE);
			r.left+=outlinesize; r.right+=outlinesize;
    		DrawText(hBitmapDC, thisText, strlen(thisText), &r, _valign | _halign | DT_NOCLIP | DT_SINGLELINE);
			r.top+=outlinesize; r.bottom+=outlinesize;
    		DrawText(hBitmapDC, thisText, strlen(thisText), &r, _valign | _halign | DT_NOCLIP | DT_SINGLELINE);
			r.top+=outlinesize; r.bottom+=outlinesize;
    		DrawText(hBitmapDC, thisText, strlen(thisText), &r, _valign | _halign | DT_NOCLIP | DT_SINGLELINE);
			r.left-=outlinesize; r.right-=outlinesize;
    		DrawText(hBitmapDC, thisText, strlen(thisText), &r, _valign | _halign | DT_NOCLIP | DT_SINGLELINE);
			r.left-=outlinesize; r.right-=outlinesize;
    		DrawText(hBitmapDC, thisText, strlen(thisText), &r, _valign | _halign | DT_NOCLIP | DT_SINGLELINE);
			r.top-=outlinesize; r.bottom-=outlinesize;
    		DrawText(hBitmapDC, thisText, strlen(thisText), &r, _valign | _halign | DT_NOCLIP | DT_SINGLELINE);
			r.left+=outlinesize; r.right+=outlinesize;
			SetTextColor(hBitmapDC, ((color&0xFF0000) >> 16) | (color& 0xFF00) | (color&0xFF)<<16);
			}
		else if (shadow)
			{
			SetTextColor(hBitmapDC, ((outlinecolor&0xFF0000) >> 16) | (outlinecolor& 0xFF00) | (outlinecolor&0xFF)<<16);
			r.left+=outlinesize; r.right+=outlinesize; r.top+=outlinesize; r.bottom+=outlinesize;
    		DrawText(hBitmapDC, thisText, strlen(thisText), &r, _valign | _halign | DT_NOCLIP | DT_SINGLELINE);
			r.left-=outlinesize; r.right-=outlinesize; r.top-=outlinesize; r.bottom-=outlinesize;
			SetTextColor(hBitmapDC, ((color&0xFF0000) >> 16) | (color& 0xFF00) | (color&0xFF)<<16);
			}
    	DrawText(hBitmapDC, thisText, strlen(thisText), &r, _valign | _halign | DT_NOCLIP | DT_SINGLELINE);
		}
	GetDIBits(hBitmapDC, hRetBitmap, 0, h, (void *)myBuffer, &bi, DIB_RGB_COLORS);
	}

  // Now render the bitmap text buffer over framebuffer, handle blending options.
  // Separate blocks here so we don4t have to make w*h tests 
  p = myBuffer;
  d = framebuffer+w*(h-1);

	 if (blend && !(onbeat && !nb))
	   for (i=0;i<h;i++)
		{
		for (j=0;j<w;j++)
			{
			if (*p != clipcolor) *d=BLEND(*p, *d);
			d++;
			p++;
			}
		d -= w*2;
		}
	  else
	  if (blendavg && !(onbeat && !nb))
	   for (i=0;i<h;i++)
		{
		for (j=0;j<w;j++)
			{
			if (*p != clipcolor) *d=BLEND_AVG(*p, *d);
			d++;
			p++;
			}
		d -= w*2;
		}
	  else
	   if (!(onbeat && !nb))
	   for (i=0;i<h;i++)
		{
		for (j=0;j<w;j++)
			{
			if (*p != clipcolor) 
				*d=*p;
			d++;
			p++;
			}
		d -= w*2;
		}
	   
  // Advance frame counter
  if (!onbeat) nf++;
  // Decrease frametimer
  if (onbeat && nb) nb--;


  return 0;
}

static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
switch (uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg, IDC_EDIT, g_ConfigThis->text);
		SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETTICFREQ, 10, 0);
		SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETRANGE, TRUE, MAKELONG(1, 400));
		SendDlgItemMessage(hwndDlg, IDC_OUTLINESIZE, TBM_SETTICFREQ, 15, 0);
		SendDlgItemMessage(hwndDlg, IDC_OUTLINESIZE, TBM_SETRANGE, TRUE, MAKELONG(1, 16));
		SendDlgItemMessage(hwndDlg, IDC_OUTLINESIZE, TBM_SETPOS, TRUE, g_ConfigThis->outlinesize);
		SendDlgItemMessage(hwndDlg, IDC_VSHIFT, TBM_SETTICFREQ, 50, 0);
		SendDlgItemMessage(hwndDlg, IDC_VSHIFT, TBM_SETRANGE, TRUE, MAKELONG(0, 200));
		SendDlgItemMessage(hwndDlg, IDC_VSHIFT, TBM_SETPOS, TRUE, g_ConfigThis->yshift+100);
		SendDlgItemMessage(hwndDlg, IDC_HSHIFT, TBM_SETTICFREQ, 50, 0);
		SendDlgItemMessage(hwndDlg, IDC_HSHIFT, TBM_SETRANGE, TRUE, MAKELONG(0, 200));
		SendDlgItemMessage(hwndDlg, IDC_HSHIFT, TBM_SETPOS, TRUE, g_ConfigThis->xshift+100);
		if (g_ConfigThis->onbeat)
			SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETPOS, TRUE, (int)(g_ConfigThis->onbeatSpeed));
		else
			SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETPOS, TRUE, (int)(g_ConfigThis->normSpeed));
        if (g_ConfigThis->enabled) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
        if (g_ConfigThis->onbeat) CheckDlgButton(hwndDlg,IDC_ONBEAT,BST_CHECKED);
        if (g_ConfigThis->blend) CheckDlgButton(hwndDlg,IDC_ADDITIVE,BST_CHECKED);
        if (g_ConfigThis->blendavg) CheckDlgButton(hwndDlg,IDC_5050,BST_CHECKED);
        if (!g_ConfigThis->blend && !g_ConfigThis->blendavg)
		   CheckDlgButton(hwndDlg,IDC_REPLACE,BST_CHECKED);
        if (g_ConfigThis->enabled) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
        if (g_ConfigThis->outline) CheckDlgButton(hwndDlg,IDC_OUTLINE,BST_CHECKED);
        if (g_ConfigThis->shadow) CheckDlgButton(hwndDlg,IDC_SHADOW,BST_CHECKED);
        if (!g_ConfigThis->outline && !g_ConfigThis->shadow) CheckDlgButton(hwndDlg,IDC_PLAIN,BST_CHECKED);
        if (g_ConfigThis->randomword) CheckDlgButton(hwndDlg,IDC_RANDWORD, BST_CHECKED);
		switch (g_ConfigThis->valign)
			{
			case DT_TOP:
				CheckDlgButton(hwndDlg,IDC_VTOP,BST_CHECKED);
				break;
			case DT_BOTTOM:
				CheckDlgButton(hwndDlg,IDC_VBOTTOM,BST_CHECKED);
				break;
			case DT_VCENTER:
				CheckDlgButton(hwndDlg,IDC_VCENTER,BST_CHECKED);
				break;
			}
		switch (g_ConfigThis->halign)
			{
			case DT_LEFT:
				CheckDlgButton(hwndDlg,IDC_HLEFT,BST_CHECKED);
				break;
			case DT_RIGHT:
				CheckDlgButton(hwndDlg,IDC_HRIGHT,BST_CHECKED);
				break;
			case DT_CENTER:
				CheckDlgButton(hwndDlg,IDC_HCENTER,BST_CHECKED);
				break;
			}
        if (g_ConfigThis->insertBlank) CheckDlgButton(hwndDlg,IDC_BLANKS,BST_CHECKED);
        if (g_ConfigThis->randomPos) {
			CheckDlgButton(hwndDlg,IDC_RANDOMPOS,BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VTOP), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VBOTTOM), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VCENTER), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HLEFT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HRIGHT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HCENTER), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HSHIFT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VSHIFT), FALSE);
			}
		return 1;
	case WM_NOTIFY:
		{
		if ((LOWORD(wParam) == IDC_VSHIFT) || (LOWORD(wParam) == IDC_HSHIFT))
			{
			g_ConfigThis->yshift = SendDlgItemMessage(hwndDlg, IDC_VSHIFT, TBM_GETPOS, 0, 0)-100;
			g_ConfigThis->xshift = SendDlgItemMessage(hwndDlg, IDC_HSHIFT, TBM_GETPOS, 0, 0)-100;
			g_ConfigThis->forceshift=1;
			g_ConfigThis->forceredraw=1;
			g_ConfigThis->shiftinit=1;
			}
		if (LOWORD(wParam) == IDC_OUTLINESIZE)
			{
			g_ConfigThis->outlinesize = SendDlgItemMessage(hwndDlg, IDC_OUTLINESIZE, TBM_GETPOS, 0, 0);
			g_ConfigThis->forceredraw=1;
			}
		if (LOWORD(wParam) == IDC_SPEED)
			{
			if (g_ConfigThis->onbeat)
				{
				g_ConfigThis->onbeatSpeed = SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_GETPOS, 0, 0);
				if (g_ConfigThis->nb > g_ConfigThis->onbeatSpeed)
					g_ConfigThis->nb = g_ConfigThis->onbeatSpeed;
				}
			else
				g_ConfigThis->normSpeed = SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_GETPOS, 0, 0);
			}
			return 0;
		}
	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT *)lParam;
			if (di->CtlID == IDC_DEFCOL || di->CtlID == IDC_DEFOUTCOL) // paint nifty color button
			{
        GR_DrawColoredButton(di,di->CtlID == IDC_DEFCOL ? g_ConfigThis->color : g_ConfigThis->outlinecolor);
			}
	  }
		return 0;
	case WM_COMMAND:
	  if (LOWORD(wParam) == IDC_HRESET)
		{
		g_ConfigThis->xshift = 0;
		SendDlgItemMessage(hwndDlg, IDC_HSHIFT, TBM_SETPOS, TRUE, 100);
		g_ConfigThis->forceshift=1;
		g_ConfigThis->forceredraw=1;
		g_ConfigThis->shiftinit=1;
		}
	  if (LOWORD(wParam) == IDC_VRESET)
		{
		g_ConfigThis->yshift = 0;
		SendDlgItemMessage(hwndDlg, IDC_VSHIFT, TBM_SETPOS, TRUE, 100);
		g_ConfigThis->forceshift=1;
		g_ConfigThis->forceredraw=1;
		g_ConfigThis->shiftinit=1;
		}
	  if (LOWORD(wParam) == IDC_EDIT)
		if (HIWORD(wParam) == EN_CHANGE)
			{
			int l;
			if (g_ConfigThis->text)
				GlobalFree(g_ConfigThis->text);
			l = SendDlgItemMessage(hwndDlg, IDC_EDIT, WM_GETTEXTLENGTH, 0, 0);
			if (!l)
				g_ConfigThis->text = NULL;
			else
				{
				g_ConfigThis->text = (char *)GlobalAlloc(GMEM_FIXED,l+2);
				GetDlgItemText(hwndDlg, IDC_EDIT, g_ConfigThis->text, l+1);
				}
			g_ConfigThis->forceredraw=1;
			}
	  if (LOWORD(wParam) == IDC_CHOOSEFONT)
		{
		g_ConfigThis->cf.hwndOwner = hwndDlg;
		ChooseFont(&(g_ConfigThis->cf));
		g_ConfigThis->updating=true;
		g_ConfigThis->myFont = CreateFontIndirect(&(g_ConfigThis->lf));
		g_ConfigThis->forceredraw=1;
		if (g_ConfigThis->hOldFont)
			SelectObject(g_ConfigThis->hBitmapDC, g_ConfigThis->hOldFont);
		if (g_ConfigThis->myFont)
			g_ConfigThis->hOldFont = (HFONT)SelectObject(g_ConfigThis->hBitmapDC, g_ConfigThis->myFont);
		else
			g_ConfigThis->hOldFont = NULL;
		g_ConfigThis->updating=false;
		}
	  if ((LOWORD(wParam) == IDC_CHECK1) ||
	     (LOWORD(wParam) == IDC_ONBEAT) ||
	     (LOWORD(wParam) == IDC_ADDITIVE) ||
	     (LOWORD(wParam) == IDC_REPLACE) ||
	     (LOWORD(wParam) == IDC_BLANKS) ||
	     (LOWORD(wParam) == IDC_RANDOMPOS) ||
	     (LOWORD(wParam) == IDC_OUTLINE) ||
	     (LOWORD(wParam) == IDC_SHADOW) ||
	     (LOWORD(wParam) == IDC_PLAIN) ||
	     (LOWORD(wParam) == IDC_HLEFT) ||
	     (LOWORD(wParam) == IDC_HRIGHT) ||
	     (LOWORD(wParam) == IDC_HCENTER) ||
	     (LOWORD(wParam) == IDC_VTOP) ||
	     (LOWORD(wParam) == IDC_VBOTTOM) ||
	     (LOWORD(wParam) == IDC_VCENTER) ||
	     (LOWORD(wParam) == IDC_RANDWORD) ||
	     (LOWORD(wParam) == IDC_5050) )
			{
			g_ConfigThis->forceredraw=1;
			g_ConfigThis->randomword=IsDlgButtonChecked(hwndDlg,IDC_RANDWORD)?1:0;
			g_ConfigThis->enabled=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
			g_ConfigThis->outline=IsDlgButtonChecked(hwndDlg,IDC_OUTLINE)?1:0;
			g_ConfigThis->shadow=IsDlgButtonChecked(hwndDlg,IDC_SHADOW)?1:0;
			g_ConfigThis->onbeat=IsDlgButtonChecked(hwndDlg,IDC_ONBEAT)?1:0;
			g_ConfigThis->blend=IsDlgButtonChecked(hwndDlg,IDC_ADDITIVE)?1:0;
			g_ConfigThis->blendavg=IsDlgButtonChecked(hwndDlg,IDC_5050)?1:0;
			if (g_ConfigThis->onbeat)
				SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETPOS, TRUE, (int)(g_ConfigThis->onbeatSpeed));
			else
				SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETPOS, TRUE, (int)(g_ConfigThis->normSpeed));
			g_ConfigThis->insertBlank=IsDlgButtonChecked(hwndDlg,IDC_BLANKS)?1:0;
			g_ConfigThis->randomPos=IsDlgButtonChecked(hwndDlg,IDC_RANDOMPOS)?1:0;
			g_ConfigThis->valign=IsDlgButtonChecked(hwndDlg,IDC_VTOP)?DT_TOP:g_ConfigThis->valign;
			g_ConfigThis->valign=IsDlgButtonChecked(hwndDlg,IDC_VCENTER)?DT_VCENTER:g_ConfigThis->valign;
			g_ConfigThis->valign=IsDlgButtonChecked(hwndDlg,IDC_VBOTTOM)?DT_BOTTOM:g_ConfigThis->valign;
			g_ConfigThis->halign=IsDlgButtonChecked(hwndDlg,IDC_HLEFT)?DT_LEFT:g_ConfigThis->halign;
			g_ConfigThis->halign=IsDlgButtonChecked(hwndDlg,IDC_HRIGHT)?DT_RIGHT:g_ConfigThis->halign;
			g_ConfigThis->halign=IsDlgButtonChecked(hwndDlg,IDC_HCENTER)?DT_CENTER:g_ConfigThis->halign;
			EnableWindow(GetDlgItem(hwndDlg, IDC_VTOP), g_ConfigThis->randomPos ? FALSE : TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VBOTTOM), g_ConfigThis->randomPos ? FALSE : TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VCENTER), g_ConfigThis->randomPos ? FALSE : TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HLEFT), g_ConfigThis->randomPos ? FALSE : TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HRIGHT), g_ConfigThis->randomPos ? FALSE : TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HCENTER), g_ConfigThis->randomPos ? FALSE : TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HSHIFT), g_ConfigThis->randomPos ? FALSE : TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VSHIFT), g_ConfigThis->randomPos ? FALSE : TRUE);
			if ((LOWORD(wParam) == IDC_ONBEAT) && g_ConfigThis->onbeat)
				g_ConfigThis->nb=g_ConfigThis->onbeatSpeed;
			if ((LOWORD(wParam) == IDC_RANDOMPOS) && g_ConfigThis->randomPos)
				g_ConfigThis->forcealign=1;
			}
      if (LOWORD(wParam) == IDC_DEFCOL ||LOWORD(wParam) == IDC_DEFOUTCOL) // handle clicks to nifty color button
      {
      int *a=&(LOWORD(wParam) == IDC_DEFCOL ? g_ConfigThis->color : g_ConfigThis->outlinecolor);
      static COLORREF custcolors[16];
      CHOOSECOLOR cs;
      cs.lStructSize = sizeof(cs);
      cs.hwndOwner = hwndDlg;
      cs.hInstance = 0;
      cs.rgbResult=((*a>>16)&0xff)|(*a&0xff00)|((*a<<16)&0xff0000);
      cs.lpCustColors = custcolors;
      cs.Flags = CC_RGBINIT|CC_FULLOPEN;
      if (ChooseColor(&cs))
	        {
		        *a = ((cs.rgbResult>>16)&0xff)|(cs.rgbResult&0xff00)|((cs.rgbResult<<16)&0xff0000);
	        	if (LOWORD(wParam) == IDC_DEFCOL)
					g_ConfigThis->color = *a;
				else
					g_ConfigThis->outlinecolor = *a;
			}
      InvalidateRect(GetDlgItem(hwndDlg,LOWORD(wParam)),NULL,TRUE);            
	  g_ConfigThis->updating=true;
	  SetTextColor(g_ConfigThis->hBitmapDC, ((g_ConfigThis->color&0xFF0000) >> 16) | (g_ConfigThis->color& 0xFF00) | (g_ConfigThis->color&0xFF)<<16);
      g_ConfigThis->forceredraw=1;
	  g_ConfigThis->updating=false;
      }
	}
return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_ConfigThis = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_TEXT,hwndParent,g_DlgProc);
}



// export stuff

C_RBASE *R_Text(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}



#else
C_RBASE *R_Text(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{ return NULL; }
#endif