#include <windowsx.h>

#include "main.h"
#include <ddraw.h>
#include <multimon.h>
#include "api.h"
#include "vid_overlay.h"
#include "vid_ddraw.h"
#include "vid_subs.h"
#include "vid_none.h"
#include "VideoOutput.h"

#include "Browser.h"
#include "video.h"
#include "../nu/AutoWide.h"
#include "WinampAttributes.h"
#include "resource.h"

#define WM_VIDEO_UPDATE_STATUS_TEXT WM_USER+0x874
#define WM_VIDEO_OPEN WM_USER+0x875
#define WM_VIDEO_CLOSE WM_USER+0x876
#define WM_VIDEO_RESIZE WM_USER+0x877
#define WM_VIDEO_CREATE WM_USER+0x900

#define VIDEO_GENFF_SIZEREQUEST (WM_USER+2048)
#undef GetSystemMetrics

#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))

VideoOutput *m_videooutput = NULL;

static int VW_OnLButtonUp(HWND hwnd, int x, int y, UINT flags);
static int VW_OnRButtonUp(HWND hwnd, int x, int y, UINT flags);
static int VW_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static int VW_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
static int VW_OnLButtonDblClk(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static BOOL VW_OnNCActivate(HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized);
extern "C" int g_video_numaudiotracks;
extern "C" int is_fullscreen_video = 0;
extern "C" int no_notify_play;
extern "C" int last_no_notify_play;
#undef GetSystemMetrics

bool sizeOnOpen = false;
int widthOnOpen, heightOnOpen;
#define VideoClassicWidth() 19
#define VideoClassicHeight() 58
static void VideoClose();

/* Resizes the windows (parent and children) based on what the size of the video should be */
static void SetVideoSize(int width, int height)
{
	if (m_videooutput && m_videooutput->is_fullscreen()) // fullscreen
	{
		m_videooutput->SetVideoPosition(0, 0, config_video_width, config_video_height);
	}
	else 	// not fullscreen
	{
		// send out ideal video size message (in case anyone wants it)
		PostMessageW(hMainWindow, WM_WA_IPC, (((width) & 0xFFFF) << 16) | ((height) & 0xFFFF), IPC_SETIDEALVIDEOSIZE);
		if (GetParent(hVideoWindow)) // if gen_ff owns the window, then signal it about the video size
			PostMessageW(GetParent(hVideoWindow), VIDEO_GENFF_SIZEREQUEST, width, height);
		else // classic skin
			SetWindowPos(hVideoWindow, 0, 0, 0, width + VideoClassicWidth(), height + VideoClassicHeight(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

/*	Resizes the windows (parent and children) based on what the size of the parent should be 	*/
void SetExteriorSize(int width, int height)
{
	// calculate interior size
	width -= VideoClassicWidth();
	height -= VideoClassicHeight();

	// pass our calculated value onto the size routine
	SetVideoSize(width, height);
}

/* lays out children (video and ad) based on a given parent window size */
static void LayoutChildren(int width, int height)
{
	if (m_videooutput && m_videooutput->is_fullscreen()) // fullscreen
	{
		m_videooutput->SetVideoPosition(0, 0, config_video_width, config_video_height);
	}
	else // not fullscreen
	{
		// calculate interior size
		width -= VideoClassicWidth();
		height -= VideoClassicHeight();

		// size the video
		if (m_videooutput)
		{
			m_videooutput->SetVideoPosition(11, 20, width, height);
			InvalidateRect(m_videooutput->getHwnd(), 0, TRUE); // force repaint of video
		}
		InvalidateRect(hVideoWindow, 0, TRUE); // force repaint of parent window
	}
}

/* Resizes the windows (parent and children) based on what the size of the parent should be
Sets the position of the parent window */
static void SetExteriorSizeAndPosition(int x, int y, int width, int height)
{
	config_video_wx = x;
	config_video_wy = y;
	SetExteriorSize(width, height);
}

void updateTrackSubmenu()
{
	HMENU menu = GetSubMenu(GetSubMenu(top_menu, 3), 13);
	HMENU audiomenu = GetSubMenu(menu, 6);
	HMENU videomenu = GetSubMenu(menu, 7);
	static int audioadded = 0;
	static int videoadded = 0;
	static int first = 1;
	if (first)
	{
		RemoveMenu(audiomenu, ID_VID_AUDIO0, MF_BYCOMMAND);
		RemoveMenu(videomenu, ID_VID_VIDEO0, MF_BYCOMMAND);
		first = 1;
	}

	if (audioadded)
	{
		for (int i = 0;i < 16;i++)
		{
			RemoveMenu(audiomenu, ID_VID_AUDIO0 + i, MF_BYCOMMAND);
		}
		audioadded = 0;
	}
	if (videoadded)
	{
		for (int i = 0;i < 16;i++)
		{
			RemoveMenu(videomenu, ID_VID_VIDEO0 + i, MF_BYCOMMAND);
		}
		videoadded = 0;
	}

	VideoOutput *out = (VideoOutput *)video_getIVideoOutput();
	if (!out || !out->getTrackSelector())
	{
		EnableMenuItem(menu, 8, MF_GRAYED | MF_BYPOSITION);
		EnableMenuItem(menu, 9, MF_GRAYED | MF_BYPOSITION);
		return ;
	}

	ITrackSelector *sel = out->getTrackSelector();

	int numaudiotracks = sel->getNumAudioTracks();
	if (numaudiotracks < 2)
	{
		EnableMenuItem(menu, 8, MF_GRAYED | MF_BYPOSITION);
	}
	else
	{
		audioadded = 1;
		int curtrack = sel->getCurAudioTrack();
		for (int i = 0;i < numaudiotracks;i++)
		{
			char t[256] = {0};
			sel->enumAudioTrackName(i, t, 255);
			InsertMenuA(audiomenu, i, MF_BYPOSITION, ID_VID_AUDIO0 + i, t);
			CheckMenuItem(audiomenu, ID_VID_AUDIO0 + i, ((i == curtrack) ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
		}
		EnableMenuItem(menu, 8, MF_ENABLED | MF_BYPOSITION);
	}

	int numvideotracks = sel->getNumVideoTracks();
	if (numvideotracks < 2)
	{
		EnableMenuItem(menu, 9, MF_GRAYED | MF_BYPOSITION);
	}
	else
	{
		videoadded = 1;
		int curtrack = sel->getCurVideoTrack();
		for (int i = 0;i < numvideotracks;i++)
		{
			char t[256] = {0};
			sel->enumVideoTrackName(i, t, 255);
			InsertMenuA(videomenu, i, MF_BYPOSITION, ID_VID_VIDEO0 + i, t);
			CheckMenuItem(videomenu, ID_VID_VIDEO0 + i, ((i == curtrack) ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
		}
		EnableMenuItem(menu, 9, MF_ENABLED | MF_BYPOSITION);
	}
}

int ShowVideoWindow(int init_state)
{
	sizeOnOpen = false;

	if (config_video_open // if we're already open
	    || !hVideoWindow // or we havn't made the video window yet
	    || !g_has_video_plugin // or we're configured to not have video
	    || !Ipc_WindowToggle(IPC_CB_WND_VIDEO, 1))  // or some plugin doesn't want us to open the video window
		return 0; // then bail out

	CheckMenuItem(main_menu, WINAMP_OPTIONS_VIDEO, MF_CHECKED);
	if(!init_state && !config_minimized) ShowWindow(hVideoWindow, SW_SHOWNA);
	//ad->show(true);
	config_video_open = 1;
	set_aot(1);
	return 1;
}

void HideVideoWindow(int autoStop)
{
	sizeOnOpen = false;

	if (!config_video_open // if we're not even open
	    || !hVideoWindow // or we havn't made the video window yet
	    //	|| !g_has_video_plugin // or we're configured to not have video
	    || !Ipc_WindowToggle(IPC_CB_WND_VIDEO, !config_video_open)) // or some plugin doesn't want us to close the video window
		return ; // then bail out

	if (GetForegroundWindow() == hVideoWindow || IsChild(hVideoWindow, GetForegroundWindow()))
	{
		SendMessageW(hMainWindow, WM_COMMAND, WINAMP_NEXT_WINDOW, 0);
	}
	CheckMenuItem(main_menu, WINAMP_OPTIONS_VIDEO, MF_UNCHECKED);
	ShowWindow(hVideoWindow, SW_HIDE);

	config_video_open = 0;

	if (autoStop && config_video_stopclose && !(GetAsyncKeyState(VK_SHIFT)&0x8000) && video_isVideoPlaying())
	{
		PostMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON4, 0);
	}
}

HMENU BuildPopupMenu();

void Vid_Cmd(windowCommand *wc)
{
	switch (wc->cmd)
	{
	case VIDCMD_FULLSCREEN:
		videoGoFullscreen();
		break;
	case VIDCMD_1X:
		SendMessageW(videoGetHwnd(), WM_COMMAND, ID_VIDEOWND_ZOOM100, 0);
		break;
	case VIDCMD_2X:
		SendMessageW(videoGetHwnd(), WM_COMMAND, ID_VIDEOWND_ZOOM200, 0);
		break;
	case VIDCMD_LIB:
		SendMessageW(videoGetHwnd(), WM_COMMAND, WINAMP_VIDEO_TVBUTTON, 0);
		break;
	case VIDPOPUP_MISC:
		DoTrackPopup(BuildPopupMenu(), wc->align, wc->x, wc->y, videoGetHwnd());
		break;
	case VIDCMD_EXIT_FS:
		videoForceFullscreenOff();
		break;
	}
}

static int VW_OnRButtonUp(HWND hwnd, int x, int y, UINT flags)
{
	POINT p;
	extern HMENU top_menu;
	GetCursorPos(&p);
	DoTrackPopup(main_menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, hMainWindow);
	return 1;
}

static int VW_OnLButtonUp(HWND hwnd, int x, int y, UINT flags)
{
	ReleaseCapture();
	videoui_handlemouseevent(x, y, -1, flags);
	return 1;
}

static int VW_OnLButtonDown( HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
	SetCapture( hwnd );
	videoui_handlemouseevent( x, y, 1, keyFlags );
	SetFocus( hwnd );

	return 1;
}

static int VW_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	videoui_handlemouseevent(x, y, 0, keyFlags);

	return 1;
}

static BOOL VW_OnNCActivate( HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized )
{
	if ( !m_videooutput || ( m_videooutput && !m_videooutput->is_fullscreen() ) )
	{
		if ( fActive == FALSE )
			draw_vw_tbar( config_hilite ? 0 : 1 );
		else
			draw_vw_tbar( 1 );
	}

	return TRUE;
}

static int VW_OnLButtonDblClk(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	return 1;
}

/*Turns off and on the screensaver*/
VOID CALLBACK ResetScreenSaver(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	// benski> this seems less hackish.  only win2k and up
	if (config_video_noss && video_isVideoPlaying())
		SetThreadExecutionState(ES_DISPLAY_REQUIRED);
	// TODO: maybe we should do the same thing with ES_SYSTEM_REQUIRED to keep the system from snoozing

	// benski> old code was here
	/*
	BOOL b;
	if (config_video_noss && video_isVideoPlaying() && SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &b, 0) && b)
	{
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0, 0, 0); // turn off
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 1, 0, 0); // turn back on
		// this is just a hack
		// basically by toggling off and on we reset the timeout count
	}
*/
}

static void VideoOpen(HWND hwnd, int width, int height)
{
	sizeOnOpen=false;

	// check if we appear to be doing a resume from a tag edit
	// and if so then we need to ignore the show window option
	if (config_video_autoopen &&
		((no_notify_play != last_no_notify_play && !last_no_notify_play) || (no_notify_play == last_no_notify_play)))
		ShowVideoWindow(0);
	last_no_notify_play = no_notify_play;

	HWND skinVidWindow = GetParent(hwnd) ? GetParent(hwnd) : hwnd;

	if (m_videooutput->is_fullscreen() || config_video_auto_fs) // go fullscreen
	{
		videoGoFullscreen();
		InvalidateRect(m_videooutput->getHwnd(), 0, TRUE);
	}
	else // not fullscreen
	{
		if (config_video_updsize)
		{
			widthOnOpen = width;
			heightOnOpen = height;
			sizeOnOpen = true;
			SetVideoSize(width, height);
		}
		LayoutChildren(config_video_width, config_video_height);

		if (!IsIconic(skinVidWindow))
			BringWindowToTop(skinVidWindow);
		InvalidateRect(hwnd, 0, TRUE);
	}
}

LRESULT CALLBACK video_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == g_scrollMsg) { wParam <<= 16; uMsg = WM_MOUSEWHEEL; }

	if (uMsg == WM_LBUTTONUP || uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_MOUSEMOVE)
		if (!m_videooutput || !m_videooutput->is_fullscreen())
		{
			switch (uMsg)
			{
				HANDLE_MSG(hwnd, WM_LBUTTONUP,   VW_OnLButtonUp);
				HANDLE_MSG(hwnd, WM_LBUTTONDOWN, VW_OnLButtonDown);
				HANDLE_MSG(hwnd, WM_MOUSEMOVE,   VW_OnMouseMove);
				HANDLE_MSG(hwnd, WM_RBUTTONUP,   VW_OnRButtonUp);
			}
		}

	switch (uMsg)
	{
	case WM_USER + 1:
		if (m_videooutput)
			SendMessageW(m_videooutput->getHwnd(), uMsg, wParam, lParam);
		break;

	case WM_USER + 2:
		if (sizeOnOpen)
		{
			//				sizeOnOpen=false;
			PostMessageW(hwnd, WM_VIDEO_RESIZE, widthOnOpen, heightOnOpen);
		}
		if (m_videooutput)
			SendMessageW(m_videooutput->getHwnd(), uMsg, wParam, lParam);
		break;

	case WM_USER + 0x100:
		if (wParam == 1 && lParam)
		{
			config_video_wx = ((POINT *)lParam)->x;
			config_video_wy = ((POINT *)lParam)->y;
			if ((!!config_snap) ^ (!!(GetKeyState(VK_SHIFT) & 0x8000)))
			{
				RECT outrc;
				EstVidWindowRect(&outrc);
				SnapWindowToAllWindows(&outrc, hVideoWindow);
				SetVidWindowRect(&outrc);
			}
			SetWindowPos(hVideoWindow, 0, config_video_wx, config_video_wy, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		return 0;
	case WM_TIMER:
		switch (wParam)
		{
			case 12345:                    // signal from elsewhere
				if (m_videooutput)
					SendMessageW(m_videooutput->getHwnd(), WM_TIMER, 0, 0);
			break;
		}
		return 0;
	case WM_WINDOWPOSCHANGING:
	case WM_WINDOWPOSCHANGED:         // we trap both windowposchanging and windowposchanged incase someone uses SetWindowPos(..., SWP_NOSENDCHANGING)
		{
			LPWINDOWPOS windowPos = (LPWINDOWPOS) lParam;
			if (windowPos->flags & SWP_NOSIZE)
			{
				break;
			}

			if (uMsg == WM_WINDOWPOSCHANGED)
			{
				config_video_width = windowPos->cx;
				config_video_height = windowPos->cy;
				LayoutChildren(config_video_width, config_video_height);
				// update the position of the tooltips on window resize
				set_vid_wnd_tooltip();
			}
		}
		break;

	case WM_NOTIFY:
		{
			LPTOOLTIPTEXT tt = (LPTOOLTIPTEXT)lParam;
			if(tt->hdr.hwndFrom = hVideoTooltipWindow)
			{
				switch (tt->hdr.code)
				{
					case TTN_SHOW:
						SetWindowPos(tt->hdr.hwndFrom,HWND_TOPMOST,0,0,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
						break;
				}
			}
		}
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *p = (MINMAXINFO *)lParam;
			p->ptMaxTrackSize.x = 16384;
			p->ptMaxTrackSize.y = 16384;
		}
		return 0;

	case WM_VIDEO_RESIZE:
		//ReplyMessage(0); // if IVideoOutput::open() was called on a different thread, this will unblock it.
		{
			int width = wParam;
			int height = lParam;
			//			idealWidth = wParam;
			SetVideoSize(width, height);
			LayoutChildren(config_video_width, config_video_height);
		}
		return 0;

	case WM_DESTROY:
		if (NULL != WASABI_API_APP) WASABI_API_APP->app_unregisterGlobalWindow(hwnd);
		if (vw_init)
			draw_vw_kill();
		break;

	case WM_DISPLAYCHANGE:
		InvalidateRect(hwnd, NULL, TRUE);
		break;

	case WM_VIDEO_OPEN:
		VideoOpen(hwnd, wParam, lParam);
		//		set_aot(1);
		return 0;
		break;

	case WM_VIDEO_CREATE:
		m_videooutput->mainthread_Create();
		break;

	case WM_VIDEO_CLOSE:
		VideoClose();
		InvalidateRect(hwnd, NULL, TRUE); //repaint
		return 0;

	case WM_VIDEO_UPDATE_STATUS_TEXT:
		if (m_videooutput && !m_videooutput->is_fullscreen()) 
			draw_vw_info((wchar_t*)wParam, 1);
		videoTextFeed->UpdateText((const wchar_t*)wParam, 1024);
		return 0;

	case WM_MOUSEWHEEL:
		return SendMessageW(hMainWindow, uMsg, wParam, lParam);

	HANDLE_MSG(hwnd, WM_QUERYNEWPALETTE, Main_OnQueryNewPalette);
	HANDLE_MSG(hwnd, WM_PALETTECHANGED, Main_OnPaletteChanged);
	HANDLE_MSG(hwnd, WM_NCACTIVATE, VW_OnNCActivate);
	HANDLE_MSG(hwnd, WM_LBUTTONDBLCLK, VW_OnLButtonDblClk);

	case WM_CONTEXTMENU:
		{
			if (lParam == MAKELONG(-1, -1))
			{
				extern HMENU top_menu;
				RECT r;
				GetWindowRect(videoGetHwnd(), &r);
				DoTrackPopup(BuildPopupMenu(), TPM_LEFTALIGN | TPM_RIGHTBUTTON, r.left, r.top, videoGetHwnd());
			}
		}
		return 0;

	case WM_KEYDOWN: case WM_KEYUP: case WM_SYSKEYDOWN: case WM_SYSKEYUP:
		if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && wParam == VK_F4)
		{
			if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
				SendMessageW(hMainWindow, WM_COMMAND, WINAMP_OPTIONS_VIDEO, 0);
		}
		/*      else if(uMsg==WM_SYSKEYDOWN && (LPARAM&(1<<29)) && wParam == VK_RETURN)
		      {
		        if (m_videooutput)
		        {
		          if(!m_videooutput->is_fullscreen()) m_videooutput->fullscreen();
		          else m_videooutput->remove_fullscreen();
		        }
		      }*/
		else
		{
			if (SendMessageW(m_videooutput->getHwnd(), uMsg, wParam, lParam)) return 0;
			if (wParam != VK_RETURN && m_videooutput->is_fullscreen() && ((GetAsyncKeyState(VK_CONTROL) | GetAsyncKeyState(VK_MENU))&0x8000)) return 0;
			{
				MSG winmsg;
				winmsg.message = uMsg;
				winmsg.hwnd = hMainWindow;
				winmsg.wParam = wParam;
				winmsg.lParam = lParam;
				if (WASABI_API_APP->app_translateAccelerators(&winmsg)) return 0;
				//if (transAccel(hwnd,uMsg,wParam,lParam)) return 0;
				//transAccelStruct tas = {hwnd, uMsg, wParam, lParam};
				//if (SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&tas, IPC_TRANSLATEACCELERATOR)) return 0;
			}
		}
		break;

	case WM_DROPFILES:
		return SendMessageW(hMainWindow, uMsg, wParam, lParam);

	case WM_SHOWWINDOW:
		if (wParam == TRUE) // showing
		{
			if (sizeOnOpen)
			{
				//sizeOnOpen=false;
				PostMessageW(hwnd, WM_VIDEO_RESIZE, widthOnOpen, heightOnOpen);
			}

			if (!IsIconic(hwnd))
				BringWindowToTop(hwnd);
			LayoutChildren(config_video_width, config_video_height);
		}

		RefreshIconicThumbnail();

		/*
		{
		      // if extra_data[EMBED_STATE_EXTRA_REPARENTING] is set, we are being reparented by the freeform lib, so we should
		      // just ignore this message because our visibility will not change once the freeform
		      // takeover/restoration is complete
		      embedWindowState *ws = (embedWindowState *)GetWindowLongW(hwnd, GWL_USERDATA);
		      if (ws != NULL && ws->extra_data[EMBED_STATE_EXTRA_REPARENTING]) 
			  {
			  }
		    }
		*/
		break;

	case WM_CLOSE:
		if (!m_videooutput || !m_videooutput->is_fullscreen())
			WASABI_API_APP->main_shutdown();
		else
			m_videooutput->remove_fullscreen();
		return 0;

	case WM_PAINT:
		if (!m_videooutput || !m_videooutput->is_fullscreen())
		{
			draw_paint_vw(hwnd);
			return 0;
		}
		break;

	case WM_CREATE:
		hVideoWindow = hwnd;
		SetTimer(hwnd, 32, 15000, ResetScreenSaver);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (config_keeponscreen&2) ? 0x49474541 : 0);
		SetWindowLongW(hwnd, GWL_STYLE, GetWindowLongW(hwnd, GWL_STYLE)&~(WS_CAPTION));
		SetWindowPos(hVideoWindow, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		//ad->CreateHWND(hwnd);
		m_videooutput = new VideoOutput(hwnd);
		if (!config_minimized)
			ShowWindow(m_videooutput->getHwnd(), SW_SHOWNA);
		SetExteriorSizeAndPosition(config_video_wx, config_video_wy, config_video_width, config_video_height);
		LayoutChildren(config_video_width, config_video_height);
		
		if (NULL != WASABI_API_APP) WASABI_API_APP->app_registerGlobalWindow(hwnd);
		
		return 0;

	case WM_COMMAND:
		return SendMessageW(hMainWindow, uMsg, wParam, lParam);

	case WM_MOVE:
		if (m_videooutput)
			SendMessageW(m_videooutput->getHwnd(), WM_MOVE, 0, 0);
		return 0;

	case WM_SETCURSOR:
		if (config_usecursors && !disable_skin_cursors)
		{
#define inreg(x,y,x2,y2) \
	((mouse_x <= ( x2 ) && mouse_x >= ( x ) &&  \
	  mouse_y <= ( y2 ) && mouse_y >= ( y )))

			if (((HWND)wParam == hVideoWindow || IsChild(hVideoWindow, (HWND)wParam)) && HIWORD(lParam) == WM_MOUSEMOVE)
			{
				int mouse_x, mouse_y;
				POINT p;
				static RECT b[] =
				    {
				        { -(275 - 264), 3, -(275 - 272), 12},                               //close
				        {0, 0, -1, 13},                               // titelbar
				        { -20, -20, -1, -1},
				    };

				int iconoffs[] = {15 + 1, 15 + 2, 15 + 4, 15 + 5};
				int b_len = 3;
				int x;
				GetCursorPos(&p);
				ScreenToClient(hVideoWindow, &p);
				mouse_x = p.x;
				mouse_y = p.y;

				for (x = 0; x < b_len; x ++)
				{
					int l, r, t, bo;
					l = b[x].left;r = b[x].right;t = b[x].top;bo = b[x].bottom;
					if (l < 0) l += config_video_width;
					if (r < 0) r += config_video_width;
					if (t < 0) t += config_video_height;
					if (bo < 0) bo += config_video_height;
					if (inreg(l, t, r, bo)) break;
				}

				if (Skin_Cursors[iconoffs[x]]) SetCursor(Skin_Cursors[iconoffs[x]]);
				else SetCursor(LoadCursorW(NULL, IDC_ARROW));
			}
			return TRUE;
		}
		else SetCursor(LoadCursorW(NULL, IDC_ARROW));
		return TRUE;

	case WM_SYSCOMMAND:
		// eat screen saver message when fullscreen
		if (((wParam & 0xfff0) == SC_SCREENSAVE || (wParam & 0xfff0) == SC_MONITORPOWER) && config_video_noss &&
		    video_isVideoPlaying())
		{
			return -1;
		}
		break;
	}

	if (FALSE != IsDirectMouseWheelMessage(uMsg))
	{
		SendMessageW(hwnd, WM_MOUSEWHEEL, wParam, lParam);
		return TRUE;
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static void VideoClose()
{
	sizeOnOpen=false;
	if (!g_has_video_plugin)
		return ;

	if (m_videooutput && m_videooutput->is_fullscreen() && config_video_remove_fs_on_stop)
		videoForceFullscreenOff();

	if ((!m_videooutput && (bool)config_video_autoclose)
	    || (m_videooutput && !m_videooutput->is_fullscreen() && (bool)config_video_autoclose))
		HideVideoWindow(false);
}

extern "C"
{
	void videoAdSizeChanged()
	{
		LayoutChildren(config_video_width, config_video_height);
	}

	void *video_getIVideoOutput()
	{
		return (void *)m_videooutput;
	}

	int video_isVideoPlaying()
	{
		if (!m_videooutput) return 0;
		return m_videooutput->isVideoPlaying();
	}

	void videoGoFullscreen()
	{
		if (m_videooutput)
		{
			m_videooutput->fullscreen();
		}
	}

	int videoIsFullscreen()
	{
		if (m_videooutput) return !!m_videooutput->is_fullscreen();
		return 0;
	}

	void videoReinit()
	{
		if (m_videooutput)
			PostMessageW(m_videooutput->getHwnd(), WM_USER + 0x1, 0, 0);
	}

	void videoForceFullscreenOff()
	{
		if (m_videooutput)
		{
			m_videooutput->remove_fullscreen();
			LayoutChildren(config_video_width, config_video_height);
		}
	}

	void videoToggleFullscreen()
	{
		if (m_videooutput)
		{
			if (!m_videooutput->is_fullscreen())
				videoGoFullscreen();
			else
				videoForceFullscreenOff();
		}
	}

	void videoSetFlip(int on)
	{
		if (m_videooutput)
		{
			m_videooutput->extended(VIDUSER_SET_VFLIP, on, 0);
		}
		// for consistency we need to update the state on the prefs page
		if(prefs_last_page == 24 && IsWindow(prefs_hwnd))
		{
			SendMessageW(prefs_hwnd, WM_USER + 33, IDC_PREFS_VIDEO_FLIPRGB, on);
		}
		CheckMenuItem(GetSubMenu(GetSubMenu(top_menu, 3), 13), ID_VIDEOWND_VERTICALLYFLIP, (on ? MF_CHECKED: MF_UNCHECKED));
	}

	DWORD videoGetWidthHeightDWORD()
	{
		if (!m_videooutput) return 0;
		return m_videooutput->GetWidthHeightDWORD();
	}

	HWND videoGetHwnd()
	{
		if (m_videooutput) return m_videooutput->getHwnd();
		return NULL;
	}

	int video_getNumAudioTracks()
	{
		VideoOutput *vid = (VideoOutput *)video_getIVideoOutput();
		if (!vid) return 1;
		ITrackSelector *sel = vid->getTrackSelector();
		if (!sel) return 1;
		return sel->getNumAudioTracks();
	}

	int video_getNumVideoTracks()
	{
		VideoOutput *vid = (VideoOutput *)video_getIVideoOutput();
		if (!vid) return 1;
		ITrackSelector *sel = vid->getTrackSelector();
		if (!sel) return 1;
		return sel->getNumVideoTracks();
	}

	int video_getCurAudioTrack()
	{
		VideoOutput *vid = (VideoOutput *)video_getIVideoOutput();
		if (!vid) return 0;
		ITrackSelector *sel = vid->getTrackSelector();
		if (!sel) return 0;
		return sel->getCurAudioTrack();
	}

	int video_getCurVideoTrack()
	{
		VideoOutput *vid = (VideoOutput *)video_getIVideoOutput();
		if (!vid) return 0;
		ITrackSelector *sel = vid->getTrackSelector();
		if (!sel) return 0;
		return sel->getCurVideoTrack();
	}

	int video_setCurAudioTrack(int track)
	{
		VideoOutput *vid = (VideoOutput *)video_getIVideoOutput();
		if (!vid) return 0;
		ITrackSelector *sel = vid->getTrackSelector();
		if (!sel) return 0;
		sel->setAudioTrack(track);
		return sel->getCurAudioTrack();
	}

	int video_setCurVideoTrack(int track)
	{
		VideoOutput *vid = (VideoOutput *)video_getIVideoOutput();
		if (!vid) return 0;
		ITrackSelector *sel = vid->getTrackSelector();
		if (!sel) return 0;
		sel->setVideoTrack(track);
		return sel->getCurVideoTrack();
	}

	HWND video_Create()
	{
		return CreateWindowExW(WS_EX_ACCEPTFILES, L"Winamp Video", getStringW(IDS_VIDEOCAPTION, NULL, 0), (WS_OVERLAPPED | WS_CLIPCHILDREN)&(~WS_CAPTION),
		                      config_video_wx, config_video_wy, config_video_width, config_video_height,
		                      hMainWindow, NULL, GetModuleHandle(NULL), NULL);
	}
}