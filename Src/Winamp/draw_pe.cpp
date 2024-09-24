/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "draw.h"
#include "api.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "resource.h"
#include <bfc/platform/minmax.h>
#include "WADrawDC.h"

HBITMAP  plMainBM=0;
extern "C" int pe_init = 0;
wchar_t playlistStr[19] = {0};
int (WINAPI *jtf_drawtext)(HDC, LPCWSTR, int, LPRECT, UINT) = 0;

void draw_pe_init()
{
	EnterCriticalSection(&g_srcdccs);
	if (pe_init)
		draw_pe_kill();
	pe_init=1;
	if (!plMainBM)
		plMainBM = draw_LBitmap(MAKEINTRESOURCE(IDB_PLEDIT), L"pledit.bmp");
	LeaveCriticalSection(&g_srcdccs);
}

void draw_pe_kill()
{
	if (!pe_init)
		return;
	EnterCriticalSection(&g_srcdccs);
	DeleteObject(plMainBM);	
	plMainBM=0;
	pe_init=0;
	LeaveCriticalSection(&g_srcdccs);
}

void draw_set_plbm(HBITMAP bm)
{
	EnterCriticalSection(&g_srcdccs);
	if (plMainBM)
		DeleteObject(plMainBM);
	plMainBM=bm;
	LeaveCriticalSection(&g_srcdccs);
}

void draw_pe_iobut(int which) // -1 = none, 0 = load, 1=save, 2=clear
{
	if (!pe_init || disable_skin_borders) // if we're not init'd (shouldn't happen) or gen_ff is active
		return;

	int offs=config_pe_width-44;

	WADrawDC hdcout(hPLWindow);
	do_palmode(hdcout);

	setSrcBM(plMainBM);

	BitBlt(hdcout, offs-3, config_pe_height-30-18*2, 3,  18*3, bmDC, 250,                111, SRCCOPY);
	BitBlt(hdcout, offs,   config_pe_height-30-18*2, 22, 18,   bmDC, (which==2)?227:204, 111, SRCCOPY);
	BitBlt(hdcout, offs,   config_pe_height-30-18,   22, 18,   bmDC, (which==1)?227:204, 130, SRCCOPY);
	BitBlt(hdcout, offs,   config_pe_height-30,      22, 18,   bmDC, (which==0)?227:204, 149, SRCCOPY);

	unsetSrcBM();
}

static void draw_pe_infostr(HDC hdcout, wchar_t *str)
{
	StringCchCopyW(playlistStr, 18, str); // use 18 instead of 19 to keep the last byte null terminated
	AutoChar narrowStr(str);
	SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)(char *)narrowStr, IPC_CB_PEINFOTEXT);
	playlistTextFeed->UpdateText(playlistStr, 19);

	if (!pe_init)
		return;
	wchar_t data[19] = {0};
	int x,xp=config_pe_width-143,yp=config_pe_height-28;

	if (disable_skin_borders) return;

	if (lstrlenW(str) < 18)
	{
		StringCchCopyW(data,19, str);
		for (x = lstrlenW(str); x < 18; x ++) data[x]=L' ';
	}
	else memcpy(data,str,18*sizeof(wchar_t));
	do_palmode(hdcout);
	setSrcBM(fontBM);
	for (x = 0; x < 18; x ++)
	{
		int c=0,c2=0;
		getXYfromChar(data[x],&c,&c2);
		BitBlt(hdcout,xp,yp,5,6,bmDC,c,c2,SRCCOPY);
		xp+=5;
	}
	unsetSrcBM();
}

/* if we ever want to use a font rather than the skin bitmap, we can uncomment this 
void draw_pe_infostr_font(char *str)
{
	if (!pe_init)
		return;
	int xp=config_pe_width-145,yp=config_pe_height-30;
	HDC hdcout;

	SendMessageW(hMainWindow, WM_WA_IPC, reinterpret_cast<WPARAM>(str), IPC_CB_PEINFOTEXT);

	if (disable_skin_borders) return;

	hdcout=draw_GetWindowDC(hPLWindow);
	do_palmode(hdcout);
	HFONT oldfont = (HFONT) SelectObject(hdcout,mfont);
	SetTextColor(hdcout,Skin_PLColors[0]);
	SetBkColor(hdcout,Skin_PLColors[2]);

	RECT r={xp, yp, xp+92, yp+10};
	DrawText(hdcout, str, min(18, lstrlen(str)), &r, DT_CENTER| DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	
	SelectObject(hdcout,oldfont);
	draw_ReleaseDC(hPLWindow,hdcout);
}
*/

