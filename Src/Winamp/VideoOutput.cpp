#include <windowsx.h>

#include "main.h"
#include "VideoOutput.h"
#include "VideoOutputChild.h"
#include "vid_none.h"
#include "vid_ddraw.h"
#include "vid_overlay.h"
#include "vid_d3d.h"
#include <cassert>
#include "directdraw.h"
#include "video.h"
#include "api.h"
#include "WinampAttributes.h"
#include "resource.h"
#include "../nu/AutoWide.h"
#include "stats.h"
#include "IVideoD3DOSD.h"

extern "C" int is_fullscreen_video;
#define WM_VIDEO_UPDATE_STATUS_TEXT WM_USER+0x874
#define WM_VIDEO_OPEN WM_USER+0x875
#define WM_VIDEO_CLOSE WM_USER+0x876
#define WM_VIDEO_RESIZE WM_USER+0x877
#define WM_VIDEO_OSDCHANGE WM_USER+0x888
#define WM_VIDEO_CREATE WM_USER+0x900
int g_bitmap_id = IDB_VIDEOLOGO;
int VideoOutput::class_refcnt = 0;
wchar_t vidoutbuf_save[1024] = {0};

static bool input_plugin_thread_safe = false;

//#define USE_GDIPLUS_VIDEO_RENDERER

IVideoOSD *posd = new IVideoD3DOSD;

HRESULT(WINAPI *_DirectDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter) = 0;

void OpenDirectDraw()
{
	static int a = 0;
	if (!_DirectDrawCreate && !a)
	{
		a++;
		HINSTANCE h = LoadLibraryW(L"ddraw.dll");
		if (h)
		{
			*(void**)&_DirectDrawCreate = (void*)GetProcAddress(h, "DirectDrawCreate");
		}
	}
}

HMENU BuildPopupMenu()
{
	HMENU menu = GetSubMenu(GetSubMenu(top_menu, 3), 13);
	{
		static int menuset = 0;
		if (!menuset)
		{
			InsertMenu(menu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
			InsertMenuA(menu, (UINT)-1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)GetSubMenu(top_menu, 0), "Winamp");
			menuset = 1;
		}
	}
	updateTrackSubmenu();
	return menu;
}

void VideoOutput::mainthread_Create()
{
	if (!video_created)
	{
		m_video_output = new Direct3DVideoOutput(video_hwnd, this);
		video_created=true;
	}
}

VideoOutput::VideoOutput(HWND parent_hwnd, int initxpos, int initypos)
		: m_logo(0),
		m_logo_w(0),
		m_logo_h(0),
		userVideo(false)
{
	video_palette = 0;
	video_created = false;
	AutoLock autoLock(guard LOCKNAME("VideoOutput::VideoOutput"));
	WNDCLASSW wc = {0, };

	wc.style = CS_DBLCLKS;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"WinampVideoChild";
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	if (!class_refcnt) RegisterClassW(&wc);
	class_refcnt++;

	m_tracksel = NULL;
	fs_reparented_rgn = 0;
	fs_reparented = 0;
	m_ignore_change = false;
	fs_has_resized = false;
	m_opened = 0;

	last_fullscreen_exit_time = 0;

	curSubtitle = NULL;
	m_statusmsg = 0;
	m_bufferstate = -1;
	m_msgcallback = 0;
	m_msgcallback_tok = 0;
	video_hwnd = 0;

	aspect = 1.0;
	m_need_change = false;

	is_fs = 0;
	memset(&oldfsrect, 0, sizeof(oldfsrect));
	memset(&lastfsrect, 0, sizeof(lastfsrect));

	m_video_output = NULL;
	resetSubtitle();
	m_lastbufinvalid = NULL;

	CreateWindowExW(0, wc.lpszClassName, L"WinampVideoChildWindow",
					parent_hwnd ? WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS :
								  (WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX)),
	               0, 0, 1, 1,
	               parent_hwnd, NULL, wc.hInstance, (void*)this);
	posd->SetParent(video_hwnd);
}

