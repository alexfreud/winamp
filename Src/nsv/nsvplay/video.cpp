#include <windows.h>
#include <ddraw.h>
#include "main.h"
#include "video.h"
#include "subtitles.h"

#include "resource.h"

#undef GetSystemMetrics

#define OSD_ENABLED 1

#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))
#define OV_COL_R 16
#define OV_COL_G 0
#define OV_COL_B 16
#define OSD_TEXT_SIZE 28
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

#define TIMER_OSD_ID 1234

#define CTRLTYPE_SYMBOL   0
#define CTRLTYPE_TEXT     1
#define CTRLTYPE_PROGRESS 2
#define CTRLTYPE_SPACER   3

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

int g_ctrl_type[NUM_WIDGETS] = {
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

const char *g_ctrl_text[NUM_WIDGETS] = {
  "Progress ", 
  "",
  "",
  "7", // rew
  "4", // play
  ";", // pause
  "<", // stop
  "8", // ffwd
  "",
  "Volume ",
  ""
};

int g_ctrl_force_width[NUM_WIDGETS] = {
  0,  
  96, // progress bar width
  32, // spacer width
  0,  // rew
  0,  // play
  0,  // pause
  0,  // stop
  0,  // ffwd
  32, // spacer width
  0, 
  64  // volume bar width
};

extern HINSTANCE g_hInstance;
extern int g_bitmap_id;

static BOOL WINAPI DDEnumCallbackEx(GUID FAR *lpGUID, LPSTR     lpDriverDescription, LPSTR     lpDriverName, LPVOID    lpContext, HMONITOR  hm) {
  VideoOutputChild *ovo=(VideoOutputChild *)lpContext;
  if(ovo->m_found_devguid) return 1;
  if(hm==ovo->m_monitor_to_find) {
    ovo->m_devguid=*lpGUID;
    ovo->m_found_devguid=1;
  }
  return 1;
}

void VideoOutputChild::update_monitor_coords(VideoOutput *parent)
{
  //find the correct monitor if multiple monitor support is present
  HWND hwnd=parent->getHwnd();
  m_found_devguid=0;
  m_mon_x=0;
  m_mon_y=0;

	HINSTANCE h=LoadLibrary("user32.dll");
	if (h) {
		HMONITOR (WINAPI *Mfp)(POINT pt, DWORD dwFlags) = (HMONITOR (WINAPI *)(POINT,DWORD)) GetProcAddress(h,"MonitorFromPoint");
    HMONITOR (WINAPI *Mfr)(LPCRECT lpcr, DWORD dwFlags) = (HMONITOR (WINAPI *)(LPCRECT, DWORD)) GetProcAddress(h, "MonitorFromRect");
    HMONITOR (WINAPI *Mfw)(HWND wnd, DWORD dwFlags)=(HMONITOR (WINAPI *)(HWND, DWORD)) GetProcAddress(h, "MonitorFromWindow");
    BOOL (WINAPI *Gmi)(HMONITOR mon, LPMONITORINFO lpmi) = (BOOL (WINAPI *)(HMONITOR,LPMONITORINFO)) GetProcAddress(h,"GetMonitorInfoA");    
		if (Mfp && Mfr && Mfw && Gmi) {
      RECT r;
      GetWindowRect(hwnd,&r);
      HMONITOR hm=Mfr(&r,NULL);
      if(hm) {
        HINSTANCE hdd = LoadLibrary("ddraw.dll");
        if(hdd) {
          typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKEXA)(GUID FAR *, LPSTR, LPSTR, LPVOID, HMONITOR);
          typedef HRESULT (WINAPI * LPDIRECTDRAWENUMERATEEX)( LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
          LPDIRECTDRAWENUMERATEEX lpDDEnumEx;
          lpDDEnumEx = (LPDIRECTDRAWENUMERATEEX) GetProcAddress(hdd,"DirectDrawEnumerateExA");
          if (lpDDEnumEx) {
            m_monitor_to_find=hm;
            lpDDEnumEx(&DDEnumCallbackEx, this, DDENUM_ATTACHEDSECONDARYDEVICES|DDENUM_NONDISPLAYDEVICES);
            if(m_found_devguid) {
              MONITORINFOEX mi;
              memset(&mi,0,sizeof(mi));
              mi.cbSize=sizeof(mi);
              if (Gmi(hm,&mi)) {
                m_mon_x=mi.rcMonitor.left;
                m_mon_y=mi.rcMonitor.top;
              }
            }
          }
          FreeLibrary(hdd);
        }
      }
    }
    FreeLibrary(h);
  }
}

int VideoOutput::get_latency()
{
  return vid_vsync?15:0;
}

#undef GetSystemMetrics
int VideoOutput::class_refcnt=0;

void VideoOutput::adjustAspect(RECT &rd)
{
  if (vid_aspectadj)
  {
    int outh=rd.bottom-rd.top;
    int outw=rd.right-rd.left;

    int newh=(int)((aspect*height*outw)/(double)width);
    int neww=(int)((width*outh)/(height*aspect));

    if (outh > newh) // black bars on top and bottom
    {
      int d=outh - newh;
      rd.top+=d/2;
      rd.bottom-=d-d/2;
    }
    else if (outw > neww) // black bars on left and right
    {
      int d=outw - neww;
      rd.left+=d/2;
      rd.right-=d-d/2;
    }
  }
}

LRESULT CALLBACK VideoOutput::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_CREATE)
  {
    SetWindowLong(hwnd,GWL_USERDATA,(long)((CREATESTRUCT *)lParam)->lpCreateParams);
    ShowWindow(hwnd,SW_SHOW);
    if (GetParent(hwnd))
    {
      RECT r;
      GetClientRect(GetParent(hwnd),&r);
      SetWindowPos(hwnd,NULL,0,0,
        r.right,
        r.bottom,
        SWP_NOACTIVATE|SWP_NOZORDER);
    }
    return 0;
  }

  VideoOutput *_This=(VideoOutput*)GetWindowLong(hwnd,GWL_USERDATA);
  if (_This) return _This->WindowProc(hwnd,uMsg,wParam,lParam);
  else return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

