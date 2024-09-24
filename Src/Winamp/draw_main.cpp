/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "draw.h"
#include "../nu/AutoWide.h"

HBITMAP tbBM, cbuttonsBM, mainBM2,oldmainBM2, numbersBM,  numbersBM_ex,
playpauseBM, posbarBM, monostereoBM, volBM,panBM;

void draw_tbuttons(int b1, int b2, int b3, int b4)
{
	setSrcBM(tbBM);
	if (b1 != -1) BitBlt(mainDC,6,3,9,9,bmDC,0,b1*9,SRCCOPY);
	if (b2 != -1) BitBlt(mainDC,244,3,9,9,bmDC,9,b2*9,SRCCOPY);
	if (b3 != -1) BitBlt(mainDC,264,3,9,9,bmDC,18,b3*9,SRCCOPY);
	if (b4 != -1) BitBlt(mainDC,254,3,9,9,bmDC,b4*9,config_windowshade?27:18,SRCCOPY);
	unsetSrcBM();
	update_area(6,3,9,9);
	update_area(243,3,274-243,9);
}

void draw_eject(int pressed)
{
	RECT r;
	if (!draw_initted) return;
	r.left = 132+7-4+1;
	r.top = 60+14+15;
	r.right = r.left+22;
	r.bottom = r.top+16;
	setSrcBM(cbuttonsBM);
	BitBlt(mainDC,r.left,r.top,r.right-r.left,r.bottom-r.top,bmDC,114,(pressed?16:0),SRCCOPY);
	unsetSrcBM();
	update_rect(r);
}

static void draw_paintDC(HDC hdc, const RECT &r)
{
	if (!draw_initted) return;

	//int dsize=(config_dsize && config_eqdsize);
	int dsize = config_dsize;

	do_palmode(hdc);

	if (!dsize)
	{
		BitBlt(hdc,r.left,r.top,r.right-r.left,r.bottom-r.top,mainDC,r.left,r.top,SRCCOPY);

		if (mainBM2)
		{
			SelectObject(mainDC2,oldmainBM2);
			DeleteObject(mainBM2);
			mainBM2=NULL;
		}
	}
	else 
	{
		if (!mainBM2)
		{
			mainBM2 = CreateCompatibleBitmap(mainDC,WINDOW_WIDTH*2,WINDOW_HEIGHT*2);
			oldmainBM2 = (HBITMAP)SelectObject(mainDC2,mainBM2);
			StretchBlt(mainDC2,0,0,WINDOW_WIDTH*2,WINDOW_HEIGHT*2,mainDC,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,SRCCOPY);
		}
		BitBlt(hdc,r.left,r.top,r.right-r.left,r.bottom-r.top,mainDC2,r.left,r.top,SRCCOPY);
	}
}

void draw_printclient(HDC hdc, LPARAM /*drawingOptions*/)
{
	RECT r;
	GetClientRect(hMainWindow,&r);
	draw_paintDC(hdc, r);
}

void draw_paint(HWND hwnd)
{
	HDC screenHdc;
	PAINTSTRUCT ps;
	RECT r;
	if (!draw_initted) return;

	if (hwnd)
	{
		GetUpdateRect(hwnd,&ps.rcPaint,0);
		EnterCriticalSection(&g_mainwndcs);	
		screenHdc = BeginPaint(hwnd,&ps);
		memcpy(&r,&ps.rcPaint,sizeof(r));
	}
	else
	{
		screenHdc = draw_GetWindowDC(hMainWindow);
		GetClientRect(hMainWindow,&r);
	}

	draw_paintDC(screenHdc, r);

	if (hwnd) 
	{
		EndPaint(hwnd,&ps);
		LeaveCriticalSection(&g_mainwndcs);
	}
	else
		draw_ReleaseDC(hMainWindow,screenHdc);
}

