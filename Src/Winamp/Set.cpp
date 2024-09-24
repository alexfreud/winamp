/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "Main.h"
#include "WinampAttributes.h"
#include "ExternalCOM.h"
#include "../nu/AutoWide.h"

static int dc_pos=0;
extern int is_fullscreen_video;
extern int g_fsapp;

static BOOL HasParent(HWND hwnd)
{
	return ((0 != (WS_CHILD & GetWindowLongPtrW(hwnd, GWL_STYLE))) && NULL != GetParent(hwnd));
}

void do_caption_autoscroll(void)
{
	wchar_t buf[4096]=L"";
	if (dc_pos < (int)lstrlenW(caption))
	{
		StringCchCopyW(buf,4096, caption+dc_pos);
		StringCchCatW(buf,4096,L" *** ");
		StringCchCopyW(buf+lstrlenW(buf),4096-lstrlenW(buf),caption);
		dc_pos++;
	}
	else
	{
		StringCchCopyW(buf,4096,L" *** "+dc_pos-lstrlenW(caption));
		StringCchCatW(buf,4096, caption);
		StringCchCopyW(buf+lstrlenW(buf),4096-lstrlenW(buf), L" *** ");
		if (++dc_pos >= (int)lstrlenW(caption)+3) dc_pos=0;
	}
	SetWindowLong(hMainWindow,GWL_STYLE,GetWindowLongW(hMainWindow,GWL_STYLE)&~(WS_CAPTION));
	SetWindowTextW(hMainWindow,buf);
	SetWindowLong(hMainWindow,GWL_STYLE,GetWindowLongW(hMainWindow,GWL_STYLE)|(WS_CAPTION));
}

void set_caption(int alt_cb, wchar_t *format, ...) 
{
	va_list v;
	if (format)
	{
		va_start(v,format);
		StringCchVPrintfW(caption, CAPTION_SIZE, format, v);
		va_end(v);
	}

	SetWindowLong(hMainWindow,GWL_STYLE,GetWindowLongW(hMainWindow,GWL_STYLE)&~(WS_CAPTION));
	SetWindowTextW(hMainWindow,caption);
	SetWindowLong(hMainWindow,GWL_STYLE,GetWindowLongW(hMainWindow,GWL_STYLE)|(WS_CAPTION));
	dc_pos=0;
	if (systray_intray) systray_minimize(caption);    

	// TODO in some cases this is crashing based on crash reports (issue with 'now playling' ??)
	JSAPI1_CurrentTitleChanged();
	PostMessageW(hMainWindow,WM_WA_IPC,(!alt_cb?IPC_CB_MISC_TITLE:IPC_CB_MISC_TITLE_RATING),IPC_CB_MISC);
	SendNotifyMessage(HWND_BROADCAST, songChangeBroadcastMessage, 0, 0);
}

void set_taskbar(void)
{
	static int a;
	if (config_taskbar == 1 || config_taskbar == 2)
	{
		systray_minimize(caption);
	}
	else
	{
		if (systray_intray) systray_restore();
	}

	ShowWindow(hMainWindow,SW_HIDE);

	if (config_taskbar == 1 || config_taskbar == 3)
		SetWindowLong(hMainWindow,GWL_EXSTYLE,GetWindowLongW(hMainWindow,GWL_EXSTYLE)|WS_EX_TOOLWINDOW);
	else 
		SetWindowLong(hMainWindow,GWL_EXSTYLE,GetWindowLongW(hMainWindow,GWL_EXSTYLE)&~WS_EX_TOOLWINDOW);
	if (a)
	{
		extern int deferring_show;
		if (!deferring_show) ShowWindow(hMainWindow,SW_SHOWNA);
	}
	a=1;
}

