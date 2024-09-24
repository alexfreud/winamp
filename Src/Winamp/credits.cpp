/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description: Unused (left for reference)
 ** Author:
 ** Created:
 **/

#include "main.h"

#if 0
#ifndef NETSCAPE
#include <math.h>

static void RenderInit(HWND hwnd);
static void RenderQuit(HWND hwnd);
static int RenderFrame(HWND hwnd);

static int w_width, w_height,w_offs;
static volatile int killsw;

void About2_Kill()
{
	killsw=1;
}

void About2_Start(HWND hwndParent)
{
	RECT r;
	killsw=0;
	Sleep(100);
	GetWindowRect(hwndParent,&r);
	w_width =r.right-r.left;
	w_height=((r.bottom-r.top)); // *3/4
	w_offs=0;//((r.bottom-r.top)*1)/9;
	w_width += 3;
	w_width &= ~3;
	RenderInit(hwndParent);
	while (!killsw)
	{
		int rtime=GetTickCount();
		RenderFrame(hwndParent);
		rtime = GetTickCount()-rtime;
		if (rtime > 16) rtime=16;
		Sleep(16-rtime);
	}
	RenderQuit(hwndParent);
}

static HFONT hFont, hOldFont;
static int th,linepos,fadepos,egg_pos;
static HDC bm_hdc, egg_hdc;
static HBITMAP bm_bitmap, bm_oldbm, egg_bm, egg_oldbm;

static int egg_dobg=0, egg_hacko;