VideoOutput::~VideoOutput()
{
	AutoLock autoLock(guard LOCKNAME("VideoOutput::~VideoOutput"));
	free(m_statusmsg);
	posd->Hide();
	if (m_video_output) m_video_output->close();
	m_video_output = 0;
	DestroyWindow(video_hwnd);
	if (m_logo)
		DeleteObject(m_logo);
	m_logo = 0;
	if (posd)
	{
		delete posd;
		posd = NULL;
	}
}

void VideoOutput::LoadLogo()
{
	static wchar_t logo_tmp[MAX_PATH];
	if(!logo_tmp[0]) StringCchPrintfW(logo_tmp,MAX_PATH,L"%s\\videologo.bmp",CONFIGDIR);
	if(PathFileExistsW(logo_tmp)) m_logo = (HBITMAP)LoadImageW(0,logo_tmp,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

	if(!m_logo) m_logo = (HBITMAP)LoadImage(hMainInstance, MAKEINTRESOURCE(g_bitmap_id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	BITMAP bm;
	GetObject(m_logo, sizeof(BITMAP), &bm);
	m_logo_w = bm.bmWidth;
	m_logo_h = bm.bmHeight;
	if (m_logo_h < 0)
		m_logo_h = -m_logo_h;
}

void VideoOutput::UpdateText(const wchar_t *videoInfo)
{
	{
		//AutoLock lock(textGuard);
		StringCchCopyW(vidoutbuf_save, 1023, (wchar_t*)videoInfo); // 1023 so that we can guarantee that this will be null terminated even in the middle of a strcpy
	}
	PostMessageW(hVideoWindow, WM_VIDEO_UPDATE_STATUS_TEXT, (WPARAM)vidoutbuf_save, 0);
	PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_VIDEOINFO, IPC_CB_MISC);
}

INT_PTR VideoOutput::extended(INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	switch (param1) // nonlocking commands
	{
		case VIDUSER_GET_VIDEOHWND:
			return (INT_PTR)video_hwnd;

		case VIDUSER_SET_INFOSTRING:
			if (param2)
				UpdateText(AutoWide((const char *)param2));
			return 0;

		case VIDUSER_SET_INFOSTRINGW:
			if (param2)
				UpdateText((const wchar_t *)param2);
			return 0;
	}
	AutoLock autoLock(guard LOCKNAME("VideoOutput::extended"));
	switch (param1)
	{
		case VIDUSER_SET_PALETTE:
		{
			RGBQUAD *palette = (RGBQUAD *)param2;
			if (m_video_output) 
				m_video_output->setPalette(palette);
			else
				video_palette = palette;
			return 0;
		}

		case VIDUSER_SET_VFLIP:
		{
			if (m_video_output)
				m_video_output->setVFlip(param2);
			return 0;
		}

		case VIDUSER_SET_TRACKSELINTERFACE:
			m_tracksel = (ITrackSelector *)param2;
			return 0;

		case VIDUSER_OPENVIDEORENDERER:
		{/*
			userVideo = true;
			m_video_output = (VideoRenderer *)param2;
			VideoOpenStruct *openStruct = (VideoOpenStruct *)param3;
			int x = openUser(openStruct->w, openStruct->h, openStruct->vflip, openStruct->aspectratio, openStruct->fmt);
			if (x)
			{
				m_video_output = 0;
				userVideo = false;
				return 0;
			}
			else*/
				return 1;
		}
		case VIDUSER_CLOSEVIDEORENDERER:
			close();
			userVideo = false;
			return 1;

		case VIDUSER_GETPOPUPMENU:
			return (INT_PTR)BuildPopupMenu();

		case VIDUSER_SET_THREAD_SAFE:
			input_plugin_thread_safe = !!param2;
			break;
	}
	return 0;
}

int VideoOutput::get_latency()
{
	return config_video_vsync2 ? 15 : 0;
}

void VideoOutput::adjustAspect(RECT &rd)
{
	AutoLock autoLock(guard LOCKNAME("VideoOutput::adjustAspect"));

	if (posd->Showing() /*&& config_video_osd*/)
	{
		rd.top += posd->GetBarHeight();
		rd.bottom -= posd->GetBarHeight();
	}

	if (config_video_aspectadj)
	{
		int outh = rd.bottom - rd.top;
		int outw = rd.right - rd.left;

		double outputaspect = aspect;

		if (config_video_useratio && config_video_ratio1 && config_video_ratio2)
		{
			RECT r;
			getViewport(&r, hVideoWindow, 1, NULL);
			int screenx = r.right - r.left;
			int screeny = r.bottom - r.top;

			if (screenx && screeny)
			{
				outputaspect *= config_video_ratio1 * screeny / ((double)screenx * (double)config_video_ratio2);
			}
		}

		int newh = (int)((outputaspect * height * outw) / (double)width);
		int neww = (int)((width * outh) / (height * outputaspect));

		if (outh > newh) // black bars on top and bottom
		{
			int d = outh - newh;
			rd.top += d / 2;
			rd.bottom -= d - d / 2;
		}
		else if (outw > neww) // black bars on left and right
		{
			int d = outw - neww;
			rd.left += d / 2;
			rd.right -= d - d / 2;
		}
	}
}

LRESULT CALLBACK VideoOutput::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	if (uMsg == g_scrollMsg) // can't check against variables in case statements
	{
		wParam <<= 16;
		uMsg = WM_MOUSEWHEEL;
	}

	if (FALSE != IsDirectMouseWheelMessage(uMsg))
	{
		SendMessageW(hwnd, WM_MOUSEWHEEL, wParam, lParam);
		return TRUE;
	}

	switch (uMsg)
	{
		case WM_MOUSEWHEEL:
			return SendMessageW(hMainWindow, uMsg, wParam, lParam);

		case WM_CREATE:
		{
			VideoOutput *vid = (VideoOutput *)((CREATESTRUCT *)lParam)->lpCreateParams;
			vid->video_hwnd = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)vid);
		}
		return 0;
		default:                                          /// pass it on to the other window procedure
			VideoOutput *_This = (VideoOutput*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
			if (_This)
				return _This->WindowProc(hwnd, uMsg, wParam, lParam);
			else
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void VideoOutput::notifyBufferState(int bufferstate) /* 0-255*/
{	
	m_bufferstate = bufferstate;
	if (bufferstate == -1 || bufferstate == 255 || (GetTickCount() - m_lastbufinvalid > 500)) // don't want to do this too often
	{
		AutoLock autoLock(guard LOCKNAME("VideoOutput::notifyBufferState"));
		if (!m_video_output || !m_video_output->onPaint(video_hwnd))
			InvalidateRect(video_hwnd, 0, TRUE);
		m_lastbufinvalid = GetTickCount();
	}
}

void VideoOutput::DrawLogo(HDC out, RECT *canvas_size)
{
	int bufferState = m_bufferstate;
	if (m_logo && config_video_logo)
	{
		HDC dc = CreateCompatibleDC(NULL);
		SelectObject(dc, m_logo);
		int xp = (canvas_size->right - canvas_size->left - m_logo_w) / 2;
		int yp = (canvas_size->bottom - canvas_size->top - m_logo_h) / 2;
		BitBlt(out, xp, yp, m_logo_w, m_logo_h, dc, 0, 0, SRCCOPY);

		if (bufferState != -1)
		{
			if (bufferState < 16) bufferState = 16;

			HGDIOBJ oldobj1 = SelectObject(out, CreateSolidBrush(RGB(0, 0, 0)));
			HGDIOBJ oldobj2 = SelectObject(out, CreatePen(PS_SOLID, 0, RGB(0, 0, 0)));
			Rectangle(out, canvas_size->left, canvas_size->top, canvas_size->right, yp);
			if (m_statusmsg)
				Rectangle(out, canvas_size->left, yp + m_logo_h, canvas_size->right, canvas_size->bottom);
			else
			{
				Rectangle(out, canvas_size->left, yp + m_logo_h + 2 + 9, canvas_size->right, canvas_size->bottom);
				Rectangle(out, xp + ((bufferState *(m_logo_w + 2)) >> 8), yp + m_logo_h + 2, canvas_size->right, yp + 9 + m_logo_h + 2);
			}
			Rectangle(out, canvas_size->left, yp, xp - 1, yp + m_logo_h + 9 + 2);
			Rectangle(out, xp + m_logo_w + 1, yp, canvas_size->right, yp + m_logo_h + 2);
			DeleteObject(SelectObject(out, oldobj2));
			DeleteObject(SelectObject(out, oldobj1));
		}

		if (m_statusmsg)
		{
			RECT subr = {0, yp + m_logo_h + 2, canvas_size->right, canvas_size->bottom};
			SetTextColor(out, RGB(255, 255, 255));
			SetBkMode(out, TRANSPARENT);
			DrawTextA(out, m_statusmsg, -1, &subr, DT_TOP | DT_CENTER | DT_NOCLIP | DT_NOPREFIX);
		}
		else
		{
			yp += m_logo_h + 2;
			if (bufferState)
			{
				HGDIOBJ oldobj1 = SelectObject(out, CreateSolidBrush(RGB(128, 128, 128)));
				HGDIOBJ oldobj2 = SelectObject(out, CreatePen(PS_SOLID, 0, RGB(255, 255, 255)));
				Rectangle(out, xp - 1, yp, xp + ((bufferState *(m_logo_w + 2)) >> 8), yp + 9);
				DeleteObject(SelectObject(out, oldobj2));
				DeleteObject(SelectObject(out, oldobj1));
			}
		}
		DeleteDC(dc);
	}
}

void VideoOutput::PaintLogo(int bufferState)
{
	// TODO: ask renderer to draw this shiz
	AutoLock autoLock(guard LOCKNAME("VideoOutput::PaintLogo"));

	PAINTSTRUCT p;
	BeginPaint(video_hwnd, &p);
	RECT r;
	GetClientRect(video_hwnd, &r);
	HDC out = p.hdc;

	HRGN hrgn = CreateRectRgnIndirect(&r);
	HBRUSH b = (HBRUSH)GetStockObject(BLACK_BRUSH);
	FillRgn(out, hrgn, b);
	DeleteObject(b);
	DeleteObject(hrgn);

	DrawLogo(out, &r);

	EndPaint(video_hwnd, &p);
}

// the big window procedure
LRESULT VideoOutput::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// the follow messages are handled w/o a lock
	switch (uMsg)
	{
		case WM_USER + 0x1:
			m_need_change = true;
			break;

		case WM_USER + 0x2:
			m_ignore_change = !!lParam;
			break;

		case WM_ERASEBKGND:
			return 1;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case ID_VIDEOWND_VIDEOOPTIONS:
					prefs_last_page = 24;
					prefs_dialog(1);
					break;

				case ID_VID_AUDIO0:
				case ID_VID_AUDIO1:
				case ID_VID_AUDIO2:
				case ID_VID_AUDIO3:
				case ID_VID_AUDIO4:
				case ID_VID_AUDIO5:
				case ID_VID_AUDIO6:
				case ID_VID_AUDIO7:
				case ID_VID_AUDIO8:
				case ID_VID_AUDIO9:
				case ID_VID_AUDIO10:
				case ID_VID_AUDIO11:
				case ID_VID_AUDIO12:
				case ID_VID_AUDIO13:
				case ID_VID_AUDIO14:
				case ID_VID_AUDIO15:
				{
					VideoOutput *out = (VideoOutput *)video_getIVideoOutput();
					if (!out)
						break;
					
					out->getTrackSelector()->setAudioTrack(LOWORD(wParam) - ID_VID_AUDIO0);
					break;
				}
				case ID_VID_VIDEO0:
				case ID_VID_VIDEO1:
				case ID_VID_VIDEO2:
				case ID_VID_VIDEO3:
				case ID_VID_VIDEO4:
				case ID_VID_VIDEO5:
				case ID_VID_VIDEO6:
				case ID_VID_VIDEO7:
				case ID_VID_VIDEO8:
				case ID_VID_VIDEO9:
				case ID_VID_VIDEO10:
				case ID_VID_VIDEO11:
				case ID_VID_VIDEO12:
				case ID_VID_VIDEO13:
				case ID_VID_VIDEO14:
				case ID_VID_VIDEO15:
				{
					VideoOutput *out = (VideoOutput *)video_getIVideoOutput();
					if (!out) break;
					out->getTrackSelector()->setVideoTrack(LOWORD(wParam) - ID_VID_VIDEO0);
					break;
				}
			}
			break;
		case WM_RBUTTONUP:
			if (!is_fs)
			{
				POINT p;
				GetCursorPos(&p);
				DoTrackPopup(BuildPopupMenu(), TPM_RIGHTBUTTON | TPM_LEFTBUTTON, p.x, p.y, hwnd);
			}
			break;
		case WM_LBUTTONDOWN:   // putting this here prevents a deadlock, but allows a race condition over video drawing =(
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			if (is_fs && config_video_osd)
			{
				if (((IVideoD3DOSD *)posd)->isOSDReadyToDraw())
				{
					if (posd->MouseDown(x, y, wParam))
						videoToggleFullscreen();
				}
			}
			else
			{
				SetCapture(video_hwnd);
				clickx = x;
				clicky = y;
				SetFocus(video_hwnd);
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			if (is_fs && config_video_osd)
			{
				if (((IVideoD3DOSD *)posd)->isOSDReadyToDraw())
				{
					posd->MouseMove(x, y, wParam);
				}
			}
			else if (GetCapture() == video_hwnd && config_easymove)
			{
				POINT p = { x, y};
				ClientToScreen(hVideoWindow, &p);
				p.x -= clickx;
				p.y -= clicky;
				SendMessageW(hVideoWindow, WM_USER + 0x100, 1, (LPARAM)&p);
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			if (GetCapture() == video_hwnd)
				ReleaseCapture();
			if (is_fs && config_video_osd)
			{
				if (((IVideoD3DOSD *)posd)->isOSDReadyToDraw())
					if (posd->MouseUp(x, y, wParam))
							videoToggleFullscreen();
			}
			SetFocus(hVideoWindow);
		}
		break;
	}

	switch (uMsg)
	{
		case WM_VIDEO_OSDCHANGE:
		{
			AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_VIDEO_OSDCHANGE"));
			if (m_video_output)
			{
				m_video_output->Refresh();
				m_video_output->timerCallback();
			}
		}
		break;
		case WM_SHOWWINDOW:
		{
			AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_SHOWWINDOW"));
			if (wParam == TRUE  // being shown
			    && !m_logo) // logo hasn't been loaded yet
				LoadLogo();
		}
		break;
		case WM_INITMENUPOPUP:
		{
			AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_INITMENUPOPUP"));
			return SendMessageW(hMainWindow, uMsg, wParam, lParam); // for popup menus
		}

		case WM_WINDOWPOSCHANGED:
		case WM_SIZE:
		{
			AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_SIZE"));
			if (m_video_output)
			{
				m_video_output->OnWindowSize();
				if (is_fs)
					if ( ((IVideoD3DOSD *)posd)->isOSDInited() )
						((IVideoD3DOSD *)posd)->UpdateOSD(hwnd, this);
			}
		}
		break;

		case WM_TIMER:
		case WM_WINDOWPOSCHANGING:
		case WM_MOVE:
		case WM_MOVING:
		{
			AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_TIMER, etc"));
			if (m_video_output && !m_ignore_change)
				m_video_output->timerCallback();

			if (uMsg == WM_TIMER)
				return 0;
		}
		break;

		case WM_LBUTTONDBLCLK:
		{
			AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_LBUTTONDBLCLK"));
			videoToggleFullscreen();
		}
		break;

		case WM_PAINT:
		{
			AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_PAINT"));
			if (m_video_output && m_video_output->onPaint(hwnd))
				return 0;
			PaintLogo(m_bufferstate);
		}

		return 0;
		break;

		case WM_PRINTCLIENT:
		{
			//AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_PAINT"));
			//if (m_video_output && m_video_output->onPaint(hwnd))
			//	return 0;
			RECT r;
			GetClientRect(video_hwnd, &r);
			DrawLogo((HDC)wParam, &r);
		}

		return 0;
		break;

		case WM_KEYDOWN:
		{
			AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_KEYDOWN"));
			if (wParam == VK_ESCAPE && is_fs)
			{
				videoToggleFullscreen();
				//remove_fullscreen();
				return 1;
			}
			if (!is_fs)
			{
				if (wParam == '3' || LOWORD(wParam) == 192 /* ` */)
				{
					SendMessageW(hwnd, WM_COMMAND, ID_VIDEOWND_ZOOM50, 0); return 0;
				}
				if (wParam == '1')
				{
					SendMessageW(hwnd, WM_COMMAND, ID_VIDEOWND_ZOOM100, 0); return 0;
				}
				if (wParam == '2')
				{
					SendMessageW(hwnd, WM_COMMAND, ID_VIDEOWND_ZOOM200, 0); return 0;
				}
			}
			if (wParam == ' ')
			{
				SendMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON3, 0); return 0;
			}
			if(wParam == 'F' && (GetAsyncKeyState(VK_SHIFT)&0x8000) && !(GetAsyncKeyState(VK_CONTROL)&0x8000))
			{
				SendMessageW(hwnd, WM_COMMAND, ID_VIDEOWND_VERTICALLYFLIP, 0); return 0;
			}
		}
		break;

		case WM_COMMAND:
		{
			AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_COMMAND"));
			switch (LOWORD(wParam))
			{
				case ID_VIDEOWND_VERTICALLYFLIP:
				{
					int new_fliprgb = !config_video_fliprgb;//IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_FLIPRGB)?1:0;
					config_video_fliprgb = 0;
					videoSetFlip(new_fliprgb);
					config_video_fliprgb = new_fliprgb;
					break;
				}

				case ID_VIDEOWND_ZOOMFULLSCREEN:
					videoGoFullscreen();
					break;

				case ID_VIDEOWND_ZOOM200:
				case ID_VIDEOWND_ZOOM100:
				case ID_VIDEOWND_ZOOM50:
					if (m_video_output)
						UpdateVideoSize(width, height, aspect, LOWORD(wParam));
					else
						UpdateVideoSize(320, 240, 1.0, LOWORD(wParam));
					break;

				default:
					SendMessageW(hMainWindow, WM_COMMAND, wParam, lParam);
					break;
			}
		}
		break;

		case WM_SETCURSOR:
		{
			AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_SETCURSOR"));
			if (is_fs)
			{
				SetCursor(posd->Showing() ? LoadCursor(NULL, IDC_ARROW) : NULL);
				return TRUE;
			}
			else
				SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		break;

		case WM_SYSCOMMAND:
		{
			AutoLock autoLock(guard LOCKNAME("VideoOutput::WM_SYSCOMMAND"));
			// eat screen saver message when fullscreen
			if (((wParam & 0xfff0) == SC_SCREENSAVE || (wParam & 0xfff0) == SC_MONITORPOWER) && config_video_noss &&
			    video_isVideoPlaying())
			{
				return -1;
			}
		}
		break;
	}

	if (m_msgcallback)
	{
		return m_msgcallback(m_msgcallback_tok, hwnd, uMsg, wParam, lParam);
	}

	return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}

void VideoOutput::fullscreen()
{
	AutoLock autoLock(guard LOCKNAME("VideoOutput::fullscreen"));
	if (!m_video_output || !m_opened)
		return ;

	if (is_fs)
		return ;

	// TODO: let the video renderer handle fullscreen itself, if it can.

	/*if (last_fullscreen_exit_time + 250 > GetTickCount()) // gay hack
	return ; // dont let you go back and forth too quickly	
	*/

	GetWindowRect(hVideoWindow, &oldfsrect); // save the old coordinates
	getViewport(&lastfsrect, video_hwnd, 1, NULL);
	if (GetParent(hVideoWindow))
	{
		fs_reparented_rgn = CreateRectRgn(0, 0, 0, 0);
		GetWindowRgn(hVideoWindow, fs_reparented_rgn);

		fs_reparented = SetParent(hVideoWindow, NULL);
		SetWindowRgn(hVideoWindow, NULL, FALSE);

		ScreenToClient(fs_reparented, (LPPOINT)&oldfsrect);
		ScreenToClient(fs_reparented, ((LPPOINT)&oldfsrect) + 1);
	}

	is_fullscreen_video = true;
	is_fs = true;
	//m_video_output->Fullscreen(true);
	SetWindowPos(hVideoWindow, HWND_TOPMOST, lastfsrect.left, lastfsrect.top, lastfsrect.right - lastfsrect.left, lastfsrect.bottom - lastfsrect.top, SWP_DRAWFRAME | SWP_NOACTIVATE);
	SetFocus(hVideoWindow);

	resetSubtitle();
}

extern "C" HWND hExternalVisWindow;

int VideoOutput::openUser(int w, int h, int vflip, double aspectratio, unsigned int fmt)
{
// TODO
	return 1;
}

int VideoOutput::open(int w, int h, int vflip, double aspectratio, unsigned int fmt)
{
	if (!video_created)
		SendMessageW(hVideoWindow, WM_VIDEO_CREATE, 0, 0);

	if (!h || !w || !fmt) // check this after creating the video window.  some plugins use this call to open the video output early
		return 0;

	AutoLock autoLock(guard LOCKNAME("VideoOutput::open"));
	
	if (!m_need_change)
		stats.IncrementStat(Stats::VIDEOS_PLAYED);

	if (hExternalVisWindow)
		PostMessageW(hExternalVisWindow, WM_USER + 1666, 1, 15);

	m_opened = false;
	userVideo = false;
	width = w;
	height = h;

	type = fmt;
	aspect = aspectratio;
	if (m_video_output)
	{
		if (type == VIDEO_MAKETYPE('N','O','N','E'))
		{
			m_opened = true;
			PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_INFO, IPC_CB_MISC);
			OpenVideoSize(width, height, aspect);
			fs_has_resized = is_fs;
			return 0;
		}
		else if (m_video_output->OpenVideo(w, h, type, vflip, aspect))
		{
			if (video_palette)
				m_video_output->setPalette(video_palette);
			video_palette = 0;
			DoTheVistaVideoDance();
			InvalidateRect(video_hwnd, NULL, TRUE);
			m_opened = true;
			PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_INFO, IPC_CB_MISC);
			if (!m_need_change) //don't update size when renegotiating video output
			{
				OpenVideoSize(width, height, aspect);
				fs_has_resized = is_fs;
			}
			return 0;
		}
	}
	return 1;
}