// better enter embedcs before calling this func
static void doMyDirtyShitholeDockingShit(HWND myWnd, RECT newr, int sedge)  // -1 to do all
{
	RECT oldr = {0};
	GetWindowRect(myWnd, &oldr);
	if (myWnd == hMainWindow)
		FixMainWindowRect(&oldr);

	for (int whichedge = 0; whichedge < 2; whichedge++) // not always used
	{
		int diff;
		embedWindowState *p=embedwndlist;

		if (sedge >= 0)
			whichedge=sedge;

		if (whichedge == 0)
			diff=newr.bottom - oldr.bottom;
		else
			diff = newr.right - oldr.right;

		// move any windows docked to the bottom, bottom. recursively.
		if (diff) for (int x = 0; ; x ++)
		{
			HWND dockwnd=NULL;
			RECT oldrect;
			int vis=0;

			if ( x == 0 )
			{
				dockwnd = hMainWindow;
				vis = config_mw_open;
			}
			else if ( x == 1 )
			{
				dockwnd = hEQWindow;
				vis = config_eq_open;
			}
			else if ( x == 2 )
			{
				dockwnd = hPLWindow;
				vis = config_pe_open;
			}
//else if (x == 3) { dockwnd=hMBWindow; vis=0; } //config_mb_open; }
			else if ( x == 4 )
			{
				dockwnd = hVideoWindow;
				vis = config_video_open;
			}
			else
			{
				if ( !p )
					break;

				dockwnd = p->me;
				vis = IsWindowVisible( p->me );
			}
			GetWindowRect(dockwnd,&oldrect);
			if (!x)
				FixMainWindowRect(&oldrect);

			if (dockwnd != myWnd)
			{
				int isdocked;
				if (!whichedge) 
					isdocked=oldrect.top == oldr.bottom && (
						(oldrect.left >= oldr.left && oldrect.left < oldr.right) ||
						(oldrect.right > oldr.left && oldrect.right <= oldr.right) ||
						(oldr.left >= oldrect.left && oldr.left < oldrect.right) ||
						(oldr.right > oldrect.left && oldr.right <= oldrect.right)
						);
				else
					isdocked=oldrect.left == oldr.right && (
						(oldrect.top >= oldr.top && oldrect.top < oldr.bottom) ||
						(oldrect.bottom > oldr.top && oldrect.bottom <= oldr.bottom) ||
						(oldr.top >= oldrect.top && oldr.top < oldrect.bottom) ||
						(oldr.bottom > oldrect.top && oldr.bottom <= oldrect.bottom)
						);

				if (isdocked)
				{
					if ( x == 0 )
					{
						if ( !whichedge )
							config_wy = newr.bottom;
						else
							config_wx = newr.right;
						
						EstMainWindowRect( &oldrect );
					}
					else if ( x == 1 )
					{
						if ( !whichedge )
							config_eq_wy = newr.bottom;
						else
							config_eq_wx = newr.right;
						
						EstEQWindowRect( &oldrect );
					}
					else if ( x == 2 )
					{
						if ( !whichedge )
							config_pe_wy = newr.bottom;
						else
							config_pe_wx = newr.right;
						
						EstPLWindowRect( &oldrect );
					}
					else if ( x == 4 && !is_fullscreen_video )
					{
						if ( !whichedge )
							config_video_wy = newr.bottom;
						else
							config_video_wx = newr.right;
						
						EstVidWindowRect( &oldrect );
					}
					else
					{
						int w = p->r.right - p->r.left;
						int h = p->r.bottom - p->r.top;

						if ( !whichedge )
							p->r.top = newr.bottom;
						else
							p->r.left = newr.right;

						p->r.right = p->r.left + w;
						p->r.bottom = p->r.top + h;
						oldrect = p->r;
					}

					if ( vis )
						doMyDirtyShitholeDockingShit( dockwnd, oldrect, whichedge );
				}
			}

			if (x>4) p=p->link;      
		}

		if (sedge >= 0) break;
	}
}

static void doScreenDockMoveSubWindows(HWND myWnd, RECT newr, int whichedge)
{
	RECT oldr = {0};
	embedWindowState *p=embedwndlist;

	GetWindowRect(myWnd,&oldr);
	if (myWnd == hMainWindow)
		FixMainWindowRect(&oldr);

	// move any windows docked to the bottom, bottom. recursively.
	for (int x = 0; ; x ++)
	{
		HWND dockwnd=NULL;
		RECT oldrect;
		int vis=0;
		RECT newrect;
		if ( x == 0 )
		{
			dockwnd = hMainWindow;
			vis = config_mw_open;
			EstMainWindowRect( &newrect );
		}
		else if ( x == 1 )
		{
			dockwnd = hEQWindow;
			vis = config_eq_open;
			EstEQWindowRect( &newrect );
		}
		else if ( x == 2 )
		{
			dockwnd = hPLWindow;
			vis = config_pe_open;
			EstPLWindowRect( &newrect );
		}
//else if (x == 3) { dockwnd=hMBWindow; vis=0;/*config_mb_open; */ EstMBWindowRect(&newrect); }
		else if ( x == 4 )
		{
			dockwnd = hVideoWindow;
			vis = config_video_open;
			EstVidWindowRect( &newrect );
		}
		else
		{
			if ( !p )
				break;

			newrect = p->r;
			dockwnd = p->me;
			vis = IsWindowVisible( p->me );
		}
		GetWindowRect(dockwnd,&oldrect);
		if (!x) FixMainWindowRect(&oldrect);

		if (dockwnd != myWnd)
		{
			int isdocked;
			if (!whichedge) 
				isdocked=oldrect.bottom == oldr.top && (
					(oldrect.left >= oldr.left && oldrect.left <= oldr.right) ||
					(oldrect.right >= oldr.left && oldrect.right <= oldr.right) ||
					(oldr.left >= oldrect.left && oldr.left <= oldrect.right) ||
					(oldr.right >= oldrect.left && oldr.right <= oldrect.right)
					);
			else
				isdocked=oldrect.right == oldr.left && (
					(oldrect.top >= oldr.top && oldrect.top <= oldr.bottom) ||
					(oldrect.bottom >= oldr.top && oldrect.bottom <= oldr.bottom) ||
					(oldr.top >= oldrect.top && oldr.top <= oldrect.bottom) ||
					(oldr.bottom >= oldrect.top && oldr.bottom <= oldrect.bottom)
					);

			if (isdocked)
			{
				int h=newrect.bottom-newrect.top;
				int w=newrect.right-newrect.left;
				if ( x == 0 )
				{
					if ( !whichedge )
						config_wy = newr.top - h;
					else
						config_wx = newr.left - w;
					
					EstMainWindowRect( &oldrect );
				}
				else if ( x == 1 )
				{
					if ( !whichedge )
						config_eq_wy = newr.top - h;
					else
						config_eq_wx = newr.left - w;
					
					EstEQWindowRect( &oldrect );
				}
				else if ( x == 2 )
				{
					if ( !whichedge )
						config_pe_wy = newr.top - h;
					else
						config_pe_wx = newr.left - w;
					
					EstPLWindowRect( &oldrect );
				}
				else if ( x == 4 && !is_fullscreen_video )
				{
					if ( !whichedge )
						config_video_wy = newr.top - h;
					else
						config_video_wx = newr.left - w;
					
					EstVidWindowRect( &oldrect );
				}
				else
				{
					int w = p->r.right - p->r.left;
					int h = p->r.bottom - p->r.top;

					if ( !whichedge )
						p->r.top = newr.top - h;
					else
						p->r.left = newr.left - w;

					p->r.right = p->r.left + w;
					p->r.bottom = p->r.top + h;
					oldrect = p->r;
				}

				if (vis)
					doScreenDockMoveSubWindows(dockwnd,oldrect,whichedge);
			}
		}

		if (x>4) p=p->link;      
	}
}