void draw_pe_miscbut(int which) // -1 = none, 0 = inf, 1 = sort, 2=misc
{
	if (!pe_init || disable_skin_borders) // if we're not init'd (shouldn't happen) or gen_ff is active
		return;

	WADrawDC hdcout(hPLWindow);
	do_palmode(hdcout);
	setSrcBM(plMainBM);

	BitBlt(hdcout, 98,  config_pe_height-30-18*2, 3,  18*3, bmDC, 200,                111, SRCCOPY);
	BitBlt(hdcout, 101, config_pe_height-30-18*2, 22, 18,   bmDC, (which==2)?177:154, 111, SRCCOPY);
	BitBlt(hdcout, 101, config_pe_height-30-18,   22, 18,   bmDC, (which==1)?177:154, 130, SRCCOPY);
	BitBlt(hdcout, 101, config_pe_height-30,      22, 18,   bmDC, (which==0)?177:154, 149, SRCCOPY);

	unsetSrcBM();
}

void draw_pe_selbut(int which) // -1 = none, 0 = all, 1 = none, 2=inv
{
	if (!pe_init)
		return;

	if (disable_skin_borders) return;

	WADrawDC hdcout(hPLWindow);
	do_palmode(hdcout);
	setSrcBM(plMainBM);

	BitBlt(hdcout,69,config_pe_height-30-18*2,3,18*3,bmDC,150,111,SRCCOPY);
	BitBlt(hdcout,72,config_pe_height-30-18*2,22,18,bmDC,(which==2)?127:104,111,SRCCOPY);
	BitBlt(hdcout,72,config_pe_height-30-18,22,18,bmDC,(which==1)?127:104,130,SRCCOPY);
	BitBlt(hdcout,72,config_pe_height-30,22,18,bmDC,(which==0)?127:104,149,SRCCOPY);

	unsetSrcBM();
}

void draw_pe_rembut(int which) // -1 = none, 0 = sel, 1 = crop, 2 = all
{
	if (!pe_init)
		return;

	if (disable_skin_borders) return;


	WADrawDC hdcout(hPLWindow);
	do_palmode(hdcout);
	setSrcBM(plMainBM);

	BitBlt(hdcout,40,config_pe_height-30-18*3,3,18*4,bmDC,100,111,SRCCOPY);
	BitBlt(hdcout,43,config_pe_height-30-18*3,22,18,bmDC,(which==3)?77:54,168,SRCCOPY);
	BitBlt(hdcout,43,config_pe_height-30-18*2,22,18,bmDC,(which==2)?77:54,111,SRCCOPY);
	BitBlt(hdcout,43,config_pe_height-30-18,22,18,bmDC,(which==1)?77:54,130,SRCCOPY);
	BitBlt(hdcout,43,config_pe_height-30,22,18,bmDC,(which==0)?77:54,149,SRCCOPY);

	unsetSrcBM();
}

void draw_pe_addbut(int which) // -1 = none, 0 = file, 1 = dir, 2 = loc
{
	if (!pe_init)
		return;

	if (disable_skin_borders) return;

	WADrawDC hdcout(hPLWindow);
	do_palmode(hdcout);
	setSrcBM(plMainBM);

	BitBlt(hdcout,11,config_pe_height-30-18*2,3,18*3,bmDC,48,111,SRCCOPY);
	BitBlt(hdcout,14,config_pe_height-30-18*2,22,18,bmDC,(which==2)?23:0,111,SRCCOPY);
	BitBlt(hdcout,14,config_pe_height-30-18,22,18,bmDC,(which==1)?23:0,130,SRCCOPY);
	BitBlt(hdcout,14,config_pe_height-30,22,18,bmDC,(which==0)?23:0,149,SRCCOPY);

	unsetSrcBM();
}

