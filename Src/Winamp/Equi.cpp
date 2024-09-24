#include "Main.h"
#include "WinampAttributes.h"
#include "resource.h"

#define inreg(x,y,x2,y2) \
	((mouse_x <= ( x2 ) && mouse_x >= ( x ) &&  \
	mouse_y <= ( y2 ) && mouse_y >= ( y )))

static int mouse_x, mouse_y, mouse_type, mouse_stats;

static int which_cap=0;
enum { NO_CAP,TITLE_CAP,TB_CAP,QB_CAP,TOGBUTS_CAP,PB_CAP, PAN_CAP,VOL_CAP, SLID_CAP=100 };

static void do_titlebar();
static void do_titlebuttons();
static void do_quickbuts();
static void do_sliders(int which);
static void do_togbuts();
static void do_volctrl();
static void do_panctrl();
static void do_presetbutton();

void equi_handlemouseevent(int x, int y, int type, int stats) 
{
	mouse_x = x;
	mouse_y = y;
	mouse_type = type;
	mouse_stats = stats;
	switch (which_cap)
	{
	case PAN_CAP:
		do_panctrl();
		return;
	case VOL_CAP:
		do_volctrl();
		return;
	case PB_CAP:
		do_presetbutton();
		return;
	case TOGBUTS_CAP:
		do_togbuts();
		return;
	case TITLE_CAP:
		do_titlebar();
		return;
	case TB_CAP:
		do_titlebuttons();
		return;
	case QB_CAP:
		do_quickbuts();
		return;
	default:
		if (which_cap >= SLID_CAP)
		{
			do_sliders(which_cap-SLID_CAP);
			return;
		}
	}
	if (config_eq_ws)
	{
		do_volctrl();
		do_panctrl();
	}
	else
	{
		for (x = 0; x < 11; x ++)
			do_sliders(x);
	}
	do_titlebuttons();
	do_quickbuts();
	do_togbuts();
	do_presetbutton();
	do_titlebar();
}

static void do_volctrl()
{
	extern int do_volbar_active;
	if (inreg(61,3,162,11) || which_cap==VOL_CAP)
	{
		if (mouse_type == 1 && !which_cap) which_cap=VOL_CAP;
		if (which_cap==VOL_CAP && mouse_stats & MK_LBUTTON)
		{
			int t=mouse_x-61;
			if (t < 0) t=0;
			if (t > 157-61) t=157-61;
			config_volume=(t*255)/(157-61);
			in_setvol(config_volume);
			draw_volumebar(config_volume,0);
			update_volume_text(-1);
			do_volbar_active=1;
		}
		if (mouse_type == -1 && which_cap == VOL_CAP)
		{
			which_cap=0;
			do_volbar_active=0;
			draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
		}
	}
	else if (which_cap==VOL_CAP)
	{
		which_cap=0;
		draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
		do_volbar_active=0;
	}
}

static void do_panctrl()
{
	extern int do_volbar_active;
	if (inreg(163,3,206,11) || which_cap==PAN_CAP)
	{
		if (mouse_type == 1 && !which_cap) which_cap=PAN_CAP;
		if (which_cap==PAN_CAP && mouse_stats & MK_LBUTTON)
		{
			int t=mouse_x-164;
			if (t < 0) t=0;
			if (t > 206-164) t=206-164;
			int p = (t*255)/(206-164)-127;
			config_pan = (p > 127 ? 127 : p);
			// changed in 5.64 to have a lower limit (~18% vs 9%) and for
			// holding shift to drop the central clamp (allows 4% balance)
			// and the above change fixes the balance to be even going +/-
			if (!(mouse_stats & MK_SHIFT)) if (config_pan < 9 && config_pan > -9) config_pan=0;
			in_setpan(config_pan);
			draw_panbar(config_pan,0);
			update_panning_text(-1);
			do_volbar_active=1;
		}
		if (mouse_type == -1 && which_cap == PAN_CAP)
		{
			draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
			do_volbar_active=0;
			which_cap=0;
		}
	}
	else if (which_cap==PAN_CAP)
	{
		draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
		do_volbar_active=0;
		which_cap=0;
	}
}

