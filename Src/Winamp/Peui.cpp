#include "Main.h"

#define inreg(x,y,x2,y2) \
        ((mouse_x <= ( x2 ) && mouse_x >= ( x ) &&  \
        mouse_y <= ( y2 ) && mouse_y >= ( y )))

static int mouse_x, mouse_y, mouse_type, mouse_stats;

static int which_cap=0;
enum { NO_CAP,TITLE_CAP,TB_CAP, SZ_CAP, VS_CAP, LB_CAP, VSB_CAP, ADD_CAP, REM_CAP, SEL_CAP, MISC_CAP, FILE_CAP, TD_CAP, MB_CAP };

static void do_titlebar(HWND hwnd);
static void do_titlebuttons();
static void do_size(HWND hwnd);
static void do_vscroll(HWND hwnd);
static void do_lb(HWND hwnd);
static void do_vsb(HWND hwnd);
static void do_addbut(HWND hwnd);
static void do_rembut(HWND hwnd);
static void do_selbut(HWND hwnd);
static void do_miscbut(HWND hwnd);
static void do_filebut(HWND hwnd);
static void do_timedisplay();
static void do_mb();

int peui_isrbuttoncaptured()
{
  if (which_cap >= ADD_CAP && which_cap <= FILE_CAP) return 1;
  return 0;
}

void peui_reset(HWND hwnd)
{
  if (which_cap>=ADD_CAP && which_cap <= FILE_CAP)
    InvalidateRect(hwnd,NULL,0);
  which_cap=0;
}

//#include "../gen_ml/ml_ipc_0313.h"
void peui_handlemouseevent(HWND hwnd, int x, int y, int type, int stats) 
{
	mouse_x = x;
	mouse_y = y;
	mouse_type = type;
	mouse_stats = stats;
	if (!which_cap)
	{
		if (playing && mouse_type == 1 && config_pe_height != 14 && config_pe_width >= 350 &&
			inreg(config_pe_width-150-75,config_pe_height-26,config_pe_width-150,config_pe_height-8))
		{
 			config_sa++;
			if (config_sa > 2) config_sa = 0;
			set_visopts();
			sa_setthread(config_sa);
			return;
		}
	}
	switch (which_cap)
	{
		case MB_CAP: do_mb(); return;
		case TD_CAP: do_timedisplay(); return;
		case MISC_CAP:  do_miscbut(hwnd); return;
		case FILE_CAP:  do_filebut(hwnd); return;
		case SEL_CAP:  do_selbut(hwnd); return;
		case ADD_CAP:  do_addbut(hwnd); return;
		case REM_CAP:  do_rembut(hwnd); return;
		case VSB_CAP:do_vsb(hwnd);return;
		case LB_CAP:  do_lb(hwnd);return;
		case TITLE_CAP:	do_titlebar(hwnd);return;
		case TB_CAP:	do_titlebuttons();return;
		case SZ_CAP:	do_size(hwnd);	return;
		case VS_CAP:	do_vscroll(hwnd);	return;
		default: break;
	}
	if (config_pe_height != 14) 
	{
		do_mb();
		do_timedisplay();
		do_filebut(hwnd);
		do_miscbut(hwnd);
		do_selbut(hwnd);
		do_addbut(hwnd);
		do_rembut(hwnd);
		do_vsb(hwnd);
		do_lb(hwnd);
		do_vscroll(hwnd);
	}
	do_titlebuttons();
	do_size(hwnd);
	do_titlebar(hwnd);

	// TODO make this not go weird / cause WM_MOUSEWHEEL to fail, etc
	#if 0
	if(!mouse_type)
	{
		int t = (mouse_y - 22) / pe_fontheight;
		if (t < 0) t = 0;
		else if (t > (config_pe_height - 38 - 22) / pe_fontheight) t = PlayList_getlength();
		//else t += pledit_disp_offs;

		// TODO alignment isn't 100% correct at times
		RECT rs = {0};
		rs.left = 12;
		rs.right = config_pe_width - 20;
		rs.top = 22+(pe_fontheight*t);
		rs.bottom = rs.top + pe_fontheight;

		if(rs.bottom <= config_pe_height - 38)
		{
			TOOLINFOW ti = {0};
			RECT r_main;
			EstPLWindowRect(&r_main);
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_SUBCLASS|TTF_TRANSPARENT;
			ti.hwnd = hPLWindow;

			ti.uId = (UINT_PTR)hPL2TooltipWindow;
			ti.lpszText = (wchar_t*)LPSTR_TEXTCALLBACK;
			ti.uId = 17;
			ti.rect = rs;

			// TODO need to get this to show the actual playlist item

			t = (mouse_y - 22) / pe_fontheight;
			if (t < 0) t = 0;
			else if (t > (config_pe_height - 38 - 22) / pe_fontheight) t = PlayList_getlength();
			else t += pledit_disp_offs;

			wchar_t ft[FILETITLE_SIZE], *ftp = ft;
			PlayList_getitem_pl(t,ft);
			while(ftp && *ftp)
			{
				if(*ftp != ' ') break;
				ftp = CharNextW(ftp);
			}
			ti.lpszText=ftp;
			///*if (c)*/
			SendMessageW(hPLTooltipWindow,TTM_DELTOOLW,0,(LPARAM) &ti);
			//if (config_ttips)
			SendMessageW(hPL2TooltipWindow,TTM_ADDTOOLW,0,(LPARAM) &ti);

			// TODO need to get the skinning code from gen_ml into winamp.exe or split it out...
			static bool skinned;
			if(!skinned)
			{
				typedef BOOL (__cdecl *MLSKINWINDOWEX)(HWND /*hwnd*/, INT /*type*/, UINT /*style*/);
				MLSKINWINDOWEX mlSkinWindowEx = (MLSKINWINDOWEX)GetProcAddress(GetModuleHandle("gen_ml.dll"), "MlSkinWindowEx");
				unsigned int skinStyle = SWS_USESKINCOLORS;
				skinStyle |= ((SWS_USESKINFONT | SWS_USESKINCURSORS)/* & style*/);
				mlSkinWindowEx(hPL2TooltipWindow, SKINNEDWND_TYPE_TOOLTIP, skinStyle);
				skinned = true;
			}
		}
	}
	#endif
}