void draw_pe_vslide(HWND hwnd, HDC hdc, int pushed, int pos) // pos 0..playlist_getlength()-num_songs
{
	if (!pe_init)
		return;
	int track_h = config_pe_height-20-38;
	int slid_h = 18;
	//int w = 25;
	int yp,y,oy;

	WADrawDC hdcout(hdc, hwnd);
	do_palmode(hdcout);

	setSrcBM(plMainBM);
	{
		int num_songs=(config_pe_height-38-20-2)/pe_fontheight;
		int t=PlayList_getlength()-num_songs;
		if (t < 1) yp = 0;
		else {
			if (pos > t) pos=t;
			yp = ((track_h-slid_h)*pos)/t;
			if (yp < 0) yp = 0;
		}
	}
	for (y = 0; y < yp-28; y += 29)
		BitBlt(hdcout,config_pe_width-15,20+y,8,29,bmDC,36,42,SRCCOPY);
	oy=y+29;
	if (slid_h + yp > oy) oy+=29;
	if (y < yp) BitBlt(hdcout,config_pe_width-15,20+y,8,yp-y,bmDC,36,42,SRCCOPY);
	BitBlt(hdcout,config_pe_width-15,20+yp,8,slid_h,bmDC,pushed?61:52,53,SRCCOPY);
	y=yp+slid_h;
	if (y < oy && y < track_h) BitBlt(hdcout,config_pe_width-15,20+y,8,((y<track_h-29)?oy-y:track_h-y),bmDC,36,42+29-(oy-y),SRCCOPY);
	y = oy;
	for (y = oy; y < track_h; y += 29)
	{
		BitBlt(hdcout,config_pe_width-15,20+y,8,(y<track_h-29)?29:track_h-y,bmDC,36,42,SRCCOPY);
	}

	unsetSrcBM();
}