void VideoOutput::draw(void *frame)
{
	if (!frame)
		return ;
	AutoLock autoLock(guard LOCKNAME("VideoOutput::draw"));

	if (m_video_output)
	{
		m_video_output->displayFrame((const char *)frame, 0, 0);
	}
}

extern wchar_t draw_vw_info_lastb[512];

void VideoOutput::close()
{
	UpdateText(L"");
	AutoLock autoLock(guard);
	if (!m_opened)
		return ;
	m_opened = false;
	if (m_video_output)
	{
		m_video_output->drawSubtitle(0);
		m_video_output->close();
	}
	m_bufferstate = -1;

	draw_vw_info_lastb[0] = 0;
	input_plugin_thread_safe = false; // reset this
	InvalidateRect(video_hwnd, NULL, true);
	PostMessageW(hVideoWindow, WM_VIDEO_CLOSE, 0, 0);
}

int VideoOutput::is_fullscreen()
{
	return is_fs;
}

void VideoOutput::showStatusMsg(const char *text)
{
	AutoLock autoLock(guard);
	m_statusmsg = _strdup(text);
	PaintLogo(m_bufferstate);
	//InvalidateRect(video_hwnd, NULL, TRUE);
}

void VideoOutput::drawSubtitle(SubsItem *item)
{
	AutoLock autoLock(guard);
	if (item == curSubtitle)
		return ;
	curSubtitle = item;

	if (m_video_output)
		m_video_output->drawSubtitle(item);
}