static void do_presetbutton()
{
	if (inreg(217,18,217+44,18+12))
	{
		if (!which_cap && mouse_stats & MK_LBUTTON)
		{
			draw_eq_presets(1);
			which_cap = PB_CAP;
		}
		if (mouse_type == -1 && which_cap == PB_CAP)
		{
			draw_eq_presets(0);
			which_cap=0;
			SendMessageW(hEQWindow,WM_COMMAND,EQ_PRESETS,0);
		}
	} else if (which_cap == PB_CAP)
	{
		which_cap=0;
		draw_eq_presets(0);
	}
}

static void do_togbuts()
{
	if (inreg(14,18,14+25+33,18+12))
	{
		int w=mouse_x >= 14+25 ? 1 : 0;
		if (mouse_type == -1)
		{
			if (w)
			{
				config_autoload_eq=!config_autoload_eq;
			}
			else
			{
				config_use_eq=!config_use_eq;
			}
			eq_set(config_use_eq, (char*)eq_tab,config_preamp);
			draw_eq_onauto(config_use_eq, config_autoload_eq, 0,0);
			PostMessageW(hMainWindow,WM_WA_IPC,IPC_CB_MISC_EQ,IPC_CB_MISC);
		}
		else if (mouse_stats & MK_LBUTTON)
		{
			which_cap = TOGBUTS_CAP;
			draw_eq_onauto(config_use_eq, config_autoload_eq, w?0:1,w?1:0);
		}
	} else if (which_cap == TOGBUTS_CAP)
	{
		which_cap=0;
		draw_eq_onauto(config_use_eq, config_autoload_eq, 0,0);
	}
}

static void do_sliders(int which)
{
	int top=39,bottom=98;
	int xoffs,w=33-21;

	if (!which) xoffs=21;
	else xoffs=78+(which-1)*(96-78);

	if (which_cap == SLID_CAP+which || inreg(xoffs,top,xoffs+w,bottom))
	{
		unsigned char *b = (which?eq_tab+which-1:&config_preamp);
		if (mouse_type == 1 || which_cap == SLID_CAP+which || (!which_cap && mouse_stats & MK_LBUTTON))
		{
			static int click_yoffs=5;
			int num_pos=63-11;
			int d;
			int p;
			int t= (mouse_type == -1 || (!(mouse_x >= xoffs-3 && mouse_x <= xoffs+w+3) && (mouse_y >= top && mouse_y <= bottom && mouse_x >= 78 && mouse_x <= 180+78) && which));
			p=63-12-((63-*b)*num_pos)/64;
			if (mouse_type == 1 && mouse_y-top >= p-1 && mouse_y-top < p + 11)
			{
				click_yoffs=mouse_y-top - p;
			} else if (mouse_type == 1)
				click_yoffs=5;
			d=((mouse_y-click_yoffs-top)*64)/num_pos;
			if (d < 0) d = 0;
			if (d > 63) d = 63;

			// changed in 5.66 for holding shift to drop the central clamp (allows 7% balance)
			if (!(mouse_stats & MK_SHIFT)) if (d >= 30 && d <= 32)	d=31;

			*b = d;
			draw_eq_slid(which,*b,t? 0 : 1);
			draw_eq_graphthingy();
			if (t)
			{
				do_posbar_active=0;
				draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());
				which_cap=0;
			} else 
			{
				wchar_t buf[128] = {0};
				float v=(float)d;
				static wchar_t preampStr[64];
				static wchar_t *bands[11] = 
				{
					preampStr,							// PREAMP
					L"70",L"180",L"320",L"600",			// Hz
					L"1",L"3",L"6",L"12",L"14",L"16"	// KHz
				};
				static wchar_t *bandsISO[11] = 
				{
					preampStr,							// PREAMP
					L"31.5",L"63",L"125",L"250",		// Hz
					L"500",L"1",L"2",L"4",L"8",L"16"	// KHz
				};
				getStringW(IDS_PREAMP,preampStr,64);
				v -= 31.5f;
				v /= 31.5f;
				v *= -12.0f;
				if (v >= -0.32 && v <= 0.32) v=0.0;

				wchar_t HZStr[16] = {0};
				getStringW((which<5+!(config_eq_frequencies==EQ_FREQUENCIES_WINAMP)?IDS_EQ_HZ:IDS_EQ_KHZ),HZStr,16);
				StringCchPrintfW(buf,128,L"EQ: %s%s: %s%0.01f %s",
								 ((config_eq_frequencies==EQ_FREQUENCIES_WINAMP)?bands[which]:bandsISO[which]),
								 (!which?L"":HZStr),
								 v>=0.0?L"+":L"", v,
								 getStringW(IDS_EQ_DB,NULL,0));

				d=0;
				do_posbar_active=1;
				draw_songname(buf,&d,-1);
				which_cap = SLID_CAP+which;
			}
			eq_set(config_use_eq, (char*)eq_tab,config_preamp);
			PostMessageW(hMainWindow,WM_WA_IPC,IPC_CB_MISC_EQ,IPC_CB_MISC);
		}
	}
}