static void draw_pe_song(HDC hdcout, int pos, wchar_t *name, int time, int sel)
{
	if (!pe_init)
		return;

	int ypos, left, right, num_songs, fontHeight;
	HFONT useFont;
	COLORREF foreground, background;
	HBRUSH useBrush;
	if (pos < 0) 
	{
		//	if (config_pe_height != 14) return;
		right = config_pe_width-29;
		ypos = config_bifont?4:2;
		left = 4;
		num_songs = 1;
		useFont=mfont;//shadefont;
		fontHeight=10;
		sel=0;
		foreground=mfont_fgcolor;
		background=mfont_bgcolor;
		useBrush=mfont_bgbrush;
	}
	else
	{
		fontHeight=pe_fontheight;
		right=config_pe_width-20;
		ypos=22+pos*fontHeight,
		left=12;
		num_songs=(config_pe_height-38-20-2)/pe_fontheight;
		useFont=font;
		if (sel&2) foreground=Skin_PLColors[1];
		else foreground= Skin_PLColors[0];
		if (!(sel&1)) background= Skin_PLColors[2];
		else background=Skin_PLColors[3];
		useBrush=(sel&1)?selbrush:normbrush;
	}
	if (pos >= num_songs) return;

	int xpos=left;
	int endp=right;
	int num_chars=(endp-xpos)/5;
	if (pos>=0 || !config_bifont)
	{	
		RECT r={xpos,ypos,endp, ypos+fontHeight};
		
			SetTextColor(hdcout,foreground);
			SetBkColor(hdcout,background);

		do_palmode(hdcout);
		HFONT oldfont = (HFONT) SelectObject(hdcout,useFont);

		int rightToLeft = (config_pe_direction == PE_DIRECTION_RTL) || (config_pe_direction == PE_DIRECTION_AUTO && (GetFontLanguageInfo(hdcout) & GCP_REORDER));
		if (time >= 0) 
		{
			char str[123];
			StringCchPrintfA(str,123," %d:%02d ",time/60,(time)%60); // format the time string

			RECT r2=r;
			if (pos == -1)
			{
				r2.top--;
				r2.bottom--;
			}
			
			SIZE timeSize;
#ifdef BENSKI_NEW_MODE_THAT_BREAKS_GEN_JUMPEX_STUPID_API_HIJACKING
			// compute width of the string
			GetTextExtentPoint32(hdcout, str, lstrlen(str), &timeSize);
#else
			DrawTextA(hdcout,str,lstrlenA(str),&r2,DT_VCENTER|DT_CALCRECT|DT_SINGLELINE/*|DT_NOCLIP*/|DT_NOPREFIX);
			timeSize.cx = r2.right-r2.left;
			r2.top = r.top;
			r2.bottom = r.bottom;
#endif
			if (timeSize.cx < endp-xpos) 
			{
				UINT alignment;
				
				if (rightToLeft)
				{
					r2.left=xpos;
					r2.right=xpos+timeSize.cx;

					r.left+=timeSize.cx;
					xpos+=timeSize.cx;
					alignment=DT_LEFT|DT_RTLREADING;
				}
				else
				{
					r2.left=endp-timeSize.cx;
					r2.right=endp;

					r.right-=timeSize.cx;
					endp-=timeSize.cx;
					alignment=DT_RIGHT;
				}

				DrawTextA(hdcout,str,lstrlenA(str),&r2,DT_VCENTER|DT_SINGLELINE|alignment/*|DT_NOCLIP*/|DT_NOPREFIX); // fg: removed DT_NOCLIP to fix xp's cleartype from painting too far
			}
		}

		UINT alignment;
		if (rightToLeft)
		{
			alignment=DT_RIGHT|DT_RTLREADING;
		}
		else
		{ 
			alignment=DT_LEFT;
		}
	
		if (pos==-1)
			CharUpperW(name);

		int nameLen = lstrlenW(name);
		int jtf_result = 0;
		if (jtf_drawtext)
			jtf_result=jtf_drawtext(hdcout,name,nameLen,&r,alignment|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);

		if (!jtf_result)
			DrawTextW(hdcout,name,nameLen,&r,alignment|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);

		if (jtf_result != 2)
			DrawTextW(hdcout,name,nameLen,&r,alignment|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX|DT_CALCRECT);

		HBRUSH oldbrush=(HBRUSH)SelectObject(hdcout,useBrush);
		HPEN oldpen=(HPEN)SelectObject(hdcout,GetStockObject(NULL_PEN));
		if (rightToLeft)
			Rectangle(hdcout, xpos, r.top, 1+(endp-r.right)+r.left, 1+ypos+fontHeight);
		else
			Rectangle(hdcout,r.right,r.top,1+endp,1+ypos+fontHeight);
		if (!pos || pos == num_songs-1) 
		{
			SelectObject(hdcout,normbrush);
			if (!pos)
				Rectangle(hdcout, left, 20, 1+right, 1+23);
			if (pos == num_songs-1)
				Rectangle(hdcout, left, ypos+fontHeight, 1+right, config_pe_height-38+1);
		}

		SelectObject(hdcout,oldfont);
		SelectObject(hdcout,oldpen);
		SelectObject(hdcout,oldbrush);
	}
	else
	{
		int x;
		char str[10]="";
		setSrcBM(fontBM);
		do_palmode(hdcout);
		if (time >= 0)
		{
			StringCchPrintfA(str,10,"%d:%02d",time/60,(time)%60);
			num_chars -= lstrlenA(str)+1;
		}

		for (x = 0; x < num_chars && name[x]; x ++)
		{
			int c2=0,c=0;
			wchar_t oc;
			oc=name[x];
			if (x==num_chars-1 && name[x+1]) oc=L'\1';
			getXYfromChar(oc,&c,&c2);
			BitBlt(hdcout,xpos,ypos,5,6,bmDC,c,c2,SRCCOPY);
			xpos += 5;
		}

		if (!name[x])
		{
			int c2=0,c=0;
			getXYfromChar(L' ',&c,&c2);
			if (time >= 0) num_chars++;
			while (x++ < num_chars)
			{
				BitBlt(hdcout,xpos,ypos,5,6,bmDC,c,c2,SRCCOPY);
				xpos += 5;
			}
		}

		if (time >= 0)
		{
			int c2=0,c=0;
			getXYfromChar(' ',&c,&c2);
			BitBlt(hdcout,xpos,ypos,5,6,bmDC,c,c2,SRCCOPY);
			xpos += 5;
			if (pos < 0)
				xpos = config_pe_width-29-5*lstrlenA(str);
			else
				xpos = config_pe_width-24-5*lstrlenA(str);
			for (x = 0; str[x]; x ++)
			{
				getXYfromChar(str[x],&c,&c2);
				BitBlt(hdcout,xpos,ypos,5,6,bmDC,c,c2,SRCCOPY);
				xpos += 5;
			}
		}

		if (xpos < config_pe_width-(pos<0?30:20))
		{
			int c2=0,c=0;
			getXYfromChar(' ',&c,&c2);
			BitBlt(hdcout,xpos,ypos,config_pe_width-(pos<0?30:20)-xpos,6,bmDC,c,c2,SRCCOPY);
		}

		unsetSrcBM();
	}
}