void VideoOutput::resetSubtitle()
{
	AutoLock autoLock(guard);
	curSubtitle = NULL;
	if (m_video_output)
		m_video_output->resetSubtitle();
	//	InvalidateRect(this->getHwnd(), 0, TRUE);
}

void VideoOutput::remove_fullscreen()
{
	AutoLock autoLock(guard);
	if (!is_fs)
	{
		is_fullscreen_video = false;
		return ;
	}

	posd->Hide();
	posd->ctrlrects_ready = 0; //tofix

	if (m_video_output)
	{
		//	m_video_output->Fullscreen(false);
	}

	resetSubtitle();

	is_fs = 0;

	if (fs_reparented)
	{
		SetParent(hVideoWindow, fs_reparented);
		SetWindowRgn(hVideoWindow, fs_reparented_rgn, FALSE);
		fs_reparented = 0;
		SetWindowPos(hVideoWindow, HWND_TOPMOST, oldfsrect.left, oldfsrect.top, oldfsrect.right - oldfsrect.left, oldfsrect.bottom - oldfsrect.top, SWP_NOACTIVATE | SWP_NOZORDER);
	}
	else
		SetWindowPos(hVideoWindow, config_aot ? HWND_TOPMOST : HWND_NOTOPMOST, oldfsrect.left, oldfsrect.top, oldfsrect.right - oldfsrect.left, oldfsrect.bottom - oldfsrect.top, SWP_NOACTIVATE);

	last_fullscreen_exit_time = GetTickCount();

	posd->Hide();

	if (!m_opened && m_video_output)
	{
		m_video_output->close();
	}

	is_fullscreen_video = false;

	if (fs_has_resized)
	{
		fs_has_resized = false;
		if (config_video_updsize)
			UpdateVideoSize(width, height, aspect);
	}
}