void draw_tbar( int active, int windowshade, int egg )
{
	int t[ 4 ] = { 0, 15, 29, 42 };
	int l = t[ ( ( active ? 0 : 1 ) + ( windowshade ? 2 : 0 ) ) ];

	if ( egg && !windowshade )
		l = active ? 57 : 72;

	if ( !draw_initted )
		return;

	setSrcBM( tbBM );

	BitBlt( mainDC, 0, 0, WINDOW_WIDTH, 14, bmDC, 27, l, SRCCOPY );

	unsetSrcBM();

	if ( windowshade && config_windowshade )
	{
		int pos = 0;
		draw_songname( L"", &pos, 0 );
	}

	update_area( 0, 0, WINDOW_WIDTH, 14 );
}

static int g_need_erase=0;

static int _draw_songname(const wchar_t *str, int startpos, int offs_in_first) 
{
	int xp;
	int o = offs_in_first;
	str += startpos;
	xp = 111;
	if (g_need_erase)
	{
		IntersectClipRect(mainDC,111,12+13,264,12+15+8);
		HGDIOBJ oldBrush=SelectObject(mainDC,mfont_bgbrush);
		Rectangle(mainDC,108,12+12,267,12+15+10);
		SelectObject(mainDC,oldBrush);
		ExtSelectClipRgn(mainDC,NULL,RGN_COPY);
		g_need_erase=0;
	}

	setSrcBM(fontBM);
	while (xp < 265 && *str)
	{
		int c2=0,c=0;
		getXYfromChar(*str++,&c,&c2);
		if (xp >= 265-5)
		{
			BitBlt(mainDC,xp,12+15,265-xp,6,bmDC,c,c2,SRCCOPY);
			xp = 265;
		} 
		else 
		{
			BitBlt(mainDC,xp,12+15,5-o,6,bmDC,c+o,c2,SRCCOPY);
			xp += 5-o;
			o = 0;
		}
	}
	while (xp < 265) BitBlt(mainDC,xp++,12+15,1,6,bmDC,4,0,SRCCOPY);
	update_area(111,10+15,265-111,10);
	unsetSrcBM();
	return 0;
}

static int _draw_songname_winfont(const wchar_t *str, int start_pix) 
{
	RECT r={111-start_pix,12+11,265,12+16+mfont_height};
	HGDIOBJ oldBrush;
	g_need_erase=1;
	IntersectClipRect(mainDC,111,12+12,265,12+16+8);
	SetTextColor(mainDC,mfont_fgcolor);
	SetBkColor(mainDC,mfont_bgcolor);
	oldBrush=SelectObject(mainDC,mfont_bgbrush);
	Rectangle(mainDC,108,12+11,268,12+16+10);
	SelectObject(mainDC,oldBrush);
	DrawTextW(mainDC,str,-1,&r, DT_LEFT | DT_SINGLELINE |DT_NOPREFIX);
	ExtSelectClipRgn(mainDC,NULL,RGN_COPY);
	update_area(111,12+11,265-111,4+9);
	return 0;
}