static void do_quickbuts()
{
	int l=42, r=67;
	if (inreg(l,65,r,74) || inreg(l,33,r,42) || inreg(l,92,r,101)) // +0
	{
		if (mouse_type == -1 && which_cap == QB_CAP) 
		{
			int v;
			which_cap=0;
			if (mouse_y <= 42) v=0;
			else if (mouse_y <= 74) v=31;
			else v=63;

			memset(eq_tab,v,10);
			{
				int x;
				for (x = 1;  x <= 10; x ++)
					draw_eq_slid(x,eq_tab[x-1],0);
			}
			eq_set(config_use_eq, (char*)eq_tab,config_preamp);
			draw_eq_graphthingy();
			PostMessageW(hMainWindow,WM_WA_IPC,IPC_CB_MISC_EQ,IPC_CB_MISC);
		}
		else if (mouse_stats & MK_LBUTTON)
		{
			which_cap=QB_CAP;
		}
	}
	else if (which_cap == QB_CAP) 
	{
		which_cap=0;
	}
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
				POINT p = { mouse_x,mouse_y};
				ClientToScreen(hEQWindow,&p);
				config_eq_wx = p.x-clickx;
				config_eq_wy = p.y-clicky;
				if ((!!config_snap) ^ (!!(mouse_stats & MK_SHIFT)))
				{
					RECT rr;
					EstEQWindowRect(&rr);
					SnapWindowToAllWindows(&rr,hEQWindow);
					SetEQWindowRect(&rr);
				}
				SetWindowPos(hEQWindow,0,config_eq_wx,config_eq_wy,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			}
			break;
		}
	}
}

static void do_titlebuttons() 
{
	if (inreg(253,3,264+9,3+9)) // kill button
	{
		int ws;
		if (mouse_x < 264) ws=1;
		else ws=0;
		if (mouse_type == -1 && which_cap == TB_CAP) 
		{
			which_cap=0;
			draw_eq_tbutton(0,0);
			if (ws==0) SendMessageW(hMainWindow,WM_COMMAND,WINAMP_OPTIONS_EQ,0);
			else SendMessageW(hMainWindow,WM_COMMAND,WINAMP_OPTIONS_WINDOWSHADE_EQ,0);
		}
		else if (mouse_stats & MK_LBUTTON)
		{
			which_cap=TB_CAP;
			if (ws) draw_eq_tbutton(0,1);
			else draw_eq_tbutton(1,0);
		}
	}
	else if (which_cap == TB_CAP) 
	{
		which_cap=0;
		draw_eq_tbutton(0,0);
	}
}

void eq_ui_handlecursor(void)
{
	int mouse_x, mouse_y;
	POINT p;
	static RECT b[] = 
	{
		{264,3,272,12},//close
		{0,0,275,13},// titelbar
	};
	int b_len;
	int x;
	if (!config_usecursors || disable_skin_cursors) return;
	b_len = sizeof(b)/sizeof(b[0]);
	GetCursorPos(&p);
	ScreenToClient(hEQWindow,&p);
	mouse_x=p.x;
	mouse_y=p.y;
	if (config_dsize && config_eqdsize) { mouse_x/=2; mouse_y/=2;}

	{
		int y;
		x=1;
		for (y = 0; y < 11; y ++)
		{
			int top=39,bottom=98;
			int xoffs,w=33-21;
			if (!y) xoffs=21;
			else xoffs=78+(y-1)*(96-78);
			if (inreg(xoffs,top,xoffs+w,bottom))
			{
				x=0;
				break;
			}
		}		
		if (x) for (x = 1; x <= b_len; x ++)
			if (inreg(b[x-1].left,b[x-1].top,b[x-1].right,b[x-1].bottom)) break;
	}

	x+=25;

	if (Skin_Cursors[x]) SetCursor(Skin_Cursors[x]);
	else SetCursor(LoadCursor(NULL,IDC_ARROW));
}