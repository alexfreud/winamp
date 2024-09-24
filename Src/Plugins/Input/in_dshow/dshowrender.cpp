//
// This file contains code that supports an alternate method playing dshow data...
// This will have dshow take control of rendering the data (audio or video).
// Used for mms streams
//
#ifdef WINAMPX

#include <windows.h>
#include <math.h>

#include "message.h"

#include "../jnetlib/jnetlib.h"


#include "in2.h"
#include <AtlBase.h>
#include <streams.h>
#include <qedit.h>
#include <qnetwork.h>
#ifdef DEFGUID
#include <initguid.h>    // declares DEFINE_GUID to declare an EXTERN_C const.
#endif

#define IPC_GET_IVIDEOOUTPUT 500

// Externs

extern HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
extern void RemoveFromRot(DWORD pdwRegister);
extern bool ReportMissingCodec(char *fn);

extern In_Module mod;     // the output module (filled in near the bottom of this file)

extern char lastfn[MAX_PATH]; // currently playing file (used for getting info on the current file)
extern char lastfn_status[256];
extern int file_length;   // file length, in bytes
extern int decode_pos_ms;   // current decoding position, in milliseconds. 
extern int paused;        // are we paused?
extern volatile int seek_needed; // if != -1, it is the point that the decode thread should seek to, in ms.
extern int m_length;
extern int paused;				// are we paused?
extern bool s_using_dsr;

// Static Vars and Defines

class IVideoOutput
{
public:
    virtual ~IVideoOutput() { }
    virtual int open(int w, int h, int vflip, double aspectratio, unsigned int fmt)=0;
    virtual void setcallback(LRESULT (*msgcallback)(void *token, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam), void *token) { }
    virtual void close()=0;
    virtual void draw(void *frame)=0;
    virtual void drawSubtitle(/*SubsItem **/ void *item) { }
    virtual void showStatusMsg(const char *text) { }
    virtual int get_latency() { return 0; }
    virtual void notifyBufferState(int bufferstate) { } /* 0-255*/
  
    virtual int extended(int param1, int param2, int param3) { return 0; } // Dispatchable, eat this!
};
static IVideoOutput * m_video_output;

static CComPtr<IGraphBuilder> s_pGraphBuilder;
static CComPtr<IMediaEventEx> s_pMediaEventEx;
static CComPtr<IMediaControl> s_pMediaControl;
static CComPtr<IVideoWindow> s_pVideoWindow;
static CComPtr<IBasicAudio> s_pBasicAudio;
static CComPtr<IBasicVideo> s_pBasicVideo;
static CComPtr<IMediaSeeking> s_pMediaSeeking;

static HWND s_dsr_notif_hwnd = NULL;
static HWND s_hVideoWnd = NULL;
static WNDPROC s_OriginalVideoWndProc = NULL;
static RECT s_parentRect;
static DWORD GraphEdit_dwRegister = 0;
static int s_buffering = 0;
static int s_bufferstat = 0;
static BOOL s_bAudioOnly;
static int s_setVolumeOnStart = -1;


// Forward Declarations
LRESULT CALLBACK dsr_TimerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dsr_SubclassParentWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void dsr_setvolume(int volume);
void dsr_handleNotifyEvents();
void dsr_stop();