static void doScreenDock()
{
	//RECT vp;
	//SystemParametersInfo(SPI_GETWORKAREA, 0, &vp, 0); //fixme: make sure we keep things in vp :)
	for (int whichedge = 0; whichedge < 2; whichedge++)
	{
		embedWindowState *p=embedwndlist;

		// move any windows docked to the bottom, bottom. recursively.
		for (int x = 0; ; x ++)
		{
			RECT vp;
			HWND dockwnd=NULL;
			RECT oldrect;
			RECT newrect;
			int at2side=0;
			int vis=0;
			if ( x == 0 )
			{
				dockwnd = hMainWindow;
				vis = config_mw_open;
				EstMainWindowRect( &newrect );
			}
			else if ( x == 1 )
			{
				dockwnd = hEQWindow;
				vis = config_eq_open;
				EstEQWindowRect( &newrect );
			}
			else if ( x == 2 )
			{
				dockwnd = hPLWindow;
				vis = config_pe_open;
				EstPLWindowRect( &newrect );
			}
//else if (x == 3) { dockwnd=hMBWindow; vis=0; /*config_mb_open; */ EstMBWindowRect(&newrect); }
			else if ( x == 4 )
			{
				dockwnd = hVideoWindow;
				vis = config_video_open;
				EstVidWindowRect( &newrect );
			}
			else
			{
				if ( !p )
					break;

				newrect = p->r;
				dockwnd = p->me;
				vis = IsWindowVisible( p->me );
			}

			GetWindowRect(dockwnd,&oldrect);

			if (!x)
				FixMainWindowRect(&oldrect);

			getViewport(&vp,NULL,0,&oldrect);

			if (config_keeponscreen&1) 
			{
				if (!whichedge)
					at2side=oldrect.bottom == vp.bottom;
				else
					at2side=oldrect.right == vp.right;
			}

			if (at2side)
			{
				if (!whichedge)
				{
					int diff=vp.bottom-newrect.bottom;
					newrect.bottom += diff;
					newrect.top += diff;
				}
				else
				{
					int diff=vp.right-newrect.right;
					newrect.right += diff;
					newrect.left += diff;
				}

				if ( x == 0 )
				{
					if ( !whichedge )
						config_wy = newrect.top;
					else
						config_wx = newrect.left;
					
					EstMainWindowRect( &oldrect );
				}
				else if ( x == 1 )
				{
					if ( !whichedge )
						config_eq_wy = newrect.top;
					else
						config_eq_wx = newrect.left;
					
					EstEQWindowRect( &oldrect );
				}
				else if ( x == 2 )
				{
					if ( !whichedge )
						config_pe_wy = newrect.top;
					else
						config_pe_wx = newrect.left;
					
					EstPLWindowRect( &oldrect );
				}
//				else if (x == 3) { if (!whichedge) config_mb_wy = newrect.top; else config_mb_wx = newrect.left; EstMBWindowRect(&oldrect); }
				else if ( x == 4 && !is_fullscreen_video )
				{
					if ( !whichedge )
						config_video_wy = newrect.top;
					else
						config_video_wx = newrect.left;
					
					EstVidWindowRect( &oldrect );
				}
				else
				{
					int w = p->r.right - p->r.left;
					int h = p->r.bottom - p->r.top;

					if ( !whichedge )
						p->r.top = newrect.top;
					else
						p->r.left = newrect.left;

					p->r.right = p->r.left + w;
					p->r.bottom = p->r.top + h;
					oldrect = p->r;
				}

				if (vis)
				{
					doScreenDockMoveSubWindows(dockwnd,oldrect,whichedge);
				}
			}

			if (x>4) p=p->link;      
		}
	}
}

