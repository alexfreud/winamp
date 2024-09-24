/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "draw.h"

HBITMAP embedBM;

int titlebar_font_widths[26] = {0};
int titlebar_font_offsets[26] = {0};
int titlebar_font_num_widths[12] = {0};
int titlebar_font_num_offsets[12] = {0};
int titlebar_font_unknown_width = 5;

static int calcTBFontTextWidth(char *text)
{
	int w=0;
	while (text && *text)
	{
		char c=*text++;
		if (c >= 'a' && c <= 'z') c+='A'-'a';

		if (c >= 'A' && c <= 'Z' && titlebar_font_widths[c-'A']) 
			w+=titlebar_font_widths[c-'A'];
		else if (c >= '0' && c <= '9' && titlebar_font_widths[c-'0'] && Skin_UseGenNums) 
			w+=titlebar_font_num_widths[c-'0'];
		else if (c == '-' && titlebar_font_widths[10] && Skin_UseGenNums) 
			w+=titlebar_font_num_widths[10];
		else if (c == ':' && titlebar_font_widths[11] && Skin_UseGenNums) 
			w+=titlebar_font_num_widths[11];
		else w+=titlebar_font_unknown_width;
	}
	return w;
}

static void drawTBText(HDC hdcout,int xp, int yp, char *buf, int maxw, int sel)
{
	while (buf && *buf)
	{
		char c=*buf++;
		int w=titlebar_font_unknown_width;
		if (c >= 'a' && c <= 'z') c+='A'-'a';

		if (c >= 'A' && c <= 'Z' && titlebar_font_widths[c-'A']) 
		{
			w=titlebar_font_widths[c-'A'];
			if (w > maxw) break;
			BitBlt(hdcout,xp,yp,w,7,bmDC,titlebar_font_offsets[c-'A'],88+(sel?8:0),SRCCOPY);
		}
		else if (c >= '0' && c <= '9' && titlebar_font_num_widths[c-'0'] && Skin_UseGenNums)
		{
			w=titlebar_font_num_widths[c-'0'];
			if (w > maxw) break;
			BitBlt(hdcout,xp,yp,w,7,bmDC,titlebar_font_num_offsets[c-'0'],72+(sel?8:0),SRCCOPY);
		}
		else if (c == '-' && titlebar_font_num_widths[10] && Skin_UseGenNums)
		{
			w=titlebar_font_num_widths[10];
			if (w > maxw) break;
			BitBlt(hdcout,xp,yp,w,7,bmDC,titlebar_font_num_offsets[10],72+(sel?8:0),SRCCOPY);
		}
		else if (c == ':' && titlebar_font_num_widths[11] && Skin_UseGenNums)
		{
			w=titlebar_font_num_widths[11];
			if (w > maxw) break;
			BitBlt(hdcout,xp,yp,w,7,bmDC,titlebar_font_num_offsets[11],72+(sel?8:0),SRCCOPY);
		}
		xp+=w;
		maxw-=w;
	}
}

void draw_embed_tbar(HWND hwnd, int state, int w)
{
	if (!disable_skin_borders)
	{
		HDC hdcout=GetWindowDC(hwnd);
		char buf[32] = {0};
		state = state?0:21;
		do_palmode(hdcout);
		setSrcBM(embedBM);
		GetWindowTextA(hwnd,buf,sizeof(buf)/sizeof(char)-1);
		buf[31]=0;
		{
			int textw_exact=calcTBFontTextWidth(buf);
			int nt;
			int xp=0;

			int textw=textw_exact + 24;
			textw -= textw % 25;

			if (textw > w-100) textw=w-100;

			BitBlt(hdcout,xp,0,25,20,bmDC,0,state,SRCCOPY);
			xp+=25;
			nt = (w - 100 - textw)/25;
			if (nt)
			{
				if (nt&1)
				{
					BitBlt(hdcout,xp,0,12,20,bmDC,104,state,SRCCOPY);
					xp+=12;
				}
				nt/=2;
				while (nt-->0)
				{
					BitBlt(hdcout,xp,0,25,20,bmDC,104,state,SRCCOPY);
					xp+=25;
				}
			}

			BitBlt(hdcout,xp,0,25,20,bmDC,26,state,SRCCOPY);
			xp+=25;

			nt = textw/25;
			if (nt)
			{
				int xstart=xp + (textw - textw_exact)/2;
				if (textw != textw_exact) xstart++;
				while (nt-->0)
				{
					BitBlt(hdcout,xp,0,25,20,bmDC,52,state,SRCCOPY);
					xp+=25;
				}
				drawTBText(hdcout,xstart,4,buf,textw,state);
			}

			BitBlt(hdcout,xp,0,25,20,bmDC,78,state,SRCCOPY);
			xp+=25;

			nt = (w - 100 - textw)/25;
			if (nt)
			{
				if (nt&1)
				{
					BitBlt(hdcout,xp,0,13,20,bmDC,104,state,SRCCOPY);
					xp+=13;
				}
				nt/=2;
				while (nt-->0)
				{
					BitBlt(hdcout,xp,0,25,20,bmDC,104,state,SRCCOPY);
					xp+=25;
				}
			}
			nt = (w - 100 - textw) % 25;
			if (nt>0)
			{
				StretchBlt(hdcout,xp,0,nt,20,bmDC,104,state,25,20,SRCCOPY);
				xp+=nt;
			}
			BitBlt(hdcout,xp,0,25,20,bmDC,130,state,SRCCOPY);
		}
		unsetSrcBM();
		ReleaseDC(hwnd,hdcout);
	}
}