static void __do_buttons(int which)
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
}

static void do_mb()
{
	if (!which_cap && mouse_type == 1 && inreg(config_pe_width-144,config_pe_height-15,config_pe_width-91,config_pe_height-8))
	{
		int t=config_pe_width-144;
		int which = 5 - (mouse_x < 45+t) - (mouse_x < 36+t) - (mouse_x < 27+t) - (mouse_x < 17+t) - (mouse_x < 7+t);
		which_cap = MB_CAP;
		__do_buttons(which);

	}
	if (which_cap == MB_CAP && mouse_type == -1)
		which_cap=0;
}



static void do_timedisplay()
{
	if (!which_cap && mouse_type == 1 && inreg(config_pe_width-87,config_pe_height-18,config_pe_width-53,config_pe_height-9))
	{
		which_cap = TD_CAP;
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
	}
	if (which_cap == TD_CAP && mouse_type == -1)
		which_cap=0;
}

static void do_filebut(HWND hwnd)
{
  int lwc=which_cap;
	if (!which_cap && mouse_type == 1 && inreg(config_pe_width-44,config_pe_height-30,config_pe_width-44+22,config_pe_height-12))
	{
		which_cap = FILE_CAP;
	}

	if (which_cap == FILE_CAP)
	{
		int doit=-1;
		if (inreg(config_pe_width-44,config_pe_height-30,config_pe_width-44+22,config_pe_height-12)) doit=0;
		else if (inreg(config_pe_width-44,config_pe_height-30-18,config_pe_width-44+22,config_pe_height-12-18)) doit=1;
		else if (inreg(config_pe_width-44,config_pe_height-30-18*2,config_pe_width-44+22,config_pe_height-12-18*2)) doit=2;
		draw_pe_iobut(doit);

		if ((config_ospb&&mouse_type==-1) || (mouse_type == 1 && lwc==FILE_CAP))
		{
			RECT r={config_pe_width-44-5,config_pe_height-12-18*3,config_pe_width-44+22,config_pe_height-12};
			InvalidateRect(hwnd,&r,FALSE);
			which_cap=0;
			switch (doit)
			{
				case 0: SendMessageW(hwnd,WM_COMMAND,ID_PE_OPEN,0); break;
				case 1: SendMessageW(hwnd,WM_COMMAND,ID_PE_SAVEAS,0); break;
				case 2: SendMessageW(hwnd,WM_COMMAND,ID_PE_CLEAR,0); break;
			}
		}
	}
}