void set_eq_wnd_tooltip()
{
	if (hEQTooltipWindow)
	{
		static int b=0;
		TOOLINFOW ti = {0};
		RECT r_main;
		EstEQWindowRect(&r_main);
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hEQWindow;
		ti.rect = r_main;

		ti.uId = (UINT_PTR)hEQTooltipWindow;
		ti.lpszText = (wchar_t*)LPSTR_TEXTCALLBACK;
		if (b) SendMessageW(hEQTooltipWindow,TTM_DELTOOLW,0,(LPARAM) &ti);
		if (config_ttips) SendMessageW(hEQTooltipWindow,TTM_ADDTOOLW,0,(LPARAM) &ti);

		{
			RECT rs[18]={{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
						 // toggle winshade / close button
						 {254,3,262,12},{264,3,272,12},
						 // toggle eq, toggle auto, presets
						 {14,18,39,30},{39,18,72,30},{217,18,261,30},
						 // windowshade volume and pan
						 {61,3,162,11},{163,3,206,11}};
			wchar_t buf[2048] = {0};
			getStringW(IDS_TOOLTIPS,buf,2048);
			wchar_t *bands[11] = 
			{
				L"",
				L"70",L"180",L"320",L"600",		// Hz
				L"1",L"3",L"6",L"12",L"14",L"16"	// KHz
			};
			wchar_t *bandsISO[11] = 
			{
				L"",
				L"31.5",L"63",L"125",L"250",	// Hz
				L"500",L"1",L"2",L"4",L"8",L"16"	// KHz
			};

			for (int x=0; x < sizeof(rs)/sizeof(rs[0]); x++) 
			{
				if(x < 11){
					int xoffs;
					if (!x) xoffs=21;
					else xoffs=78+(x-1)*(96-78);
					rs[x].left = xoffs;
					rs[x].top = 39;
					rs[x].right = xoffs+(33-21);
					rs[x].bottom = 98;
				}

				ti.uId = x;
				if(!x) ti.lpszText = getStringW(IDS_PREAMP,NULL,0);

				else if(x > 15){
					if(config_eq_ws)
					{
						// re-use the main tooltip text and handle the 'toggle windowshade' and 'close' buttons as applicable
						wchar_t buf[2048], *p=buf;
						getStringW(IDS_TOOLTIPS,buf,2048);
						ti.lpszText=(wchar_t*)SendMessageW(hMainWindow,WM_WA_IPC,(x - 4),IPC_CB_GETTOOLTIPW);
						if (!ti.lpszText) ti.lpszText=AutoWide((char*)SendMessageW(hMainWindow,WM_WA_IPC,(x - 4),IPC_CB_GETTOOLTIP));
						if (!ti.lpszText){
							for(int i = 0; i < (x - 4); i++){
								ti.lpszText=p;
								while (p && *p && *p != L'|') p++;
								if (p) *p++=0;
							}
						}
					}
					else
						ti.lpszText = 0;
				}
				else if(x > 12){
					wchar_t buf[2048], *p=buf;
					getStringW(IDS_EQ_TOOLTIPS,buf,2048);
					for(int i = 0; i < x - 12; i++){
						ti.lpszText=p;
						while (p && *p && *p != L'|') p++;
						if (p) *p++=0;
					}
				}
				else if(x > 10){
					// re-use the main tooltip text and handle the 'toggle windowshade' and 'close' buttons as applicable
					wchar_t buf[2048], *p=buf;
					getStringW(IDS_TOOLTIPS,buf,2048);
					ti.lpszText=(wchar_t*)SendMessageW(hMainWindow,WM_WA_IPC,x-11,IPC_CB_GETTOOLTIPW);
					if (!ti.lpszText) ti.lpszText=AutoWide((char*)SendMessageW(hMainWindow,WM_WA_IPC,x-11,IPC_CB_GETTOOLTIP));
					if (!ti.lpszText){
						for(int i = 0; i < (x == 11 ? 1 : 3); i++){
							ti.lpszText=p;
							while (p && *p && *p != L'|') p++;
							if (p) *p++=0;
						}
					}
				}
				else{
					wchar_t HZStr[16] = {0};
					getStringW((x<5+!(config_eq_frequencies==EQ_FREQUENCIES_WINAMP)?IDS_EQ_HZ:IDS_EQ_KHZ),HZStr,16);
					StringCchPrintfW(buf,80,L"%s%s",
									 ((config_eq_frequencies==EQ_FREQUENCIES_WINAMP)?bands[x]:bandsISO[x]),
									 (!x?L"":HZStr));
					ti.lpszText = buf;
				}

				ti.rect = rs[x];
				if (config_dsize)
				{
					ti.rect.left *= 2;
					ti.rect.right *= 2;
					ti.rect.top *= 2;
					ti.rect.bottom *= 2;
				}
				SendMessageW(hEQTooltipWindow,TTM_DELTOOLW,0,(LPARAM) &ti);
				if (config_ttips) SendMessageW(hEQTooltipWindow,TTM_ADDTOOLW,0,(LPARAM) &ti);
			}
		}

		b=config_ttips;
	}
}

void set_vid_wnd_tooltip()
{
	if (hVideoTooltipWindow)
	{
		static int d=0;
		TOOLINFOW ti = {0};
		RECT r_main;
		EstVidWindowRect(&r_main);
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hVideoWindow;
		ti.rect = r_main;

		ti.uId = (UINT_PTR)hVideoTooltipWindow;
		ti.lpszText = (wchar_t*)LPSTR_TEXTCALLBACK;
		if (d) SendMessageW(hVideoTooltipWindow,TTM_DELTOOLW,0,(LPARAM) &ti);
		if (config_ttips) SendMessageW(hVideoTooltipWindow,TTM_ADDTOOLW,0,(LPARAM) &ti);

		{
			RECT rs[]={{0,3,0,12},{9,0,24,0},{25,0,40,0},{41,0,56,0},{57,0,72,0},{73,0,89,0}};
	
			rs[0].left = config_video_width-11;
			rs[0].right = config_video_width-2;

			rs[1].top = rs[2].top = rs[3].top = rs[4].top = rs[5].top = config_video_height-29;
			rs[1].bottom = rs[2].bottom = rs[3].bottom = rs[4].bottom = rs[5].bottom = config_video_height-11;

			wchar_t buf[2048], *p=buf;
			getStringW(IDS_VID_TOOLTIPS,buf,2048);
			for (int x=0; x < sizeof(rs)/sizeof(rs[0]); x++)
			{
				if(!x)
				{
					// re-use the main tooltip text and handle the 'toggle windowshade' and 'close' buttons as applicable
					wchar_t buf2[2048], *p2=buf2;
					getStringW(IDS_TOOLTIPS,buf2,2048);
					for(int i = 0; i < 3; i++){
						ti.lpszText=p2;
						while (p2 && *p2 && *p2 != L'|') p2++;
						if (p2) *p2++=0;
					}
				}
				else
				{
					ti.lpszText=p;
					while (p && *p && *p != L'|') p++;
					if (p) *p++=0;
				}
				ti.uId = x;

				ti.rect = rs[x];
				if (d) SendMessageW(hVideoTooltipWindow,TTM_DELTOOLW,0,(LPARAM) &ti);
				if (config_ttips) SendMessageW(hVideoTooltipWindow,TTM_ADDTOOLW,0,(LPARAM) &ti);
			}
		}

		d=config_ttips;
	}
}

void set_pl_wnd_tooltip()
{
	if (hPLTooltipWindow)
	{
		static int c=0;
		TOOLINFOW ti = {0};
		RECT r_main;
		EstPLWindowRect(&r_main);
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hPLWindow;
		ti.rect = r_main;

		ti.uId = (UINT_PTR)hPLTooltipWindow;
		ti.lpszText = (wchar_t*)LPSTR_TEXTCALLBACK;
		if (c) SendMessageW(hPLWindow,TTM_DELTOOLW,0,(LPARAM) &ti);
		if (config_ttips) SendMessageW(hPLWindow,TTM_ADDTOOLW,0,(LPARAM) &ti);

		{
					   // toggle winshade / close button
			RECT rs[]={{0,3,0,12},{0,3,0,12},
					   // add, rem, sel, misc, list buttons
					   {14,0,36,0},{43,0,65,0},{72,0,94,0},{101,0,123,0},{0},
					   // windowshade mappings - order is out of kilt so it maps more easily to the string
					   // (open, previuos, play, pause, stop, next, time)
					   {0},{0},{0},{0},{0},{0},{0},
					   // up/down scroll buttons
					   {0},{0},
					   // resize horizontally (winshade)
					   /*{0},
					   // playlist item truncated text*/
					   {0}};
	
			rs[0].left = config_pe_width - 11;
			rs[0].right = config_pe_width - 2;

			rs[1].left = config_pe_width - 21;
			rs[1].right = config_pe_width - 12;

			rs[2].top = rs[3].top = rs[4].top = rs[5].top = rs[6].top = (config_pe_height - 30);
			rs[2].bottom = rs[3].bottom = rs[4].bottom = rs[5].bottom = rs[6].bottom = (config_pe_height - 12);

			rs[6].left = config_pe_width - 44;
			rs[6].right = config_pe_width - 22;

			rs[7].top = rs[8].top = rs[9].top = rs[10].top = rs[11].top = rs[12].top = rs[13].top = (config_pe_height - 16);
			rs[7].bottom = rs[8].bottom = rs[9].bottom = rs[10].bottom = rs[11].bottom = rs[12].bottom = rs[13].bottom = (config_pe_height - 8);

			rs[7].left = config_pe_width - 98;
			rs[7].right = config_pe_width - 90;

			rs[8].left = config_pe_width - 144;
			rs[8].right = config_pe_width - 137;
			rs[9].left = config_pe_width - 136;
			rs[9].right = config_pe_width - 127;
			rs[10].left = config_pe_width - 126;
			rs[10].right = config_pe_width - 117;
			rs[11].left = config_pe_width - 116;
			rs[11].right = config_pe_width - 109;
			rs[12].left = config_pe_width - 108;
			rs[12].right = config_pe_width - 100;
			rs[13].left = config_pe_width - 87;
			rs[13].right = config_pe_width - 54;

			wchar_t buf[2048], *p=buf;
			getStringW(IDS_PL_TOOLTIPS,buf,2048);
			for (int x=0; x < sizeof(rs)/sizeof(rs[0]); x++) 
			{
				if(x < 2)
				{
					// re-use the main tooltip text and handle the 'toggle windowshade' and 'close' buttons as applicable
					wchar_t buf2[2048], *p2=buf2;
					getStringW(IDS_TOOLTIPS,buf2,2048);
					for(int i = 0; i < (x == 1 ? 1 : 3); i++){
						ti.lpszText=p2;
						while (p2 && *p2 && *p2 != L'|') p2++;
						if (p2) *p2++=0;
					}
				}
				else if(x == 16){
					if(config_pe_height == 14)
					{
						rs[x].left = config_pe_width - 29;
						rs[x].right = config_pe_width - 22;
						rs[x].top = 3;
						rs[x].bottom = 13;
						ti.lpszText=p;
						while (p && *p && *p != L'|') p++;
						if (p) *p++=0;
					}
					else
						ti.lpszText = 0;
				}
				else if(x >= 14)
				{
					rs[x].left = config_pe_width - 16;
					rs[x].right = config_pe_width - 6;
					rs[x].top = config_pe_height - (x == 14 ? 37 : 31);
					rs[x].bottom = config_pe_height - (x == 14 ? 32 : 25);

					ti.lpszText=p;
					while (p && *p && *p != L'|') p++;
					*p++=0;
				}
				else if(x > 6)
				{
					if (config_pe_height != 14)
					{
						wchar_t buf2[2048], *p2=buf2;
						getStringW(IDS_TOOLTIPS,buf2,2048);
						for(int i = 0; i < 17 + (x - 6); i++){
							ti.lpszText=p2;
							while (p2 && *p2 && *p2 != L'|') p2++;
							*p2++=0;
						}
					}
					else
						ti.lpszText = 0;
				}
				else
				{
					ti.lpszText=p;
					while (p && *p && *p != L'|') p++;
					if (p) *p++=0;
					if (config_pe_height == 14 && x > 1)
						ti.lpszText = 0;
				}
				ti.uId = x;

				ti.rect = rs[x];
				if (c) SendMessageW(hPLTooltipWindow,TTM_DELTOOLW,0,(LPARAM) &ti);
				if (config_ttips) SendMessageW(hPLTooltipWindow,TTM_ADDTOOLW,0,(LPARAM) &ti);
			}
		}

		c=config_ttips;
	}
}

void set_main_wnd_tooltip()
{
	if (hTooltipWindow)
	{
		static int a=0;
		TOOLINFOW ti = {0};
		RECT r_main;
		EstMainWindowRect(&r_main);
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hMainWindow;
		ti.rect = r_main;

		ti.uId = (UINT_PTR)hTooltipWindow;
		ti.lpszText = (wchar_t*)LPSTR_TEXTCALLBACK;
		if (a) SendMessageW(hTooltipWindow,TTM_DELTOOLW,0,(LPARAM) &ti);
		if (config_ttips) SendMessageW(hTooltipWindow,TTM_ADDTOOLW,0,(LPARAM) &ti);

		{
			RECT rs[]={{254,3,262,12},{244,3,253,12},{264,3,272,12},{5,3,17,13},
			{219,58,219+23,58+12},{219+23,58,219+23+23,58+12},
			{11,24,19,31},{11,32,19,39},{11,40,19,47},{11,48,19,55},{11,56,19,62},
			{107,43+15,107+68,51+15},{177,43+15,177+38,51+15},{11+7,58+15,256+7,66+15},{133+7-4+29,75+15,181+7-4-4+29,87+15},{182+7-4-4+29,75+15,182+29+7-4-4+29,87+15},
			{246+7,76+15,258+7,90+15},{132+7-4+1,60+14+15,132+7-4+22+1,60+14+15+16},
			{7+8,15+72,7+31,15+91},{7+32,15+72,7+53,15+91},{7+54,15+72,7+77,15+91},{7+78,15+72,7+100,15+91},{7+101,15+72,7+123,15+91},
			{36,26,96,39},{105,24,266,35},
			// windowshade mappings - order is out of kilt so it maps more easily to the string
			// (seeker, open, previuos, play, pause, stop, next, time)
			{226,4,243,11},{216,2,224,11},{168,2,176,11},{177,2,186,11},{187,2,195,11},{196,2,204,11},{205,2,215,11},{125,4,157,10}};

			wchar_t buf[2048], *p=buf;
			getStringW(IDS_TOOLTIPS,buf,2048);
			for (int x=0; x < sizeof(rs)/sizeof(rs[0]); x++)
			{
				ti.uId = x;
				ti.lpszText=(wchar_t*)SendMessageW(hMainWindow,WM_WA_IPC,x,IPC_CB_GETTOOLTIPW);
				if (!ti.lpszText) ti.lpszText=AutoWide((char*)SendMessageW(hMainWindow,WM_WA_IPC,x,IPC_CB_GETTOOLTIP));
				if (!ti.lpszText) ti.lpszText=p;

				int ws_parts = sizeof(rs)/sizeof(rs[0]) - 8;
				if(x < ws_parts)
				{
					while (p && *p && *p != L'|') p++;
					if (p) *p++=0;
				}
				else
				{
					if(config_windowshade)
					{
						getStringW(IDS_TOOLTIPS,buf,2048);
						p=buf;
						for(int i = 0; i < (x - ws_parts == 0 ? 14 : x - 8); i++){
							ti.lpszText=p;
							while (p && *p && *p != L'|') p++;
							if (p) *p++=0;
						}
					}
					else
						ti.lpszText = 0;
				}
				ti.rect = rs[x];
				if (config_dsize)
				{
					ti.rect.left *= 2;
					ti.rect.right *= 2;
					ti.rect.top *= 2;
					ti.rect.bottom *= 2;
				}
				if (a) SendMessageW(hTooltipWindow,TTM_DELTOOLW,0,(LPARAM) &ti);
				if (config_ttips) SendMessageW(hTooltipWindow,TTM_ADDTOOLW,0,(LPARAM) &ti);
			}
		}

		a=config_ttips;
	}
}

void set_aot(int dodockingstuff) 
{
	// do size handling stuff here
	if (dodockingstuff>0) 
	{
	    //new mode is inspired by christophe's sexiness
		RECT newr;
		EnterCriticalSection(&embedcs);
		EstMainWindowRect(&newr);
		doMyDirtyShitholeDockingShit(hMainWindow,newr,-1);
		EstEQWindowRect(&newr);
		doMyDirtyShitholeDockingShit(hEQWindow,newr,-1);
		EstPLWindowRect(&newr);
		doMyDirtyShitholeDockingShit(hPLWindow,newr,-1);

		// check to see if any windows used to be docked to the right/bottom, and 
		// if they did, move them to the bottom and any attached windows

		// this actually works OK!
		if ((config_keeponscreen&1) && !g_fsapp) doScreenDock();

		LeaveCriticalSection(&embedcs);
	}

	set_eq_wnd_tooltip();
	set_vid_wnd_tooltip();
	set_pl_wnd_tooltip();
	set_main_wnd_tooltip();

	CheckMenuItem(main_menu,WINAMP_OPTIONS_AOT,config_aot?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(main_menu,WINAMP_OPTIONS_DSIZE,config_dsize?MF_CHECKED:MF_UNCHECKED);

	if ((config_keeponscreen&1) && !g_fsapp) 
	{
		RECT r;
		if (config_mw_open)
		{
			RECT thisr;
			EstMainWindowRect( &thisr );
			getViewport( &r, NULL, 0, &thisr );

			if ( config_wx + ( thisr.right - thisr.left - 10 ) < r.left )
				config_wx = r.left - ( thisr.right - thisr.left - 10 );

			if ( config_wy + ( thisr.bottom - thisr.top - 10 ) < r.top )
				config_wy = r.top - ( thisr.bottom - thisr.top - 10 );

			if (config_wx > r.right - 10)
				config_wx = r.left + 10; //config_wx = r.right - 10;

			if (config_wy > r.bottom - 10)
				config_wy = r.top + 10; //config_wy = r.bottom - 10;
		}

		if (config_eq_open)
		{
			RECT thisr;
			EstEQWindowRect( &thisr );
			getViewport( &r, NULL, 0, &thisr );

			if ( config_eq_wx + ( thisr.right - thisr.left - 10 ) < r.left )
				config_eq_wx = r.left - ( thisr.right - thisr.left - 10 );

			if ( config_eq_wy + ( thisr.bottom - thisr.top - 10 ) < r.top )
				config_eq_wy = r.top - ( thisr.bottom - thisr.top - 10 );

			if (config_eq_wx > r.right - 10)
				config_eq_wx = r.left + 10;  //r.right - 10;

			if (config_eq_wy > r.bottom - 10)
				config_eq_wy = r.top + 10; //r.bottom - 10;
		}

		if (config_pe_open)
		{
			RECT thisr;
			EstPLWindowRect( &thisr );
			getViewport( &r, NULL, 0, &thisr );

			if ( config_pe_wx + ( thisr.right - thisr.left - 10 ) < r.left )
				config_pe_wx = r.left - ( thisr.right - thisr.left - 10 );

			if ( config_pe_wy + ( thisr.bottom - thisr.top - 10 ) < r.top )
				config_pe_wy = r.top - ( thisr.bottom - thisr.top - 10 );

			if (config_pe_wx > r.right - 10)
				config_pe_wx = r.left + 10; //r.right - 10;

			if (config_pe_wy > r.bottom - 10)
				config_pe_wy = r.top + 10; //r.bottom - 10;
		}

		if (config_video_open && !is_fullscreen_video)
		{
			RECT thisr;
			EstVidWindowRect( &thisr );
			getViewport( &r, NULL, 0, &thisr );

			if ( config_video_wx + ( thisr.right - thisr.left - 10 ) < r.left )
				config_video_wx = r.left - ( thisr.right - thisr.left - 10 );

			if ( config_video_wy + ( thisr.bottom - thisr.top - 10 ) < r.top )
				config_video_wy = r.top - ( thisr.bottom - thisr.top - 10 );

			if (config_video_wx > r.right - 10)
				config_video_wx = r.left + 10; //r.right - 10;

			if (config_video_wy > r.bottom - 10)
				config_video_wy = r.top + 10; // r.bottom - 10;
		}
		EnterCriticalSection(&embedcs);
		{
			embedWindowState *p=embedwndlist;
			while (p)
			{
				if (IsWindowVisible(p->me))
				{
					RECT thisr = p->r;
					getViewport( &r, NULL, 0, &thisr );
					if ( thisr.right < r.left - 10 )
						p->r.left = r.left - ( thisr.right - thisr.left - 10 );

					if ( thisr.bottom < r.top - 10 )
						p->r.top = r.top - ( thisr.bottom - thisr.top - 10 );

					if (thisr.left > r.right - 10)
						p->r.left = r.left + 10; // r.right - 10;

					if (thisr.top > r.bottom - 10)
						p->r.top = r.top + 10; // r.bottom - 10;

					p->r.right = p->r.left + (thisr.right - thisr.left);
					p->r.bottom = p->r.top + (thisr.bottom - thisr.top);
				}
				p=p->link;
			}
		}
		LeaveCriticalSection(&embedcs);
	}

	{
	    HDWP hwdp=BeginDeferWindowPos(10);
		RECT r;
		int do_pl_inv=0;
		extern int g_fsapp;

		EstMainWindowRect(&r);
		if (config_aot)
		{
			if (g_fsapp)
			{
				if (config_dropaotfs)
				{
					hwdp=DeferWindowPos(hwdp,hMainWindow, HWND_NOTOPMOST, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOACTIVATE|(config_mw_open?0:SWP_NOMOVE|SWP_NOSIZE));
				}
				else
				{
					hwdp=DeferWindowPos(hwdp,hMainWindow, HWND_TOPMOST, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOACTIVATE|(config_mw_open?0:SWP_NOMOVE|SWP_NOSIZE));
				}
			}
			else
			{
				hwdp=DeferWindowPos(hwdp,hMainWindow, HWND_TOPMOST, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOACTIVATE|(config_mw_open?0:SWP_NOMOVE|SWP_NOSIZE));
			}
		}
		else
		{
			hwdp=DeferWindowPos(hwdp,hMainWindow, HWND_NOTOPMOST, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOACTIVATE|(config_mw_open?0:SWP_NOMOVE|SWP_NOSIZE));
		}

		if (hEQWindow && !GetParent(hEQWindow)) // we check to see if the windows have no parent, for the gen_ff
		{
			EstEQWindowRect(&r);
			hwdp=DeferWindowPos(hwdp,hEQWindow, 0, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOACTIVATE|SWP_NOZORDER);
		}

		if (hPLWindow && !GetParent(hPLWindow))
		{
			EstPLWindowRect(&r);
			hwdp=DeferWindowPos(hwdp,hPLWindow, 0, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOACTIVATE|SWP_NOZORDER);
			do_pl_inv=1;
		}

/*		if (hMBWindow && !GetParent(hMBWindow))
		{
			EstMBWindowRect(&r);
			hwdp=DeferWindowPos(hwdp,hMBWindow, 0, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_DRAWFRAME|SWP_NOACTIVATE|SWP_NOZORDER);
		}*/

		if (hVideoWindow && !GetParent(hVideoWindow) && !is_fullscreen_video)
		{
			EstVidWindowRect(&r);
			hwdp=DeferWindowPos(hwdp,hVideoWindow, 0, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOCOPYBITS|SWP_NOACTIVATE|SWP_NOZORDER);
		}

		// traverse the embed windows
		EnterCriticalSection(&embedcs);
		{
			embedWindowState *p=embedwndlist;
			while (p)
			{
				if (!HasParent(p->me))
					hwdp=DeferWindowPos(hwdp,p->me, 0, p->r.left, p->r.top, p->r.right-p->r.left, p->r.bottom-p->r.top, SWP_DRAWFRAME|SWP_NOACTIVATE|SWP_NOZORDER);

				p=p->link;
			}
		}
		LeaveCriticalSection(&embedcs);

		EndDeferWindowPos(hwdp);

		if(do_pl_inv)
			InvalidateRect(hPLWindow,NULL,FALSE); // fix pl bug?
	}

	// region
	if (!config_minimized && config_mw_open) 
	{
		HRGN lasthrgn=NULL;
		int *plist=0, *nlist=0, len=0;

		if (skin_directory[0])
			len = Skin_GetRegionPointList(0,&plist,&nlist);
		else
			len=0;

		if (len)
		{
			POINT *points;
			int x;
			int a=config_dsize?1:0;
			int ma=0;
			for (x=0; x < len; x ++)
				ma+=nlist[x];
			points= (POINT *) GlobalAlloc(GPTR,sizeof(POINT) * ma);
			for (x = 0; x < ma; x ++)
			{
				points[x].x = *plist++<<a;
				points[x].y = *plist++<<a;
			}
			lasthrgn = CreatePolyPolygonRgn(points,nlist,len,WINDING);
			GlobalFree(points);
		}
		else
		{
			RECT r;
			EstMainWindowRect(&r);
			lasthrgn=CreateRectRgn(0,0,r.right-r.left,r.bottom-r.top);
		}
		if (lasthrgn) SetWindowRgn(hMainWindow,lasthrgn,TRUE);
	  else SetWindowRgn(hMainWindow,NULL,FALSE);	
	}

	// region
	if (!config_minimized && config_eq_open && hEQWindow) 
	{
		int *plist, *nlist, len = Skin_GetRegionPointList(1,&plist,&nlist);
		if (len)
		{
			POINT *points;
			int x;
			int a=config_dsize?1:0;
			int ma=0;
			for (x=0; x < len; x ++)
				ma+=nlist[x];
			points= (POINT *) GlobalAlloc(GPTR,sizeof(POINT) * ma);
			for (x = 0; x < ma; x ++)
			{
				points[x].x = *plist++<<a;
				points[x].y = *plist++<<a;
			}
			HRGN lasthrgn = CreatePolyPolygonRgn(points,nlist,len,WINDING);
			GlobalFree(points);
			SetWindowRgn(hEQWindow,lasthrgn,TRUE);
		}
		else SetWindowRgn(hEQWindow,NULL,FALSE);
	}

	// region
	/*if (!config_minimized && config_pe_open && hPLWindow) 
	{
		int *plist, *nlist, len = Skin_GetRegionPointList(2,&plist,&nlist);
		if (len)
		{
			POINT *points;
			int x;
			int a=config_dsize?1:0;
			int ma=0;
			for (x=0; x < len; x ++)
				ma+=nlist[x];
			points= (POINT *) GlobalAlloc(GPTR,sizeof(POINT) * ma);
			for (x = 0; x < ma; x ++)
			{
				points[x].x = *plist++<<a;
				points[x].y = *plist++<<a;
			}
			HRGN lasthrgn = CreatePolyPolygonRgn(points,nlist,len,WINDING);
			if (config_pe_height != 14)
			{
				HRGN allhrng = CreateRectRgn(0, 0, config_pe_width, config_pe_height);
				CombineRgn(lasthrgn, lasthrgn, allhrng, RGN_XOR);
				DeleteRgn(allhrng);
			}
			GlobalFree(points);
			SetWindowRgn(hPLWindow,lasthrgn,TRUE);
		}
		else SetWindowRgn(hPLWindow,NULL,FALSE);
	}*/

	// refresh windows
	if (config_mw_open) InvalidateRect(hMainWindow,NULL,FALSE);
	if (hEQWindow) InvalidateRect(hEQWindow,NULL,FALSE);
	
	if (hVisWindow)
	{
		int x,y,w,h;
		if (config_windowshade)
		{
			x=79;
			y=5;
			w=38;
			h=5;
		}
		else
		{
			x=24;
			y=43;
			w=76;
			h=16;
		}
		if (config_dsize) 
		{
			x*=2;
			y*=2;
			w*=2;
			h*=2;
		}
		SetWindowPos(hVisWindow,0,x,y,w,h,SWP_NOZORDER|SWP_NOACTIVATE);
	}

	if (hPLVisWindow)
	{
		int x,y,w,h;
		x=config_pe_width-150-75+2;
		y=config_pe_height-26;
		w=(config_pe_width >= 350 && config_pe_height != 14 ? 72 : 0);
		h=16;
		SetWindowPos(hPLVisWindow,0,x,y,w,h,SWP_NOZORDER|SWP_NOACTIVATE);
	}

	draw_clutterbar(0);
	SendMessageW(hMainWindow, WM_WA_IPC, config_aot, IPC_CB_ONTOGGLEAOT);
}

void set_visopts(void)
{
	if (prefs_last_page == 22 && IsWindow(prefs_hwnd))
	{
		prefs_last_page=0;
		prefs_dialog(1);
		prefs_last_page=22;
		prefs_dialog(1);
	}
}

void set_priority(void)
{
	int tab[5]={IDLE_PRIORITY_CLASS,NORMAL_PRIORITY_CLASS,ABOVE_NORMAL_PRIORITY_CLASS,HIGH_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS};
	if (config_priority > 4) config_priority=1;
	SetPriorityClass(GetCurrentProcess(),tab[config_priority]);
}