// Macros
#define BeginEnumFilters(pFilterGraph, pEnumFilters, pBaseFilter) \
{CComPtr<IEnumFilters> pEnumFilters; \
  if(pFilterGraph && SUCCEEDED(pFilterGraph->EnumFilters(&pEnumFilters))) \
{ \
  for(CComPtr<IBaseFilter> pBaseFilter; S_OK == pEnumFilters->Next(1, &pBaseFilter, 0); pBaseFilter = NULL) \
{ \
  
#define EndEnumFilters }}}

static void SendStatus(int status, int arg ) {
	mod.fire_winampstatus( status, arg );
}





//------------------------------------------------------------------------------
// Name: CheckVisibility()
// Desc: Set global values for presence of video window.
//------------------------------------------------------------------------------
bool CheckVisibility(void)  // returns false for failure
{
  HRESULT hr;
  if ((!s_pVideoWindow) || (!s_pBasicVideo))
  {
    s_bAudioOnly = TRUE;  // Audio-only files have no video interfaces.
    return TRUE;
  }
  s_bAudioOnly = FALSE;

  long lVisible;
  hr = s_pVideoWindow->get_Visible(&lVisible);  // If this is an audio-only clip, get_Visible() won't work.
  if ((FAILED(hr)) && (hr == E_NOINTERFACE))
  {
    s_bAudioOnly = TRUE;
    return TRUE;
  }
  return !FAILED(hr);
}


static RECT fitMediaToWindow(RECT rectWindow, int mediaWidth, int mediaHeight)
{
  RECT retval;

  int windowWidth = rectWindow.right - rectWindow.left;
  int windowHeight = rectWindow.bottom - rectWindow.top;

  if (mediaHeight*windowWidth > mediaWidth*windowHeight)
  {
    // Gap is on left&right sides
    int nOutWidth = windowHeight* mediaWidth / mediaHeight;
    int nGap = (windowWidth - nOutWidth)/2;
    retval.top = rectWindow.top;
    retval.bottom = retval.top + windowHeight;
    retval.left = rectWindow.left + nGap;
    retval.right = retval.left + nOutWidth;
  }
  else
  {
    // Gap is on the top/bottom sides
    int nOutHeight = windowWidth* mediaHeight / mediaWidth;
    int nGap = (windowHeight - nOutHeight)/2;
    retval.left = rectWindow.left;
    retval.right = retval.left + windowWidth;
    retval.top = rectWindow.top + nGap;
    retval.bottom = retval.top + nOutHeight;
  }

  return retval;
}




void dsr_releaseObjects()
{
  if(s_dsr_notif_hwnd) 
  {
    KillTimer(s_dsr_notif_hwnd,112);
    DestroyWindow(s_dsr_notif_hwnd);
  }
  s_dsr_notif_hwnd=NULL;

  if ((s_OriginalVideoWndProc) && (s_hVideoWnd))
  {
     SetWindowLong(s_hVideoWnd, GWL_WNDPROC, (LONG_PTR)s_OriginalVideoWndProc);
     s_OriginalVideoWndProc = NULL;
     s_hVideoWnd = NULL;
  }

  s_pMediaEventEx = NULL;
  s_pGraphBuilder = NULL;
  s_pMediaControl = NULL;
  s_pVideoWindow = NULL;
  s_pBasicAudio = NULL;
  s_pBasicVideo = NULL;
  s_pMediaSeeking = NULL;

  s_using_dsr = false;
}


// Prefix used for functions here are dsd_ (

int dsr_play(char *fn)
{
  HRESULT hr;
  
  hr = s_pGraphBuilder.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER);
  if (FAILED(hr)) return 1;

#ifdef _DEBUG
  AddToRot(s_pGraphBuilder, &GraphEdit_dwRegister);
#endif

  s_pGraphBuilder->QueryInterface(IID_IMediaEvent, (void **)&s_pMediaEventEx);
  if(!s_pMediaEventEx)
  {
    dsr_releaseObjects();
    return 1;
  }
  
  m_video_output=(IVideoOutput *)SendMessage(mod.hMainWindow,WM_USER,0,IPC_GET_IVIDEOOUTPUT);
  if(!m_video_output) 
  {
    dsr_releaseObjects();
    return -200;    // Can't play file
  }

  // Create window that will receive filter notifications
  static int classReg=0;
  if(!classReg)
  {
    WNDCLASS wc={0,}; 
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = dsr_TimerWndProc;
    wc.hInstance = mod.hDllInstance;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.lpszClassName = "in_dshowClass2";
    if (!RegisterClass(&wc))
    {
      dsr_releaseObjects();
      return 1;
    }
    classReg=1;
  }
  s_dsr_notif_hwnd=CreateWindow("in_dshowClass2","dshow_notif2",NULL,0,0,1,1,NULL,NULL,mod.hDllInstance,NULL);
  SetTimer(s_dsr_notif_hwnd,112,500,0);

  // Build a normal graph (start rendering)
  WCHAR f[4096];
  MultiByteToWideChar(CP_ACP,0,fn,lstrlen(fn)+1,f,4096);
  hr = s_pGraphBuilder->RenderFile(f,NULL);
  if (FAILED(hr)) {
    dsr_handleNotifyEvents();
    dsr_releaseObjects();
    if ((hr == CLASS_E_CLASSNOTAVAILABLE) || (hr == VFW_E_UNSUPPORTED_VIDEO) || (hr == VFW_E_NO_DECOMPRESSOR))
    {
      if (ReportMissingCodec(fn)) // returns true if we sent a message
        return -500;  // Unsupported format
      return -200; // Can't play file
    }
    return 1;
  }

  // Check if it's a partial playing of the file (likely video codec missing)
  if ((hr == VFW_S_PARTIAL_RENDER) || (hr == VFW_S_VIDEO_NOT_RENDERED))
  {
    if (!ReportMissingCodec(fn)) // Report the missing codec if we can determine it
      mod.fire_winampstatus(WINAMPX_STATUS_MISSING_AVI_CODEC, 0); // If we can't report a null codec missing
  }
  
  if (FAILED(s_pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&s_pMediaControl)) ||
      FAILED(s_pGraphBuilder->QueryInterface(IID_IMediaSeeking, (void **)&s_pMediaSeeking)) ||
      FAILED(s_pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME)))
  {
    dsr_releaseObjects();
    return 1;
  }
 

  // Get length of file
  int mylength = -1;
	LONGLONG length;
	if (SUCCEEDED(s_pMediaSeeking->GetDuration(&length)))
  {
	  mylength=(int)(length/10000);
  }
	m_length = mylength;

  // Query for video interfaces, which may not be relevant for audio files
  s_pGraphBuilder->QueryInterface(IID_IVideoWindow, (void **)&s_pVideoWindow);
  s_pGraphBuilder->QueryInterface(IID_IBasicVideo,  (void **)&s_pBasicVideo);

  // Query for audio interfaces, which may not be relevant for video-only files
  s_pGraphBuilder->QueryInterface(IID_IBasicAudio, (void **)&s_pBasicAudio);
  if (s_setVolumeOnStart != -1)
  {
    dsr_setvolume(s_setVolumeOnStart);
    s_setVolumeOnStart = -1;
  }

  // Is this an audio-only file (no video component)?
  CheckVisibility();
  if (!s_bAudioOnly)
  {
    m_video_output->open(1, 1, 0, 1.0f, VIDEO_MAKETYPE('N','O','N','E')); // Dummy Size of 1x1
    s_hVideoWnd = (HWND)m_video_output->extended(VIDUSER_GET_VIDEOHWND, 0, 0);
    InvalidateRect(s_hVideoWnd, NULL, TRUE);

    // Setup the video window
    s_pVideoWindow->put_Owner((OAHWND) s_hVideoWnd);
    s_pVideoWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);

    RECT grc;
    GetClientRect(s_hVideoWnd, &grc);
    s_parentRect = grc;

    if ((s_pBasicVideo) && (s_pVideoWindow))
    {
      long videoWidth, videoHeight;
      if (SUCCEEDED(s_pBasicVideo->GetVideoSize(&videoWidth, &videoHeight)))
      {
        RECT r = fitMediaToWindow(grc, videoWidth, videoHeight);
        s_pVideoWindow->SetWindowPosition(r.left, r.top, r.right-r.left, r.bottom-r.top);
      }
    }

    // Intercept resize messages
    s_OriginalVideoWndProc = (WNDPROC) SetWindowLong(s_hVideoWnd, GWL_WNDPROC, (LONG_PTR)dsr_SubclassParentWndProc);
  }

  // Run the graph to play the media file
  hr = s_pMediaControl->Run();
  if (FAILED(hr)) 
	{
		dsr_stop();
		return 1; 
	}

  return 0;
}


