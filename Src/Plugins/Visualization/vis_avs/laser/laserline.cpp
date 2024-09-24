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
#include <math.h>
#include "../r_defs.h"
extern "C" {
#include "ld32.h"
};


static int fix(int a)
{
  return ((a&0xff0000)>>16)|(a&0xff00)|((a&0xff)<<16);
}

static int dist(int x1, int y1, int x2, int y2)
{
  x1-=x2;
  y1-=y2;
  return x1*x1+y1*y1;
}

static double actdist(double x1, double y1, double x2, double y2)
{
  x1-=x2;
  y1-=y2;
  return sqrt(x1*x1+y1*y1);
}

static __inline int getval(int x1, int y1, int x2, int y2, int xv)
{
  if (x1==x2) return y1;
  return y1+MulDiv(xv-x1,y2-y1,x2-x1);
}

static void doclip(int &x1, int &y1, int x2, int y2)
{
  if (x1 > 8000 && x2 <= 8000)
  {
    y1=getval(x1,y1,x2,y2,8000);
    x1=8000;
  }
  if (x1 < -8000 && x2 >= -8000)
  {
    y1=getval(x1,y1,x2,y2,-8000);
    x1=-8000;
  }
}

void LineDrawList(C_LineListBase *list, int *fb, int w, int h)
{
  LineType *ll;

  static struct
  {
  FRAMESTRUCTEX frame;
  PTSTRUCT points[32768];
  } d;
  memset(&d.frame,0,sizeof(d.frame));
  int cp=0;
  int lastendx=-10000,lastendy=-10000;
  
  int w2=w/2;
  int h2=h/2;
  int numl=list->GetUsedLines();
  ll=list->GetLineList();
  while (numl-->0 && cp < sizeof(d.points)/sizeof(PTSTRUCT)-2)
  {
    int x1,y1,x2,y2;
    // draw to screen
    {
      x1=(int) (ll->x1 * w2) + w2;
      x2=(int) (ll->x2 * w2) + w2;
      y1=(int) (ll->y1 * h2) + h2;
      y2=(int) (ll->y2 * h2) + h2;
      if (ll->mode==0)
      {
        line(fb,x1,y1,x2,y2,w,h,ll->color);
      }
      else 
      {
        if (x1 >= 0 && x1 < w && y1 >= 0 && y1 < h)
        {
          int o=x1+y1*w;
          fb[o]=BLEND(fb[o],ll->color);
        }
      }
    }
    
    x1=(int) (ll->x1*8000.0);
    x2=(int) (ll->x2*8000.0);

    y1=(int) (ll->y1*-8000.0);
    y2=(int) (ll->y2*-8000.0);


    if (ll->mode==0)
    {
      doclip(x1,y1,x2,y2);
      doclip(x2,y2,x1,y1);
      doclip(y1,x1,y2,x2);
      doclip(y2,x2,y1,x1);

      if (x1 >= -8000 && x1 <= 8000 && y1 >= -8000 && y1 <= 8000 &&
          x2 >= -8000 && x2 <= 8000 && y2 >= -8000 && y2 <= 8000)
      {
        // x1, y1 is the new, far point
        if (cp && dist(x1,y1,lastendx,lastendy) < dist(x2,y2,lastendx,lastendy))
        {
          int t;
          t=x1; x1=x2; x2=t;
          t=y1; y1=y2; y2=t;
        }

        // if new start point is too far away, blank to that point
        if (dist(x2,y2,lastendx,lastendy) > 400*400 || !cp)
        {
          memset(&d.points[cp],0,sizeof(d.points[0]));
          if (cp)
          {
            d.points[cp-1].Status=4096;
          }
          d.points[cp].Status=0;
          d.points[cp].XCoord=x2;
          d.points[cp].YCoord=y2;
          cp++;
        }
        else if (cp>1 && d.points[cp-1].RGBValue)
        {
          double a1=atan2(d.points[cp-2].XCoord-x2,d.points[cp-2].YCoord-y2);
          double a2=atan2(x2-x1,y2-y1);
          if (fabs(a1-a2) >= 1.0*3.14159/180.0)
          {
            d.points[cp-1].Status=4096;
          }

        }
        lastendx=x1;
        lastendy=y1;
        memset(&d.points[cp],0,sizeof(d.points[0]));
        d.points[cp].RGBValue=fix(ll->color);
        d.points[cp].Status=0;
        d.points[cp].XCoord=x1;
        d.points[cp].YCoord=y1;
        cp++;
      }
    }
    else
    {
      if (x1 > -8000 && x1 < 8000 && y1 > -8000 && y1 < 8000)
      {
        if (dist(x1,y1,lastendx,lastendy) > 30*30)
        {
          memset(&d.points[cp],0,sizeof(d.points[0]));
          d.points[cp].Status=0;
          d.points[cp].XCoord=x1;
          d.points[cp].YCoord=y1;
          cp++;
        }
        memset(&d.points[cp],0,2*sizeof(d.points[0]));
        d.points[cp].RGBValue=fix(ll->color);
        d.points[cp].Status=4096;
        d.points[cp].XCoord=x1;
        d.points[cp].YCoord=y1;
        cp++;
        d.points[cp].Status=4096;
        d.points[cp].XCoord=x1;
        d.points[cp].YCoord=y1;
        cp++;
        lastendx=x1;
        lastendy=y1;
      }
    }
    
    ll++;
  }
  if (cp)
  {
    d.points[cp-1].Status=4096;
  }

  memset(&d.frame,0,sizeof(d.frame));
  d.frame.VectorFlag=1;
  d.frame.NumPoints=max(cp,1);
  d.frame.ScanRate=100;
  memcpy(d.frame.FrameNote,"AVS/Laser Frame        ",24);

  laser_sendframe(&d,sizeof(d.frame)+sizeof(PTSTRUCT)*max(cp,16));
}
#endif