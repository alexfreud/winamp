/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "Main.h"
#include <malloc.h>
#include "api.h"
#include "resource.h"

int minimize_hack_winamp;

#define inreg(x,y,x2,y2) \
	((mouse_x <= ( x2 ) && mouse_x >= ( x ) &&  \
	mouse_y <= ( y2 ) && mouse_y >= ( y )))

static int mouse_x, mouse_y, mouse_type, mouse_stats;

static int do_titlebar_clicking;

static int do_buttonbar();
static int do_shuffle();
static int do_peeq();
static int do_repeat();
static int do_eject();
static int do_icon();
static int do_posbar();
static int do_volbar();
static int do_panbar();
static int do_songname();
static int do_titlebar();
static int do_titlebuttons();
static int do_timedisplay();
static int do_clutterbar();

static BOOL HasParent(HWND hwnd)
{
	return ((0 != (WS_CHILD & GetWindowLongPtrW(hwnd, GWL_STYLE))) && NULL != GetParent(hwnd));
}

void ui_handlemouseevent(int x, int y, int type, int stats) 
{
	mouse_x = x;
	mouse_y = y;
	mouse_type = type;
	mouse_stats = stats;
	if (do_titlebar_clicking || (!do_posbar() &&
		!do_volbar() && !do_panbar() && !do_songname())) 	
	{
		if (do_titlebar_clicking || !(do_buttonbar()+
			do_shuffle()+
			do_repeat()+
			do_eject()+
			do_peeq()+
			do_icon()+
			do_clutterbar()+
			do_timedisplay()))
		{	
			if (do_titlebar_clicking || !do_titlebuttons())	do_titlebar();
		}
	}
}

static int __do_buttons(int which)
{        
	int m = WINAMP_BUTTON1 + which;
	if (which == 5)
	{
		if (mouse_stats & MK_CONTROL) SendMessageW(hMainWindow,WM_COMMAND,WINAMP_FILE_LOC,0);
		else if (mouse_stats & MK_SHIFT) SendMessageW(hMainWindow,WM_COMMAND,WINAMP_FILE_DIR,0);
		else SendMessageW(hMainWindow,WM_COMMAND,WINAMP_FILE_PLAY,0);
	}
	else
	{
		if (mouse_stats & MK_SHIFT) m += 100;
		else if (mouse_stats & MK_CONTROL) m += 110;
		SendMessageW(hMainWindow,WM_COMMAND,m,0);
	}
	return 0;
}

static int do_clutterbar_active;
static int do_clutterbar()
{
	int en=0,t=0;

	if (mouse_type == -2)
		return 0;

	if (inreg(11,24,19,31))
	{
		en=1;
		if (mouse_type == -1)
		{
			POINT p = {14,25};
			extern HMENU g_submenus_options;
			HMENU hmenu = g_submenus_options;
			if (config_dsize)
			{
				p.x *= 2;
				p.y *= 2;
			}
			ClientToScreen(hMainWindow,&p);
			DoTrackPopup(hmenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON, p.x, p.y, hMainWindow);
		}
		if (mouse_stats & MK_LBUTTON)
		{
		wchar_t songnameStr[128] = {0};
			draw_songname(getStringW(IDS_OPTIONS_MENU,songnameStr,128),&t,-1);
		}
	}
	if (inreg(11,32,19,39))
	{
		en=2;
		if (mouse_type == -1) SendMessageW(hMainWindow,WM_COMMAND,WINAMP_OPTIONS_AOT,0);
		if (mouse_stats & MK_LBUTTON) 
		{
			wchar_t songnameStr[128] = {0};
			if (config_aot)
				draw_songname(getStringW(IDS_DISABLE_AOT,songnameStr,128),&t,-1);
			else
				draw_songname(getStringW(IDS_ENABLE_AOT,songnameStr,128),&t,-1);
		}
	}
	if (inreg(11,40,19,47))
	{
		en=3;
		if (mouse_type == -1) 
		{
			if (FileName[0]) in_infobox(hMainWindow,FileName);
		}
		if (mouse_stats & MK_LBUTTON)
		{
			wchar_t songnameStr[128] = {0};
			draw_songname(getStringW(IDS_FILE_INFO_BOX,songnameStr,128),&t,-1);
		}
	}
	if (inreg(11,48,19,55))
	{
		en=4;
		if (mouse_type == -1) SendMessageW(hMainWindow,WM_COMMAND,WINAMP_OPTIONS_DSIZE,0);
		if (mouse_stats & MK_LBUTTON) 
		{
			wchar_t songnameStr[128] = {0};
			if (config_dsize) draw_songname(getStringW(IDS_DISABLE_DOUBLESIZE_MODE,songnameStr,128),&t,-1);
			else draw_songname(getStringW(IDS_ENABLE_DOUBLESIZE_MODE,songnameStr,128),&t,-1);
		}
	}
	if (inreg(11,56,19,62))
	{
		en=5;
		if (mouse_type == -1) 
		{
			POINT p={14,56};
			extern HMENU g_submenus_vis;
			if (config_dsize) {p.x*=2;p.y*=2;}
			ClientToScreen(hMainWindow,&p);
			DoTrackPopup(g_submenus_vis, TPM_LEFTALIGN|TPM_RIGHTBUTTON, p.x, p.y, hMainWindow);
		}
		if (mouse_stats & MK_LBUTTON)
		{
			wchar_t songnameStr[128] = {0};
			draw_songname(getStringW(IDS_VISUALIZATION_MENU,songnameStr,128),&t,-1);
		}
	}
	if (inreg(9,20,19,65) && mouse_stats & MK_LBUTTON)
	{
		draw_clutterbar(1+en);
		do_clutterbar_active=1;
		return 1;
	}
	if (do_clutterbar_active || mouse_type == -1)
	{
		draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
		do_clutterbar_active=0;
		draw_clutterbar(0);
	}
	return 0;
}

