#include "Main.h"
#include "VideoOSD.h"

#include "resource.h"


#include "draw.h"

#include "../nu/AutoChar.h"
#define OSD_TEXT_R 192
#define OSD_TEXT_G 192
#define OSD_TEXT_B 192
#define OSD_TEXT_R_HILITE 255
#define OSD_TEXT_G_HILITE 255
#define OSD_TEXT_B_HILITE 255
#define OSD_VOL_COL_R 0
#define OSD_VOL_COL_G 0
#define OSD_VOL_COL_B 192
#define OSD_VOL_BKCOL_R 0
#define OSD_VOL_BKCOL_G 0
#define OSD_VOL_BKCOL_B 64

#define CTRL_PROGRESSTEXT 0
#define CTRL_PROGRESS 1
#define CTRL_PROGRESSSPACER 2
#define CTRL_REW      3
#define CTRL_PLAY     4
#define CTRL_PAUSE    5
#define CTRL_STOP     6
#define CTRL_FFWD     7
#define CTRL_VOLSPACER 8
#define CTRL_VOLTEXT  9
#define CTRL_VOL      10

#define CTRLTYPE_SYMBOL   0
#define CTRLTYPE_TEXT     1
#define CTRLTYPE_PROGRESS 2
#define CTRLTYPE_SPACER   3

IVideoOSD *temp;

int g_ctrl_type[ NUM_WIDGETS ] =
{
	CTRLTYPE_TEXT,
	CTRLTYPE_PROGRESS,
	CTRLTYPE_SPACER,
	CTRLTYPE_SYMBOL,
	CTRLTYPE_SYMBOL,
	CTRLTYPE_SYMBOL,
	CTRLTYPE_SYMBOL,
	CTRLTYPE_SYMBOL,
	CTRLTYPE_SPACER,
	CTRLTYPE_TEXT,
	CTRLTYPE_PROGRESS
};


#define SHOW_STREAM_TITLE_AT_TOP 1

char progStr[64] = {0}, volStr[64] = {0};
const char *g_ctrl_text[NUM_WIDGETS] =
  {
    progStr/*"Progress "*/,
    "",
    "",
    "7",              // rew
    "4",              // play
    ";",              // pause
    "<",              // stop
    "8",              // ffwd
    "",
    volStr/*"Volume "*/,
    ""
  };


int g_ctrl_force_width[NUM_WIDGETS] =
  {
    0,
    256/*96*/,              // progress bar width
    32,              // spacer width
    0,               // rew
    0,               // play
    0,               // pause
    0,               // stop
    0,               // ffwd
    32,              // spacer width
    0,
    96/*64*/  // volume bar width
  };

IVideoOSD::IVideoOSD()
		: ctrlrects_ready(0), parent(0)
{
	temp = this;

	getString(IDS_OSD_PROGRESS_TEXT,progStr,64);
	getString(IDS_OSD_VOLUME_TEXT,volStr,64);

	last_close_height = 0;
	last_close_width = 0;
	osdMemBMW = 0;
	osdMemBMH = 0;
	osdLastMouseX = -1;
	osdLastMouseY = -1;
	ignore_mousemove_count = 0;
	osdLastClickItem = 0;
	show_osd = false;

	for (int i = 0; i < NUM_WIDGETS; i++)
		SetRect(&ctrlrect[i], 0, 0, 0, 0);
}

IVideoOSD::~IVideoOSD()
{
	KillTimer(parent, (UINT_PTR)this);
}

bool IVideoOSD::Showing()
{
	return show_osd;
}

bool IVideoOSD::Ready()
{
	return !!ctrlrects_ready;
}

int IVideoOSD::GetBarHeight()
{
	return (/*show_osd && */ctrlrects_ready) ? (ctrlrect_all.bottom - ctrlrect_all.top) : 0;
}