void draw_pe_tbar(HWND hwnd, HDC hdc, int state)
{
	if (!pe_init)
		return;
	if (!disable_skin_borders)
	{
		WADrawDC hdcout(hdc, hwnd);
		state = state?0:21;
		do_palmode(hdcout);
		setSrcBM(plMainBM);
		if (config_pe_height != 14) 
		{
			int nt;
			int xp=0;
			BitBlt(hdcout,xp,0,25,20,bmDC,0,state,SRCCOPY);
			xp+=25;
			nt = (config_pe_width - 25 - 25 - 100)/25;
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
			nt = (config_pe_width - 25 - 25 - 100)/25;
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
			nt = (config_pe_width - 25 - 25 - 100)%25;
			if (nt)
			{
				StretchBlt(hdcout,xp,0,nt,20,bmDC,127,state,25,20,SRCCOPY);
				xp+=nt;
			}
			BitBlt(hdcout,xp,0,25,20,bmDC,153,state,SRCCOPY);
		}
		else
		{
			int xpos=0;
			int n = (config_pe_width - 50)/25;
			BitBlt(hdcout,xpos,0,25,14,bmDC,72,42,SRCCOPY);
			xpos+=25;
			n--;
			while (n--)
			{
				BitBlt(hdcout,xpos,0,25,14,bmDC,72,57,SRCCOPY);
				xpos+=25;
			}
			n = (config_pe_width - 50)%25;
			if (n)
			{
				StretchBlt(hdcout,xpos,0,n,14,bmDC,72,57,24,14,SRCCOPY);
				xpos+=n;
			}
			BitBlt(hdcout,xpos,0,50,14,bmDC,99,state?57:42,SRCCOPY);
		}
		unsetSrcBM();
		
		if (config_pe_height == 14)
		{
			wchar_t ft[FILETITLE_SIZE] = {0};
			int q=PlayList_getPosition();
			if (q >= 0 && q < PlayList_getlength())
			{        
				if (!g_has_deleted_current)
				{
					PlayList_getitem_pl(PlayList_getPosition(),ft);
					q=PlayList_getsonglength(PlayList_getPosition());
				}
				else
				{
					StringCchCopyW(ft,FILETITLE_SIZE, FileTitle);
					q=in_getlength();
				}
			}
			else
			{
				getStringW(IDS_PLEDIT_NOFILE_WSHADE,ft,FILETITLE_SIZE);
				q=-1;
			}
			draw_pe_song(hdcout, -1,ft,q,state?0:2);
		}
	}
}

void draw_pe_timedisp(HDC hdc, int minutes, int seconds, int tlm, int clear)
{
	if (!pe_init)
		return;
	static int lastm, lasts, lastc=1,lasttlm;
	int x=config_pe_width-94+8;
	int y=config_pe_height-15;
	if (!hPLWindow) return;
	if (disable_skin_borders) return;

	WADrawDC mainDC(hdc, hPLWindow);
	if (minutes == -666) { minutes=lastm; seconds=lasts; clear=lastc; tlm=lasttlm; }

	lastm=minutes; lasts=seconds; lastc=clear; lasttlm=tlm;

	do_palmode(mainDC);

	setSrcBM(fontBM);
	if (clear)
	{
		BitBlt(mainDC,x,y,4,6,bmDC,142,0,SRCCOPY);
		BitBlt(mainDC,x+4,y,5,6,bmDC,142,0,SRCCOPY);
		BitBlt(mainDC,x+5+4,y,5,6,bmDC,142,0,SRCCOPY);
		BitBlt(mainDC,x+5+4+5,y,5,6,bmDC,142,0,SRCCOPY);
		BitBlt(mainDC,x+5+4+5+4+4,y,5,6,bmDC,142,0,SRCCOPY);
		BitBlt(mainDC,x+5+4+5+4+5+4,y,5,6,bmDC,142,0,SRCCOPY);		  
	}
	else
	{
		if (tlm)
		{
			if (minutes < 0) minutes=-minutes;
			if (seconds < 0) seconds=-seconds;
			if (!(minutes/100)) BitBlt(mainDC,x,y,4,6,bmDC,142,0,SRCCOPY);
			BitBlt(mainDC,x+((minutes/100)?0:4),y,3,6,bmDC,75,6,SRCCOPY);
		}
		else 
		{
			if (!(minutes/100)) BitBlt(mainDC,x,y,4,6,bmDC,142,0,SRCCOPY);
			BitBlt(mainDC,x+((minutes/100)?0:4),y,3,6,bmDC,142,0,SRCCOPY);
		}

		if (minutes/100) BitBlt(mainDC,x+4,y,5,6,bmDC,5*((minutes/100)%10),6,SRCCOPY);
		BitBlt(mainDC,x+5+4,y,5,6,bmDC,5*((minutes/10)%10),6,SRCCOPY);
		BitBlt(mainDC,x+5+4+5,y,5,6,bmDC,5*((minutes)%10),6,SRCCOPY);
		BitBlt(mainDC,x+5+4+5+4+4,y,5,6,bmDC,5*((seconds/10)%10),6,SRCCOPY);
		BitBlt(mainDC,x+5+4+5+4+5+4,y,5,6,bmDC,5*((seconds)%10),6,SRCCOPY);
	}
	unsetSrcBM();
}