static void do_miscbut(HWND hwnd)
{
  int lwc=which_cap;
	if (!which_cap && mouse_type == 1 && inreg(101,config_pe_height-30,122,config_pe_height-12))
	{
		which_cap = MISC_CAP;
	}

	if (which_cap == MISC_CAP)
	{
		int doit=-1;
		if (inreg(101,config_pe_height-30,122,config_pe_height-12)) doit=0;
		else if (inreg(101,config_pe_height-30-18,122,config_pe_height-12-18)) doit=1;
		else if (inreg(101,config_pe_height-30-18*2,122,config_pe_height-12-18*2)) doit=2;
		draw_pe_miscbut(doit);
		if ((config_ospb&&mouse_type==-1) || (mouse_type == 1 && lwc==MISC_CAP))
		{
			RECT r={97,config_pe_height-12-18*3,123,config_pe_height-12};
			which_cap=0;
			switch (doit)
			{
				case 1: 
					{
						POINT p={122,config_pe_height-30-18};
						ClientToScreen(hwnd,&p);
						DoTrackPopup(GetSubMenu(GetSubMenu(GetSubMenu(top_menu,2),0),0),TPM_LEFTALIGN,p.x,p.y,hwnd);
					}	
				break;
				case 2: 
					{
						POINT p={122,config_pe_height-30-18*2};
						ClientToScreen(hwnd,&p);
						DoTrackPopup(GetSubMenu(GetSubMenu(GetSubMenu(top_menu,2),0),1),TPM_LEFTALIGN,p.x,p.y,hwnd);
					}	
				break;
				case 0: 
					{
						POINT p={122,config_pe_height-30};
						ClientToScreen(hwnd,&p);
						DoTrackPopup(GetSubMenu(GetSubMenu(GetSubMenu(top_menu,2),0),2),TPM_LEFTALIGN,p.x,p.y,hwnd);
					}	
				break;
			}
			InvalidateRect(hwnd,&r,FALSE);
		}
	}
}


static void do_selbut(HWND hwnd)
{
  int lwc=which_cap;
	if (!which_cap && mouse_type == 1 && inreg(72,config_pe_height-30,93,config_pe_height-12))
	{
		which_cap = SEL_CAP;
	}

	if (which_cap == SEL_CAP)
	{
		int doit=-1;
		if (inreg(72,config_pe_height-30,93,config_pe_height-12)) doit=0;
		else if (inreg(72,config_pe_height-30-18,93,config_pe_height-12-18)) doit=1;
		else if (inreg(72,config_pe_height-30-18*2,93,config_pe_height-12-18*2)) doit=2;
		draw_pe_selbut(doit);
		if ((config_ospb&&mouse_type==-1) || (mouse_type == 1 && lwc==SEL_CAP))
		{
			RECT r={68,config_pe_height-12-18*3,95,config_pe_height-12};
			InvalidateRect(hwnd,&r,FALSE);
			which_cap=0;
			switch (doit)
			{
				case 0: SendMessageW(hwnd,WM_COMMAND,ID_PE_SELECTALL,0); break;
				case 1: SendMessageW(hwnd,WM_COMMAND,ID_PE_NONE,0); break;
				case 2: SendMessageW(hwnd,WM_COMMAND,IDC_SELECTINV,0); break;
			}
		}
	}
}