static int do_timedisplay() 
{ 
	if (mouse_type == -2)
		return 0;
	if (((!config_windowshade && inreg(36,26,96,39)) || 
		(config_windowshade && inreg(129,3,129+28,3+6))
		) && mouse_type == 1)
	{
		config_timeleftmode = !config_timeleftmode;
		if (!config_timeleftmode)
		{
			CheckMenuItem(main_menu,WINAMP_OPTIONS_ELAPSED,MF_CHECKED);
			CheckMenuItem(main_menu,WINAMP_OPTIONS_REMAINING,MF_UNCHECKED);
		} 
		else 
		{
			CheckMenuItem(main_menu,WINAMP_OPTIONS_ELAPSED,MF_UNCHECKED);
			CheckMenuItem(main_menu,WINAMP_OPTIONS_REMAINING,MF_CHECKED);
		}
		SendMessageW(hMainWindow,WM_TIMER,UPDATE_DISPLAY_TIMER+4,0);
		return 1;
	}
	if (playing && inreg(27,40,99,61) && mouse_type == 1)
	{
		config_sa++;
		if (config_sa > 2) config_sa = 0;
		set_visopts();
		sa_setthread(config_sa);
		return 1;
	}
	if (config_windowshade && inreg(78,4,116,11) && mouse_type == 1)
	{
		config_sa++;
		if (config_sa > 2) config_sa = 0;
		set_visopts();
		sa_setthread(config_sa);
		return 1;
	}
	if (config_windowshade && inreg(168,2,213+11,11) && mouse_type == 1)
	{
		int which = 5 - (mouse_x < 215) - (mouse_x < 204) - (mouse_x < 195) - (mouse_x < 186) - (mouse_x < 176);
		__do_buttons(which);
		return 1;
	}
	return 0;
}

