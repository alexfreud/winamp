/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "draw.h"
#include "resource.h"
#include "WADrawDC.h"

HBITMAP vwMainBM;
extern "C" int vw_init=0;



void draw_vw_init()
{
	if (vw_init)
		draw_vw_kill();
	vw_init=1;
	vwMainBM = draw_LBitmap(MAKEINTRESOURCE(IDB_VIDEO),L"video.bmp");
}

void draw_vw_kill()
{
	if (!vw_init)
		return;
	DeleteObject(vwMainBM);	
	
}

void draw_vw_mbuts(int whichb)
{
  if (!disable_skin_borders && hVideoWindow && vw_init)
  {
	  int numb=5;
	  int x;
		WADrawDC hdcout(hVideoWindow);

    do_palmode(hdcout);
	  setSrcBM(vwMainBM);
	  for (x = 0; x < numb; x ++)
	  {
		  int nx=9+x*15;
		  int ny=51;
		  if (x==whichb) 
		  {
			  nx+=158-9;
			  ny=42;
		  }
		  BitBlt(hdcout,9+x*15,config_video_height-29,15,18,bmDC,nx,ny,SRCCOPY);
	  }
	  unsetSrcBM();
  }
}

void draw_vw_tbar(int state)
{
  if (!disable_skin_borders  && hVideoWindow && vw_init)
  {
		WADrawDC hdcout(hVideoWindow);
	  state = state?0:21;
      do_palmode(hdcout);
	  setSrcBM(vwMainBM);
	  {
		  int nt;
		  int xp=0;
		  BitBlt(hdcout,xp,0,25,20,bmDC,0,state,SRCCOPY);
		  xp+=25;
		  nt = (config_video_width - 25 - 25 - 100)/25;
		  if (nt)
		  {
			  if (nt&1)
			  {
				  BitBlt(hdcout,xp,0,12,20,bmDC,127,state,SRCCOPY);
				  xp+=12;
			  }
			  nt/=2;
			  while (nt-->0)
			  {
				  BitBlt(hdcout,xp,0,25,20,bmDC,127,state,SRCCOPY);
				  xp+=25;
			  }
		  }
		  BitBlt(hdcout,xp,0,100,20,bmDC,26,state,SRCCOPY);
		  xp+=100;
		  nt = (config_video_width - 25 - 25 - 100)/25;
		  if (nt)
		  {
			  if (nt&1)
			  {
				  BitBlt(hdcout,xp,0,13,20,bmDC,127,state,SRCCOPY);
				  xp+=13;
			  }
			  nt/=2;
			  while (nt-->0)
			  {
				  BitBlt(hdcout,xp,0,25,20,bmDC,127,state,SRCCOPY);
				  xp+=25;
			  }
		  }
      nt = (config_video_width - 25 -25 - 100) %25;
      if (nt)
      {
        StretchBlt(hdcout,xp,0,nt,20,bmDC,127,state,25,20,SRCCOPY);
        xp+=nt;
      }
		  BitBlt(hdcout,xp,0,25,20,bmDC,153,state,SRCCOPY);
	  }
	  unsetSrcBM();
  }
}

void draw_vw(HDC hdcout)
{
	 if (!hVideoWindow  || !vw_init)
		 return;
  do_palmode(hdcout);
	draw_vw_tbar(GetForegroundWindow()==hVideoWindow?1:(config_hilite?0:1));
  if (!disable_skin_borders)
  {
	  setSrcBM(vwMainBM);
	  {
		  int y=(config_video_height-20-38)/29;
		  int yp=20,x,xp;
		  while (y-->0)
		  {
			  BitBlt(hdcout,0,yp,11,29,bmDC,127,42,SRCCOPY);
			  BitBlt(hdcout,config_video_width-8,yp,8,29,bmDC,139,42,SRCCOPY);
			  yp += 29;
		  }
      y=(config_video_height-20-38)%29;
      if (y)
      {
			  StretchBlt(hdcout,0,yp,11,y,bmDC,127,42,11,29,SRCCOPY);
			  StretchBlt(hdcout,config_video_width-8,yp,8,y,bmDC,139,42,8,29,SRCCOPY);
			  yp += y;
      }
		  BitBlt(hdcout,0,yp,125,38,bmDC,0,42,SRCCOPY);
		  x=(config_video_width-125-125)/25;
		  xp=125;
		  while (x-->0)
		  {
			  BitBlt(hdcout,xp,yp,25,38,bmDC,127,81,SRCCOPY);
			  xp+=25;
		  }
      x=(config_video_width-125-125)%25;
      if (x)
      {
			  StretchBlt(hdcout,xp,yp,x,38,bmDC,127,81,25,38,SRCCOPY);
			  xp+=x;
      }
		  BitBlt(hdcout,xp,yp,125,38,bmDC,0,81,SRCCOPY);
		  draw_vw_info(NULL,0);
	  }
	  unsetSrcBM();
  }
}

wchar_t draw_vw_info_lastb[512] = {0};
void draw_vw_info(wchar_t *t, int erase)
{
	if (!disable_skin_borders && hVideoWindow&&vw_init)
	{
		WADrawDC hdcout(hVideoWindow);
		if (!t) t=draw_vw_info_lastb;
		else lstrcpynW(draw_vw_info_lastb,t,sizeof(draw_vw_info_lastb));

		if (erase)
		{
			//int y=(config_video_height-20-38)/29;
			int yp=config_video_height-38,x,xp;
			do_palmode(hdcout);
			setSrcBM(vwMainBM);
			BitBlt(hdcout,0,yp,125,38,bmDC,0,42,SRCCOPY);
			x=(config_video_width-125-125)/25;
			xp=125;
			while (x-->0)
			{
				BitBlt(hdcout,xp,yp,25,38,bmDC,127,81,SRCCOPY);
				xp+=25;
			}
			x=(config_video_width-125-125)%25;
			if (x)
			{
				StretchBlt(hdcout,xp,yp,x,38,bmDC,127,81,25,38,SRCCOPY);
				xp+=x;
			}
			BitBlt(hdcout,xp,yp,125,38,bmDC,0,81,SRCCOPY);
			unsetSrcBM();
		}

		RECT r={92,config_video_height-27,config_video_width-25,config_video_height-13};
		HGDIOBJ oldFont=SelectObject(hdcout,mfont);
		SetTextColor(hdcout,Skin_PLColors[4]);
		SetBkColor(hdcout,Skin_PLColors[5]);
		DrawTextW(hdcout,t,-1,&r,DT_SINGLELINE|DT_LEFT|DT_NOPREFIX);
		SelectObject(hdcout,oldFont);
	}
}


void draw_paint_vw(HWND hwnd)
{
	if (!hVideoWindow || !vw_init)
		 return;
	PAINTSTRUCT ps;
	RECT r;
	EnterCriticalSection(&g_mainwndcs);
	draw_vw(BeginPaint(hwnd,&ps));
	r=ps.rcPaint;
	EndPaint(hwnd,&ps);
	LeaveCriticalSection(&g_mainwndcs);
}

void draw_vw_tbutton(int b3)
{
	if (!disable_skin_borders  && hVideoWindow && vw_init)
	{
		WADrawDC hdcout(hVideoWindow);
		do_palmode(hdcout);
		setSrcBM(vwMainBM);
		if (!b3)
			BitBlt(hdcout,config_video_width-11,3,9,9,bmDC,167,3,SRCCOPY);
		else
			BitBlt(hdcout,config_video_width-11,3,9,9,bmDC,148,42,SRCCOPY);
		unsetSrcBM();
	}
}