void IVideoOSD::HitTest(int x, int y, int dragging)
{
	if (!show_osd) return ;

	// dragging == -1: just a mousemove (no clicking)
	// dragging ==  0: user clicked
	// dragging ==  1: user clicked before, and is now dragging/moving mouse

	if (dragging < 1)
		osdLastClickItem = -1;

	// transform (x,y) from screen coords into coords relative to the memDC
	RECT lastfsrect;
	getViewport(&lastfsrect, parent, 1, NULL);
	y = y - ((lastfsrect.bottom - lastfsrect.top) - (ctrlrect_all.bottom - ctrlrect_all.top));

	int i0 = 0;
	int i1 = NUM_WIDGETS;
	if (dragging == 1)
	{
		i0 = osdLastClickItem;
		i1 = osdLastClickItem + 1;
	}

	for (int i = i0; i < i1; i++)
	{
		if (dragging == 1 || (x >= ctrlrect[i].left && x <= ctrlrect[i].right && y >= ctrlrect[i].top && y <= ctrlrect[i].bottom))
		{
			float t = (x - ctrlrect[i].left) / (float)(ctrlrect[i].right - ctrlrect[i].left);
			if (t < 0) t = 0;
			if (t > 1) t = 1;
			if (dragging < 1)
				osdLastClickItem = i;

			switch (i)
			{
				case CTRL_VOL:
					if (dragging >= 0)
					{
						int v = (int)(t * 255);
						config_volume = v;
						in_setvol(v);
						draw_volumebar(config_volume, 0);
					}
					return ;
				case CTRL_PROGRESS:

					if (dragging >= 0)
					{
						int len = in_getlength();
						if (len > 0 && !PlayList_ishidden(PlayList_getPosition()))
						{
							if (in_seek((int)(t*len*1000)) < 0)
								SendMessageW(hMainWindow, WM_WA_MPEG_EOF, 0, 0);
						}
					}
					return ;
				case CTRL_PAUSE:
					if (dragging == 0)
					{
						PostMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON3, 0);
					}
					return ;
				case CTRL_PLAY:
					if (dragging == 0)
					{
						PostMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON2, 0);
					}
					return ;
				case CTRL_STOP:
					if (dragging == 0)
					{
						PostMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON4, 0);
					}
					return ;
				case CTRL_REW:
				case CTRL_FFWD:
					if (dragging == 0)
					{
						if (i == CTRL_REW)
							PostMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON1, 0);
						else
							PostMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON5, 0);
					}
					return ;
				default:
					if (dragging < 1)
						osdLastClickItem = -1;
					break;
			}
		}
	}
}

void IVideoOSD::Show()
{
	if (!show_osd)
	{

		show_osd = true;
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		RECT r;
		GetClientRect(parent, &r);
		r.bottom = r.top + GetBarHeight();
		InvalidateRect(parent, &r, TRUE);
		GetClientRect(parent, &r);
		r.top = r.bottom - GetBarHeight();
		InvalidateRect(parent, &r, TRUE);

		PostMessageW(parent, WM_USER + 0x888, 0, 0);
	}

	KillTimer(parent, (UINT_PTR)this);
	SetTimer(parent, (UINT_PTR)this, 3000, IVideoOSD::TimerCallback);

//	Draw();
}

void CALLBACK IVideoOSD::TimerCallback(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR idEvent, DWORD/* dwTime*/)
{
	IVideoOSD *videoOSD = (IVideoOSD *)idEvent;
	videoOSD->Hide();
}

void IVideoOSD::Hide()
{


	RECT r;
	GetClientRect(parent, &r);
	r.bottom = r.top + GetBarHeight();
	InvalidateRect(parent, &r, TRUE);
	GetClientRect(parent, &r);
	r.top = r.bottom - GetBarHeight();
	InvalidateRect(parent, &r, TRUE);

	PostMessageW(parent, WM_USER + 0x888, 0, 0);

	if (show_osd)
	{
		//MessageBeep(0xFFFFFFFF);
		show_osd = false;
		KillTimer(parent, (UINT_PTR)this);
		SetCursor(NULL);
	}
	//ctrlrects_ready = 0;
}