void VideoOutput::notifyBufferState(int bufferstate) /* 0-255*/
{
  m_bufferstate=bufferstate;
#ifdef ACTIVEX_CONTROL
  PostMessage( video_hwnd, STATUS_MSG, STATUS_PREBUFFER, bufferstate );
#endif
  if (!m_video_output) {
    if(GetTickCount()-m_lastbufinvalid>500) {
      InvalidateRect(video_hwnd,NULL,FALSE);
      m_lastbufinvalid=GetTickCount();
    }
  }
}

LRESULT VideoOutput::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_TIMER:
    case WM_WINDOWPOSCHANGING:
    case WM_WINDOWPOSCHANGED:
    case WM_SIZE:
    case WM_MOVE:
    case WM_MOVING:
      if (uMsg == WM_TIMER && wParam == TIMER_OSD_ID) {
        hideOSD();
        return 0;
      }
      EnterCriticalSection(&m_cs);
      if(m_video_output) m_video_output->timerCallback();
      LeaveCriticalSection(&m_cs);
      if (uMsg == WM_TIMER) return 0;
    break;

    case WM_LBUTTONDOWN:
      if(is_fs) 
        osdHitTest(LOWORD(lParam),HIWORD(lParam),0);
#ifdef ACTIVEX_CONTROL
	  SendMessage( video_hwnd, STATUS_MSG, STATUS_MOUSEPRESS, 1 );
#endif
      break;
      
    case WM_PAINT:
      {
        if (m_video_output && m_video_output->onPaint(hwnd,(HDC)wParam)) return 0;
        if (m_logo && !m_video_output) 
        {
          PAINTSTRUCT p;
          BeginPaint(hwnd,&p);

          RECT r;
          GetClientRect(hwnd,&r);
          
          HDC out=p.hdc;

          HDC dc=CreateCompatibleDC(NULL);
          SelectObject(dc,m_logo);
          int xp=(r.right-r.left-m_logo_w)/2;
          int yp=(r.bottom-r.top-m_logo_h)/2;
          BitBlt(out,xp,yp,m_logo_w,m_logo_h,dc,0,0,SRCCOPY);

          int bs=m_bufferstate;
          if (bs < 16) bs=16;


          HGDIOBJ oldobj1=SelectObject(out,CreateSolidBrush(RGB(0,0,0)));
          HGDIOBJ oldobj2=SelectObject(out,CreatePen(PS_SOLID,0,RGB(0,0,0)));
          Rectangle(out,r.left,r.top,r.right,yp);
          if (m_statusmsg)
            Rectangle(out,r.left,yp+m_logo_h,r.right,r.bottom);
          else
          {
            Rectangle(out,r.left,yp+m_logo_h+2+9,r.right,r.bottom);
            Rectangle(out,xp + ((bs * (m_logo_w+2))>>8),yp+m_logo_h+2,r.right, yp+9+m_logo_h+2);
          }
          Rectangle(out,r.left,yp,xp-1,yp+m_logo_h+9+2);
          Rectangle(out,xp+m_logo_w+1,yp,r.right,yp+m_logo_h+2);
          DeleteObject(SelectObject(out,oldobj2));
          DeleteObject(SelectObject(out,oldobj1));

          if (m_statusmsg)
          {
            RECT subr={0,yp+m_logo_h+2,r.right,r.bottom};
            SetTextColor(out,RGB(255,255,255));
            SetBkMode(out,TRANSPARENT);
            DrawText(out,m_statusmsg,-1,&subr,DT_TOP|DT_CENTER|DT_NOCLIP|DT_NOPREFIX);
          }
          else
          {
            yp+=m_logo_h+2;
            if (bs) 
            {
              HGDIOBJ oldobj1=SelectObject(out,CreateSolidBrush(RGB(128,128,128)));
              HGDIOBJ oldobj2=SelectObject(out,CreatePen(PS_SOLID,0,RGB(255,255,255)));
              Rectangle(out,xp-1,yp,xp + ((bs * (m_logo_w+2))>>8), yp+9);
              DeleteObject(SelectObject(out,oldobj2));
              DeleteObject(SelectObject(out,oldobj1));
            }
          }
          DeleteDC(dc);
          EndPaint(hwnd,&p);
          return 0;
        }
      }
      break;

    case WM_USER+0x1:
      m_need_change=1;
      break;