static void RenderInit(HWND hwnd)
{
	RECT r = {0,0,w_width,w_height+40};
	TEXTMETRIC tm;

	egg_dobg=0;

	egg_hacko=eggstat;
	if (egg_hacko) egg_oldbm=(HBITMAP)SelectObject(egg_hdc=CreateCompatibleDC(NULL),egg_bm=LoadBitmap(hMainInstance,MAKEINTRESOURCE(IDB_CAT)));
	
	bm_hdc=CreateCompatibleDC(egg_hacko?egg_hdc:NULL);
	bm_bitmap=CreateCompatibleBitmap(egg_hacko?egg_hdc:bm_hdc,r.right,r.bottom);
	bm_oldbm=(HBITMAP)SelectObject(bm_hdc,bm_bitmap);

	BitBlt(bm_hdc,0,0,r.right,r.bottom,bm_hdc,0,0,BLACKNESS);
	SetMapMode(bm_hdc,MM_TEXT);
	hFont=CreateFont(16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
		DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DRAFT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Times New Roman");
	hOldFont=(HFONT)SelectObject(bm_hdc,hFont);
	GetTextMetrics(bm_hdc,&tm);
	th=tm.tmHeight;
	if (th > 39) th = 39;
	SetTextColor(bm_hdc,RGB(255,255,255));
	SetBkColor(bm_hdc,RGB(0,0,0));
	linepos=6;
	fadepos=256;
}

static void RenderQuit(HWND hwnd)
{
	SelectObject(bm_hdc,hOldFont);
	SelectObject(bm_hdc,bm_oldbm);
	DeleteObject(bm_bitmap);
	DeleteDC(bm_hdc);
	DeleteObject(hFont);
	if (egg_oldbm || egg_hdc)
	{
		SelectObject(egg_hdc,egg_oldbm);
		DeleteObject(egg_bm);
		DeleteDC(egg_hdc);
		egg_bm=NULL;
		egg_hdc=NULL;
	}
}

typedef struct 
{
	char *col1,*col2;
} t_line;


#define BLINE {"",}
static t_line text_lines[] =
{
	BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,
	{ APP_NAME " [tm]",},
	{ "———————————————",},
	{ "Copyright © 1997-2000 - Nullsoft",},
	BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,
	{"CREDITS",},
	BLINE,
	{"Unit One",},
	{"——————",},
	{"PRODUCTION AND DESIGN","Justin Frankel"},
	BLINE,
	{"CREW","Tom Pepper"},
	{"","Robert Lord"},
	{"","Ian Rogers"},
	{"","Steve Gedikian"},
	{"","Brennan Underwood"},
  BLINE,
  {"MIKMOD PLUG-IN","Jake Stine"},
	BLINE,
	{"SPLASH SCREEN","Christian Lundquist"},
	BLINE,
	BLINE,BLINE,BLINE,BLINE,
	{"Unit Two",},
	{"——————",},
	{"STUNT COORDINATOR","Jean-Hugues Royer"},
	BLINE,
	{"STUNTS","Jay Downing"},
	{"","Tim Russell"},
	{"","Rob Markovic"},
	{"","Peter A. DeNitto"},
	{"","Colten Edwards"},
	{"","Mike Wickenden"},
	{"","Peter Hollandare"},
	{"","Nicholas Head"},
	{"","Craig Vallelunga"},
	{"","Jason Reimer"},
	{"","Kenric Tam"},
	BLINE,
	{"PUPPETEER","Rob 'Wonderful"},
	{"","    Wawb' Bresner"},
	BLINE,
	{"ICON TRAINERS", "Torsten Daeges"},
	{"","Ben Lowery"},
	BLINE,
	{"LLAMA WRANGLER","Tom Pepper"},
	BLINE,
	{"ANIMAL TRAINER","Nova Hall"},
	BLINE,
	{"ASSISTANT TO FIFI","Robert Lord"},
	BLINE,
	{"WATCHING ANIME","Ted Cooper"},
	BLINE,
	{"NUDE SCENES","Charlie Hinz"},
	BLINE,
	{"KARATE SCENE","Dan Khamsing"},
	{"COORDINATORS","Thanh Tran"},
	BLINE,
	{"TOPLESS DANCER","Jenn Spencer"},
	BLINE,
	{"HAIR AND MAKEUP DESIGN","Brennan Underwood"},
	{"FOR MR. FRANKEL",""},
	BLINE,
	{"MYSTERY LADY","Al"},
	BLINE,
	{"COSTUME DESIGNER","Casey Scales"},
	BLINE,
	{"PRIME NUMBERS","Cap Petschulat"},
	BLINE,
	{"BIG BIZ EXPLOITER","Kenneth Chen"},
	BLINE,
	{"RESIDENT DENTIST","Meng"},
	BLINE,
	{"NITE FIEND","David Pui"},
	BLINE,
	{"PYROTECHNICS","Jaben Cargman"},
	BLINE,
	{"CATERING","Charles H. Frankel"},
	{"","Kathleen Blake-Frankel"},
	{"","Loretta Spinster"},
	BLINE,
	{"BEER","Peregrine Computing",},
	BLINE,
	{"ELEVATOR MUSIC","The Robies",},
	BLINE,
	{"GAFFER","Adara Frankel"},
	BLINE,
	{"BEST BOY","Paul Garcia"},
	BLINE,
	{"KEY GRIP","Josh Marso"},
	BLINE,
	{"GRIPS","Ryan Underwood"},
	{"","Alex Derbes"},
	{"","Mike Wickenden"},
	BLINE,
	{"OFTEN ANNOYING","Angelo Sotira"},
	{"PUBLIC MANIPULATION","Andrew Smith"},
	BLINE,
	{"CRASH TEST DUMMY","Alun Wile"},
	BLINE,
	{"GENETIC ENGINEERING","Jawed Karim"},
	BLINE,
	{"BREAST EXAMINER","Shaun Curtis"},
	BLINE,
	{"TRANSLATOR","Alix Reyes"},
	BLINE,
	{"WARFARE TECH","Thanh Tran"},
	BLINE,
	{"SHIPBUILDING", "Gary Calpo"},
	BLINE,
	{"BAD HUMOR","Adara Blake"},
	BLINE,
	{"ANAL PROBER","Dave \"Lestat\" Wile"},
	BLINE,
	{"CLEARANCES","Franc Zijderveld"},
	BLINE,
	{"JANITORS","Ian Lyman"},
	{"","Andrew McCann"},
	BLINE,
	{"PLASTERER","Dmitry Boldyrev"},
	BLINE,
	{"CARPENTRY","Marc Pirotte"},
	{"","Dána M. Epp"},
	{"","Graham Batty"},
	{"","John Stephens"},
	{"","Jon Lippincott"},
	{"","Doug Mealing"},
	{"","Jessica Wirna"},
	{"","Chris Fitzpatrick"},
	BLINE,
	{"EXTRAS","Bill Harper"},
	{"","Dana Dahlstrom"},
	{"","Allen Anderson"},
	{"","Diane Downard"},
	{"","Tima Kunayev"},
	BLINE,
	{"STAND-INS","Ryan Houle"},
	{"","Bryan Burton"},
	{"","Justin Derbes"},
	BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,
	{"Special Thanks:",},
	BLINE,
	{"The City of Detroit, Michigan",},
	{"US Department of Justice",},
	{"Dallas Square-Dancing Hall of Fame",},
	{"Lighthouse Communications",},
	{"Samsung USA",},
	{"Phoenix International Raceway",},
	{"San Diego Zoo",},
	{"Audi America",},
	BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,
	{"Filmed in Amazing Technicolor®",},
	BLINE,BLINE,
	{"Soundtrack available on Fuckit Records",},
	BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,
	{"—————————————————",},
	{"No animals were harmed in the filming",},
	{"and/or production of this product",},
	{"—————————————————",},
	BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,
	BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,
	{"© MM Nullsoft Inc.",},
	{"http://www.nullsoft.com/",},
	BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,BLINE,
	{NULL,NULL}
};

static int RenderFrame(HWND hwnd)
{
	static int y;
	if (fadepos < 256)
	{
		fadepos-=2;
		if (fadepos <= 0) 
		{
			fadepos=256;
			y=0;
			BitBlt(bm_hdc,0,0,w_width,w_height+th+1,bm_hdc,
				0,0,BLACKNESS);
		}
	}
	else if (!y)
	{
		RECT r={0,w_height,w_width,w_height+th};
		y=th;

		if (!text_lines[linepos].col1)
		{
			egg_pos=0;
			egg_dobg=0;
			linepos=0;
			fadepos=255;
		}
		else if (1)
		{
			if (egg_hacko) {
				int h=(th*80)/w_width,h2,h3;
				h2=h;
				if (h2+egg_pos >= 60)
				{
					h2=60-egg_pos;
					h-=h2;
					h3=(h2*w_width)/80;
				} else h=0;
				if (h2) StretchBlt(bm_hdc,0,w_height,w_width,th+1,egg_hdc, 0,egg_pos,80, h2, egg_dobg?SRCCOPY:BLACKNESS);
				egg_pos+=h2;
				if (egg_pos>=60) { egg_pos=0; egg_dobg=!egg_dobg; }
				if (h) StretchBlt(bm_hdc,0,w_height+h3,w_width,th+1,egg_hdc, 0,egg_pos,80, h, egg_dobg?SRCCOPY:BLACKNESS);
				egg_pos+=h;
				SetBkMode(bm_hdc,TRANSPARENT);
			}	
			if (!text_lines[linepos].col2)
				DrawText(bm_hdc,text_lines[linepos++].col1,-1,&r,DT_CENTER);
			else 
			{
				RECT r1={0,w_height,w_width/2 - 10,w_height+th};
				RECT r2={w_width/2 + 10,w_height,w_width,w_height+th};
				DrawText(bm_hdc,text_lines[linepos].col1,-1,&r1,DT_RIGHT);
				DrawText(bm_hdc,text_lines[linepos++].col2,-1,&r2,DT_LEFT);
			}
		}
		BitBlt(bm_hdc,0,0,w_width,w_height+th+1,bm_hdc,
			0,1,SRCCOPY);
	} 
	else
	{
		y--;
		BitBlt(bm_hdc,0,0,w_width,w_height+th+1,bm_hdc,
			0,1,SRCCOPY);
	}

	{
		HDC hdc=GetDC(hwnd);
		BitBlt(hdc,0,w_offs,w_width,w_height,bm_hdc,0,0,SRCCOPY);
		ReleaseDC(hwnd,hdc);
	}
	return 0;
}

#endif
#endif