void IVideoOSD::Draw()
{
	HDC hdc = GetDC(parent);
	if (show_osd)
	{
		HGDIOBJ osdProgressBrushBg = CreateSolidBrush(RGB(OSD_VOL_BKCOL_R, OSD_VOL_BKCOL_G, OSD_VOL_BKCOL_B));
		HGDIOBJ osdProgressBrushFg = CreateSolidBrush(RGB(OSD_VOL_COL_R, OSD_VOL_COL_G, OSD_VOL_COL_B));
		HGDIOBJ osdProgressPenBg = CreatePen(PS_SOLID, 0, RGB(OSD_TEXT_R, OSD_TEXT_G, OSD_TEXT_B));
		HGDIOBJ osdProgressPenFg = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
		HGDIOBJ osdProgressPenBgHilite = CreatePen(PS_SOLID, 0, RGB(OSD_TEXT_R_HILITE, OSD_TEXT_G_HILITE, OSD_TEXT_B_HILITE));
		HGDIOBJ osdBlackBrush = CreateSolidBrush(RGB(0, 0, 0)); //OV_COL_R,OV_COL_G,OV_COL_B));
		HFONT osdFontSymbol = CreateFontA(OSD_TEXT_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SYMBOL_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Webdings");

		HDC osdMemDC = CreateCompatibleDC(hdc);
		HBITMAP	osdMemBM = 0;  // memory bitmap (for memDC)
		HBITMAP osdOldBM = 0;  // old bitmap (from memDC)


		COLORREF fg = GetTextColor(osdMemDC);
		COLORREF bg = GetBkColor(osdMemDC);
		SetTextColor(osdMemDC, RGB(OSD_TEXT_R, OSD_TEXT_G, OSD_TEXT_B));
		SetBkColor(osdMemDC, RGB(0, 0, 0)); //OV_COL_R,OV_COL_G,OV_COL_B));

		HGDIOBJ oldfont = SelectObject(osdMemDC, osdFontText);
		HGDIOBJ oldbrush = SelectObject(osdMemDC, osdProgressBrushBg);
		HGDIOBJ oldpen = SelectObject(osdMemDC, osdProgressPenBg);

		RECT fullr;
		GetClientRect(parent, &fullr);

		/*		ClientToScreen(parent, (LPPOINT)&fullr);
				ClientToScreen(parent, ((LPPOINT)&fullr) + 1);
				// transform coords from windows desktop coords (where 0,0==upper-left corner of the primary monitor)
				// to the coords for the monitor we're displaying on:
				fullr.top -= m_mon_y;
				fullr.left -= m_mon_x;
				fullr.right -= m_mon_x;
				fullr.bottom -= m_mon_y;
		*/
		int streaming = (in_getlength() < 0) || !in_mod || !in_mod->is_seekable;

		if (ctrlrects_ready != streaming + 1)
		{
			ctrlrects_ready = streaming + 1;

			int net_width = 0;
			int max_height = 0;
			int i;
			for (i = 0; i < NUM_WIDGETS; i++)
			{
				SetRect(&ctrlrect[i], 0, 0, 0, 0);
				if (streaming && (i == CTRL_PROGRESS || i == CTRL_PROGRESSTEXT || i == CTRL_PROGRESSSPACER || i == CTRL_FFWD || i == CTRL_REW))
				{
					// disable progress bar + seek arrows when the NSV is a stream
					ctrlrect[i].right = -1;
					continue;
				}
				else if (g_ctrl_force_width[i] != 0)
				{
					SetRect(&ctrlrect[i], 0, 0, g_ctrl_force_width[i], 0);
				}
				else
				{
					SelectObject(osdMemDC, (g_ctrl_type[i] == CTRLTYPE_SYMBOL) ? osdFontSymbol : osdFontText);
					SetRect(&ctrlrect[i], 0, 0, 256, 256);
					ctrlrect[i].bottom = DrawTextA(osdMemDC, g_ctrl_text[i], -1, &ctrlrect[i], DT_SINGLELINE | DT_CALCRECT);
				}
				net_width += ctrlrect[i].right - ctrlrect[i].left;
				max_height = max(max_height, ctrlrect[i].bottom - ctrlrect[i].top);
			}

			// now we know the size of all the controls; now place them.
			int x = (fullr.right + fullr.left) / 2 - net_width / 2;
			SetRect(&ctrlrect_all, 0, 0, 0, 0);
			for (i = 0; i < NUM_WIDGETS; i++)
			{
				if (ctrlrect[i].right >= 0) // if control is not disabled...
				{
					int this_width = ctrlrect[i].right - ctrlrect[i].left;
					int this_height = ctrlrect[i].bottom - ctrlrect[i].top ;
					if (this_height == 0) this_height = max_height * 2 / 3; // progress bars
					ctrlrect[i].top = max_height / 2 - this_height / 2;
					ctrlrect[i].bottom = max_height / 2 + this_height / 2;
					ctrlrect[i].left = x;
					ctrlrect[i].right = x + this_width;
					if (ctrlrect_all.bottom == 0)
					{
						ctrlrect_all.top = ctrlrect[i].top ;
						ctrlrect_all.bottom = ctrlrect[i].bottom;
					}
					else
					{
						ctrlrect_all.top = min(ctrlrect_all.top , ctrlrect[i].top);
						ctrlrect_all.bottom = max(ctrlrect_all.bottom, ctrlrect[i].bottom);
					}
					x += this_width;
				}
			}
		}

		int w = fullr.right - fullr.left;
		int h = ctrlrect_all.bottom - ctrlrect_all.top;
		if (!osdMemBM || osdMemBMW != w || osdMemBMH != h)
		{
			if (osdMemBM)
			{
				SelectObject(osdMemDC, osdOldBM);
				DeleteObject(osdMemBM);
			}
			osdMemBM = CreateCompatibleBitmap(hdc, w, h);
			osdOldBM = (HBITMAP)SelectObject(osdMemDC, osdMemBM);
			osdMemBMW = w;
			osdMemBMH = h;
		}

		RECT temp;
		SetRect(&temp, 0, 0, w, h);
		FillRect(osdMemDC, &temp, (HBRUSH)osdBlackBrush);

		for (int i = 0; i < NUM_WIDGETS; i++)
		{
			if (g_ctrl_type[i] == CTRLTYPE_PROGRESS)
			{
				int progress = 0;
				int max_progress = ctrlrect[i].right - ctrlrect[i].left;
				switch (i)
				{
					case CTRL_VOL:
						progress = config_volume * max_progress / 255;
						break;
					case CTRL_PROGRESS:
						if (playing)
						{
							int len = in_getlength();
							if (len > 0) progress = (in_getouttime() / 1000) * max_progress / len;
						}
						if (progress > max_progress) progress = max_progress;
						break;
				}

				SelectObject(osdMemDC, osdProgressBrushBg);
				SelectObject(osdMemDC, (i == osdLastClickItem) ? osdProgressPenBgHilite : osdProgressPenBg);
				RoundRect(osdMemDC, ctrlrect[i].left, ctrlrect[i].top, ctrlrect[i].right, ctrlrect[i].bottom, 3, 3);
				SelectObject(osdMemDC, osdProgressBrushFg);
				SelectObject(osdMemDC, osdProgressPenFg);
				Rectangle(osdMemDC, ctrlrect[i].left + 1, ctrlrect[i].top + 1, ctrlrect[i].left + progress, ctrlrect[i].bottom);
			}
			else if (g_ctrl_type[i] == CTRLTYPE_SYMBOL ||
			         g_ctrl_type[i] == CTRLTYPE_TEXT)
			{
				SelectObject(osdMemDC, (g_ctrl_type[i] == CTRLTYPE_SYMBOL) ? osdFontSymbol : osdFontText);
				SetTextColor(osdMemDC, (i == osdLastClickItem) ? RGB(OSD_TEXT_R_HILITE, OSD_TEXT_G_HILITE, OSD_TEXT_B_HILITE) : RGB(OSD_TEXT_R, OSD_TEXT_G, OSD_TEXT_B));
				DrawTextA(osdMemDC, g_ctrl_text[i], -1, &ctrlrect[i], DT_SINGLELINE);
			}
		}

		int x0 = fullr.left;
		int y0 = fullr.bottom - (ctrlrect_all.bottom - ctrlrect_all.top);
		BitBlt(hdc, x0, y0, w, h, osdMemDC, 0, 0, SRCCOPY);

		// display stream title @ the top:
#if (SHOW_STREAM_TITLE_AT_TOP)
		if (1)
		{
			RECT temp;
			SetRect(&temp, 0, 0, w, h);
			FillRect(osdMemDC, &temp, (HBRUSH)osdBlackBrush);

			SelectObject(osdMemDC, osdFontText);
			SetTextColor(osdMemDC, RGB(OSD_TEXT_R, OSD_TEXT_G, OSD_TEXT_B));
			AutoChar narrowTitle(FileTitle);
			wchar_t buf[FILETITLE_SIZE+32] = {0};

			StringCchPrintfW(buf, sizeof(buf)/sizeof(*buf), L"%s (%d %s)", FileTitle ? FileTitle : L"", g_brate, getStringW(IDS_KBPS,NULL,0));
			if ((config_fixtitles&2))
			{
				wchar_t *p = buf;
				while (p && *p)
				{
					if (*p == '_') // replace _ with space
						*p = ' ';
					p = CharNextW(p);
				}
			}
			DrawTextW(osdMemDC, buf, -1, &temp, DT_SINGLELINE | DT_CENTER);

			SelectObject(osdMemDC, osdFontSymbol);
			DrawTextW(osdMemDC, L"2", -1, &temp, DT_SINGLELINE | DT_RIGHT);
			RECT rr = {0, 0, w, h};
			DrawTextW(osdMemDC, L"2", -1, &rr, DT_SINGLELINE | DT_RIGHT | DT_CALCRECT);
			last_close_height = rr.bottom - rr.top;
			last_close_width = rr.right - rr.left;


			int x0 = fullr.left;
			int y0 = fullr.top;
			BitBlt(hdc, x0, y0, w, h, osdMemDC, 0, 0, SRCCOPY);
		}

		SelectObject(osdMemDC, oldpen);
		SelectObject(osdMemDC, oldbrush);
		SelectObject(osdMemDC, oldfont);
		SetTextColor(osdMemDC, fg);
		SetBkColor(osdMemDC, bg);

		DeleteObject(osdProgressBrushBg);
		DeleteObject(osdProgressBrushFg);
		DeleteObject(osdBlackBrush);
		DeleteObject(osdProgressPenBg);
		DeleteObject(osdProgressPenFg);
		DeleteObject(osdProgressPenBgHilite);
		DeleteObject(osdFontSymbol);
		if (osdMemDC)
		{
			SelectObject(osdMemDC, osdOldBM);	// delete our doublebuffer
			DeleteDC(osdMemDC);
		}
		if (osdMemBM) DeleteObject(osdMemBM);
	}
#endif
	ReleaseDC(parent, hdc);
}

bool IVideoOSD::CloseHitTest(int x, int y)
{
	RECT r;
	GetClientRect(parent, &r);
	return (x > r.right - last_close_width && y < last_close_height);
}

bool IVideoOSD::Mouse(int x, int y, WPARAM wParam, bool moving)
{
	if (wParam & MK_LBUTTON)
	{
		Show();
		if (CloseHitTest(x, y))
			return true;
		ignore_mousemove_count = 2;
		HitTest(x, y, moving ? 1 : 0);
	}
	else
	{
		if (((osdLastMouseX - x) || (osdLastMouseY - y)) && (ignore_mousemove_count--))
		{
			Show();
			HitTest(x, y, moving ? -1 : 0);
			ignore_mousemove_count = 2;
		}
	}

	osdLastMouseX = x;
	osdLastMouseY = y;
	return false;
}

