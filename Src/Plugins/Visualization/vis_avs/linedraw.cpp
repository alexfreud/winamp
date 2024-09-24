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
#include "r_defs.h"
#include <math.h>

#define SWAP(x,y,temp) ( temp ) = ( x ); ( x ) = ( y ); ( y ) = ( temp )
#define ABS(x) (( x ) < 0 ? - ( x ) : ( x ))

void line(int *fb, int x1,int y1,int x2,int y2, int width, int height, int color, int lw) 
{
  int dy = ABS(y2-y1); 
  int dx = ABS(x2-x1);

#ifdef LASER
  lw=1;
  #define BLEND_LINE(fb,color) (*(fb)=BLEND(*(fb),(color)))
#else
  if (lw<1) lw=1;
  else if (lw>255)lw=255;
#endif

  int lw2=lw/2;
  if (!dx) // optimize vertical draw
  {
    x1-=lw2;
    if (x1+lw >= 0 && x1 < width) 
    {
      int d=max(min(y1,y2),0);
	    int ye=min(max(y1,y2),height-1);
      if (x1<0)
      {
        lw+=x1;
        x1=0;
      }
      if (x1+lw >= width) lw=width-x1;
	    fb += d*width+x1;
      width-=lw;
	    if (lw>0) while (d++ < ye)
      {
        int x=lw;
        while (x--) { BLEND_LINE(fb, color); fb++; }
		    fb+=width;
	    }
    }
	  return;
  }
  if (y1==y2) // optimize horizontal draw.
  {
    y1-=lw2;
    if (y1+lw >= 0 && y1 < height)
    {
      int d=max(min(x1,x2),0);
      int xe=min(max(x1,x2),width-1);
      if (y1<0)
      {
        lw+=y1;
       
        
        y1=0;
      }
      if (y1+lw >= height) lw=height-y1;
      fb+=y1*width+d;
      width-=xe-d;
      int y=lw;
      while (y--)
      {
        int lt=d;
        while (lt++<xe) { BLEND_LINE(fb,color); fb++; }
        fb+=width;
      }
    }
    return;
  }

  if (dy <= dx)  // x major, low slope
  {
    // first things first for better line drawing, let's see if we can't get the 
    // width calculated right.
    // lw is the width in pixels. if dy = 0, then lw=lw. if dy=dx, then:
    /*


        __________ C=30d
       |\
       | \ lw
       |  \_____ A=90d
    ?  |  /       
       | /   pc2
       |/ ____ B = 60d

       tan(C) = sin(C)/cos(C) = (dy / dx)
       cos(C) = lw / ?

        sin(C)/(lw / ? ) = (dy / dx)
        sin(C) * ? / lw = (dy/dx)
        sin(C) * ?  = lw * dy/dx;
        ? = lw * dy/dx / sin(C)


        cos(C) = lw / ?  === ? = lw / cos(C)
        tan(C) = dy / dx;

        C = atan2(dy,dx);
        ? = lw / cos(C)

    */

#if 0// lame
    if (lw>1 && (GetAsyncKeyState(VK_SHIFT)&0x8000)){
      double d=atan2((double)dy,(double)dx);
      lw = (int) (lw / cos(d));
      if (lw<1)lw=1;
    }
#endif


    if (x2 < x1) 
    {
      int temp;
      SWAP(x1,x2,temp);
      SWAP(y1,y2,temp);
    }

    int yincr = y2>y1?1:-1;
    int offsincr= y2>y1?width:-width;
    y1-=lw2;
    int offs = y1 * width + x1;
    int d = dy + dy - dx;
    int Eincr = dy + dy;
    int NEincr = d - dx;
    if (x2 >= 0 && x1 < width) 
    {
      if (x1<0)
      {
        int v;
        v=yincr * -x1;
        if (dx) v= (v * dy)/dx;

        y1 += v;
        offs += v*width - x1;
        x1=0;
      }

      if (x2 > width) x2=width;
      while (x1<x2)
      {
        int yp=y1;
        int ype=y1+lw;
        int *newfb=fb+offs;
        if (yp < 0) 
        {
          newfb-=yp*width;
          yp=0;
        }
        if (ype>height) ype=height;
        while (yp++ < ype)
        {
          BLEND_LINE(newfb,color);
          newfb+=width;
        }
        if (d < 0) d += Eincr;
        else 
        {
          d += NEincr;
          y1 += yincr;
          offs += offsincr;
        }
        offs++;
        x1++;
      }
    }
  } 
  else
  {
#if 0//lame
    if (lw>1 && (GetAsyncKeyState(VK_SHIFT)&0x8000)){
      double d=atan2((double)dx,(double)dy);
      lw = (int) (lw / cos(d));
      if (lw<1)lw=1;
    }
#endif
    if (y2 < y1) 
    {
      int temp;
      SWAP(x1,x2,temp);
      SWAP(y1,y2,temp);
    }

    int yincr=(x2>x1)?1:-1;
    int d = dx + dx - dy;
    int Eincr = dx + dx;
    int NEincr = d - dy;
    x1-=lw2;
    int offs = y1 * width + x1;
    if (y2 >= 0 && y1 < height) 
    {
      if (y1<0)
      {
        int v;
        v=yincr * -y1;
        if (dy) v= (v * dx)/dy;

        x1 += v;
        offs += v - y1*width;
        y1=0;
      }
      if (y2 > height) y2 = height;
      while (y1 < y2)
      {
        int xp=x1;
        int xpe=x1+lw;
        int *newfb=fb+offs;
        if (xp<0)
        {
          newfb-=xp;
          xp=0;
        }
        if (xpe > width) xpe=width;
        while (xp++ < xpe) { BLEND_LINE(newfb, color); newfb++; }

        if (d < 0) d += Eincr;
        else 
        {
          d += NEincr;
          x1 += yincr;
          offs += yincr;
        }
        offs += width;
        y1++;
      }
    }
  }
}