void draw_pe_tbutton(int b2, int b3, int b2_ws)
{
	if (!pe_init)
		return;
	if (disable_skin_borders) return;
	{
			WADrawDC hdcout(hPLWindow);
		do_palmode(hdcout);
		setSrcBM(plMainBM);
		if (!b3)
			BitBlt(hdcout,config_pe_width-11,3,9,9,bmDC,167,3,SRCCOPY);
		else
			BitBlt(hdcout,config_pe_width-11,3,9,9,bmDC,52,42,SRCCOPY);
		if (!b2)
		{
			if (!b2_ws) BitBlt(hdcout,config_pe_width-20,3,9,9,bmDC,158,3,SRCCOPY);
			else BitBlt(hdcout,config_pe_width-20,3,9,9,bmDC,128,45,SRCCOPY);
		}
		else
			BitBlt(hdcout,config_pe_width-20,3,9,9,bmDC,b2_ws?150:62,42,SRCCOPY);
		unsetSrcBM();
	}
}

static bool IntersectY(int top1, int bottom1, int top2, int bottom2) 
{
	int top = MAX(top1, top2);
	int bottom = MIN(bottom1, bottom2);
	return top <= bottom;
}

static void draw_pl(HWND hwnd, HDC hdcout)
{
	if (!pe_init)
		return;
	do_palmode(hdcout);
	draw_pe_tbar(hwnd, hdcout, GetForegroundWindow()==hwnd?1:(config_hilite?0:1));
	setSrcBM(plMainBM);
	if (config_pe_height != 14) 
	{
		int y;
		int yp=20;
		y=(config_pe_height-20-38)/29;
		while (y-->0)
		{
			BitBlt(hdcout,0,yp,12,29,bmDC,0,42,SRCCOPY);
			BitBlt(hdcout,config_pe_width-20,yp,5,29,bmDC,31,42,SRCCOPY);
			BitBlt(hdcout,config_pe_width-7,yp,7,29,bmDC,31+13,42,SRCCOPY);
			yp += 29;
		}
		y=(config_pe_height-20-38)%29;
		if (y)
		{
			StretchBlt(hdcout,0,yp,12,y,bmDC,0,42,12,29,SRCCOPY);
			StretchBlt(hdcout,config_pe_width-20,yp,5,y,bmDC,31,42,5,29,SRCCOPY);
			StretchBlt(hdcout,config_pe_width-7,yp,7,y,bmDC,31+13,42,7,29,SRCCOPY);
			yp += y;
		}

		if (!disable_skin_borders)
		{
			BitBlt(hdcout,0,yp,125,38,bmDC,0,72,SRCCOPY);
			int x=(config_pe_width-125-150)/25;

			int xp=125, s=0;
			if (x >= 3) { x-=3; s=1;}
			while (x-->0)
			{
				BitBlt(hdcout,xp,yp,25,38,bmDC,179,0,SRCCOPY);
				xp+=25;
			}
			x = (config_pe_width-125-150)%25;
			if (x)
			{
				StretchBlt(hdcout,xp,yp,x,38,bmDC,179,0,25,38,SRCCOPY);
				xp+=x;
			}
			if (s)
			{
				BitBlt(hdcout,xp,yp,75,38,bmDC,205,0,SRCCOPY);
				xp+=75;
			}
			BitBlt(hdcout,xp,yp,150,38,bmDC,126,72,SRCCOPY);
		}
	}
	unsetSrcBM();
}