static int do_titlebar() 
{
	if (mouse_type == -2)
		return 0;
	if (do_titlebar_clicking || config_easymove || mouse_y < 14)
	{
		static int clickx, clicky;
		switch (mouse_type)
		{
		case 1:
			if (!do_titlebar_clicking)
			{
				EnterCriticalSection(&embedcs);
				{
					typedef struct 
					{
						int open;
						int at;
						RECT r;
						embedWindowState *isemb;
					} rec;
					int x;
					int cnt = 5+embedwndlist_cnt;        
					embedWindowState *p=embedwndlist;
					rec *a = (rec *)_malloca(cnt * sizeof(rec));
					//rec *a = (rec*)malloc(cnt * sizeof(rec));
					memset(a,0,cnt * sizeof(rec));

					a[0].at=1;
					a[0].open=1;
					EstMainWindowRect(&a[0].r);
					EstEQWindowRect(&a[1].r);a[1].open=config_eq_open;
					EstPLWindowRect(&a[2].r);a[2].open=config_pe_open;
					//EstMBWindowRect(&a[3].r);a[3].open=config_mb_open;
					EstVidWindowRect(&a[4].r);a[4].open=config_video_open;

					for (x = 5; x < cnt && p; x ++)
					{
						a[x].open=IsWindowVisible(p->me) && !HasParent(p->me);
						a[x].isemb=p;
						a[x].r = p->r;
						p->attached=0;
						p = p->link;
					}

					x=0;

					do_titlebar_clicking=1;

					if ((!!config_snap) ^ (!!(mouse_stats & MK_SHIFT)))
					{
						int t = 0;
						for (;;)
						{
							if (a[x].open&&a[x].at) for (int b = 0; b < cnt; b ++)
							{
								if (b != x && !a[b].at && IsWindowAttached(a[x].r,a[b].r))
								{
									a[b].at=1;
									t=1;
								}
							}
							if (!x)
							{
								if (!t) break;
								t=0;
							}
							x++;
							x%=cnt;
						}
						if (a[1].at) do_titlebar_clicking|=2;
						if (a[2].at) do_titlebar_clicking|=4;
						if (a[3].at) do_titlebar_clicking|=8;
						if (a[4].at) do_titlebar_clicking|=16;
						for (x = 5; x < cnt && a[x].isemb; x ++) 
							a[x].isemb->attached = a[x].at;
					}

					_freea(a);
				}
				LeaveCriticalSection(&embedcs);
				clickx=mouse_x;
				clicky=mouse_y;
			}
			return 1;
		case -1:
			if (do_titlebar_clicking)
			{
				do_titlebar_clicking=0;	
			}
			return 1;
		case 0:
			if (do_titlebar_clicking)
			{
				RECT eqr,plr,mwr,mbr,vwr,omwr;
				embedWindowState *p;
				int skip_eq = GetParent(hEQWindow) != NULL;
				//int skip_mb = 1;//GetParent(hMBWindow) != NULL;
				int skip_pl = GetParent(hPLWindow) != NULL;
				int skip_vw = GetParent(hVideoWindow) != NULL;

				EnterCriticalSection(&embedcs);

				EstEQWindowRect(&eqr);
				//EstMBWindowRect(&mbr);
				EstPLWindowRect(&plr);
				EstVidWindowRect(&vwr);
				EstMainWindowRect(&mwr);

				omwr=mwr;
				MoveRect(&mwr,mouse_x-clickx,mouse_y-clicky);

				// snap to nondocked windows
				if ((!!config_snap) ^ (!!(mouse_stats & MK_SHIFT)))
				{
					embedWindowState *state=embedwndlist;
					SnapToScreen(&mwr);
					if (!skip_eq && config_eq_open && !(do_titlebar_clicking&2)) SnapWindowToWindow(&mwr,eqr);
					if (!skip_pl && config_pe_open && !(do_titlebar_clicking&4)) SnapWindowToWindow(&mwr,plr);
//					if (!skip_mb && config_mb_open && !(do_titlebar_clicking&8)) SnapWindowToWindow(&mwr,mbr);
					if (!skip_vw && config_video_open && !(do_titlebar_clicking&16)) SnapWindowToWindow(&mwr,vwr);

					while (state)
					{
						if (IsWindowVisible(state->me) && !HasParent(state->me) && !state->attached) 
							SnapWindowToWindow(&mwr,state->r);
						state=state->link;
					}
				}


				// move docked windows the same amount the the main window has moved
				{
					embedWindowState *state=embedwndlist;
					int movex=mwr.left-omwr.left;
					int movey=mwr.top-omwr.top;
					if (do_titlebar_clicking&2) MoveRect(&eqr,movex,movey);
					if (do_titlebar_clicking&4) MoveRect(&plr,movex,movey);
					if (do_titlebar_clicking&8) MoveRect(&mbr,movex,movey);
					if (do_titlebar_clicking&16) MoveRect(&vwr,movex,movey);
					while (state)
					{
						if (!HasParent(state->me) && state->attached) MoveRect(&state->r,movex,movey);
						state=state->link;
					}
				}

				if ((!!config_snap) ^ (!!(mouse_stats & MK_SHIFT)))
				{
					// dock the attached windows to the screen
					if (config_keeponscreen&1) 
					{
						RECT omwr;
						int x;
						p = embedwndlist;
						for (x=0;;x++) // snap to screen
						{
							int op=0;
							int mask=1<<(x+1);
							intptr_t isact=(do_titlebar_clicking & mask);
							int m=do_titlebar_clicking&~mask;
							RECT *r=NULL;
							if (x == 0) {op = config_eq_open && !skip_eq; r=&eqr; }
							else if (x == 1) { op = config_pe_open && !skip_pl; r=&plr; }
//							else if (x == 2) { op = config_mb_open && !skip_mb; r=&mbr; }
							else if (x == 3) { op = config_video_open && !skip_vw; r=&vwr; }
							else
							{
								if (!p) break;
								op=IsWindowVisible(p->me) && !HasParent(p->me);
								isact=p->attached;
								m=do_titlebar_clicking;
								r=&p->r;
								p =p->link;
							}

							if (op && isact)
							{
								embedWindowState *p2 = embedwndlist;
								RECT oldr=*r;
								SnapToScreen(r);
								MoveRect(&mwr,r->left-oldr.left,r->top-oldr.top);

								if (m & 2) MoveRect(&eqr,r->left-oldr.left,r->top-oldr.top);
								if (m & 4) MoveRect(&plr,r->left-oldr.left,r->top-oldr.top);
								if (m & 8) MoveRect(&mbr,r->left-oldr.left,r->top-oldr.top);
								if (m & 16) MoveRect(&vwr,r->left-oldr.left,r->top-oldr.top);
								while (p2)
								{
									if (&p2->r != r && p2->attached)
									{
										MoveRect(&p2->r,r->left-oldr.left,r->top-oldr.top);
									}
									p2=p2->link;
								}
							}
						} // each window docking to screen

						omwr=mwr;
						SnapToScreen(&mwr);
						if (do_titlebar_clicking & 2) MoveRect(&eqr,mwr.left-omwr.left,mwr.top-omwr.top);
						if (do_titlebar_clicking & 4) MoveRect(&plr,mwr.left-omwr.left,mwr.top-omwr.top);
						if (do_titlebar_clicking & 8) MoveRect(&mbr,mwr.left-omwr.left,mwr.top-omwr.top);
						if (do_titlebar_clicking & 16) MoveRect(&vwr,mwr.left-omwr.left,mwr.top-omwr.top);
						p = embedwndlist;
						while (p)
						{
							if (p->attached)
							{
								MoveRect(&p->r,mwr.left-omwr.left,mwr.top-omwr.top);
							}
							p=p->link;
						}
					} // kepponscreen&1


					// dock the attached windows to the non attached windows
					{
						int x;
						p = embedwndlist;
						for (x = 0;; x ++)
						{
							int op=0;
							int mask=1<<(x+1);
							intptr_t isact=do_titlebar_clicking & mask;
							int m = do_titlebar_clicking&~mask;
							RECT *r=NULL;
							if (x == 0) { op = config_eq_open; r=&eqr; }
							else if (x == 1) { op = config_pe_open; r=&plr; }
//							else if (x == 2) { op = config_mb_open; r=&mbr; }
							else if (x == 3) { op = config_video_open; r=&vwr; }
							else
							{
								if (!p) break;
								op=IsWindowVisible(p->me) && !HasParent(p->me);
								isact=p->attached;
								m=do_titlebar_clicking;
								r=&p->r;
								p =p->link;
							}

							if (!isact && op) // if this isn't docked
							{
								int x2;
								embedWindowState *p2 = embedwndlist;
								for (x2=0;; x2 ++)
								{
									int op2;
									RECT *r2;
									int mask2=1<<(x2+1);
									int isa2=(m & mask2);
									if (x2 == 0) { op2 = config_eq_open; r2=&eqr; }
									else if (x2 == 1) { op2 = config_pe_open; r2=&plr; }
//									else if (x2 == 2) { op2 = config_mb_open; r2=&mbr; }
									else if (x2 == 3) { op2 = config_video_open; r2=&vwr; }
									else
									{
										if (!p2) break;
										op2=IsWindowVisible(p2->me) && !HasParent(p2->me);
										isa2=&p2->r != r && p2->attached;
										r2=&p2->r;
										p2 =p2->link;
									}

									if (op2 && isa2)
									{
										embedWindowState *p3 = embedwndlist;
										RECT r4=*r2,r3=*r2;
										RECT omwr=mwr;
										SnapWindowToWindow(&r4,*r);
										MoveRect(&mwr,r4.left-r3.left,r4.top-r3.top);
										if (m & 2) MoveRect(&eqr,mwr.left-omwr.left,mwr.top-omwr.top);
										if (m & 4) MoveRect(&plr,mwr.left-omwr.left,mwr.top-omwr.top);
										if (m & 8) MoveRect(&mbr,mwr.left-omwr.left,mwr.top-omwr.top);
										if (m & 16) MoveRect(&vwr,mwr.left-omwr.left,mwr.top-omwr.top);
										while (p3)
										{
											if (&p3->r != r && p3->attached)
											{
												MoveRect(&p3->r,mwr.left-omwr.left,mwr.top-omwr.top);
											}
											p3=p3->link;
										}
									}
								}
							}
						}
					}				// dock to nonattached
				} // if snapping

				{
					// calculate number of windows to move
					int deferred_move_count = 1;
					if (do_titlebar_clicking&2 && !skip_eq) deferred_move_count ++;// final EQ positioning
					if (do_titlebar_clicking&4 && !skip_pl) deferred_move_count ++;// final PE positioning
					if (do_titlebar_clicking&16 && !skip_vw) deferred_move_count ++;// final VW positioning

					p = embedwndlist;
					while (p)
					{
						if (p->attached)	deferred_move_count ++;
						p=p->link;
					}

					HDWP hdwp=BeginDeferWindowPos(deferred_move_count);

					if (do_titlebar_clicking&2 && !skip_eq) // final EQ positioning
					{
						SetEQWindowRect(&eqr);
						hdwp=DeferWindowPos(hdwp,hEQWindow,0,config_eq_wx,config_eq_wy,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
					}
					if (do_titlebar_clicking&4 && !skip_pl) // final PE positioning
					{
						SetPLWindowRect(&plr);
						hdwp=DeferWindowPos(hdwp,hPLWindow,0,config_pe_wx,config_pe_wy,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
					}
					if (do_titlebar_clicking&16 && !skip_vw) // final VW positioning
					{
						SetVidWindowRect(&vwr);
						hdwp=DeferWindowPos(hdwp,hVideoWindow,0,config_video_wx,config_video_wy,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
					}

					SetMainWindowRect(&mwr);
					hdwp=DeferWindowPos(hdwp,hMainWindow,0,config_wx,config_wy,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

					p = embedwndlist;
					while (p)
					{
						if (p->attached)
						{
							hdwp=DeferWindowPos(hdwp,p->me,0,p->r.left,p->r.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
						}
						p=p->link;
					}

					EndDeferWindowPos(hdwp);
				}
				LeaveCriticalSection(&embedcs);
			}
			return 1;
		}
	}
	return 0;
}

static int title_buttons_active[4];
static int do_titlebuttons() 
{
	if (mouse_type == -2)
		return 0;
	if (inreg(254,3,262,12)) // windowshade button
	{
		if (mouse_type == -1)
		{
			SendMessageW(hMainWindow,WM_COMMAND,WINAMP_OPTIONS_WINDOWSHADE,0);
			draw_tbuttons(-1,-1,-1,0);
			title_buttons_active[3]=0;
		}
		else if (mouse_stats & MK_LBUTTON)
		{
			title_buttons_active[3]=1;
			draw_tbuttons(-1,-1,-1,1);
		}
		if (mouse_type) return 1;
	} 
	else if (title_buttons_active[3]) 
	{
		title_buttons_active[3]=0;
		draw_tbuttons(-1,-1,-1,0);
	}


	if (inreg(244,3,253,12)) // minimize button
	{
		if (mouse_type == -1)
		{
			if (GetAsyncKeyState(VK_SHIFT)>>15)
				minimize_hack_winamp=1;
			else
				minimize_hack_winamp=0;
			ShowWindow(hMainWindow,SW_MINIMIZE);
			draw_tbuttons(-1,0,-1,-1);
			title_buttons_active[1]=0;
		}
		else if (mouse_stats & MK_LBUTTON)
		{
			title_buttons_active[1]=1;
			draw_tbuttons(-1,1,-1,-1);
		}
		if (mouse_type) return 1;
	} 
	else if (title_buttons_active[1]) 
	{
		title_buttons_active[1]=0;
		draw_tbuttons(-1,0,-1,-1);
	}

	if (inreg(264,3,272,12)) // kill button
	{
		if (mouse_type == -1) 
		{
			title_buttons_active[2]=0;
			draw_tbuttons(-1,-1,0,-1);
			if (GetAsyncKeyState(VK_SHIFT)&0x8000)
				SendMessageW(hMainWindow,WM_COMMAND,WINAMP_MAIN_WINDOW,0);
			else
				WASABI_API_APP->main_shutdown();
		}
		else if (mouse_stats & MK_LBUTTON)
		{
			title_buttons_active[2]=1;
			draw_tbuttons(-1,-1,1,-1);
		}
		if (mouse_type) return 1;
	}
	else if (title_buttons_active[2]) 
	{
		title_buttons_active[2]=0;
		draw_tbuttons(-1,-1,0,-1);
	}

	if (inreg(5,3,17,13))
	{
		if (mouse_stats & MK_LBUTTON && !title_buttons_active[0])
		{
			SetTimer(hMainWindow,666,155,NULL);
			title_buttons_active[0]=1;
			draw_tbuttons(1,-1,-1,-1);
		}
		if (mouse_type) return 1;
	}
	else if (title_buttons_active[0]) 
	{
		KillTimer(hMainWindow,666);
		title_buttons_active[0]=0;
		draw_tbuttons(0,-1,-1,-1);
	}

	return 0;
}


static int do_buttonbar_needupdate;

static int do_buttonbar() 
{
	if (mouse_type == -2)
		return 0;
	if (inreg(7+8,15+72,7+123,15+91)) 
	{
		int which = 4 - (mouse_x < 100+7) - (mouse_x < 77+7) - (mouse_x < 53+7) - (mouse_x < 31+7);
		switch (mouse_type) 
		{
		case 1: // mousedown
			draw_buttonbar(which);
			do_buttonbar_needupdate=1;
			break;
		case 0: // mousemove
			if (mouse_stats & MK_LBUTTON) 
			{
				draw_buttonbar(which);
				do_buttonbar_needupdate=1;
			}
			break;
		case -1: // mouseup
			do_buttonbar_needupdate=0;
			draw_buttonbar(-1);
			__do_buttons(which);
			break;
		}
		return 1;
	}
	if (do_buttonbar_needupdate)
	{
		do_buttonbar_needupdate=0;
		draw_buttonbar(-1);
	}
	return 0;
}


static int do_eq_needupdate, do_pe_needupdate;

static int do_peeq() 
{
	if (mouse_type == -2)
	{
		if (do_eq_needupdate || do_pe_needupdate)
		{
			draw_eqplbut(config_eq_open,0,config_pe_open,0);
			do_eq_needupdate=0;
			do_pe_needupdate=0;
			return 1;
		}
		
		return 0; 
	}

	if (inreg(219,58,219+23,58+12))  // eq
	{
		switch (mouse_type) 
		{
		case 1:
			draw_eqplbut(config_eq_open,1,config_pe_open,0);
			do_eq_needupdate=1;
			break;
		case 0:
			if (mouse_stats & MK_LBUTTON) 
			{
				draw_eqplbut(config_eq_open,1,config_pe_open,0);
				do_eq_needupdate=1;
			}
			break;
		case -1:
			do_eq_needupdate=0;
			SendMessageW(hMainWindow,WM_COMMAND,WINAMP_OPTIONS_EQ,0);
			break;
		}
		return 1;
	} 
	if (do_eq_needupdate) 
	{
		draw_eqplbut(config_eq_open,0,config_pe_open,0);
		do_eq_needupdate=0;
	}
	if (inreg(219+23,58,219+23+23,58+12))  // pl
	{
		switch (mouse_type) 
		{
		case 1:
			draw_eqplbut(config_eq_open,0,config_pe_open,1);
			do_pe_needupdate=1;
			break;
		case 0:
			if (mouse_stats & MK_LBUTTON) 
			{
				draw_eqplbut(config_eq_open,0,config_pe_open,1);
				do_pe_needupdate=1;
			}
			break;
		case -1:
			do_pe_needupdate=0;
			SendMessageW(hMainWindow,WM_COMMAND,WINAMP_OPTIONS_PLEDIT,0);
			SetForegroundWindow(hMainWindow);
			break;
		}
		return 1;
	} 
	if (do_pe_needupdate) 
	{
		draw_eqplbut(config_eq_open,0,config_pe_open,0);
		do_pe_needupdate=0;
	}

	return 0;
}

static int do_shuffle_needupdate;

static int do_shuffle() 
{
	if (mouse_type == -2)
		return 0;
	if (inreg(133+7-4+29,75+15,181+7-4-4+29,87+15)) 
	{
		switch (mouse_type) 
		{
		case 1:
			draw_shuffle(config_shuffle,1);
			do_shuffle_needupdate=1;
			break;
		case 0:
			if (mouse_stats & MK_LBUTTON) 
			{
				draw_shuffle(config_shuffle,1);
				do_shuffle_needupdate=1;
			}
			break;
		case -1:
			do_shuffle_needupdate=0;
			SendMessageW(hMainWindow,WM_COMMAND,WINAMP_FILE_SHUFFLE,0);
			break;
		}
		return 1;
	} 
	if (do_shuffle_needupdate) 
	{
		draw_shuffle(config_shuffle,0);
		do_shuffle_needupdate=0;
	}
	return 0;
}


static int do_repeat_needupdate;

static int do_repeat() 
{
	if (mouse_type == -2)
		return 0;
	if (inreg(182+7-4-4+29,75+15,182+29+7-4-4+29,87+15)) 
	{
		switch (mouse_type) 
		{
		case 1:
			draw_repeat(config_repeat,1);
			do_repeat_needupdate=1;
			break;
		case 0:
			if (mouse_stats & MK_LBUTTON) 
			{
				draw_repeat(config_repeat,1);
				do_repeat_needupdate=1;
			}
			break;
		case -1:
			do_repeat_needupdate=0;
			SendMessageW(hMainWindow,WM_COMMAND,WINAMP_FILE_REPEAT,0);
			break;
		}
		return 1;
	} 
	if (do_repeat_needupdate) 
	{
		draw_repeat(config_repeat,0);
		do_repeat_needupdate=0;
	}
	return 0;
}

static int do_eject_needupdate;

static int do_eject() 
{
	if (mouse_type == -2)
	{
		if (do_eject_needupdate)
		{
				draw_eject(0);
				//do_eject_needupdate=0;
				return 1;
		}
		return 0;
	}
	if (inreg(132+7-4+1,60+14+15,132+7-4+22+1,60+14+15+16)) 
	{
		switch (mouse_type) 
		{
		case 1:
			draw_eject(1);
			do_eject_needupdate=1;
			break;
		case 0:
			if (mouse_stats & MK_LBUTTON) 
			{
				draw_eject(1);
				do_eject_needupdate=1;
			}
			break;
		case -1:
			do_eject_needupdate=0;
			draw_eject(0);
			if (mouse_stats & MK_CONTROL) SendMessageW(hMainWindow,WM_COMMAND,WINAMP_FILE_LOC,0);
			else if (mouse_stats & MK_SHIFT) SendMessageW(hMainWindow,WM_COMMAND,WINAMP_FILE_DIR,0);
			else SendMessageW(hMainWindow,WM_COMMAND,WINAMP_FILE_PLAY,0);
			break;
		}
		return 1;
	} 
	if (do_eject_needupdate) 
	{
		draw_eject(0);
		do_eject_needupdate=0;
	}
	return 0;
}


static int do_icon()
{
	if (mouse_type == -2)
		return 0;
	if (inreg(246+7,76+15,258+7,90+15))
	{
		if (mouse_type == -1) 
			SendMessageW(hMainWindow,WM_COMMAND,WINAMP_LIGHTNING_CLICK,0);
		return 1;
	}
	return 0;
}

int do_posbar_active, do_posbar_clickx=14;

static int do_posbar()
{
	int a,t;

	if (mouse_type == -2)
	{
		if (do_posbar_active)
		{
			do_posbar_active=0;
			return 1;
		}
		return 0;
	}

	if (!playing || !in_mod || !in_mod->is_seekable || PlayList_ishidden(PlayList_getPosition())) 
	{
		if (mouse_type == -1 && do_posbar_active) do_posbar_active=0;
		return 0;
	}
	

	if ((mouse_type == -1 
		|| (mouse_type==0 && (mouse_stats&MK_SHIFT) && (mouse_stats&MK_LBUTTON))) && do_posbar_active) 
	{
		if (mouse_type == -1) 
			do_posbar_active=0;
		if (config_windowshade)
		{
			a = mouse_x - 228;
			a *= (257-29-10);
			a/=13;
		}
		else {
			a = mouse_x - do_posbar_clickx - (10+7); // posbar_clickx is offset from start of bar
			do_posbar_clickx = 14;
		}

		if (a < 0) a = 0;
		if (a > 257-29-10) a = 257-29-10;
		t = MulDiv(in_getlength(),1000 * a,(257-29-10));
		if (in_seek(t) < 0) StopPlaying(0);
		else 
		{
			//      ui_drawtime(in_getouttime()/1000,0);
		}
		if (mouse_type == -1) 
		{
			draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
			return 1;
		}
	}
	if (mouse_type == 1)
		if ((inreg(11+7,58+15,256+7,66+15) && !config_windowshade) || (inreg(228,3,228+14,12) && config_windowshade) ) 
		{
			do_posbar_active=1;
			if (!config_windowshade)
			{
				int curpos = 11 + 7 + 
					((in_getouttime() * (257-10-29)) / in_getlength()) / 1000;
				if (mouse_x <= curpos + 29 && mouse_x >= curpos-1) do_posbar_clickx = mouse_x - curpos;
				else do_posbar_clickx = 14;
			}
			else do_posbar_clickx = 0;
		}

		if (do_posbar_active) 
		{
			if (config_windowshade)
			{
				a = mouse_x - 228;
				a *= (257-29-10);
				a/=13;
			}
			else
			{
				a = mouse_x - do_posbar_clickx - (10+7);
			}
			if (a < 0) a = 0;
			if (a > 257-29-10) a = 257-29-10;
			t = MulDiv(in_getlength(),1000 * a,(257-29-10))/1000;
			{
				wchar_t buf[256] = {0}, seekStr[64] = {0};
				int a=in_getlength();
				if (in_mod && in_mod->is_seekable)
				{
					if (a) draw_positionbar((t*256) / a,1); 
					else draw_positionbar(0,1);
				}
				StringCchPrintfW(buf,256, L"%s: %02d:%02d/%02d:%02d (%d%%)",getStringW(IDS_SEEK_TO,seekStr,64),t/60,t%60,
					a/60,a%60,(t*100)/(a?a:1)
					);
				if (config_windowshade) draw_time(t/60,t%60,0);
				t=0;
				draw_songname(buf,&t,-1);
			}
			return 1;
		} 
		return 0;
}

void ui_drawtime(int time_elapsed, int mode) 
{
	if (time_elapsed < 0) time_elapsed = 0;
	if (!mode && paused) 
	{
		static int i;
		draw_time(time_elapsed/60,time_elapsed%60,i);
		i ^= 1;
	} 
	else 
	{
		if (!do_posbar_active || !config_windowshade)
			draw_time(time_elapsed/60,time_elapsed%60,0);
	}
	if (playing) 
		if (!do_posbar_active && in_mod && in_mod->is_seekable) {
			if (in_getlength()) draw_positionbar((time_elapsed*256) / in_getlength(),mode); 
			else draw_positionbar(0,mode);
		}
}

int do_volbar_clickx=4, do_volbar_active;

static int do_volbar()
{
	if (mouse_type == -2)
	{
		if (do_volbar_active)
		{
			do_volbar_active=0;
			draw_volumebar(config_volume,0);
			return 1;
		}
		return 0;
	}

	if (mouse_type == -1 && do_volbar_active)
	{
		do_volbar_active=0;
		do_volbar_clickx = 7;
		draw_volumebar(config_volume,0);
		in_setvol(config_volume);
		draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
		return 1;
	}

	if (inreg(107,43+15,107+68,51+15) && mouse_type == 1) 
	{
		int vs = 107 + (config_volume*51)/255;
		if (mouse_x < vs+15 && mouse_x >= vs) do_volbar_clickx = mouse_x - vs;
		else do_volbar_clickx = 7;
		do_volbar_active=1;
	}

	if (do_volbar_active) 
	{
		int v = mouse_x - do_volbar_clickx - (107);
		//    v = (v * 256) / (195-132-12);
		v = (v*255)/51;
		if (v < 0) v = 0;
		if (v > 255) v = 255;
		config_volume=v;
		draw_volumebar(config_volume,1);
		in_setvol(config_volume);
		update_volume_text(-1);
		return 1;
	}
	return 0;
}


int do_panbar_clickx=4, do_panbar_active;

static int do_panbar()
{
	if (mouse_type == -2)
	{
		if (do_panbar_active)
		{
			draw_panbar(config_pan,0);
			do_panbar_active=0;
			return 1;
		}
		return 0;
	}

	if (mouse_type == -1 && do_panbar_active)
	{
		do_panbar_active=0;
		do_panbar_clickx = 7;
		if (config_pan < 27 && config_pan > -27) config_pan = 0;
		draw_panbar(config_pan,0);
		in_setpan(config_pan);
		draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
		return 1;
	}

	if (inreg(177,43+15,177+38,51+15) && mouse_type == 1) 
	{
		int vs = 177 + 12 + (config_pan*12)/127;
		if (mouse_x < vs+15 && mouse_x >= vs) do_panbar_clickx = mouse_x - vs;
		else do_panbar_clickx = 7;
		do_panbar_active=1;
	}

	if (do_panbar_active) 
	{
		int v = mouse_x - do_panbar_clickx - (177);
		if (v < 0) v = 0;
		if (v > 24) v = 24;
		v-=12;
		// changed in 5.64 to have a lower limit (~16% vs 24%) and for
		// holding shift to drop the central clamp (allows 7% balance)
		if (!(mouse_stats & MK_SHIFT)) if (v < 2 && v > -2) v = 0;
		v *= 127;
		v/=12;
		config_pan = v;
		draw_panbar(config_pan,1);
		in_setpan(config_pan);
		update_panning_text(-1);
		return 1;
	}
	return 0;
}


int ui_songposition = 0;
int ui_songposition_tts=10;
int do_songname_clickx = -1;
int do_songname_active = 0;

void ui_doscrolling()
{
	if (!do_clutterbar_active && !ui_songposition_tts && !do_songname_active && !do_volbar_active && !do_panbar_active && !do_posbar_active)
	{
		ui_songposition+=5;
		draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
	} else if (ui_songposition_tts) ui_songposition_tts--;
	else ui_songposition_tts = 10;
}

static int do_songname()
{
	if (mouse_type == -2)
		return 0;
	if (mouse_stats & MK_LBUTTON || mouse_type == -1) if (do_songname_active || (inreg(117,24,266,35) && mouse_type == 1))
	{
		if (mouse_type == -1)
		{
			do_songname_clickx = -1;
			do_songname_active=0;
			ui_songposition_tts=10;
			return 1;
		} 
		if (do_songname_active)
		{
			ui_songposition -= mouse_x - do_songname_clickx;
			draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
		}
		do_songname_clickx = mouse_x;
		do_songname_active=1;
		return 1;
	}
	return 0;
}


void ui_reset() 
{
	title_buttons_active[0] = title_buttons_active[1] = title_buttons_active[2] = 0;
	do_clutterbar_active=0;
	do_songname_clickx = -1;
	do_songname_active=0;
	do_buttonbar_needupdate = 0;
	do_shuffle_needupdate = 0;
	do_eq_needupdate = do_pe_needupdate = 0;
	do_repeat_needupdate = 0;
	do_eject_needupdate = 0;
	do_posbar_active = 0;
	do_posbar_clickx = 14;
	do_volbar_active = 0;
	do_volbar_clickx = 7;
	do_panbar_active = 0;
	do_panbar_clickx = 7;
	ui_songposition = 0;
	ui_songposition_tts=0;
}


void ui_handlecursor(void)
{
	POINT p = {0};
	static RECT b_normal[] = 
	{
		{107,43+15,177+38,51+15},// vol & bal
		{11+7,58+15,256+7,66+15},// pos
		{254,3,262,12},//wshade
		{244,3,253,12},//min
		{264,3,272,12},//close
		{5,3,17,13},//mainmenu
		{0,0,275,13},// titelbar
		{105,24,266,35},
	}, b_windowshade[] = 
	{
		{254,3,262,12},//wshade
		{244,3,253,12},//min
		{228,3,228+14,12}, // seeker
		{264,3,272,12},//close
		{5,3,17,13},//mainmenu
	};

		int x;

		if (0 != (WS_DISABLED & GetWindowLongPtrW(hMainWindow, GWL_STYLE)))
			x = (config_windowshade)	 ? 14 : 8;
		else
		{
			int b_len;
			
			RECT r;
			GetCursorPos(&p);
			GetWindowRect(hMainWindow,&r);
			FixMainWindowRect(&r);
			int mouse_x=p.x-r.left;
			int mouse_y=p.y-r.top;
			if (config_dsize) { mouse_x/=2; mouse_y/=2;}

			RECT *b;
			if (config_windowshade)	
			{
				b=b_windowshade;
				b_len = sizeof(b_windowshade)/sizeof(b_windowshade[0]);
			}
			else 
			{
				b=b_normal;
				b_len = sizeof(b_normal)/sizeof(b_normal[0]);
			}
			for (x = 0; x < b_len; x ++) 
				if (inreg(b[x].left,b[x].top,b[x].right,b[x].bottom)) break;

			if (config_windowshade) x+=9;
		}
		

		if (Skin_Cursors[x]) SetCursor(Skin_Cursors[x]);
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
}

#include "../Plugins/General/gen_ml/menu.h"
BOOL DoTrackPopup(HMENU hMenu, UINT fuFlags, int x, int y, HWND hwnd)
{
	if(hMenu == NULL)
	{
		return NULL;
	}

	HWND ml_wnd = (HWND)sendMlIpc(0, 0);
	if (IsWindow(ml_wnd))
	{
		return Menu_TrackPopup(ml_wnd, hMenu, fuFlags, x, y, hwnd, NULL);
	}
	else
	{ 
		return TrackPopupMenu(hMenu, fuFlags, x, y, 0, hwnd, NULL);
	}
}