void draw_embed(HWND hwnd, HDC hdcout, int w, int h, int flags)
{
	if (!disable_skin_borders)
	{
		do_palmode(hdcout);
		{ 
			// fg>this is here in case a child temporarily unparents itself from the embedwnd, like avs when it docks to its editor.
			// when the child is there, fillrect is clipped
			RECT r={11,20,w-8, h-14};
			FillRect(hdcout, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
		}
		draw_embed_tbar(hwnd,GetForegroundWindow()==hwnd?1:(config_hilite?0:1),w);
		setSrcBM(embedBM);
		{
			int y=(h-20-38)/29;
			int yp=20,x,xp;
			while (y-->0)
			{
				BitBlt(hdcout,0,yp,11,29,bmDC,127,42,SRCCOPY);
				BitBlt(hdcout,w-8,yp,8,29,bmDC,139,42,SRCCOPY);
				yp += 29;
			}
			y=(h-20-38)%29;
			if (y)
			{
				StretchBlt(hdcout,0,yp,11,y,bmDC,127,42,11,29,SRCCOPY);
				StretchBlt(hdcout,w-8,yp,8,y,bmDC,139,42,8,29,SRCCOPY);
				yp += y;
			}

			// 24 pixel lamity
			BitBlt(hdcout,0,yp,11,24,bmDC,158,42,SRCCOPY);
			BitBlt(hdcout,w-8,yp,8,24,bmDC,170,42,SRCCOPY);
			yp += 24;

			BitBlt(hdcout,0,yp,125,14,bmDC,0,42,SRCCOPY);
			x=(w-125-125)/25;
			xp=125;
			while (x-->0)
			{
				BitBlt(hdcout,xp,yp,25,14,bmDC,127,72,SRCCOPY);
				xp+=25;
			}
			x=(w-125-125)%25;
			if (x)
			{
				StretchBlt(hdcout,xp,yp,x,14,bmDC,127,72,25,14,SRCCOPY);
				xp+=x;
			}
			BitBlt(hdcout,xp,yp,125,14,bmDC,0,57,SRCCOPY);
			if (flags & EMBED_FLAGS_NORESIZE)
			{
				BitBlt(hdcout,xp+112,yp+2,7,7,bmDC,118,72,SRCCOPY);
			}
		}
		unsetSrcBM();
	}
}

void draw_paint_emb(HWND hwnd, int w, int h, int flags)
{
	PAINTSTRUCT ps;
	draw_embed(hwnd,BeginPaint(hwnd,&ps),w,h,flags);
	EndPaint(hwnd,&ps);
}

void draw_embed_tbutton(HWND hwnd, int b3, int w)
{
	if (!disable_skin_borders)
	{
		HDC hdcout=GetWindowDC(hwnd);
		do_palmode(hdcout);
		setSrcBM(embedBM);
		if (!b3)
			BitBlt(hdcout,w-11,3,9,9,bmDC,144,3,SRCCOPY);
		else
			BitBlt(hdcout,w-11,3,9,9,bmDC,148,42,SRCCOPY);
		unsetSrcBM();
		ReleaseDC(hwnd,hdcout);
	}
}