static void draw_pl2(HWND hwnd, HDC hdc, RECT &r)
{
	if (config_pe_height != 14) 
	{
		{
			int x,t;
			int num_songs=(config_pe_height-38-20-2)/pe_fontheight;
			if (pledit_disp_offs < 0) pledit_disp_offs=0;
			t=PlayList_getlength()-pledit_disp_offs;
			if (t < 1) 
			{
				pledit_disp_offs=0;
				t=PlayList_getlength();
			}
			if (t > num_songs) t = num_songs;
			//LeaveCriticalSection(&g_mainwndcs);
			for (x = 0; x < t; x ++)
			{
				int qt,b;

				qt=22+x*pe_fontheight;
				b=qt+pe_fontheight-1;
				if (IntersectY(qt, b, r.top, r.bottom)
					|| (x == t-1 && config_pe_height-38 >= r.top))
				{
					wchar_t ft[FILETITLE_SIZE] = {0};
					PlayList_getitem_pl(x+pledit_disp_offs,ft);

					extern int g_has_deleted_current;
				//	EnterCriticalSection(&g_mainwndcs);
					draw_pe_song(hdc, x,ft,PlayList_getsonglength(x+pledit_disp_offs),(PlayList_getselect(x+pledit_disp_offs) ? 1 : 0) + ((!g_has_deleted_current && PlayList_getPosition() == x+pledit_disp_offs) ? 2 : 0));
					//LeaveCriticalSection(&g_mainwndcs);
				}
			}
			//EnterCriticalSection(&g_mainwndcs);
			wchar_t blank[] = L" ";
			for (; x < num_songs; x ++)
			{
				draw_pe_song(hdc, x,blank ,-1,0);
			}
			if (r.right > config_pe_width-18 )
			{
				draw_pe_vslide(hwnd, hdc, 0, pledit_disp_offs);
			}
		}
		draw_pe_timedisp(hdc, -666,0,0,0);
		{
			wchar_t str[64]=L"", str2[32]=L"";
			{
				int /*x=0,*/v,t,p=0;
				int seltime=0,st2=0,st1=0, ttime=0;
				v = PlayList_getlength();
				for (t = 0; t < v; t ++)
				{
					int a = PlayList_getsonglength(t);
					if (PlayList_getselect(t))
					{
						p++;
						if (a<0)
							st2=2;
						else seltime += a;
					}
					if (a<0) st1=1;
					//else x += a;
					if (a > 0) ttime += a;
				}
				if (!seltime && !st2) StringCchCopyW(str,64, L"0:00");
				else if (seltime) 
				{
					if (seltime < 60*60)
						StringCchPrintfW(str, 64, L"%d:%02d",seltime/60,seltime%60);
					else
						StringCchPrintfW(str,64, L"%d:%02d:%02d",seltime/60/60,(seltime/60)%60,seltime%60);
					if (st2) StringCchCatW(str,64,L"+");
				} else if (st2) StringCchCatW(str,64,L"?");

				if (!ttime && !st1) StringCchCopyW(str2,32, L"0:00");
				else if (ttime) 
				{
					if (ttime < 60*60)
						StringCchPrintfW(str2,32, L"%d:%02d",ttime/60,ttime%60);
					else
						StringCchPrintfW(str2,32,L"%d:%02d:%02d",ttime/60/60,(ttime/60)%60,ttime%60);
					if (st1) StringCchCatW(str2,32,L"+");
				} else if (st1) StringCchCatW(str2,32,L"?");
			}
			StringCchCatW(str,64,L"/");
			StringCchCatW(str,64,str2);

			draw_pe_infostr(hdc, str);
		}
	}
}

static void draw_paintDC_pe(HWND hwnd, HDC hdc, RECT &r)
{
	draw_pl(hwnd, hdc);
}

void draw_printclient_pe(HWND hwnd, HDC hdc, LPARAM /*drawingOptions*/)
{
	RECT r;
	GetClientRect(hwnd,&r);
	draw_paintDC_pe(hwnd, hdc, r);
	draw_pl2(hwnd, hdc, r);
}

void draw_paint_pe(HWND hwnd)
{
	if (!pe_init)
		return;
	PAINTSTRUCT ps;
	RECT r;
	HDC hdc=BeginPaint(hwnd,&ps);
	EnterCriticalSection(&g_mainwndcs);
	r=ps.rcPaint;
	draw_paintDC_pe(hwnd, hdc, r);
	WADrawDC windowDC(0, hwnd);
	draw_pl2(hwnd, windowDC, r);
	LeaveCriticalSection(&g_mainwndcs);
	EndPaint(hwnd,&ps);
}