static void do_rembut(HWND hwnd)
{
  int lwc=which_cap;
	if (!which_cap && mouse_type == 1 && inreg(43,config_pe_height-30,64,config_pe_height-12))
	{
		which_cap = REM_CAP;
	}

	if (which_cap == REM_CAP)
	{
		int doit=-1;
		if (inreg(43,config_pe_height-30,64,config_pe_height-12)) doit=0;
		else if (inreg(43,config_pe_height-30-18,64,config_pe_height-12-18)) doit=1;
		else if (inreg(43,config_pe_height-30-18*2,64,config_pe_height-12-18*2)) doit=2;
		else if (inreg(43,config_pe_height-30-18*3,64,config_pe_height-12-18*3)) doit=3;
		draw_pe_rembut(doit);
		if ((config_ospb&&mouse_type==-1) || (mouse_type == 1 && lwc==REM_CAP))
		{
			RECT r={39,config_pe_height-12-18*4,65,config_pe_height-12};
			which_cap=0;
			switch (doit)
			{
				case 0: SendMessageW(hwnd,WM_COMMAND,IDC_PLAYLIST_REMOVEMP3,0); break;
				case 1: SendMessageW(hwnd,WM_COMMAND,IDC_PLAYLIST_CROP,0); break;
				case 2: SendMessageW(hwnd,WM_COMMAND,ID_PE_CLEAR,0); break;
				case 3:					
					{
						POINT p={64,config_pe_height-30-18*3};
						ClientToScreen(hwnd,&p);
						DoTrackPopup(GetSubMenu(GetSubMenu(GetSubMenu(top_menu,2),2),3),TPM_LEFTALIGN,p.x,p.y,hwnd);
					}	
				break;
			}
			InvalidateRect(hwnd,&r,FALSE);
		}
	}
}

static void do_addbut(HWND hwnd)
{
  int lwc=which_cap;
	if (!which_cap && mouse_type == 1 && inreg(14,config_pe_height-30,35,config_pe_height-12))
	{
		which_cap = ADD_CAP;
	}

	if (which_cap == ADD_CAP)
	{
		int doit=-1;
		if (inreg(14,config_pe_height-30,35,config_pe_height-12)) doit=0;
		else if (inreg(14,config_pe_height-30-18,35,config_pe_height-12-18)) doit=1;
		else if (inreg(14,config_pe_height-30-18*2,35,config_pe_height-12-18*2)) doit=2;
		draw_pe_addbut(doit);
		if ((config_ospb&&mouse_type==-1) || (mouse_type == 1 && lwc==ADD_CAP))
		{
			RECT r={11,config_pe_height-12-18*3,36,config_pe_height-12};
			InvalidateRect(hwnd,&r,FALSE);
			which_cap=0;
			switch (doit)
			{
				case 0: SendMessageW(hwnd,WM_COMMAND,IDC_PLAYLIST_ADDMP3,0); break;
				case 1: SendMessageW(hwnd,WM_COMMAND,IDC_PLAYLIST_ADDDIR,0); break;
				case 2: SendMessageW(hwnd,WM_COMMAND,IDC_PLAYLIST_ADDLOC,0); break;
			}
		}
	}

}

static void do_vsb(HWND hwnd)
{
	if (!which_cap && mouse_type == 1 && inreg(config_pe_width-15,config_pe_height-36,config_pe_width-7,config_pe_height-32))
	{
		SendMessageW(hwnd,WM_COMMAND,ID_PE_SCROLLUP,0);
		which_cap = VSB_CAP;
	}
	if (!which_cap && mouse_type == 1 && inreg(config_pe_width-15,config_pe_height-31,config_pe_width-7,config_pe_height-27))
	{
		SendMessageW(hwnd,WM_COMMAND,ID_PE_SCROLLDOWN,0);
		which_cap = VSB_CAP;
	}

	if (which_cap == VSB_CAP && mouse_type == -1) which_cap=0;
}

int shiftsel_1=-1;