void draw_time(int minutes, int seconds, int clear) {
	int ex=0;
	int tlm;
	if (!draw_initted) return;
  
	tlm=config_timeleftmode && in_mod && in_mod->is_seekable;
	if (tlm) 
	{
		int s;
		int tmp=in_getlength();
		s = minutes*60+seconds;
		s = tmp - s;
		if (tmp>0)
		{
			minutes = s/60;
			seconds = -(s%60);
		}
		else
			minutes=seconds=tlm=0;
	}
	if (config_windowshade)
		setSrcBM(fontBM);
	else 
	{
		if (numbersBM) setSrcBM(numbersBM);
		else 
		{
			setSrcBM(numbersBM_ex);
			ex=1;
		}
	}
	if (clear) 
	{
		if (config_windowshade)
		{
			BitBlt(mainDC,126,4,3,6,bmDC,142,0,SRCCOPY);
			BitBlt(mainDC,130,4,3,6,bmDC,142,0,SRCCOPY);
			BitBlt(mainDC,130+4,4,5,6,bmDC,142,0,SRCCOPY);
			BitBlt(mainDC,130+4+5,4,5,6,bmDC,142,0,SRCCOPY);
			BitBlt(mainDC,130+4+5+4+4,4,5,6,bmDC,142,0,SRCCOPY);
			BitBlt(mainDC,130+4+5+4+5+4,4,5,6,bmDC,142,0,SRCCOPY);
		}
		else
		{
			BitBlt(mainDC,23+7+6,11+15,9,13,bmDC,90,0,SRCCOPY);
			if (ex)
				BitBlt(mainDC,36+4-2,11+15,9,13,bmDC,90,0,SRCCOPY);
			else
				BitBlt(mainDC,36+4,32,5,1,bmDC,9,6,SRCCOPY);
			BitBlt(mainDC,35+7+6,11+15,9,13,bmDC,90,0,SRCCOPY);
			BitBlt(mainDC,47+7+6,11+15,9,13,bmDC,90,0,SRCCOPY);
			BitBlt(mainDC,65+7+6,11+15,9,13,bmDC,90,0,SRCCOPY);
			BitBlt(mainDC,77+7+6,11+15,9,13,bmDC,90,0,SRCCOPY);
		}
	} 
	else 
	{
		if (config_windowshade)
		{
			if (tlm)
			{
				if (minutes < 0) minutes=-minutes;
				if (seconds < 0) seconds=-seconds;
				if (minutes/100)
					BitBlt(mainDC,130-4,4,3,6,bmDC,75,6,SRCCOPY);
				else
					BitBlt(mainDC,130,4,3,6,bmDC,75,6,SRCCOPY);
			}
			else 
			{
				if (!(minutes/100))
				{
					BitBlt(mainDC,130,4,3,6,bmDC,142,0,SRCCOPY);
				}
				BitBlt(mainDC,130-4,4,3,6,bmDC,142,0,SRCCOPY);
			}
			if (minutes / 100)
				BitBlt(mainDC,130-1,4,5,6,bmDC,5*((minutes/100)%10),6,SRCCOPY);
			BitBlt(mainDC,130+4,4,5,6,bmDC,5*((minutes/10)%10),6,SRCCOPY);
			BitBlt(mainDC,130+4+5,4,5,6,bmDC,5*((minutes)%10),6,SRCCOPY);
			BitBlt(mainDC,130+4+5+4+4,4,5,6,bmDC,5*((seconds/10)%10),6,SRCCOPY);
			BitBlt(mainDC,130+4+5+4+5+4,4,5,6,bmDC,5*((seconds)%10),6,SRCCOPY);
		}
		else
		{
			BitBlt(mainDC,23+7+6,11+15,9,13,bmDC,90,0,SRCCOPY);
			if (tlm)
			{
				int t= 36+4-2;
				if (minutes < 0) minutes=-minutes;
				if (seconds < 0) seconds=-seconds;
				if (minutes/100)
				{
					BitBlt(mainDC,23+7+6,11+15,9,13,bmDC,((minutes/100)%10)*9,0,SRCCOPY);
					//	t-=4;
				}
				if (ex)
					BitBlt(mainDC,t-2,11+15,9,13,bmDC,99,0,SRCCOPY);
				else
					BitBlt(mainDC,t,32,5,1,bmDC,20,6,SRCCOPY);
			}
			else 
			{
				if (minutes/100)
					BitBlt(mainDC,23+7+6,11+15,9,13,bmDC,((minutes/100)%10)*9,0,SRCCOPY);
			}

			BitBlt(mainDC,35+7+6,11+15,9,13,bmDC,((minutes/10)%10)*9,0,SRCCOPY);
			BitBlt(mainDC,47+7+6,11+15,9,13,bmDC,((minutes)%10)*9,0,SRCCOPY);
			BitBlt(mainDC,65+7+6,11+15,9,13,bmDC,((seconds/10)%10)*9,0,SRCCOPY);
			BitBlt(mainDC,77+7+6,11+15,9,13,bmDC,((seconds)%10)*9,0,SRCCOPY);
		}
	}
	unsetSrcBM();

	if (config_pe_open && config_pe_height != 14)
	{
		draw_pe_timedisp(NULL, minutes,seconds,tlm,clear);
	}
	if (config_windowshade) update_area(125,4,32,6);
	else update_area(36,11+15,96-36+4,13);
}