void VideoOutput::UpdateVideoSize(int newWidth, int newHeight, double newAspect, int zoom)
{
    if (!m_opened)
        return;

	// fill in default values if we have 0s
	if (!newWidth)
		newWidth = 320;
	if (!newHeight)
		newHeight = 240;

	switch (zoom)
	{
		case ID_VIDEOWND_ZOOM200:
			newWidth *= 2;
			newHeight *= 2;
			break;
		case ID_VIDEOWND_ZOOM50:
			newWidth /= 2;
			newHeight /= 2;
			break;
	}

	// establish a minimum window size
	if (newWidth < 256)
		newWidth = 256;
	if (newHeight < 64)
		newHeight = 64;

	if (newAspect > 0.001) // floating point can be weird about checking == 0
	{
		if (newAspect < 1.0)
			newWidth = (int)((double)newWidth / newAspect);
		else
			newHeight = (int)((double)newHeight * newAspect);
	}

	//SendNotifyMessage(hVideoWindow, WM_VIDEO_RESIZE, newWidth, newHeight);
	PostMessageW(hVideoWindow, WM_VIDEO_RESIZE, newWidth, newHeight);
}

void VideoOutput::OpenVideoSize(int newWidth, int newHeight, double newAspect)
{
	// fill in default values if we have 0s
	if (!newWidth)
		newWidth = 320;
	if (!newHeight)
		newHeight = 240;

	// establish a minimum window size
	if (newWidth < 256)
		newWidth = 256;
	if (newHeight < 64)
		newHeight = 64;

	if (newAspect > 0.001) // floating point can be weird about checking == 0
	{
		if (newAspect < 1.0)
			newWidth = (int)((double)newWidth / newAspect);
		else
			newHeight = (int)((double)newHeight * newAspect);
	}

	PostMessageW(hVideoWindow, WM_VIDEO_OPEN, newWidth, newHeight);
}

void VideoOutput::SetVideoPosition(int x, int y, int width, int height)
{
	AutoLock autoLock(guard);

	SetWindowPos(getHwnd(), 0, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}