#ifdef ACTIVEX_CONTROL
	case STATUS_MSG:
	  SendStatus( wParam, lParam );
	  break;
#endif

    case WM_KEYDOWN:
      if(wParam==27 && is_fs) remove_fullscreen();
      break;

    case WM_MOUSEMOVE:
      if(is_fs) {
        if (ignore_mousemove_count>0) {
          ignore_mousemove_count--;
        }
        else if (abs(osdLastMouseX - LOWORD(lParam)) + abs(osdLastMouseY - HIWORD(lParam)) > 1) {
          KillTimer(hwnd, TIMER_OSD_ID);
          showOSD();
          SetTimer(hwnd, TIMER_OSD_ID, 2000, NULL);

          if (wParam & MK_LBUTTON)
            osdHitTest(LOWORD(lParam),HIWORD(lParam),1);
          else
            osdHitTest(LOWORD(lParam),HIWORD(lParam),-1);
        }
        osdLastMouseX = LOWORD(lParam);
        osdLastMouseY = HIWORD(lParam);
      }
      break;
  }
  if (m_msgcallback)
  {
    return m_msgcallback(m_msgcallback_tok,hwnd, uMsg, wParam, lParam);
  }
 
	return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}

VideoOutput::VideoOutput(HWND parent_hwnd, int initxpos, int initypos)
{
  curSubtitle=NULL;
  m_statusmsg=0;
  m_bufferstate=0;
  m_msgcallback=0;
  m_msgcallback_tok=0;
  video_hwnd=video_parent_hwnd=0;
  decoder=0;

  vid_aspectadj=true;
  vid_overlays=true;
  vid_ddraw=true;
  vid_vsync=true;
  aspect=1.0;
  m_need_change=false;

  width=height=flip=uyvy_output=yuy2_output=is_fs=ignore_mousemove_count=show_osd=0;
  oldfsparent=0;
  memset(&oldfsrect,0,sizeof(oldfsrect));
  memset(&lastfsrect,0,sizeof(lastfsrect));
  oldfsstyle=0;

  m_video_output=NULL;

  osdFontText=NULL;
  osdFontSymbol=NULL;
  osdProgressBrushBg=NULL;
  osdProgressBrushFg=NULL;
  osdProgressPenBg=NULL;
  osdProgressPenFg=NULL;
  osdProgressPenBgHilite=NULL;
  osdBlackBrush=NULL;
  osdMemDC=NULL;
  osdMemBM=NULL;
  osdOldBM=NULL;
  osdMemBMW=0;
  osdMemBMH=0;
  osdLastMouseX=-1;
  osdLastMouseY=-1;

  for (int i=0; i<NUM_WIDGETS; i++)
    SetRect(&ctrlrect[i], 0, 0, 0, 0);
  ctrlrects_ready = 0;

  resetSubtitle();

	WNDCLASS wc={0,};	

  wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = "NSVplay";
  LOGBRUSH lb={BS_SOLID,RGB(OV_COL_R,OV_COL_G,OV_COL_B),};
  wc.hbrBackground=CreateBrushIndirect(&lb);
	if (!class_refcnt) RegisterClass(&wc);
  class_refcnt++;

  m_logo=(HBITMAP)LoadImage(g_hInstance,MAKEINTRESOURCE(g_bitmap_id),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
  BITMAP bm;
  GetObject(m_logo, sizeof(BITMAP), &bm);
  m_logo_w=bm.bmWidth;
  m_logo_h=bm.bmHeight;
  if(m_logo_h<0) m_logo_h=-m_logo_h;

  InitializeCriticalSection(&m_cs);

  video_hwnd=CreateWindowEx(0,wc.lpszClassName, "NSV Player",parent_hwnd?WS_CHILD:(WS_OVERLAPPEDWINDOW&(~WS_MAXIMIZEBOX)),
					initxpos,initypos,320,200,
					parent_hwnd, NULL,wc.hInstance,(void*)this);

  video_parent_hwnd=parent_hwnd;

  m_lastbufinvalid=0;
  
#ifdef ACTIVEX_CONTROL
  m_firstframe = 1;
#endif
}

VideoOutputChild *VideoOutput::createVideoOutput(int n) {
  if(!vid_overlays && !vid_ddraw) vid_overlays=true;

  if(!vid_overlays) n++;
  if(n==0) return new OverlayVideoOutput();
  if(!vid_ddraw) n++;
  if(n==1) return new DDrawVideoOutput();

  return 0;
}

int VideoOutput::open(int w, int h, int vflip, double aspectratio, unsigned int fmt)
{
  EnterCriticalSection(&m_cs);
  delete(m_video_output);
  m_video_output=NULL;

  if (!w) w=320;
  if (!h) h=240;
  width=w;
  height=h;
  flip=vflip;
  type=fmt;
  is_fs=0;
  ignore_mousemove_count=0;
  show_osd=0;
  aspect=aspectratio;

  for(int i=0;m_video_output=createVideoOutput(i);i++) {
    if(m_video_output->create(this,w,h,fmt,vflip,aspectratio)) {
      LeaveCriticalSection(&m_cs);
      if (!GetParent(video_hwnd)) {
        RECT r,r2;
        int ow=width,oh=height;
        if (aspect > 0.001)
        {
          if (aspect < 1.0) ow=(int)(ow/aspect);
          else oh=(int)(oh*aspect);
        }
        GetWindowRect(video_hwnd,&r);
        GetClientRect(video_hwnd,&r2);
        SetWindowPos(video_hwnd,NULL,0,0,
          ow+(r.right-r.left)-(r2.right-r2.left),
          oh+(r.bottom-r.top)-(r2.bottom-r2.top),
          SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
      }
      return 0;
    }
    delete(m_video_output);
  }
  LeaveCriticalSection(&m_cs);
  return 1;
}

void VideoOutput::draw(void *frame)
{
  if (!m_video_output || !frame) return;
  if ((m_video_output && m_video_output->needChange()) || m_need_change) {
    open(width,height,flip,aspect,type);
    m_need_change=0;
  }
#ifdef ACTIVEX_CONTROL
  if ( m_firstframe ) {
	m_firstframe = 0;
	PostMessage( video_hwnd, STATUS_MSG, STATUS_FIRSTFRAME, 1 );
  }
#endif
  if (m_video_output) m_video_output->displayFrame((const char *)frame,0,0);
}

VideoOutput::~VideoOutput()
{
  free(m_statusmsg);
  delete(m_video_output);
  DestroyWindow(video_hwnd);
  if (!--class_refcnt) UnregisterClass("NSVplay",GetModuleHandle(NULL));
  if(osdFontText) DeleteObject(osdFontText);
  if(osdFontSymbol) DeleteObject(osdFontSymbol);
  if(osdProgressBrushBg) DeleteObject(osdProgressBrushBg);
  if(osdProgressBrushFg) DeleteObject(osdProgressBrushFg);
  if(osdBlackBrush     ) DeleteObject(osdBlackBrush     );
  if(osdProgressPenBg  ) DeleteObject(osdProgressPenBg  );
  if(osdProgressPenFg  ) DeleteObject(osdProgressPenFg  );
  if(osdProgressPenBgHilite) DeleteObject(osdProgressPenBgHilite);
  if(osdMemDC) {
	  SelectObject(osdMemDC,osdOldBM);	// delete our doublebuffer
	  DeleteDC(osdMemDC);
  }
  if(osdMemBM) DeleteObject(osdMemBM);	


  DeleteCriticalSection(&m_cs);
}

void VideoOutput::close()
{
  delete(m_video_output);
  m_video_output=NULL;
}

void VideoOutput::getViewport(RECT *r, HWND wnd, int full) {
  POINT *p=NULL;
  RECT *sr=NULL; 
	if (p || sr || wnd) {
	  HINSTANCE h=LoadLibrary("user32.dll");
	  if (h) {
			HMONITOR (WINAPI *Mfp)(POINT pt, DWORD dwFlags) = (HMONITOR (WINAPI *)(POINT,DWORD)) GetProcAddress(h,"MonitorFromPoint");
      HMONITOR (WINAPI *Mfr)(LPCRECT lpcr, DWORD dwFlags) = (HMONITOR (WINAPI *)(LPCRECT, DWORD)) GetProcAddress(h, "MonitorFromRect");
      HMONITOR (WINAPI *Mfw)(HWND wnd, DWORD dwFlags)=(HMONITOR (WINAPI *)(HWND, DWORD)) GetProcAddress(h, "MonitorFromWindow");
      BOOL (WINAPI *Gmi)(HMONITOR mon, LPMONITORINFO lpmi) = (BOOL (WINAPI *)(HMONITOR,LPMONITORINFO)) GetProcAddress(h,"GetMonitorInfoA");    
			if (Mfp && Mfr && Mfw && Gmi) {
			  HMONITOR hm = NULL;
			  if (p)
				  hm=Mfp(*p,MONITOR_DEFAULTTONULL);
				else if (sr)
				  hm=Mfr(sr,MONITOR_DEFAULTTONULL);
				else if (wnd)
				  hm=Mfw(wnd,MONITOR_DEFAULTTONULL);
        if (hm) {
          MONITORINFOEX mi;
          memset(&mi,0,sizeof(mi));
          mi.cbSize=sizeof(mi);

          if (Gmi(hm,&mi)) {
            if(!full) *r=mi.rcWork;
            else *r=mi.rcMonitor;
            FreeLibrary(h);
            return;
          }          
        }
			}
			FreeLibrary(h);
		}
	}
  if (full)
  { // this might be borked =)
    r->top=r->left=0;
    r->right=::GetSystemMetrics(SM_CXSCREEN);
    r->bottom=::GetSystemMetrics(SM_CYSCREEN);
  }
  else
  {
    SystemParametersInfo(SPI_GETWORKAREA,0,r,0);
  }
}

void VideoOutput::fullscreen()
{
  if (is_fs) return;
  if(!m_video_output) return;
  is_fs=1; 
  ignore_mousemove_count=2;

  oldfsparent=GetParent(video_hwnd);
  oldfsstyle=GetWindowLong(video_hwnd,GWL_STYLE);
  if (!oldfsparent) GetWindowRect(video_hwnd,&oldfsrect);
  else GetClientRect(video_hwnd,&oldfsrect);
  getViewport(&lastfsrect,video_hwnd,1);

  SetParent(video_hwnd,NULL);
  SetWindowLong(video_hwnd,GWL_STYLE,WS_POPUP|WS_VISIBLE);
  SetWindowPos(video_hwnd, HWND_TOPMOST, lastfsrect.left, lastfsrect.top, lastfsrect.right-lastfsrect.left, lastfsrect.bottom-lastfsrect.top, SWP_DRAWFRAME);
  SetFocus(video_hwnd);

  resetSubtitle();

  //showOSD();

  //SetCursor(NULL);
}

void VideoOutput::getOutputSize(int *w, int *h)
{
  RECT r2;
  GetClientRect(video_hwnd,&r2);
  *w=r2.right-r2.left;
  *h=r2.bottom-r2.top;
}

void VideoOutput::setOutputSize(int w, int h)
{
  RECT r,r2;
  GetWindowRect(video_hwnd,&r);
  GetClientRect(video_hwnd,&r2);
  SetWindowPos(video_hwnd, 0, 0,0, 
          w+(r.right-r.left)-(r2.right-r2.left),
          h+(r.bottom-r.top)-(r2.bottom-r2.top),  
      SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
}

void VideoOutput::remove_fullscreen()
{
  if(!is_fs) return;

  SetParent(video_hwnd,oldfsparent);
  SetWindowLong(video_hwnd,GWL_STYLE,oldfsstyle);
  // note: when returning from fullscreen *on a secondary monitor*,
  //       be careful how you set the new window Z order.
  //   nsvplay.exe:  only HWND_NOTOPMOST works
  //   nsvplayX.exe: only HWND_TOP works
  SetWindowPos(video_hwnd, oldfsparent ? HWND_TOP : HWND_NOTOPMOST, oldfsrect.left, oldfsrect.top, oldfsrect.right-oldfsrect.left, oldfsrect.bottom-oldfsrect.top, SWP_FRAMECHANGED);
  SetFocus(oldfsparent ? oldfsparent : video_hwnd);

  is_fs=0;
  show_osd=0;
  ctrlrects_ready=0;
  resetSubtitle();

  hideOSD();
}

int VideoOutput::is_fullscreen()
{
  return is_fs;
}

void VideoOutput::showStatusMsg(const char *text)
{
  m_statusmsg=_strdup(text);
  InvalidateRect(video_hwnd,NULL,TRUE); 
}

void VideoOutput::drawSubtitle(SubsItem *item)
{
  if(!item) {
    if(curSubtitle) {
      m_video_output->drawSubtitle(NULL);
      curSubtitle=NULL;
    }
    return;
  }

  if(curSubtitle==item) return;

  curSubtitle=item;

  m_video_output->drawSubtitle(curSubtitle);
}

void VideoOutput::resetSubtitle()
{
  curSubtitle=NULL;
  if(m_video_output) m_video_output->resetSubtitle();
}

void VideoOutput::showOSD() {
  if(OSD_ENABLED && m_video_output) {
    KillTimer(video_hwnd, TIMER_OSD_ID);
    if (!show_osd) 
      m_video_output->showOSD();
    SetTimer(video_hwnd, TIMER_OSD_ID, 2000, NULL);
    show_osd = 1;
    SetCursor(LoadCursor(NULL, IDC_ARROW));
  }
}

void VideoOutput::hideOSD() {
  if(OSD_ENABLED && m_video_output) {
    KillTimer(video_hwnd, TIMER_OSD_ID);
    m_video_output->hideOSD();
    show_osd = 0;
    SetCursor(NULL);
  }
}

void VideoOutput::drawOSD(HDC hdc, RECT *rg) {
  if(m_video_output && show_osd) {

    if (!osdMemDC          ) osdMemDC           = CreateCompatibleDC(hdc);
    if (!osdFontText)   osdFontText=CreateFont(OSD_TEXT_SIZE,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_OUTLINE_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Arial");
    if (!osdFontSymbol) osdFontSymbol=CreateFont(OSD_TEXT_SIZE,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,SYMBOL_CHARSET,OUT_OUTLINE_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,"Webdings");
    if (!osdProgressBrushBg) osdProgressBrushBg = CreateSolidBrush(RGB(OSD_VOL_BKCOL_R,OSD_VOL_BKCOL_G,OSD_VOL_BKCOL_B));
    if (!osdProgressBrushFg) osdProgressBrushFg = CreateSolidBrush(RGB(OSD_VOL_COL_R,OSD_VOL_COL_G,OSD_VOL_COL_B));
    if (!osdBlackBrush     ) osdBlackBrush      = CreateSolidBrush(RGB(0,0,0));//OV_COL_R,OV_COL_G,OV_COL_B));
    if (!osdProgressPenBg  ) osdProgressPenBg   = CreatePen(PS_SOLID,0,RGB(OSD_TEXT_R,OSD_TEXT_G,OSD_TEXT_B));
    if (!osdProgressPenFg  ) osdProgressPenFg   = CreatePen(PS_NULL,0,RGB(0,0,0));
    if (!osdProgressPenBgHilite) osdProgressPenBgHilite = CreatePen(PS_SOLID,0,RGB(OSD_TEXT_R_HILITE,OSD_TEXT_G_HILITE,OSD_TEXT_B_HILITE));
    
    COLORREF fg = GetTextColor(osdMemDC);
    COLORREF bg = GetBkColor(osdMemDC);
    SetTextColor(osdMemDC, RGB(OSD_TEXT_R,OSD_TEXT_G,OSD_TEXT_B));
    SetBkColor(osdMemDC, RGB(0,0,0));//OV_COL_R,OV_COL_G,OV_COL_B));
    
    HGDIOBJ oldfont  = SelectObject(osdMemDC, osdFontText);
    HGDIOBJ oldbrush = SelectObject(osdMemDC, osdProgressBrushBg);
    HGDIOBJ oldpen   = SelectObject(osdMemDC, osdProgressPenBg);

    RECT fullr;
    GetClientRect(video_hwnd,&fullr);
    ClientToScreen(video_hwnd,(LPPOINT)&fullr);
    ClientToScreen(video_hwnd,((LPPOINT)&fullr) + 1);
    // transform coords from windows desktop coords (where 0,0==upper-left corner of the primary monitor)
    // to the coords for the monitor we're displaying on:
    fullr.top -= m_video_output->m_mon_y;
    fullr.left -= m_video_output->m_mon_x;
    fullr.right -= m_video_output->m_mon_x;
    fullr.bottom -= m_video_output->m_mon_y;

    if (!ctrlrects_ready) {
      ctrlrects_ready = 1;

      int net_width = 0;
      int max_height = 0;
      int streaming = (decoder && decoder->getlen()==-1) ? 1 : 0;
      
      for (int i=0; i<NUM_WIDGETS; i++) {
        if (streaming && (i==CTRL_PROGRESS || i==CTRL_PROGRESSTEXT || i==CTRL_PROGRESSSPACER || i==CTRL_FFWD || i==CTRL_REW)) {
          // disable progress bar + seek arrows when the NSV is a stream
          ctrlrect[i].right = -1;
          continue;
        }
        else if (g_ctrl_force_width[i] != 0) {
          SetRect(&ctrlrect[i], 0, 0, g_ctrl_force_width[i], 0);
        }
        else {
          SelectObject(osdMemDC, (g_ctrl_type[i] == CTRLTYPE_SYMBOL) ? osdFontSymbol : osdFontText);
          SetRect(&ctrlrect[i], 0, 0, 256, 256);
          ctrlrect[i].bottom = DrawText(osdMemDC, g_ctrl_text[i], -1, &ctrlrect[i], DT_SINGLELINE|DT_CALCRECT);
        }
        net_width += ctrlrect[i].right - ctrlrect[i].left;
        max_height = max(max_height, ctrlrect[i].bottom - ctrlrect[i].top);
      }

      // now we know the size of all the controls; now place them.
      int x = (fullr.right + fullr.left)/2 - net_width/2;
      SetRect(&ctrlrect_all, 0, 0, 0, 0);
      for (i=0; i<NUM_WIDGETS; i++)
      {
        if (ctrlrect[i].right >= 0) // if control is not disabled...
        {
          int this_width  = ctrlrect[i].right  - ctrlrect[i].left;
          int this_height = ctrlrect[i].bottom - ctrlrect[i].top ;
          if (this_height==0) this_height = max_height*2/3;// progress bars
          ctrlrect[i].top    = max_height/2 - this_height/2;
          ctrlrect[i].bottom = max_height/2 + this_height/2;
          ctrlrect[i].left   = x;
          ctrlrect[i].right  = x + this_width;
          if (ctrlrect_all.bottom==0) {
            ctrlrect_all.top    = ctrlrect[i].top   ;
            ctrlrect_all.bottom = ctrlrect[i].bottom;
          } 
          else {
            ctrlrect_all.top    = min(ctrlrect_all.top   , ctrlrect[i].top   );
            ctrlrect_all.bottom = max(ctrlrect_all.bottom, ctrlrect[i].bottom);
          }
          x += this_width;
        }
      }     
    }

    int w = fullr.right - fullr.left;
    int h = ctrlrect_all.bottom - ctrlrect_all.top;
    if (!osdMemBM || osdMemBMW != w || osdMemBMH != h) {
      if (osdMemBM) {
	      SelectObject(osdMemDC,osdOldBM);	
        DeleteObject(osdMemBM);	
      }
      osdMemBM = CreateCompatibleBitmap(hdc,w,h);
      osdOldBM = (HBITMAP)SelectObject(osdMemDC, osdMemBM);
      osdMemBMW = w;
      osdMemBMH = h;
    }

    RECT temp;
    SetRect(&temp, 0, 0, w, h);
    FillRect(osdMemDC, &temp, (HBRUSH)osdBlackBrush);

    for (int i=0; i<NUM_WIDGETS; i++) {
      if (g_ctrl_type[i] == CTRLTYPE_PROGRESS)
      {
        int progress = 0;
        int max_progress = ctrlrect[i].right - ctrlrect[i].left;
        switch(i)
        {
        case CTRL_VOL: 
          if (decoder)
            progress = decoder->getvolume()*max_progress/255; 
          break;
        case CTRL_PROGRESS: 
          if (decoder)
          {
            int len = decoder->getlen();
            if (len>0)
              progress = decoder->getpos()*max_progress/len;
          }
          break;
        }

        SelectObject(osdMemDC, osdProgressBrushBg);
        SelectObject(osdMemDC, (i==osdLastClickItem) ? osdProgressPenBgHilite : osdProgressPenBg);
        RoundRect(osdMemDC, ctrlrect[i].left, ctrlrect[i].top, ctrlrect[i].right, ctrlrect[i].bottom, 3, 3);
        SelectObject(osdMemDC, osdProgressBrushFg);
        SelectObject(osdMemDC, osdProgressPenFg);
        Rectangle(osdMemDC, ctrlrect[i].left+1, ctrlrect[i].top+1, ctrlrect[i].left + progress, ctrlrect[i].bottom);
      }
      else if (g_ctrl_type[i] == CTRLTYPE_SYMBOL ||
               g_ctrl_type[i] == CTRLTYPE_TEXT)
      {
        SelectObject(osdMemDC, (g_ctrl_type[i] == CTRLTYPE_SYMBOL) ? osdFontSymbol : osdFontText);
        SetTextColor(osdMemDC, (i==osdLastClickItem) ? RGB(OSD_TEXT_R_HILITE,OSD_TEXT_G_HILITE,OSD_TEXT_B_HILITE) : RGB(OSD_TEXT_R,OSD_TEXT_G,OSD_TEXT_B));
        DrawText(osdMemDC, g_ctrl_text[i], -1, &ctrlrect[i], DT_SINGLELINE);
      }
    }

    int x0 = fullr.left;
    int y0 = fullr.bottom - (ctrlrect_all.bottom - ctrlrect_all.top);
    BitBlt(hdc,x0,y0,w,h,osdMemDC,0,0,SRCCOPY);

    // display stream title @ the top:
#if (SHOW_STREAM_TITLE_AT_TOP)
    if (decoder)
    {
      RECT temp;
      SetRect(&temp, 0, 0, w, h);
      FillRect(osdMemDC, &temp, (HBRUSH)osdBlackBrush);

      SelectObject(osdMemDC, osdFontText);
      SetTextColor(osdMemDC, RGB(OSD_TEXT_R,OSD_TEXT_G,OSD_TEXT_B));
      char *t=decoder->getTitle();
      char *buf=(char*)malloc(32+(t?strlen(t):0));

      wsprintf(buf, "%s (%d kbps)", t?t:"", decoder->getBitrate()/1000);
      char *p=buf;
      while (*p)
      {
        if (*p == '_') *p=' ';
        p++;
      }
      DrawText(osdMemDC, buf, -1, &temp, DT_SINGLELINE|DT_CENTER);
      free(buf);

      SelectObject(osdMemDC, osdFontSymbol);
      DrawText(osdMemDC, "2r", -1, &temp, DT_SINGLELINE|DT_RIGHT);

      int x0 = fullr.left;
      int y0 = fullr.top;
      BitBlt(hdc,x0,y0,w,h,osdMemDC,0,0,SRCCOPY);
    }

    SelectObject(osdMemDC, oldpen);
    SelectObject(osdMemDC, oldbrush);
    SelectObject(osdMemDC, oldfont);
    SetTextColor(osdMemDC, fg);
    SetBkColor(osdMemDC, bg);
  }
#endif

}

void VideoOutput::osdHitTest(int x, int y, int dragging)
{ 
  // dragging == -1: just a mousemove (no clicking)
  // dragging ==  0: user clicked
  // dragging ==  1: user clicked before, and is now dragging/moving mouse

  if (dragging<1)
    osdLastClickItem = -1;

  // transform (x,y) from screen coords into coords relative to the memDC
  y = y - ((lastfsrect.bottom - lastfsrect.top) - (ctrlrect_all.bottom - ctrlrect_all.top));

  int i0 = 0;
  int i1 = NUM_WIDGETS;
  if (dragging==1) {
    i0 = osdLastClickItem;
    i1 = osdLastClickItem+1;
  }

  for (int i=i0; i<i1; i++)
  {
    if (dragging==1 || (x >= ctrlrect[i].left && x <= ctrlrect[i].right && y >= ctrlrect[i].top  && y <= ctrlrect[i].bottom))
    {
      float t = (x - ctrlrect[i].left) / (float)(ctrlrect[i].right - ctrlrect[i].left);
      if (t<0) t=0;
      if (t>1) t=1;
      if (dragging<1)
        osdLastClickItem = i;
      
      switch(i)
      {
      case CTRL_VOL:
        if (decoder && dragging>=0) decoder->setvolume((int)(t*255));
        return;
      case CTRL_PROGRESS:
        if (decoder && dragging>=0) 
        {
          int len = decoder->getlen();
          if (len > 0)
            decoder->seek((int)(t*len));
        }
        return;
      case CTRL_PAUSE:
        if (decoder && dragging>=0) decoder->pause(1);
        return;
      case CTRL_PLAY:
        if (decoder && dragging>=0) decoder->pause(0);
        return;
      case CTRL_STOP:
        if (decoder && dragging>=0) {
          decoder->pause(1);
          remove_fullscreen();
        }
        return;
      case CTRL_REW:
      case CTRL_FFWD:
        if (decoder && dragging>=0) 
        {
          int pos = decoder->getpos();
          int len = decoder->getlen();
          if (len > 0)
          {
            if (i==CTRL_REW) 
              pos = max(0, pos-15000);  // milliseconds to rewind
            else
              pos = min(len, pos+30000);  // milliseconds to skip ahead
            decoder->seek(pos);
          }
        }
        return;
      default:
        if (dragging<1)
          osdLastClickItem = -1;
        break;
      }
    }
  }

  if (dragging==0)
    remove_fullscreen();
}