void draw_playicon(int whichicon) // 0 = none, 1 = play, 2 = stop, 4 = pause, 8 = lost sync play
{
	int offset;
	RECT r;
	if (!draw_initted) return;
	r.left = 19+7;
	r.top = 13+15;
	r.right = r.left+9;
	r.bottom = r.top+9;
	switch (whichicon) 
	{
		case 0: offset = 27; break;
		case 1: offset = 0; break;
		case 2: offset = 18; break;
		case 4: offset = 9; break;
		case 8: offset = 0; break;
		default: return;
	}
	setSrcBM(playpauseBM);
	BitBlt(mainDC,r.left,r.top,r.right-r.left,r.bottom-r.top,bmDC,offset,0,SRCCOPY);
	r.left -= 2;
	r.right = r.left+3;
	if (whichicon == 1 || whichicon == 8)
	{
		offset = (whichicon==1?36:39);
		BitBlt(mainDC,r.left,r.top,r.right-r.left,r.bottom-r.top,bmDC,offset,0,SRCCOPY);
	}
	else
	{
		r.right = r.left+2;
		offset = 27;
		BitBlt(mainDC,r.left,r.top,r.right-r.left,r.bottom-r.top,bmDC,offset,0,SRCCOPY);
	}
	r.right = r.left + 11;
	unsetSrcBM();
	update_rect(r);
}

void draw_buttonbar(int buttonpressed) // starts at 0 with leftmost, -1 = no button 
{
	if (!draw_initted) return;
	setSrcBM(cbuttonsBM);
	if (buttonpressed == -1)
	{
		BitBlt(mainDC,8+8,58+14+15+1,114,18,bmDC,0,0,SRCCOPY);
	}
	else 
	{
		int d1[5] = { 0, 23, 46, 69, 92 }; // width of first section
		int d2[5] = { 23, 46, 69, 92, 114 }; // start of next button
		if (buttonpressed)
			BitBlt(mainDC,8+8,58+14+15+1,d1[buttonpressed],18,bmDC,0,0,SRCCOPY);
		BitBlt(mainDC,8+8+d1[buttonpressed],58+14+15+1,d2[buttonpressed]-d1[buttonpressed],
			   18,bmDC,d1[buttonpressed],18,SRCCOPY);
		if (buttonpressed != 4)
			BitBlt(mainDC,8+8+d2[buttonpressed],58+14+15+1,114-d2[buttonpressed],18,
		 		   bmDC,d2[buttonpressed],0,SRCCOPY);
	}
	unsetSrcBM();
	update_area(8+7,58+14+15,116,20);
}