// stop playing.
void dsr_stop()
{ 
	if (s_pMediaControl)
    s_pMediaControl->Stop();
  if (s_pVideoWindow)
  {
    s_pVideoWindow->put_Visible(OAFALSE);
    s_pVideoWindow->put_Owner(NULL);
  }

	if (m_video_output)
    m_video_output->close();
	mod.outMod->Close();
	mod.SAVSADeInit();
	dsr_releaseObjects();
	m_length=-1;
	lastfn[0]=0;
}



// Standard Pause Routines
void dsr_pause() { 
	paused=1; 
  if (s_pMediaControl)
  {
    s_pMediaControl->Pause();
  }
}
void dsr_unpause() { 
	paused=0; 
  if (s_pMediaControl)
  {
    s_pMediaControl->Run();
  }
}



int dsr_getoutputtime() { 
	if (s_bufferstat)
    return s_bufferstat;

  if (s_pMediaSeeking)
  {
    LONGLONG pos;
    if (SUCCEEDED(s_pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME)) &&
	      SUCCEEDED(s_pMediaSeeking->GetCurrentPosition(&pos)))
    {
	    return (int)(pos/10000);
    }
  }
  return 0;
}

void dsr_setoutputtime(int time_in_ms) { 
  if(s_pMediaSeeking) {
    DWORD dwCaps = AM_SEEKING_CanSeekAbsolute;
    if (s_pMediaSeeking->CheckCapabilities(&dwCaps) == S_OK)
    {
      int oldpause=paused;
      if(oldpause) dsr_unpause();
      LONGLONG l=((LONGLONG)time_in_ms)*10000;
      s_pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
      s_pMediaSeeking->SetPositions(&l,AM_SEEKING_AbsolutePositioning,NULL,AM_SEEKING_NoPositioning);
      mod.outMod->Flush(time_in_ms);
      if(oldpause) dsr_pause();
    }
  }
}