static void do_lb(HWND hwnd)
{
	static int move_mpos,stt;

	if (!which_cap && mouse_type == 1 && inreg(12,20,config_pe_width-20,config_pe_height-38))
	{
		int wh=(mouse_y-22)/pe_fontheight + pledit_disp_offs;
		if (!(mouse_stats & MK_CONTROL) && !PlayList_getselect(wh))
		{
			int x,t=PlayList_getlength();
			for (x = 0; x < t; x ++)
				PlayList_setselect(x,0);
		}
		if (mouse_stats & MK_SHIFT)
		{
			if (shiftsel_1 != -1) 
			{
				int x,t=max(min(PlayList_getlength(),shiftsel_1),min(PlayList_getlength(),wh));
				if (!(mouse_stats  & MK_CONTROL)  && PlayList_getselect(wh))
				{
					int x,t=PlayList_getlength();
					for (x = 0; x < t; x ++)
						PlayList_setselect(x,0);
				}
				for (x = min(shiftsel_1,wh); x <=t; x ++)
				{
					PlayList_setselect(x,1);
				}
				stt=1;
			}
		}
		else
		{
			if (PlayList_getselect(wh) && mouse_stats & MK_CONTROL) PlayList_setselect(wh,0);
			else 
			{
				int y = wh-pledit_disp_offs;
				if (y < PlayList_getlength()-pledit_disp_offs && y < (config_pe_height-38-20-2)/pe_fontheight)
				{
					PlayList_setselect(wh,1);
				}
			}
			shiftsel_1=wh;
		}
		
		{
			RECT r1={12,22,config_pe_width-20,config_pe_height-38};
			RECT r2={12,20+(wh-pledit_disp_offs)*pe_fontheight,config_pe_width-20,20+(wh+1-pledit_disp_offs)*pe_fontheight};
			if (!(mouse_stats & MK_CONTROL) || (mouse_stats & MK_SHIFT))
				InvalidateRect(hwnd,&r1,FALSE);
			else
				InvalidateRect(hwnd,&r2,FALSE);
		}
		move_mpos=(mouse_y-22)/pe_fontheight+pledit_disp_offs;
		which_cap=LB_CAP;
	}
	if (which_cap == LB_CAP)
	{
    extern int g_has_deleted_current;
		int m=(mouse_y-22)/pe_fontheight+pledit_disp_offs;
		if (m!=move_mpos)
		{
			int v,x;
			v = PlayList_getlength();
			x=1;
			while (m>move_mpos)
			{
				if (!PlayList_getselect(v-1)) for (x = v-2; x >= 0; x --)
						{
							if (!PlayList_getselect(x))
							{
								continue;
							}
              if (shiftsel_1 == x) shiftsel_1=x+1;
							PlayList_swap(x,x+1);
							if (x == PlayList_getPosition()) 
								PlayList_advance(1);
							else if (x+1 == PlayList_getPosition()) 
								PlayList_advance(-1);
						}
				move_mpos++;
			}
			while (m<move_mpos)
			{
				if (!PlayList_getselect(0)) for (x = 1; x < v; x ++)
				{
					if (!PlayList_getselect(x)) continue;
          if (shiftsel_1 == x) shiftsel_1=x-1;
					PlayList_swap(x,x-1);
					if (x == PlayList_getPosition()) 
						PlayList_advance(-1);
					else if (x-1 == PlayList_getPosition()) 
						PlayList_advance(1);
				}
				move_mpos--;
			}

			if (mouse_y < 10)
			{
				RECT r={0,0,config_pe_width,config_pe_height-37};
//				move_mpos++;
				pledit_disp_offs --;
				if (pledit_disp_offs < 0) { pledit_disp_offs=0; move_mpos=0;}
				InvalidateRect(hwnd,&r,FALSE);
			}	
			else if (mouse_y > config_pe_height-28)
			{
				RECT r={0,0,config_pe_width,config_pe_height-37};
				int num_songs=(config_pe_height-38-20-2)/pe_fontheight;
				int t=PlayList_getlength()-num_songs;
	//			move_mpos--;
				pledit_disp_offs ++;
				if (pledit_disp_offs > t) { pledit_disp_offs=max(t,0); move_mpos=PlayList_getlength(); }
				InvalidateRect(hwnd,&r,FALSE);
			}
			else {
				RECT r1={12,22,config_pe_width-20,config_pe_height-38};
				InvalidateRect(hwnd,&r1,FALSE);
			}
      if (!g_has_deleted_current)
      {
  			PlayList_getcurrent(FileName,FileTitle,FileTitleNum);
	  		draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
      }
		}

		if (mouse_type == -1) 
		{
			if (!stt)
			{
				int wh=(mouse_y-22)/pe_fontheight + pledit_disp_offs;
				if (!(mouse_stats & MK_CONTROL) && PlayList_getselect(wh))
				{
					int x,t=PlayList_getlength();
					for (x = 0; x < t; x ++)
						PlayList_setselect(x,0);
					PlayList_setselect(wh,1);
					{
						RECT r1={12,22,config_pe_width-20,config_pe_height-38};
						InvalidateRect(hwnd,&r1,FALSE);
					}
			}
	
			}
			stt=0;
			which_cap=0;
		}
	}
}