void draw_bitmixrate(int bitrate, int mixrate) 
{
	static int l1=-1, l2=-1;
	if (bitrate < 0) bitrate=  l1;
	if (mixrate < 0) mixrate = l2;
	if (bitrate < 0) return;
	if (!draw_initted) return;
	setSrcBM(fontBM);
	if (bitrate/10000)
	{
  		if (bitrate/100000) BitBlt(mainDC,111,28+15,5,6,bmDC,(((bitrate/100000)%10))*5,6,SRCCOPY);
		else BitBlt(mainDC,111,28+15,5,6,bmDC,100,12,SRCCOPY); // blank
 		BitBlt(mainDC,111+5,28+15,5,6,bmDC,(((bitrate/10000)%10))*5,6,SRCCOPY);
		{
			int x,y;
			getXYfromChar(L'C',&x,&y);
			BitBlt(mainDC,111+5*2,28+15,5,6,bmDC,x,y,SRCCOPY);
		}
	}
	else if (bitrate/1000)
	{
  		BitBlt(mainDC,111,28+15,5,6,bmDC,(((bitrate/1000)%10))*5,6,SRCCOPY);
 		BitBlt(mainDC,111+5,28+15,5,6,bmDC,(((bitrate/100)%10))*5,6,SRCCOPY);
		{
			int x,y;
			getXYfromChar(L'H',&x,&y);
			BitBlt(mainDC,111+5*2,28+15,5,6,bmDC,x,y,SRCCOPY);
		}
	}
	else
	{
		if (bitrate/100)
			BitBlt(mainDC,111,28+15,5,6,bmDC,(((bitrate/100)%10))*5,6,SRCCOPY);
		else
			BitBlt(mainDC,111,28+15,5,6,bmDC,100,12,SRCCOPY); // blank
		if (bitrate/10) 
			BitBlt(mainDC,111+5,28+15,5,6,bmDC,(((bitrate/10)%10))*5,6,SRCCOPY);
		else
			BitBlt(mainDC,111+5,28+15,5,6,bmDC,100,12,SRCCOPY);
		BitBlt(mainDC,111+5*2,28+15,5,6,bmDC,((bitrate%10))*5,6,SRCCOPY);
	}

	if (mixrate/10) BitBlt(mainDC,156,28+15,5,6,bmDC,(((mixrate/10)%10))*5,6,SRCCOPY);
	else BitBlt(mainDC,156,28+15,5,6,bmDC,100,12,SRCCOPY);
	BitBlt(mainDC,156+5,28+15,5,6,bmDC,((mixrate%10))*5,6,SRCCOPY);
	unsetSrcBM();
	update_area(111,28+15,176-111,6);
	l1 = bitrate;
	l2 = mixrate;
}

void draw_positionbar(int position, int pressed) // position is 0-256
{
	RECT r;
	int op=position;
	if (!draw_initted) return;
	position = (position * (248-29)) / 256;
	if (position < 0) position = 0;
	if (position > (248-29)) position = 248-29;
	r.left = 9+7;
	r.top = 57+15;
	r.right = r.left+248;
	r.bottom = r.top+10;
	setSrcBM(posbarBM);
	if (position) 
		BitBlt(mainDC,r.left,r.top,position,r.bottom-r.top,bmDC,0,0,SRCCOPY);
	BitBlt(mainDC,r.left+position,r.top,29,10,bmDC,pressed?278:248,0,SRCCOPY);
	if (position != 248-29) 
		BitBlt(mainDC,r.left+position+29,r.top,248-29-position,r.bottom-r.top,
			   bmDC,position+29,0,SRCCOPY);
	unsetSrcBM();
	update_rect(r);
	if (config_windowshade)
	{
		int a;
		r.left = 226;
		r.top=4;
		r.right=243;
		r.bottom=11;
		op = (op * 12) / 256;
		if (++op < 1) op = 1;
		if (op > 13) op = 13;
		if (op < 6) a=0;
		else if (op < 9) a=1;
		else a=2;
		setSrcBM(tbBM);
		BitBlt(mainDC,r.left,r.top,r.right-r.left,r.bottom-r.top,bmDC,0,36,SRCCOPY);
		BitBlt(mainDC,r.left+op,r.top,3,r.bottom-r.top,bmDC,17+a*3,36,SRCCOPY);
		unsetSrcBM();
		update_rect(r);
	}
}

void draw_monostereo(int value) // 0 is clear, 1 is mono, 2 is stereo, 3 is downmixed stereo
{
	static int l = -1;
	if (!draw_initted) return;
	if (value < 0) value = l;
	if (value < 0) return;
	setSrcBM(monostereoBM);
	BitBlt(mainDC,7+205,41,28,12,bmDC,29,value==1?0:12,SRCCOPY);
	BitBlt(mainDC,7+232,41,29,12,bmDC,0,value==2?0:12,SRCCOPY);
	unsetSrcBM();
	update_area(199,41,7+232+29-199,12);
	l = value;
}