void dsr_setvolume(int volume)
{
  if (s_pBasicAudio)
  {
    volume = min(volume, 255);
    volume = max(volume, 0);
    s_pBasicAudio->put_Volume(log(volume+1)/log(256)*10000-10000); // Map (0,255) to (-10000,0) log scale (as ActiveX is in DB's)
  }
  else
  {
    s_setVolumeOnStart = volume;
  }
}



LRESULT CALLBACK dsr_TimerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
  if(uMsg==WM_TIMER && wParam==112 && s_dsr_notif_hwnd)
  {
    dsr_handleNotifyEvents();
    if(s_buffering)
    {
      BeginEnumFilters(s_pGraphBuilder, pEF, pBF)
      {
        if(CComQIPtr<IAMNetworkStatus, &IID_IAMNetworkStatus> pAMNS = pBF)
        {
          long BufferingProgress = 0;
          if(SUCCEEDED(pAMNS->get_BufferingProgress(&BufferingProgress)) && BufferingProgress > 0)
          {
            wsprintf(lastfn_status,"Buffering (%ld%%)",BufferingProgress);
            if(m_video_output) m_video_output->extended(VIDUSER_SET_INFOSTRING,(int)&lastfn_status,0);

            int bpos=BufferingProgress;
            int csa = mod.SAGetMode();
            char tempdata[75*2]={0,};
            int x;
            if (csa&1)
            {
              for (x = 0; x < bpos*75/100; x ++)
              {
                tempdata[x]=x*16/75;
              }
            }
            if (csa&2)
            {
              int offs=(csa&1) ? 75 : 0;
              x=0;
              while (x < bpos*75/100)
              {
                tempdata[offs + x++]=-6+x*14/75;
              }
              while (x < 75)
              {
                tempdata[offs + x++]=0;
              }
            }
            if (csa==4)
            {
              tempdata[0]=tempdata[1]=(bpos*127/100);
            }
            if (csa) mod.SAAdd(tempdata,++s_bufferstat,(csa==3)?0x80000003:csa);
            
            PostMessage(mod.hMainWindow,WM_USER,0,243);
            SendStatus(WINAMPX_STATUS_PREBUFFERING_PCT, 255*BufferingProgress/100);

            break;
          }
        }
      }
      EndEnumFilters
    }
  }

  return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}