static void do_titlebar(HWND hwnd) 
{
#ifdef FFSKIN
	if ((GetParent(hwnd) == NULL) && (which_cap == TITLE_CAP || (!which_cap && (config_easymove || mouse_y < 14)))) 
#else
	if (which_cap == TITLE_CAP || (!which_cap && (config_easymove || mouse_y < 14))) 
#endif
	{
		static int clickx, clicky;
		switch (mouse_type)
	{
		case 1:
			which_cap=TITLE_CAP;
			clickx=mouse_x;
			clicky=mouse_y;
		break;
		case -1:
			which_cap=0;
		break;
		case 0:
			if (which_cap == TITLE_CAP && mouse_stats & MK_LBUTTON)
			{
				POINT p = { mouse_x,mouse_y};
				ClientToScreen(hwnd,&p);
				config_pe_wx = p.x-clickx;
				config_pe_wy = p.y-clicky;
				if ((!!config_snap) ^ (!!(mouse_stats & MK_SHIFT)))
				{
					RECT outrc;
					EstPLWindowRect(&outrc);
					SnapWindowToAllWindows(&outrc,hwnd);
					SetPLWindowRect(&outrc);
				}

				SetWindowPos(hwnd,0,config_pe_wx,config_pe_wy,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			}
		break;
		}
	}
}

static void do_titlebuttons() 
{
	int w=0;
	w=inreg(config_pe_width-10,3,config_pe_width-1,3+9)?1:w;
	w=inreg(config_pe_width-20,3,config_pe_width-11,3+9)?2:w;

	if (w) // kill button
	{
		if (mouse_type == -1 && which_cap == TB_CAP) 
		{
			which_cap=0;
			draw_pe_tbutton(0,0,0);
			SendMessageW(hMainWindow,WM_COMMAND,w==1?WINAMP_OPTIONS_PLEDIT:WINAMP_OPTIONS_WINDOWSHADE_PL,0);
		}
		else if (mouse_stats & MK_LBUTTON)
		{
			which_cap=TB_CAP;
			draw_pe_tbutton(w==2?1:0,w==1?1:0,config_pe_height==14?1:0);
		}
	}
	else if (which_cap == TB_CAP) 
	{
		which_cap=0;
		draw_pe_tbutton(0,0,config_pe_height==14?1:0);
	}

}

static void do_size(HWND hwnd)
{
	if (which_cap == SZ_CAP || (config_pe_height != 14 && !which_cap && 
		mouse_x > config_pe_width-20 && mouse_y > config_pe_height-20 && 
		((config_pe_width-mouse_x + config_pe_height-mouse_y) <= 30))
		|| 
		(config_pe_height == 14 && !which_cap && mouse_x > config_pe_width-29 && mouse_x < config_pe_width-20)
		)
	{
		static int dx,dy;
		if (!which_cap && mouse_type == 1)
		{
			dx=config_pe_width-mouse_x;
			dy=config_pe_height-mouse_y;
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
//			if (x >= GetSystemMetrics(SM_CXSCREEN)) x = GetSystemMetrics(SM_CXSCREEN)-24;
//  			if (y >= GetSystemMetrics(SM_CYSCREEN)) y = GetSystemMetrics(SM_CYSCREEN)-28;
      if (!config_embedwnd_freesize)
      {
			  x += 24;
			  x -= x%25;
			  y += 28;
			  y -= y%29;
      }
			if (x < 275) x = 275;
			if (y < 20+38+29+29) y = 20+38+29+29;
			config_pe_width = x;
			if (config_pe_height != 14) config_pe_height= y;
			SetWindowPos(hwnd,0,0,0,config_pe_width,config_pe_height,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
			if (hPLVisWindow)
			{
				int x,y,w,h;
				x=config_pe_width-150-75+2;
				y=config_pe_height-26;
				w=(config_pe_width >= 350 && config_pe_height != 14 ? 72 : 0);
				h=16;
				SetWindowPos(hPLVisWindow,0,x,y,w,h,SWP_NOZORDER|SWP_NOACTIVATE);
			}
		}
	}

}

static void do_vscroll(HWND hwnd)
{
	int top=20,bottom=config_pe_height-38;
	int num_songs=(config_pe_height-38-20-2)/pe_fontheight;
	int xoffs=config_pe_width-15,w=8;

	if (inreg(xoffs,top,xoffs+w,bottom) || which_cap == VS_CAP)
	{
		if ((!which_cap && mouse_type == 1) || which_cap == VS_CAP)
		{
			int d;
			int p;
			static int click_yoffs=9;
			int a=PlayList_getlength()-num_songs;
			{
				if (a < 1) p = top;
				else 
				{
					p = top + ((config_pe_height-38-20-18)*pledit_disp_offs) / a;
				}
			}
			if (mouse_type == 1 && mouse_y >= p && mouse_y < p + 20)
			{
				click_yoffs=mouse_y - p -1;
			} else if (mouse_type == 1)	click_yoffs=9;

			d=((mouse_y-click_yoffs-top)*a)/(config_pe_height-38-20-18);
			

			pledit_disp_offs = d;
			if (pledit_disp_offs > a) pledit_disp_offs = a;

			draw_pe_vslide(hwnd, NULL, 1,pledit_disp_offs);
			{
				RECT r={12,22,config_pe_width-19,config_pe_height-37};
				InvalidateRect(hwnd,&r,FALSE);
			}

			if (mouse_type == -1)
			{
				draw_pe_vslide(hwnd, NULL, 0,pledit_disp_offs);
				which_cap=0;
			} else 
			{
				which_cap = VS_CAP;
			}
		}
	} 
}

	static const RECT b_normal[] = 
	{
		{-(275-254),3,-(275-262),12},//wshade
		{-(275-264),3,-(275-272),12},//close
		{0,0,-1,13},// titelbar
		{-15,20,-7,-38},
		{-20,-20,-1,-1},
  }, 
  b_windowshade[] = 
	{
		{-(275-254),3,-(275-262),12},//wshade
		{-(275-264),3,-(275-272),12},//close
		{-29,3,-20,12},//size
	};

void pe_ui_handlecursor(HWND hwnd)
{
	int mouse_x, mouse_y;
	POINT p;
	const RECT *b;
	int b_len;
	int x;

	if (!config_usecursors || disable_skin_cursors) return;
	GetCursorPos(&p);
	ScreenToClient(hwnd,&p);
	mouse_x=p.x;
	mouse_y=p.y;
	if (config_pe_height == 14)	
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
	{
		int l,r,t,bo;
		l=b[x].left;r=b[x].right;t=b[x].top;bo=b[x].bottom;
		if (l < 0) l += config_pe_width;
		if (r < 0) r += config_pe_width;
		if (t < 0) t += config_pe_height;
		if (bo < 0) bo += config_pe_height;
		if (inreg(l,t,r,bo)) break;
	}

  if (config_pe_height == 14) x+=21;
  else x+=15;

  if (Skin_Cursors[x]) SetCursor(Skin_Cursors[x]);
  else SetCursor(LoadCursor(NULL,IDC_ARROW));

}
