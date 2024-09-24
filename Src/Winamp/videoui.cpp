#include "Main.h"
#include "video.h"
#include "resource.h"



#define inreg(x,y,x2,y2) \
        ((mouse_x <= ( x2 ) && mouse_x >= ( x ) &&  \
        mouse_y <= ( y2 ) && mouse_y >= ( y )))

static int mouse_x, mouse_y, mouse_type, mouse_stats;

static int which_cap=0;
enum { NO_CAP,TITLE_CAP,TB_CAP, SZ_CAP,VW_CAP};

static void do_titlebar();
static void do_titlebuttons();
static void do_size();
static void do_vw();

void videoui_handlemouseevent(int x, int y, int type, int stats) 
{
  mouse_x = x;
  mouse_y = y;
  mouse_type = type;
  mouse_stats = stats;
  switch (which_cap)
  {
	case VW_CAP: do_vw(); return;
	case TITLE_CAP:	do_titlebar();return;
	case TB_CAP:	do_titlebuttons();return;
	case SZ_CAP:	do_size();	return;
	default: break;
  }
  do_vw();
  do_titlebuttons();
  do_size();
  do_titlebar();
}

static void do_titlebar() 
{
	if (which_cap == TITLE_CAP || (!which_cap && (config_easymove || mouse_y < 14)))
	{
		static int clickx, clicky;
		switch (mouse_type)
		{
			case 1:
				{
					which_cap=TITLE_CAP;
					clickx=mouse_x;
					clicky=mouse_y;
				}
			break;
			case -1:
				which_cap=0;
			break;
			case 0:
				if (which_cap == TITLE_CAP && mouse_stats & MK_LBUTTON)
				{
					POINT p = { mouse_x, mouse_y};
					ClientToScreen(hVideoWindow,&p);
					p.x-=clickx;
					p.y-=clicky;
					SendMessageW(hVideoWindow,WM_USER+0x100,1,(LPARAM)&p);
				}
			break;
		}
	}
}

static void do_titlebuttons() 
{
	int w=0;
	w=inreg(config_video_width-10,3,config_video_width-1,3+9)?1:0;

	if (w) // kill button
	{
		if (mouse_type == -1 && which_cap == TB_CAP) 
		{
			which_cap=0;
			draw_vw_tbutton(0);
			SendMessageW(hMainWindow,WM_COMMAND,WINAMP_OPTIONS_VIDEO,0);
		}
		else if (mouse_stats & MK_LBUTTON)
		{
			which_cap=TB_CAP;
			draw_vw_tbutton(w?1:0);
		}
	}
	else if (which_cap == TB_CAP) 
	{
		which_cap=0;
		draw_vw_tbutton(0);
	}

}

static void do_vw() 
{
  HWND videoGetHwnd();
	int w=0;
	w=inreg(9,config_video_height-29,89,config_video_height-11)?1:0;

	if (w)
	{
		w=(mouse_x-9)/15;
		if (mouse_type == -1 && which_cap == VW_CAP) 
		{
			which_cap=0;
			draw_vw_mbuts(-1);
			switch (w)
			{
				case 0: 
          videoGoFullscreen();
        break;
				case 1: 
          if (videoGetHwnd()) SendMessageW(videoGetHwnd(),WM_COMMAND,ID_VIDEOWND_ZOOM100,0);
        break;
				case 2: 
          if (videoGetHwnd()) SendMessageW(videoGetHwnd(),WM_COMMAND,ID_VIDEOWND_ZOOM200,0);
        break;
				case 3: 
          SendMessageW(hMainWindow,WM_COMMAND,WINAMP_VIDEO_TVBUTTON,0);
        break;
				case 4: 
          // menu
          if (videoGetHwnd()) SendMessageW(videoGetHwnd(),WM_RBUTTONUP,0,0);
        break;
			}
		}
		else if (mouse_stats & MK_LBUTTON)
		{
			which_cap=VW_CAP;
			draw_vw_mbuts(w);
		}
	}
	else if (which_cap == VW_CAP) 
	{
		which_cap=0;
		draw_vw_mbuts(-1);
	}
}


static void do_size()
{
	if (which_cap == SZ_CAP || (!which_cap && 
		mouse_x > config_video_width-20 && mouse_y > config_video_height-20 && 
		((config_video_width-mouse_x + config_video_height-mouse_y) <= 30)))
	{
		static int dx,dy;
		if (!which_cap && mouse_type == 1)
		{
			dx=config_video_width-mouse_x;
			dy=config_video_height-mouse_y;
			which_cap=SZ_CAP;
		}
		if (which_cap == SZ_CAP)
		{
			int x,y;
			if (mouse_type == -1)
			{
				which_cap=0;
			}
			x=mouse_x + dx;
			y=mouse_y + dy;
			int old_x = x;
			int old_y = y;
				if (config_video_width != x || config_video_height != y) // don't bother resizing if we're at the current size already.
			{
			if (x >= GetSystemMetrics(SM_CXSCREEN)) x = GetSystemMetrics(SM_CXSCREEN)-24;
			if (y >= GetSystemMetrics(SM_CYSCREEN)) y = GetSystemMetrics(SM_CYSCREEN)-28;


      if (!config_embedwnd_freesize)
      {
  			x += 24;
	  		x -= x%25;
		  	y += 28;
			  y -= y%29;
      }
			if (x < 275) x = 275;
			if (y < 20+38+29+29) y = 20+38+29+29;
			//config_video_width = x;
			//config_video_height= y;
			
		if (!((old_x < config_video_width && x > config_video_width)
			 || (old_y < config_video_height && y > config_video_height))) // don't snap out a size if we're moving the mouse inward (video might not have started snapped)
		{

			SetExteriorSize(x,y);
			//SetWindowPos(hVideoWindow,0,0,0,x,y,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
			{
				HDC hdc=GetWindowDC(hVideoWindow);
				draw_vw(hdc);
				ReleaseDC(hVideoWindow,hdc);
			}
			}
			}
		}
	}
}