void dsr_handleNotifyEvents()
{
//  char s[256];
	for (;;) { 
		long	evCode, param1, param2;
		HRESULT h = s_pMediaEventEx->GetEvent(&evCode, &param1, &param2, 0);
		if (FAILED(h)) break;
		switch(evCode) {
		case EC_BUFFERING_DATA:
			{
//        sprintf(s, "Handling Event: EC_BUFFERING_DATA: %d\n", param1);
//        OutputDebugString(s);
				s_buffering=param1;
				if(!s_buffering) 
				{
					lastfn_status[0]=0;
					s_bufferstat=0;
					PostMessage(mod.hMainWindow,WM_USER,0,243);

					break;
				}
			}
			break;

    case EC_COMPLETE:
      {
//        OutputDebugString("Handling Event: EC_COMPLETE\n");
        PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
      }

    default:
      {
//        sprintf(s, "Handling Event: 0x%x : param1=0x%x; param2 = 0x%x\n", evCode, param1, param2);
//        OutputDebugString(s);
      }
		}

		s_pMediaEventEx->FreeEventParams(evCode, param1, param2);
	}
}


// Subclass of the Window containing the code.  (Used to check if the parent got resized)
LRESULT CALLBACK dsr_SubclassParentWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  LRESULT retval = CallWindowProc(s_OriginalVideoWndProc, hwnd, uMsg, wParam, lParam);

  if ((uMsg == WM_SIZE) && (s_hVideoWnd))
  {
    RECT grc;
    GetClientRect(s_hVideoWnd, &grc);
    
    if (!EqualRect(&grc, &s_parentRect))
    {
      s_parentRect = grc;

      if ((s_pBasicVideo) && (s_pVideoWindow))
      {
        long videoWidth, videoHeight;
        if (SUCCEEDED(s_pBasicVideo->GetVideoSize(&videoWidth, &videoHeight)))
        {
          RECT r = fitMediaToWindow(grc, videoWidth, videoHeight);
          s_pVideoWindow->SetWindowPosition(r.left, r.top, r.right-r.left, r.bottom-r.top);
        }
      }
    }
  }

  if ((uMsg == WM_PARENTNOTIFY) && (LOWORD(wParam)==WM_RBUTTONDOWN))
  { 
    SendStatus(WINAMPX_STATUS_VIDEO_RIGHT_CLICK, lParam);
  }

  if ((uMsg == WM_PARENTNOTIFY) && (LOWORD(wParam)==WM_LBUTTONDOWN))
  {
    static DWORD dwLastTime = 0;
    int dwTimeNow = GetTickCount();
    if (((dwTimeNow - dwLastTime) < GetDoubleClickTime()) && (dwLastTime != 0))
    {
      PostMessage(hwnd, WM_USER_DSR_LBUTTONDBLCLK, 0, 0);  // Notify the video window that we double clicked
      dwLastTime = 0;
   }
    else
      dwLastTime = dwTimeNow;
  }

  if ((uMsg == WM_USER_DSR_FULLSCREEN) && (s_pVideoWindow))
  {
    s_pVideoWindow->HideCursor(wParam ? OATRUE : OAFALSE);
  }

  return retval;
}

#endif // WINAMPX