void draw_songname(const wchar_t *name, int *out_position, int songlen)  // position is the number of chars over it is
{
	int position = *out_position;
	wchar_t buf[2048];
	const wchar_t *draw_buf = buf;
	int len_of_str, len_of_spacer;
	int mode=!config_bifont;

	HGDIOBJ hOldFont;

	if (!draw_initted || !name)
		return;

	if (*name)
	{
		static int hold;
		if (songlen >= 0)
		{
			if (hold > 0)
			{
				hold--;
				return;
			}

			if(config_dotitlenum)
				StringCchPrintfW(buf,2048, L"%d. %s (%d:%02d)",PlayList_getPosition()+1, name,songlen/60,songlen%60);
			else
				StringCchPrintfW(buf,2048, L"%s (%d:%02d)",name,songlen/60,songlen%60);
		}
		else if (name == FileTitle) // benski> ok, ok, ok, this is a big hack
		{
			if (hold > 0)
			{
				hold--;
				return;
			}
			StringCchPrintfW(buf,2048, L"%d. %s", PlayList_getPosition()+1, name);
		}
		else
		{
			KillTimer(hMainWindow, UPDATE_DISPLAY_TIMER + 2);
			SetTimer(hMainWindow, UPDATE_DISPLAY_TIMER + 2, 1000, NULL);
			hold = (config_autoscrollname&1 ? ((songlen == -2) ? 5 : 1) : 0);
			StringCchCopyW(buf, 2048, name);
		}
	}
	else
		StringCchPrintfW(buf,2048, L"%S %S", app_name, app_version_string);

	if (!mode) 
	{
		len_of_str = lstrlenW(buf)*5;
		len_of_spacer=lstrlenW(L"  ***  ")*5;
	}
	else 
	{
		RECT r;
		hOldFont=SelectObject(mainDC,mfont);
		DrawTextW(mainDC,buf,-1,&r, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
		len_of_str=r.right-r.left;
		DrawTextW(mainDC,L"  ***  ",-1,&r, DT_CALCRECT | DT_SINGLELINE |DT_NOPREFIX);
		len_of_spacer=r.right-r.left;
	}

	const int textAreaWidth = 258-112;
	wchar_t obuf[4096];
	if (len_of_str > textAreaWidth)
	{
		StringCchPrintfW(obuf, 4096, L"%s  ***  %s", buf, buf);
		while (position < 0) position += len_of_str+len_of_spacer;
		if (position >= len_of_str+len_of_spacer) position -= len_of_str+len_of_spacer;
		draw_buf = obuf;
	}
	else
		position=0;

	if (!mode) 
		_draw_songname(draw_buf,position/5,position%5);
	else
	{
		_draw_songname_winfont(draw_buf, position);
		SelectObject(mainDC,hOldFont);
	}
	*out_position = position;
}

#define PANBAR_WIDTH 38
#define PANBAR_SLIDER_WIDTH 14
#define PANBAR_LENGTH (PANBAR_WIDTH-PANBAR_SLIDER_WIDTH)
#define ABS(x) (( x ) > 0 ? ( x ) : - ( x ))
void draw_panbar(int volume, int pressed) // volume is 0-256
{
	int ypos = ((ABS(volume) * 27) / 127)*15;
	RECT r;
	if (!draw_initted) return;
	if (ypos < 0) ypos=0; else if (ypos > 27*15) ypos=27*15;
	r.left = 177;
	r.top = 42+15;
	r.right = r.left + PANBAR_WIDTH;
	r.bottom = r.top + 13;
	setSrcBM(panBM);
	BitBlt(mainDC, r.left,r.top,r.right-r.left,r.bottom-r.top,bmDC,9,ypos,SRCCOPY);
	{
		int xpos = ((volume*12)/127)+12;
		if (xpos > PANBAR_LENGTH) 
			xpos = PANBAR_LENGTH; 
		else if (xpos < 0)
			xpos = 0;
		BitBlt(mainDC,r.left+xpos,r.top+1,14,11,bmDC,pressed?0:15,422,SRCCOPY);
	}
	unsetSrcBM();
	update_rect(r);
}

void draw_shuffle(int on, int pressed) 
{
	RECT r;
	if (!draw_initted) return;
	r.left = 164;
	r.top = 89;
	r.right = r.left+79-28-4;
	r.bottom = r.top+15;
	setSrcBM(shufflerepeatBM);
	BitBlt(mainDC,r.left,r.top,r.right-r.left,r.bottom-r.top,bmDC,28,(on?30:0) + (pressed?15:0),SRCCOPY);
	unsetSrcBM();
	update_rect(r);
}

void draw_repeat(int on, int pressed) 
{
	RECT r;
	if (!draw_initted) return;
	r.left = 182+7-4-4+29;
	r.top = 60+14+15;
	r.right = r.left+28;
	r.bottom = r.top+15;
	setSrcBM(shufflerepeatBM);
	BitBlt(mainDC,r.left,r.top,r.right-r.left,r.bottom-r.top,bmDC,0,(on?30:0) + (pressed?15:0),SRCCOPY);
	unsetSrcBM();
	update_rect(r);
}

void update_panning_text(int songlen)
{
	wchar_t buf[128] = {0};
	int v=config_pan;
	v *= 100;
	v/=127;
	if (v)
	{
		wchar_t lorrStr[32] = {0}, balanceStr[64] = {0};
		StringCchPrintfW(buf, 128, L"%s: %d%% %s",getStringW(IDS_BALANCE,balanceStr,64),v<0?-v:v,v<0?getStringW(IDS_BALANCE_LEFT,lorrStr,32):getStringW(IDS_BALANCE_RIGHT,lorrStr,32));
	}
	else 
		getStringW(IDS_BALANCE_CENTRE,buf,128);
	v=0;
	draw_songname(buf,&v,songlen);
}

void update_volume_text(int songlen)
{
	wchar_t buf[128] = {0}, volStr[64] = {0};
	int v = config_volume;
	v *= 100;
	v/=255;
	StringCchPrintfW(buf, 128,L"%s: %d%%",getStringW(IDS_VOLUME,volStr,64),v);
	draw_songname(buf,&v,songlen);
}

#define VOLBAR_WIDTH 68
#define VOLBAR_SLIDER_WIDTH 14
#define VOLBAR_LENGTH (VOLBAR_WIDTH-VOLBAR_SLIDER_WIDTH)
void draw_volumebar(int volume, int pressed) // volume is 0-256
{
	int ypos = ((volume * 27) / 255)*15;
	RECT r;
	if (!draw_initted) return;
	if (ypos < 0) ypos=0; else if (ypos > 27*15) ypos=27*15;
	r.left = 107;
	r.top = 42+15;
	r.right = r.left + VOLBAR_WIDTH;
	r.bottom = r.top + 13;
	setSrcBM(volBM);
	BitBlt(mainDC, r.left,r.top,r.right-r.left,r.bottom-r.top,bmDC,0,ypos,SRCCOPY);
	{
		int xpos = (volume*51)/255;
		if (xpos > VOLBAR_LENGTH) xpos = VOLBAR_LENGTH; else if (xpos < 0) xpos = 0;
		BitBlt(mainDC,r.left+xpos,r.top+1,14,11,bmDC,pressed?0:15,422,SRCCOPY);
	}
	unsetSrcBM();
	update_rect(r);
}

void draw_eqplbut(int eqon, int eqpressed, int plon, int plpressed)
{
	RECT urect = { 219, 58, 265, 70 };
	int x, y;
	if (!draw_initted) return ;
	x = eqpressed ? 46 : 0;
	y = eqon ? 12 : 0;
	setSrcBM(shufflerepeatBM);
	BitBlt(mainDC, urect.left, urect.top, (urect.right - urect.left) / 2, urect.bottom - urect.top,
	       bmDC, x, y + 61, SRCCOPY);
	x = plpressed ? 46 : 0;
	y = plon ? 12 : 0;
	BitBlt(mainDC, urect.left + (urect.right - urect.left) / 2, urect.top, (urect.right - urect.left) / 2, urect.bottom - urect.top,
	       bmDC, x + 23, y + 61, SRCCOPY);
	unsetSrcBM();
	update_rect(urect);
}